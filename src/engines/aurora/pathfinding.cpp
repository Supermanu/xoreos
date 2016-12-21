/* xoreos - A reimplementation of BioWare's Aurora engine
 *
 * xoreos is the legal property of its developers, whose names
 * can be found in the AUTHORS file distributed with this source
 * distribution.
 *
 * xoreos is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 3
 * of the License, or (at your option) any later version.
 *
 * xoreos is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with xoreos. If not, see <http://www.gnu.org/licenses/>.
 */

/** @file
 *  Base class for pathfinding
 */

#include <algorithm>

#include "src/common/util.h"
#include "src/common/vector3.h"
#include "src/common/vec3util.h"
#include "src/common/aabbnode.h"

#include "src/graphics/graphics.h"

#include "src/engines/aurora/pathfinding.h"


namespace Engines {

bool sortByLenght(Common::Vector3 vec1, Common::Vector3 vec2) {
	return vec1.length() < vec2.length();
}

Pathfinding::Pathfinding() : _verticesCount(0), _facesCount(0) {
	_creatureWidth = 0.f;
	_creaturePos = Common::Vector3(0.f, 0.f, 0.f);
}

Pathfinding::~Pathfinding() {
}

Pathfinding::Node::Node() {
}

Pathfinding::Node::Node(uint32 faceID, float pX, float pY, float pZ, uint32 par)
                        : face(faceID), x(pX), y(pY), z(pZ), parent(par) {
}

bool Pathfinding::Node::operator<(const Node &node) const {
	return G + H < node.G + node.H;
}

bool Pathfinding::findPath(float startX, float startY, float startZ,
						   float endX, float endY, float endZ, std::vector<uint32> &facePath, float width, uint32 nbrIt) {
	// A* algorithm.

	_facesToDraw.clear();
	_creaturePos = Common::Vector3(startX, startY, startZ);
	_creatureWidth = width;
// 	warning("finding path... with iter %u", nbrIt);
	facePath.clear();

	// Find faces start and end points belong.
	uint32 startFace = findFace(startX, startY, startZ);
	uint32 endFace = findFace(endX, endY, endZ);

	// Check if start and end points are in the walkmesh.
	if (startFace == UINT32_MAX || endFace == UINT32_MAX)
		return false;

	// Check if start and end points are in the same face.
	if (startFace == endFace) {
// 		warning("start face == end face");
		facePath.push_back(startFace);
		return true;
	}

	// Init nodes and lists.
	Node startNode = Node(startFace, startX, startY, startZ);
	Node endNode   = Node(endFace, endX, endY, endZ);

	startNode.G = 0.f;
	startNode.H = getHeuristic(startNode, endNode);

	std::vector<Node> openList;
	std::vector<Node> closedList;
	openList.push_back(startNode);

// 	warning("starting the loop");
	// Searching...

// 	while (!openList.empty()) {
	for (uint32 it = 0; it < nbrIt; ++it) {
// 		warning("openlist size %zu", openList.size());
		if (openList.empty())
			break;

		Node current = openList.front();
// 		warning("current face %u", current.face);
		_facesToDraw.push_back(current.face);

		if (current.face == endNode.face) {
			reconstructPath(current, closedList, facePath);
			_pointsToDraw.push_back(Common::Vector3(endX, endY, endZ));
			_facesToDraw = facePath;
			return true;
		}

		openList.erase(openList.begin());
		closedList.push_back(current);

		std::vector<uint32> adjNodes;
		getAdjacentNodes(current, adjNodes);
// 		warning("adjNodes size %zu", adjNodes.size());
		for (std::vector<uint32>::iterator a = adjNodes.begin(); a != adjNodes.end(); ++a) {
// 			warning("adj node %u", *a);
			// Check if it has been already evaluated.
			if (hasNode(*a, closedList))
				continue;

			if (!goThrough(current.face, *a, width))
				continue;

			// Distance from start point to this node.
			float x, y, z;
			float gScore = current.G + getDistance(current, *a, x, y, z);

			// Check if it is a new node.
			Node *adjNode = getNode(*a, openList);
			bool isThere = adjNode > 0;
			if (!isThere) {
				adjNode = new Node(*a, x, y, z);
				adjNode->parent = current.face;
			} else if (gScore >= adjNode->G) {
				continue;
			}

			// adjNode is the best node up to now, update/add.
			adjNode->parent = current.face;
			adjNode->G = gScore;
			adjNode->H = getHeuristic(*adjNode, endNode);
			if (!isThere)
				openList.push_back(*adjNode);

			std::sort(openList.begin(), openList.end());
		}
	}

	return false;
}

void Pathfinding::SSFA(Common::Vector3 start, Common::Vector3 end, std::vector<uint32> &facePath, std::vector<Common::Vector3> &path, float width) {
// 	warning("start point (%f, %f, %f)", start._x, start._y, start._y);
// 	warning("end point (%f, %f, %f)", end._x, end._y, end._y);
	_pointsToDraw.clear();
	// Based on the Simple Stupid Funnel Algorithm from Mikko Mononen (http://digestingduck.blogspot.be)
	Common::Vector3 apex, left, right, lastLeft, lastRight;
	Common::Vector3 newLeft, newRight;
	Common::Vector3 otherVert = start;
	bool apexIsLeft = false;
	left = start; lastLeft = start;
	right = start; lastRight = start;
	apex = start;
	uint32 leftIndex = 0;
	uint32 rightIndex = 0;

	path.clear();
	path.push_back(start);
	_pointsToDraw.push_back(start);

	for (uint32 f = 0; f < facePath.size(); ++f) {
// 		warning("Entering side %u", f);
		// Find next vertex
		Common::Vector3 vertA, vertB, vertOne, vertTwo, vertThree;
		if (f == facePath.size() - 1) {
			// Consider the end point as the last side we need to go.
			vertA = end;
			vertB = end;
		} else {
			getSharedVertices(facePath[f], facePath[f + 1], vertA, vertB);
			getVertices(facePath[f], vertOne, vertTwo, vertThree);

			if (vertOne != vertA && vertOne != vertB)
				otherVert = vertOne;
			else if (vertTwo != vertA && vertTwo != vertB)
				otherVert = vertTwo;
			else
				otherVert = vertThree;
		}

		// Move along the funnel.
// 		warning("vertA(%f, %f, %f), vertB(%f, %f, %f), left(%f, %f, %f), right(%f, %f, %f)",
// 			vertA[0], vertA[1], vertA[2], vertB[0], vertB[1], vertB[2], left[0], left[1], left[2], right[0], right[1], right[2]
// 		);

		if (right == left) {
			// If we start or restart the pulling.
			if (triangleArea2(apex, vertA, vertB) < 0.f) {
				newLeft = vertA;
				newRight = vertB;
			} else {
				newLeft = vertB;
				newRight = vertA;
			}

			lastLeft = apex;
			lastRight = apex;
		} else {
			lastLeft = newLeft;
			lastRight = newRight;

			if (triangleArea2(otherVert, vertA, vertB) < 0.f) {
				newLeft = vertA;
				newRight = vertB;
			} else {
				newLeft = vertB;
				newRight = vertA;
			}
		}

		Common::Vector3 adjLeft = newLeft;
		Common::Vector3 adjRight = newRight;
		Common::Vector3 leftApex = apex;
		Common::Vector3 rightApex = apex;
		// Take creature's width
// 		Common::Vector3 creatureSize(0.f, 0.f, 0.f);
		if (/*f != facePath.size() - 1 && */(newLeft - newRight).length() >= width) {
			float halfWidth = width / 2.f;
			// Check if the newright/left needs to be adjusted.
// 			if (!walkableCircle(newLeft, halfWidth)) {
				Common::Vector3 segment = newLeft - apex;
				if (apex == start) {
					if (f != facePath.size() - 1)
						adjLeft = newLeft + getOrthonormalVec(segment, true) * (halfWidth);
					// Otherwise f is the end point and we don't want to adjust.
				} else {
					if (apexIsLeft) {
						// Apex and left side are on the same side.
						Common::Vector3 adj = getOrthonormalVec(segment, true) * (halfWidth);
						adjLeft += adj;
						leftApex += adj;
					} else {
						// Apex and left side are on the opposite side.
						// Compute from where the tangent goes.
						float halfLength = segment.length() / 2;
						float sin = sqrt(halfLength * halfLength - halfWidth * halfWidth) / halfLength;
						float cos = halfWidth / halfLength;
						Common::Vector3 startTan = segment.norm() * halfWidth;
						// Rotate anti-clockwise to the start of the tangent.
						float x = startTan._x * cos - startTan._y * sin;
						float y = startTan._x * sin + startTan._y * cos;
						startTan._x = x;
						startTan._y = y;
						leftApex += startTan;
						adjLeft -= startTan;
					}
				}
// 			}

// 			if (!walkableCircle(newRight, halfWidth)) {
				/*Common::Vector3 */segment = newRight - apex;
				if (apex == start) {
					if (f != facePath.size() - 1)
						adjRight = newRight + getOrthonormalVec(segment, false) * (halfWidth);
					// Apex and left side are on the same side.
				} else {
					if (!apexIsLeft) {
						// Apex and right side are on the same side.
						Common::Vector3 adj = getOrthonormalVec(segment, false) * (halfWidth);
						adjRight += adj;
						rightApex += adj;
					} else {
						// Apex and right side are on the opposite side.
						// Compute from where the tangent goes.
						float halfLength = segment.length() / 2;
						float sin = sqrt(halfLength * halfLength - halfWidth * halfWidth) / halfLength;
						float cos = halfWidth / halfLength;
						Common::Vector3 startTan = segment.norm() * halfWidth;
						// Rotate clockwise to the start of the tangent.
						float x = startTan._x * cos + startTan._y * sin;
						float y = startTan._y * cos - startTan._x * sin;
						startTan._x = x;
						startTan._y = y;
						rightApex += startTan;
						adjRight -= startTan;
					}
				}
// 			}
		}

// 		_pointsToDraw.push_back(adjLeft);
// 		_pointsToDraw.push_back(adjRight);

		// Update right if needed
		// First, check if the new point tighten the funnel
		if (triangleArea2(apex, right, adjRight) >= 0.f) {
// 			warning("We're on the good side (right)");
			// Second, check if we are still in the funnel i.e. we don't cross the other side.
			if (triangleArea2(apex, left, adjRight) < 0.f || apex == right) {
				right = newRight;
// 				warning("We're still in the funnel (rightindex %u)", f);
// 				_pointsToDraw.push_back(right);
				rightIndex = f;
			} else {
// 				warning("We're outside the funnel (right)");
				// Change apex to current left
				apex = left;
				right = apex;
				apexIsLeft = true;
				// Add it to the path
				if (path.back() != left) {
					path.push_back(left);
				}
// 					_pointsToDraw.push_back(apex);
// 					_pointsToDraw.push_back(adjLeft);
				_pointsToDraw.push_back(left);


				// Restart from that point
				rightIndex = leftIndex;
				f = leftIndex;
				continue;
			}
		}

		// Update left if needed
		// First, check if the new point tighten the funnel
		if (triangleArea2(apex, left, adjLeft) <= 0.f) {
// 			warning("We're on the good side (left)");
			// Second, check if we are still in the funnel i.e. we don't cross the other side.
			if (triangleArea2(apex, right, adjLeft) > 0.f || apex == left) {
// 				warning("We're still in the funnel (leftindex %u)", f);
				left = newLeft;
				leftIndex = f;
// 				_pointsToDraw.push_back(adjLeft);
			} else {
// 				warning("We're outside the funnel left");
				// Change apex to current left
				apex = right;
				left = apex;
				apexIsLeft = false;
				// Add it to the path
				if (path.back() != right) {
					path.push_back(right);
				}
// 					_pointsToDraw.push_back(apex);
// 					_pointsToDraw.push_back(adjRight);
				_pointsToDraw.push_back(right);


				// Restart from that point
				leftIndex = rightIndex;
				f = rightIndex;
				continue;
			}
		}
	}

	if (path.back() != end)
		path.push_back(end);

	std::vector<Common::Vector3> finalPath;
// 	manageCreatureSize(path, width / 2, finalPath);
// 	for (uint32 i = 0; i < finalPath.size(); ++i)
// 		_pointsToDraw.push_back(finalPath[i]);

	_pointsToDraw.push_back(end);
}

void Pathfinding::manageCreatureSize(std::vector<Common::Vector3> &smoothedPath, float halfWidth, std::vector<Common::Vector3> &finalPath) {
	if (smoothedPath.size() < 3) {
		finalPath = smoothedPath;
		return;
	}

	finalPath.clear();
	finalPath.push_back(smoothedPath.front());

	Common::Vector3 middlePoint, segment;
	Common::Vector3 middleSquare, firstSquare, secondSquare, orthoVec;
// 	warning("point 0 (%f, %f, %f)", smoothedPath[0][0], smoothedPath[0][1], smoothedPath[0][2]);

	for (uint32 point = 0; point < smoothedPath.size() - 2; ++point) {
// 		warning("point %u (%f, %f, %f)", point + 1, smoothedPath[point + 1][0], smoothedPath[point + 1][1], smoothedPath[point + 1][2]);
		if (smoothedPath[point] == smoothedPath[point + 1])
			continue;

		middlePoint = smoothedPath[point + 1];
		segment = smoothedPath[point + 2] - smoothedPath[point];

		bool clockwise = !isToTheLeft(smoothedPath[point], smoothedPath[point + 2], middlePoint);
		orthoVec = getOrthonormalVec(segment, clockwise) * halfWidth;
		middleSquare = middlePoint + orthoVec;
		// Check if we are in a small face
		if (!walkable(middleSquare))
			middleSquare = middlePoint - orthoVec;

		secondSquare = middleSquare + (segment.norm() * halfWidth);
		firstSquare = middleSquare + (segment.norm() * (-1) * halfWidth);

// 		finalPath.push_back(middleSquare);
		finalPath.push_back(firstSquare);
		finalPath.push_back(secondSquare);

	}

// 	warning("point %lu (%f, %f, %f)", smoothedPath.size() - 1, smoothedPath[smoothedPath.size() - 1][0], smoothedPath[smoothedPath.size() - 1][1], smoothedPath[smoothedPath.size() - 1][2]);
	finalPath.push_back(smoothedPath.back());
}

bool Pathfinding::isToTheLeft(Common::Vector3 startSegment, Common::Vector3 endSegment, Common::Vector3 point) const {
	return (endSegment - startSegment).cross(point - startSegment)._z > 0;
}

Common::Vector3 Pathfinding::getOrthonormalVec(Common::Vector3 segment, bool clockwise) const {
	Common::Vector3 copy = segment;
	// Sinus of 90 degrees.
	float sinus = (clockwise ? -1 : 1);
	copy._x = - segment._y * sinus;
	copy._y = segment._x * sinus;
	return copy.norm();
}

Common::Vector3 Pathfinding::getCreatureSizePoint(Common::Vector3 &from, Common::Vector3 &left, Common::Vector3 &right, float halfWidth, bool alongLeft) {
	Common::Vector3 impVec = alongLeft ? left : right;
	Common::Vector3 othVec = alongLeft ? right : left;

	Common::Vector3 height = (impVec - from).norm();

	Common::Matrix4x4 rotMat = Common::Matrix4x4();
	rotMat.rotateZAxisLocal((alongLeft ? 1.f : -1.f) * 90.f);
	height = rotMat.vectorRotate(height) * halfWidth;

	return ((othVec - impVec).norm() * height.dot((othVec - impVec).norm()));

// 	float ab = (nearVertex - from).length();
// 	float bd = (farVertex - nearVertex).length();
// 	float ad = (farVertex - from).length();
//
// 	float beta = acos((ab * ab + bd * bd - ad * ad) / (2 * ab * bd));
// 	float beta1 = acos(halfWidth / ab);
// 	float bc = halfWidth / cos(beta - beta1);
//
// 	return nearVertex + (farVertex - nearVertex).norm() * bc;
// 	float abbd = ab.dot(bd);
//
// 	// Check orthogonality.
// 	if (abbd == 0)
// 		return nearVertex +(bd.norm() * halfWidth);
//
// 	float alpha = (sqrt(ab.dot(ab) + halfWidth * halfWidth) - ab.dot(ab)) / ab.dot(bd);
// 	warning("alpha %f", alpha);
// 	return nearVertex + (bd * abs(alpha));
}

void Pathfinding::smoothPath(Common::Vector3 start, Common::Vector3 end, std::vector<uint32> &facePath, std::vector<Common::Vector3> &path) {
	// Simple smooth path algorithm.
	path.push_back(start);

	if (facePath.size() < 3) {
		path.push_back(end);
		return;
	}

// 	warning("Total faces: %lu", facePath.size());
	uint32 endFace = facePath.size() - 1;
	for (uint32 currFace = 0; currFace < facePath.size() - 2; ++currFace) {
		bool isIn = segmentInFace(facePath[currFace + 1], path.back(), end);
		if (isIn && segmentInFace(facePath[currFace + 2], path.back(), end)) {
			std::vector<Common::Vector3> inters;
			getIntersections(path.back(), end, facePath[currFace + 1], inters);
// 			warning("Intersection size: %lu", inters.size());
			for (uint32 in = 0; in < inters.size(); ++in) {
// 				warning("Intersect: (%f, %f)", inters[in]._x, inters[in]._y);
				// Check if the segment doesn't go outside.
				bool isInPrevious = inFace(facePath[currFace], inters[in]);
				bool isInNext = inFace(facePath[currFace + 2], inters[in]);
// 				warning("Check intersection, previous: %u, next: %u", isInPrevious, isInNext);
				if ((!isInPrevious && !isInNext)) {
// 					warning("segment outside(main) of %u", currFace +2);
					std::vector<Common::Vector3> vertices;
					vertices.resize(3);
					getVertices(facePath[currFace + 1], vertices[0], vertices[1], vertices[2]);
					for (uint32 v = 0; v < 3; ++v)
						vertices[v] = vertices[v] - path.back();
					std::sort(vertices.begin(), vertices.end(), sortByLenght);

					path.push_back(vertices[0] + path.back());
// 					warning("PUSHING-closest vertices: (%f, %f)", path.back()._x, path.back()._y);
					break;
				}
			}
		}


		if (!isIn) {
			if (endFace < currFace)
				throw;

// 			warning("Segment not in face %u", currFace + 1);
			// Compute the intersection with the last face within the path.
			Common::Vector3 inter;
			getClosestIntersection(path.back(), end, facePath[currFace], inter);
// 			warning("PUSHING-get closest intersect to the start: (%f, %f)", inter._x, inter._y);
			path.push_back(inter);

// 			warning("looping back");
			for (uint32 f = endFace - 1; f > currFace; --f) {
// 				warning("Endface: %u", f);
				Common::Vector3 vA, vB, vC;
				getVertices(facePath[f], vA, vB, vC);
				std::vector<Common::Vector3> verts;
				verts.push_back(vA - path.back());
				verts.push_back(vB - path.back());
				verts.push_back(vC - path.back());
				std::sort(verts.begin(), verts.end(), sortByLenght);
				if (!segmentInFace(facePath[currFace + 1], path.back(), verts.front() + path.back()))
					continue;

// 				std::vector<Common::Vector3> inters;
				Common::Vector3 goodEndVert = verts.front() + path.back();
// 				warning("Good endface?: %u", f);
				if (segmentInFace(facePath[currFace + 2], path.back(), goodEndVert)) {
					std::vector<Common::Vector3> inters;
					getIntersections(path.back(), goodEndVert, facePath[currFace + 1], inters);
// 					warning("Intersection size: %lu", inters.size());
					for (uint32 in = 0; in < inters.size(); ++in) {
// 						warning("Intersect: (%f, %f)", inters[in]._x, inters[in]._y);
						// Check if the segment doesn't go outside.
						bool isInPrevious = inFace(facePath[currFace], inters[in]);
						bool isInNext = inFace(facePath[currFace + 2], inters[in]);
// 						warning("Check intersection, previous: %u, next: %u", isInPrevious, isInNext);
						if ((!isInPrevious && !isInNext)) {
// 							warning("segment outside of %u", currFace +2);
							std::vector<Common::Vector3> vertices;
							vertices.resize(3);
							getVertices(facePath[currFace + 1], vertices[0], vertices[1], vertices[2]);
							for (uint32 v = 0; v < 3; ++v)
								vertices[v] = vertices[v] - path.back();
							std::sort(vertices.begin(), vertices.end(), sortByLenght);

							path.push_back(vertices.front() + path.back());
// 							warning("PUSHING-closest vertices: (%f, %f)", path.back()._x, path.back()._y);
							break;
						}
					}
				}


				getVertices(facePath[currFace + 1], vA, vB, vC);
				Common::Vector3 intersect;
				getClosestIntersection(goodEndVert, path.back(), facePath[currFace + 1], intersect);
// 				warning("PUSHING-Next intersection: (%f, %f)", intersect._x, intersect._y);
				path.push_back(intersect);
				break;
			}
		} else {
// 			warning("Face %u in the line", currFace + 1);
		}
	}

	path.push_back(end);

	if (inFace(facePath[0], path[1]))
		path.erase(++path.begin());

	_pointsToDraw.clear();
	_pointsToDraw = path;
}

void Pathfinding::getIntersections(Common::Vector3 &start, Common::Vector3 &end, uint32 face, std::vector<Common::Vector3> &intersects) {
	Common::Vector3 vA, vB, vC, inter1, inter2, inter3;
	getVertices(face, vA, vB, vC);

	if (getIntersection(start, end, vA, vB, inter1))
		intersects.push_back(inter1);

	if (getIntersection(start, end, vB, vC, inter2))
		intersects.push_back(inter2 );

	if (getIntersection(start, end, vA, vC, inter3))
		intersects.push_back(inter3);
}

void Pathfinding::getClosestIntersection(Common::Vector3 &start, Common::Vector3 &end, uint32 face, Common::Vector3 &intersect) {
	Common::Vector3 vA, vB, vC, inter1, inter2, inter3;
	getVertices(face, vA, vB, vC);

	std::vector<Common::Vector3> inters;
	if (getIntersection(start, end, vA, vB, inter1))
		inters.push_back(inter1 - start);

	if (getIntersection(start, end, vB, vC, inter2))
		inters.push_back(inter2 - start);

	if (getIntersection(start, end, vA, vC, inter3))
		inters.push_back(inter3 - start);

	if (inters.empty()) {
// 		warning("getClosestIntersection: no intersection");
		return;
	}

	std::sort(inters.begin(), inters.end(), sortByLenght);

	intersect = inters.front() + start;
}

void Pathfinding::getVertices(uint32 faceID, Common::Vector3& vA, Common::Vector3& vB, Common::Vector3& vC) const {
	uint32 vertexIdA = _faces[faceID * 3];
	uint32 vertexIdB = _faces[faceID * 3 + 1];
	uint32 vertexIdC = _faces[faceID * 3 + 2];

	vA = Common::Vector3(_vertices[vertexIdA * 3], _vertices[vertexIdA * 3 + 1], _vertices[vertexIdA * 3 + 2]);
	vB = Common::Vector3(_vertices[vertexIdB * 3], _vertices[vertexIdB * 3 + 1], _vertices[vertexIdB * 3 + 2]);
	vC = Common::Vector3(_vertices[vertexIdC * 3], _vertices[vertexIdC * 3 + 1], _vertices[vertexIdC * 3 + 2]);
}

bool Pathfinding::walkableCircle(Common::Vector3 center, float radius) {
	std::vector<Common::AABBNode *> nodesIn;
	for (std::vector<Common::AABBNode *>::iterator n = _AABBTrees.begin(); n != _AABBTrees.end(); ++n) {
		if (*n)
			(*n)->getNodesInCircle(center, radius, nodesIn);
	}

	Common::Vector3 vertices[3];
	for (std::vector<Common::AABBNode *>::iterator n = nodesIn.begin(); n != nodesIn.end(); ++n) {
		// Check that at least one vertex is inside the circle.
		getVertices((*n)->getProperty(), vertices[0], vertices[1], vertices[2]);
		for (uint8 v = 0; v < 3; ++v)
			if (inCircle(center, radius, vertices[v], vertices[(v + 1) % 3])) {
				// The face is in the circle.
				if (!walkable((*n)->getProperty()))
					return false;

				// The face is inside the circle but is walkable.
				break;
			}
	}

	return true;
}

bool Pathfinding::segmentInFace(uint32 faceID, Common::Vector3 segStart, Common::Vector3 segEnd) {
	Common::Vector3 vA, vB, vC, inter;
	getVertices(faceID, vA, vB, vC);

	if (getIntersection(segStart, segEnd, vA, vB, inter))
		return true;

	if (getIntersection(segStart, segEnd, vA, vC, inter))
		return true;

	if (getIntersection(segStart, segEnd, vC, vB, inter))
		return true;

	return false;
}

bool Pathfinding::hasNode(uint32 face, std::vector<Node> &nodes) const {
	Node dummy(0, 0.f, 0.f, 0.f);
	return hasNode(face, nodes, dummy);
}

bool Pathfinding::hasNode(uint32 face, std::vector<Node> &nodes, Node &node) const {
	for (std::vector<Node>::iterator n = nodes.begin(); n != nodes.end(); ++n) {
		if ((*n).face == face) {
			node = *n;
			return true;
		}
	}

	return false;
}

Pathfinding::Node *Pathfinding::getNode(uint32 face, std::vector<Node> &nodes) const {
	for (std::vector<Node>::iterator n = nodes.begin(); n != nodes.end(); ++n) {
		if ((*n).face == face) {
			return &(*n);
		}
	}

	return 0;
}

void Pathfinding::reconstructPath(Node &endNode, std::vector<Node> &closedList, std::vector<uint32> &path) {
	Node &cNode = endNode;
	path.push_back(endNode.face);
	path.push_back(endNode.parent);

	_pointsToDraw.clear();
	_pointsToDraw.push_back(Common::Vector3(cNode.x, cNode.y, cNode.z));

	while (cNode.parent != UINT32_MAX) {
		for (std::vector<Node>::iterator n = closedList.begin(); n != closedList.end(); ++n) {
			if (cNode.parent != (*n).face)
				continue;

			cNode = (*n);
			_pointsToDraw.push_back(Common::Vector3(cNode.x, cNode.y, cNode.z));
			if (cNode.parent != UINT32_MAX)
				path.push_back(cNode.parent);

			break;
		}
	}

	std::reverse(path.begin(), path.end());

	std::reverse(_pointsToDraw.begin(), _pointsToDraw.end());
	for (uint32 p = 0; p < _pointsToDraw.size(); ++p) {
// 		warning("path %u: (%f, %f, %f)", p, _pointsToDraw[p]._x, _pointsToDraw[p]._y, _pointsToDraw[p]._z);
	}
}

void Pathfinding::drawWalkmesh() {
	if (_faces.empty())
		return;

	glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
	for (uint32 f = 0; f < _facesCount; ++f) {
		glBegin(GL_TRIANGLES);
		glColor3f(0.5, 0.5, 0.5);
		uint32 vI_1 = _faces[f * 3 + 0];
		glVertex3f(_vertices[vI_1 * 3], _vertices[vI_1 * 3 + 1], _vertices[vI_1 * 3 + 2]);
		uint32 vI_2 = _faces[f * 3 + 1];
		glVertex3f(_vertices[vI_2 * 3], _vertices[vI_2 * 3 + 1], _vertices[vI_2 * 3 + 2]);
		uint32 vI_3 = _faces[f * 3 + 2];
		glVertex3f(_vertices[vI_3 * 3], _vertices[vI_3 * 3 + 1], _vertices[vI_3 * 3 + 2]);
		glEnd();
	}
	glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
	for (uint32 f = 0; f < _facesCount; ++f) {
		glBegin(GL_TRIANGLES);
		if (walkable(f))
			glColor4f(0.5, 0, 0.5, 0.4);
		else
			glColor4f(0.1, 0.5, 0.5, 0.4);

		uint32 vI_1 = _faces[f * 3 + 0];
		glVertex3f(_vertices[vI_1 * 3], _vertices[vI_1 * 3 + 1], _vertices[vI_1 * 3 + 2]);
		uint32 vI_2 = _faces[f * 3 + 1];
		glVertex3f(_vertices[vI_2 * 3], _vertices[vI_2 * 3 + 1], _vertices[vI_2 * 3 + 2]);
		uint32 vI_3 = _faces[f * 3 + 2];
		glVertex3f(_vertices[vI_3 * 3], _vertices[vI_3 * 3 + 1], _vertices[vI_3 * 3 + 2]);
		glEnd();
	}

	// Draw path.
	for (uint32 f = 0; f < _facesToDraw.size(); ++f) {
		glBegin(GL_TRIANGLES);
		glColor4f(0.5, 0.2, 0.5, 0.4);

		uint32 vI_1 = _faces[_facesToDraw[f] * 3 + 0];
		glVertex3f(_vertices[vI_1 * 3], _vertices[vI_1 * 3 + 1], _vertices[vI_1 * 3 + 2]);
		uint32 vI_2 = _faces[_facesToDraw[f] * 3 + 1];
		glVertex3f(_vertices[vI_2 * 3], _vertices[vI_2 * 3 + 1], _vertices[vI_2 * 3 + 2]);
		uint32 vI_3 = _faces[_facesToDraw[f] * 3 + 2];
		glVertex3f(_vertices[vI_3 * 3], _vertices[vI_3 * 3 + 1], _vertices[vI_3 * 3 + 2]);
		glEnd();
	}

// 	float start[3] = { 11.5f, 27.f, -1.27 };
// 	float end[3]   = { 28.828587, 20.333252, -1.27 };

// 	glLineWidth(2.f);
// 	glBegin(GL_LINES);
// 	glColor4f(1.f, 1.0, 1.0, 0.5);
// 	glVertex3f(start[0], start[1], start[2]);
// 	glVertex3f(end[0], end[1], end[2]);
// 	glEnd();
// 	glLineWidth(1.f);

	if (_pointsToDraw.size() < 1)
		return;

	for (uint32 p = 0; p < _pointsToDraw.size() - 1; ++p) {
		glLineWidth(3.f);
		glBegin(GL_LINES);
		glColor4f(1.f, 0.0, 1.0, 0.5);
		glVertex3f(_pointsToDraw[p]._x, _pointsToDraw[p]._y, _pointsToDraw[p]._z + 0.05);
		glVertex3f(_pointsToDraw[p + 1]._x, _pointsToDraw[p + 1]._y, _pointsToDraw[p + 1]._z + 0.05);
		glEnd();
		glLineWidth(1.f);

	}

	if (_creatureWidth > 0.f) {
		glBegin(GL_LINES);
		glColor4f(1.f, 0.3, 1.0, 0.8);
		glVertex3f(_creaturePos._x + 0.5f * _creatureWidth, _creaturePos._y + 0.5f * _creatureWidth, _creaturePos._z + 0.05);
		glVertex3f(_creaturePos._x - 0.5f * _creatureWidth, _creaturePos._y + 0.5f * _creatureWidth, _creaturePos._z + 0.05);
		glVertex3f(_creaturePos._x - 0.5f * _creatureWidth, _creaturePos._y + 0.5f * _creatureWidth, _creaturePos._z + 0.05);
		glVertex3f(_creaturePos._x - 0.5f * _creatureWidth, _creaturePos._y - 0.5f * _creatureWidth, _creaturePos._z + 0.05);
		glVertex3f(_creaturePos._x - 0.5f * _creatureWidth, _creaturePos._y - 0.5f * _creatureWidth, _creaturePos._z + 0.05);
		glVertex3f(_creaturePos._x + 0.5f * _creatureWidth, _creaturePos._y - 0.5f * _creatureWidth, _creaturePos._z + 0.05);
		glVertex3f(_creaturePos._x + 0.5f * _creatureWidth, _creaturePos._y - 0.5f * _creatureWidth, _creaturePos._z + 0.05);
		glVertex3f(_creaturePos._x + 0.5f * _creatureWidth, _creaturePos._y + 0.5f * _creatureWidth, _creaturePos._z + 0.05);
		glEnd();
	}
}

bool Pathfinding::walkable(uint32 faceIndex) const {
	if (faceIndex == UINT32_MAX)
		return false;

	return _faceProperty[faceIndex] != 2 && _faceProperty[faceIndex] != 7
	&& _faceProperty[faceIndex] != 0 && _faceProperty[faceIndex] != 8;
}

bool Pathfinding::walkable(Common::Vector3 point) {
	uint32 face = findFace(point[0], point[1], point[2]);
	return walkable(face);
}

// uint32 Pathfinding::findFace(float x, float y, float z) const {
// 	for (uint32 f = 0; f < _facesCount; ++f) {
// 		if (inFace(f, x, y, z))
// 			return f;
// 	}
//
// 	return UINT32_MAX;
// }

uint32 Pathfinding::findFace(float x, float y, float z, bool onlyWalkable) {
	for (std::vector<Common::AABBNode *>::iterator it = _AABBTrees.begin(); it != _AABBTrees.end(); ++it) {
		if (*it == 0)
			continue;

		if (!(*it)->isIn(x, y, z))
			continue;

		std::vector<Common::AABBNode *> nodes;
		(*it)->getNodes(x, y, nodes);
		for (uint n = 0; n < nodes.size(); ++n) {
			uint32 face = nodes[n]->getProperty();
			// Check walkability
			if (onlyWalkable && !walkable(face))
				continue;

			if (!inFace(face, Common::Vector3(x, y, z)))
				continue;

			return face;
		}
	}

	return UINT32_MAX;
}

uint32 Pathfinding::findFace(float x1, float y1, float z1, float x2, float y2, float z2, Common::Vector3 &intersect) {
	warning("Looking for a face");
	for (std::vector<Common::AABBNode *>::iterator it = _AABBTrees.begin(); it != _AABBTrees.end(); ++it) {
		if (*it == 0)
			continue;

		if (!(*it)->isIn(x1, y1, z1, x2, y2, z2))
			continue;

// 		Common::AABBNode *node = (*it)->getNode(x1, y1, z1, x2, y2, z2);
// 		uint32 face = node->getProperty();
// 		_facesToDraw.clear();
// 		_facesToDraw.push_back(face);
// 		inFace(face, Common::Vector3(x1, y1, z1), Common::Vector3(x2, y2, z2), intersect);
// 		return face;
		std::vector<Common::AABBNode *> nodes;
		(*it)->getNodes(x1, y1, z1, x2, y2, z2, nodes);
		for (uint n = 0; n < nodes.size(); ++n) {
			uint32 face = nodes[n]->getProperty();
			if (!inFace(face, Common::Vector3(x1, y1, z1), Common::Vector3(x2, y2, z2), intersect))
				continue;

			_facesToDraw.clear();
			_facesToDraw.push_back(face);
			warning("face found : %u", face);
			if (_adjFaces[face * 3] != UINT32_MAX)
				_facesToDraw.push_back(_adjFaces[face * 3]);
			if (_adjFaces[face * 3 + 1] != UINT32_MAX)
				_facesToDraw.push_back(_adjFaces[face * 3 + 1]);
			if (_adjFaces[face * 3 + 2] != UINT32_MAX)
				_facesToDraw.push_back(_adjFaces[face * 3 + 2]);
			return face;
		}
	}

	// Face not found
	return UINT32_MAX;
}

bool Pathfinding::goThrough(uint32 fromFace, uint32 toFace, float width) {
	if (width <= 0.f)
		return true;

// 	warning("check (%u -> %u)", fromFace, toFace);
	Common::Vector3 vec1, vec2;
	getSharedVertices(fromFace, toFace, vec1, vec2);
	// Check if the shared side is large enough
	if ((vec2 - vec1).length() > width)
		return true;

// 	warning("want to go there");
	Common::Vector3 center, side1, side2, intersect;
	center = vec1 + ((vec2 - vec1) * 0.5f);
	side1 = center + ((vec1 - vec2).norm() * (width / 2));
	side2 = center - ((vec1 - vec2).norm() * (width / 2));
	bool test1 = walkable(findFace(side1._x, side1._y, side1._z)) && walkable(findFace(side2._x, side2._y, side2._z));
	if (test1)
		return true;

	side1 = vec1;
	side2 = vec1 + ((vec2 - vec1).norm() * width);
	bool test2 = walkable(findFace(side1._x, side1._y, side1._z)) && walkable(findFace(side2._x, side2._y, side2._z));
	if (test2)
		return true;

	side1 = vec2;
	side2 = vec2 + ((vec1 - vec2).norm() * width);
	bool test3 = walkable(findFace(side1._x, side1._y, side1._z)) && walkable(findFace(side2._x, side2._y, side2._z));
	if (test3)
		return true;

// 	warning("check (%u -> %u) neighbour %i, %i, %i", fromFace, toFace, test1, test2, test3);

	return false;
}

void Pathfinding::getAdjacentNodes(Node &node, std::vector<uint32> &adjNodes) {
	adjNodes.clear();

	// Get adjacent faces
	for (uint8 f = 0; f < 3; ++f) {
		// Get adjacent face.
		uint32 face = _adjFaces[node.face * 3 + f];

		// Check if it is a border
		if (face == UINT32_MAX)
			continue;

		// Check if it is the parent node.
		if (face != node.parent)
			adjNodes.push_back(face);
	}
}

float Pathfinding::getDistance(Node &fromNode, uint32 toFace, float &toX, float &toY, float &toZ) const {
	// Get vertices from the closest edge to the face we are looking at.
	for (uint8 f = 0; f < 3; ++f) {
		if (_adjFaces[fromNode.face * 3 + f] != toFace)
			continue;

		uint32 vert1 = _faces[fromNode.face * 3 + f];
		uint32 vert2 = _faces[fromNode.face * 3 + (f + 1) % 3];

		// Compute the center of the edge.
		toX = (_vertices[vert1 * 3] + _vertices[vert2 * 3]) / 2;
		toY = (_vertices[vert1 * 3 + 1] + _vertices[vert2 * 3 + 1]) / 2;
		toZ = (_vertices[vert1 * 3 + 2] + _vertices[vert2 * 3 + 2]) / 2;

		// Compute the distance.
		return getDistance(fromNode.x, fromNode.y, fromNode.z, toX, toY, toZ);
	}

// 	warning("getDistance(): Face is not adjacent");
	return -1.f;
}

float Pathfinding::getDistance(float fX, float fY, float fZ, float tX, float tY, float tZ) const {
	// Compute the distance.
	float dX = fX - tX;
	float dY = fY - tY;
	float dZ = fZ - tZ;

	return sqrt(dX * dX + dY * dY + dZ * dZ);
}

float Pathfinding::getHeuristic(Node &node, Node &endNode) const {
	return getDistance(node.x, node.y, node.z, endNode.x, endNode.y, endNode.z);
}

bool Pathfinding::inFace(uint32 faceID, Common::Vector3 point) const {
	Common::Vector3 vA, vB, vC;
	getVertices(faceID, vA, vB, vC);

	return Common::inFace(point, vA, vB, vC);
}

bool Pathfinding::inFace(uint32 faceID, Common::Vector3 lineStart, Common::Vector3 lineEnd, Common::Vector3 &intersect) {
	Common::Vector3 vA, vB, vC;
	getVertices(faceID, vA, vB, vC);

	return Common::inFace(vA, vB, vC, lineStart, lineEnd, intersect);
}

bool Pathfinding::hasVertex(uint32 face, Common::Vector3 vertex) const {
	Common::Vector3 vert[3];
	getVertices(face, vert[0], vert[1], vert[2]);

	for (uint8 i = 0; i < 3; ++i) {
		if (vert[i] == vertex)
			return true;
	}

	return false;
}

bool Pathfinding::getSharedVertices(uint32 face1, uint32 face2, Common::Vector3 &vert1, Common::Vector3 &vert2) const {
// 	warning("getSharedVertices f1: %u, f2: %u", face1, face2);
	Common::Vector3 v[3];
	getVertices(face1, v[0], v[1], v[2]);
// 	warning("face1: (%f, %f, %f), (%f, %f, %f), (%f, %f, %f)", v[0]._x, v[0]._y, v[0]._z, v[1]._x, v[1]._y, v[1]._z, v[2]._x, v[2]._y, v[2]._z);
	getVertices(face2, v[0], v[1], v[2]);
// 	warning("face2: (%f, %f, %f), (%f, %f, %f), (%f, %f, %f)", v[0]._x, v[0]._y, v[0]._z, v[1]._x, v[1]._y, v[1]._z, v[2]._x, v[2]._y, v[2]._z);
	for (uint8 i = 0; i < 3; ++i) {
		if (_adjFaces[face1 * 3 + i] == face2) {
			Common::Vector3 vertices[3];
			getVertices(face1, vertices[0], vertices[1], vertices[2]);
			vert1 = vertices[i];
			vert2 = vertices[(i + 1) % 3];
// 			warning("vert1: (%f, %f, %f)", vert1._x, vert1._y, vert1._z);
// 			warning("vert2: (%f, %f, %f)", vert2._x, vert2._y, vert2._z);
			return true;
		}
	}

	// The faces are not adjacent.
	return false;
}

} // namespace Engines
