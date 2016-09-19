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

#include "src/graphics/graphics.h"

#include "src/engines/aurora/pathfinding.h"


namespace Engines {

bool sortByLenght(Common::Vector3 vec1, Common::Vector3 vec2) {
	return vec1.length() < vec2.length();
}

Pathfinding::Pathfinding() : _verticesCount(0), _facesCount(0) {
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

// 	warning("finding path...");
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

// 	warning("starting the loob");
	// Searching...

// 	while (!openList.empty()) {
	for (uint32 it = 0; it < nbrIt; ++it) {
		Node current = openList.front();

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
		for (std::vector<uint32>::iterator a = adjNodes.begin(); a != adjNodes.end(); ++a) {
			// Check if it has been already evaluated.
			if (hasNode(*a, closedList))
				continue;

			// Distance from start point to this node.
			float x, y, z;
			float gScore = current.G + getDistance(current, *a, x, y, z);

			// Check if it is a new node.
			Node adjNode;
			if (!hasNode(*a, openList, adjNode)) {
				Node newNode(*a, x, y, z);
				newNode.parent = current.face;
				openList.push_back(newNode);
			} else if (gScore >= adjNode.G) {
				continue;
			}

			// adjNode is the best node up to now.
			adjNode.parent = current.face;
			adjNode.G = gScore;
			adjNode.H = getHeuristic(adjNode, endNode);
			std::sort(openList.begin(), openList.end());
		}
	}

	return false;
}

void Pathfinding::smoothPath(Common::Vector3 start, Common::Vector3 end, std::vector<uint32> &facePath, std::vector<Common::Vector3> &path) {
	// Simple smooth path algorithm.
	path.push_back(start);

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

void Pathfinding::getVertices(uint32 faceID, Common::Vector3 &vA, Common::Vector3 &vB, Common::Vector3 &vC) {
	uint32 vertexIdA = _faces[faceID * 3];
	uint32 vertexIdB = _faces[faceID * 3 + 1];
	uint32 vertexIdC = _faces[faceID * 3 + 2];

	vA = Common::Vector3(_vertices[vertexIdA * 3], _vertices[vertexIdA * 3 + 1], _vertices[vertexIdA * 3 + 2]);
	vB = Common::Vector3(_vertices[vertexIdB * 3], _vertices[vertexIdB * 3 + 1], _vertices[vertexIdB * 3 + 2]);
	vC = Common::Vector3(_vertices[vertexIdC * 3], _vertices[vertexIdC * 3 + 1], _vertices[vertexIdC * 3 + 2]);
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

bool Pathfinding::getIntersection(Common::Vector3 segStart1, Common::Vector3 segEnd1, Common::Vector3 segStart2, Common::Vector3 segEnd2, Common::Vector3 &intersect) {

// 	warning("getIntersection: segStart1(%f, %f), segEnd1(%f, %f), segStart2(%f, %f), segEnd2(%f, %f)",
// 			segStart1._x, segStart1._y, segEnd1._x, segEnd1._y, segStart2._x, segStart2._y, segEnd2._x, segEnd2._y
// 	);
	Common::Vector3 r = segEnd1 - segStart1;
	Common::Vector3 s = segEnd2 - segStart2;
// 	warning("r: (%f, %f)", r._x, r._y);
// 	warning("s: (%f, %f)", s._x, s._y);
	float rs = (r * s)._z;
// 	warning("rs: %f", rs);
	float qpr = ((segStart2 - segStart1) * r)._z;
// 	warning("qpr: %f", qpr);

	if (rs == 0 && qpr == 0) {
		// The two segments are parallel
		if (qpr == 0) {
			// The two segments are colinear.
			float t0 = (segStart2 - segStart1).dot(r) / r.dot(r);
			float t1 = t0 + s.dot(r) / (r.dot(r));

			if ((t0 <= 1 && t0 >= 0) || (t1 <= 1.f && t1 >= 0.f)) {
				// Segments are overlaping.
				float lenghtS1 = segStart1.length();
// 				float lenghtE1 = segEnd1.length();
				float lenghtS2 = segStart2.length();
				float lenghtE2 = segEnd2.length();

				// segStart1 or segEnd2 must be in seg2.
				if (lenghtS1 <= MAX(lenghtS2, lenghtE2) && lenghtS1 >= MIN(lenghtS2, lenghtE2)) {
					intersect = segStart1;
				} else {
					intersect = segEnd1;
				}
				return true;
			}
		}
		return false;
	}

	float u = qpr / rs;
	float t = ((segStart2 - segStart1) * s)._z / rs;

// 	warning("u: %f, t: %f", u, t);
	if (u <= 1 && u >= 0 && t <= 1 && t >= 0) {
		// The segments are intersecting.
		intersect = segStart1 + (segEnd1 - segStart1) * t;
// 		warning("Intersect: (%f, %f)", intersect._x, intersect._y);
		return true;
	}

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
}

bool Pathfinding::walkable(uint32 faceIndex) const {
	return _faceProperty[faceIndex] != 2 && _faceProperty[faceIndex] != 7;
}

uint32 Pathfinding::findFace(float x, float y, float z) const {
	for (uint32 f = 0; f < _facesCount; ++f) {
		if (inFace(f, x, y, z))
			return f;
	}

	return UINT32_MAX;
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
	return inFace(faceID, point._x, point._y, point._z);
}

bool Pathfinding::inFace(uint32 faceID, float x, float y, float z) const {
	// Use Barycentric Technique.
	uint32 vertexIdA = _faces[faceID * 3];
	uint32 vertexIdB = _faces[faceID * 3 + 1];
	uint32 vertexIdC = _faces[faceID * 3 + 2];

	Common::Vector3 vA(_vertices[vertexIdA * 3], _vertices[vertexIdA * 3 + 1], _vertices[vertexIdA * 3 + 2]);
	Common::Vector3 vB(_vertices[vertexIdB * 3], _vertices[vertexIdB * 3 + 1], _vertices[vertexIdB * 3 + 2]);
	Common::Vector3 vC(_vertices[vertexIdC * 3], _vertices[vertexIdC * 3 + 1], _vertices[vertexIdC * 3 + 2]);

	Common::Vector3 v0 = vC - vA;
	Common::Vector3 v1 = vB - vA;
	Common::Vector3 v2 = Common::Vector3(x, y, z) - vA;

	float dot00 = v0.dot(v0);
	float dot01 = v0.dot(v1);
	float dot02 = v0.dot(v2);
	float dot11 = v1.dot(v1);
	float dot12 = v1.dot(v2);

	// Compute barycentric coordinates
	float denom = 1.f / (dot00 * dot11 - dot01 * dot01);
	float u = (dot11 * dot02 - dot01 * dot12) * denom;
	float v = (dot00 * dot12 - dot01 * dot02) * denom;

// 	warning("In face %f, %f", u, v);
	// Check if point is in triangle
	return (u + 0.0001 >= 0) && (v + 0.0001 >= 0) && (u + v - 0.0001 < 1);
}

} // namespace Engines
