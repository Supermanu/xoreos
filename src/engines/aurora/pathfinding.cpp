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

#include <boost/geometry/geometries/point_xy.hpp>
#include <boost/geometry/geometries/box.hpp>
#include <boost/geometry/geometries/polygon.hpp>
#include <boost/geometry/algorithms/intersects.hpp>
#include <boost/geometry/geometries/register/point.hpp>

#include "src/common/util.h"
#include "src/common/maths.h"
#include "src/common/vector3.h"
#include "src/common/aabbnode.h"

#include "src/graphics/graphics.h"

#include "src/engines/aurora/astaralgorithm.h"
#include "src/engines/aurora/pathfinding.h"

namespace bg = boost::geometry;

BOOST_GEOMETRY_REGISTER_POINT_2D(Common::Vector3, float, cs::cartesian, _x, _y);
typedef bg::model::point<float, 3, bg::cs::cartesian> bPoint3D;

namespace Engines {

Pathfinding::Pathfinding(std::vector<bool> walkableProperties, uint32 polygonEdges) :
                       _polygonEdges(polygonEdges), _verticesCount(0), _facesCount(0),
                       _walkableProperties(walkableProperties), _aStarAlgorithm(0) {
	_creatureWidth = 0.f;
	_creaturePos = Common::Vector3(0.f, 0.f, 0.f);
}

Pathfinding::~Pathfinding() {
	delete _aStarAlgorithm;
}

bool Pathfinding::findPath(float startX, float startY, float endX, float endY,
	                       std::vector<uint32> &facePath, float width, uint32 nbrIt) {
	if (!_aStarAlgorithm)
		error("An AStar algorithm must be set");

	bool result = _aStarAlgorithm->findPath(startX, startY, endX, endY, facePath, width, nbrIt);
	_facesToDraw.clear();
	_facesToDraw.assign(facePath.begin(), facePath.end());
	return result;
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

void Pathfinding::minimizePath(std::vector<Common::Vector3> &path, float halfWidth) {
	if (path.size() < 3)
		return;

	std::vector<Common::Vector3> newPath;
	newPath.push_back(path.front());
	// Check if we can bypass a step in the path.
	for (uint32 step = 0; step < path.size() - 2; ++step) {
		// Build a rectangle between step and step + 2 and check if it is walkable.
		Common::Vector3 shift = getOrthonormalVec(path[step + 2] - path[step]);
		std::vector<Common::Vector3> rectangle;
		Common::Vector3 rightBottomCorner = path[step] + shift * halfWidth;
		rectangle.push_back(rightBottomCorner);
		Common::Vector3 leftBottomCorner = path[step] - shift * halfWidth;
		rectangle.push_back(leftBottomCorner);
		Common::Vector3 leftTopCorner = path[step + 2] - shift * halfWidth;
		rectangle.push_back(leftTopCorner);
		Common::Vector3 rightTopCorner = path[step + 2] + shift * halfWidth;
		rectangle.push_back(rightTopCorner);

		if (!walkablePolygon(rectangle)) {
			newPath.push_back(path[step + 1]);
			continue;
		}

		// Bypass step + 1 point.
		newPath.push_back(path[step + 2]);
		++step;
	}

	newPath.push_back(path.back());
	if (path.size() == newPath.size())
		return;

	path.clear();
	path.assign(newPath.begin(), newPath.end());
}

void Pathfinding::smoothPath(float startX, float startY, float endX, float endY, std::vector<uint32> &facePath,
                            std::vector<Common::Vector3> &path, float width, float stopLength) {
	// Use Vector3 for simplicity and vectorial operations.
	Common::Vector3 start(startX, startY, 0.f);
	Common::Vector3 end(endX, endY, 0.f);

	// Vector that will store positions of each vertex of the path of faces.
	std::vector<Common::Vector3> tunnel;
	// Indicate if the position is to the left or to the right.
	std::vector<bool> tunnelLeftRight;
	// Vector that will store if the path is next to a wall/object.
	std::vector<bool> tunnelFree;

	// Vector that will store the vertices that we will keep.
	std::vector<uint32> funnelIdx;
	float halfWidth = width / 2.f;
	float currentLength = 0.f;
	uint32 stopVertex = UINT32_MAX;

	tunnel.push_back(start);
	tunnelLeftRight.push_back(true);

	// Fill the tunnel vector and the related left/right vector.
	getVerticesTunnel(facePath, tunnel, tunnelLeftRight);
	// Add the ending point.
	tunnel.push_back(end);
	tunnel.push_back(end);
	// Consider the last point as a left position.
	tunnelLeftRight.push_back(true);
	tunnelLeftRight.push_back(false);

	tunnelFree.resize(tunnel.size());

	uint32 apex = 0;
	uint32 feeler[2] = {0, 0};
	Common::Vector3 feelerVector[2];

	_pointsToDraw.clear();
	for (uint32 c = 1; c < tunnel.size(); c++ ) {
		// Get the vector between the apex and the next vertex on the path of faces.
		Common::Vector3 v = tunnel[c] - tunnel[apex];

		// Check if we are next to a wall.
		tunnelFree[c] = walkableAASquare(tunnel[c], halfWidth);

		// In order to take the creature's width into account, we modify the position of the vertex by
		// taking the tangent of the circle (radius halfwidth) around the considered vertex.
		// If the apex and the considered vertex is on the same side of the funnel, it makes no sense to
		// compute the tangent. If the distance between the apex and the considered vertex (the length of v)
		// is below half of the width, it also makes no sense to compute the tangent as the two vertices are
		// too close. This is again the case if we are not next to a wall.
		if (halfWidth != 0.f && v.length() > halfWidth && !tunnelFree[c] ) {
			if (apex == 0) {
				// Check if we are not considering the ending point. Otherwise, we just want a straight line.
				if (c < tunnel.size() - 2) {
					// Compute the tangent.
					float len = v.length();
					float fsin = halfWidth / len * (tunnelLeftRight[c] ? -1.f : 1.0f);
					float fcos = sqrt(len * len - halfWidth * halfWidth) / len;
					float vX = v._x * fcos - v._y * fsin;
					float vY = v._x * fsin + v._y * fcos;
					v._x = vX;
					v._y = vY;
				}
			// Check if the current point is the ending point.
			} else if ( c >= tunnel.size() - 2 ) {
				// Compute the tangent.
				float len = v.length();
				float fsin = halfWidth / len * (tunnelLeftRight[apex] ? 1.0f : -1.f);
				float fcos = sqrt(abs(len * len - halfWidth * halfWidth)) / len;
				float vX = v._x * fcos - v._y * fsin;
				float vY = v._x * fsin + v._y * fcos;
				v._x = vX;
				v._y = vY;
			// Check if the apex and the considered vertex are on the opposite side.
			} else if ( tunnelLeftRight[c] != tunnelLeftRight[apex]) {
				// Compute the tangent.
				float len = v.length() * 0.5f;
				float fsin = halfWidth / len * (tunnelLeftRight[c] ? -1.f : 1.0f);
				float fcos = sqrt(abs(len * len - halfWidth * halfWidth)) / len;
				float vX = v._x * fcos - v._y * fsin;
				float vY = v._x * fsin + v._y * fcos;
				v._x = vX;
				v._y = vY;
			}
		}

		_pointsToDraw.push_back(v + tunnel[apex]);
		// Are we outside of the feeler ? Or have we just move the apex to new vertex?
		// The cross operation can state if the vector is on the left or right relatively.
		if (apex == feeler[tunnelLeftRight[c]]
		    || (v.cross(feelerVector[tunnelLeftRight[c]])._z < 0.f) != tunnelLeftRight[c]) {

			feeler[tunnelLeftRight[c]] = c;
			feelerVector[tunnelLeftRight[c]] = v;

			// Ensure that the opposite feeler is not the apex. Are we crossing the opposite feeler or
			// is it the ending point?
			if (apex != feeler[!tunnelLeftRight[c]] && (tunnel[c] == tunnel[feeler[!tunnelLeftRight[c]]]
			    || (v.cross(feelerVector[!tunnelLeftRight[c]])._z < 0.f) != tunnelLeftRight[c])) {
				funnelIdx.push_back(apex);

				// Update the path length.
				currentLength += (tunnel[feeler[!tunnelLeftRight[c]]] - tunnel[apex]).length();
				if (stopVertex == UINT32_MAX && currentLength >= stopLength)
					stopVertex = funnelIdx.size();

				// Move the apex to the opposite feeler.
				apex = feeler[!tunnelLeftRight[c]];

				// Sometimes the current vertex can be closer than the one on the opposite feeler.
				// In that case, swap them.
				if (v.length() < (feelerVector[!tunnelLeftRight[c]]).length()) {
					Common::Vector3 tmpV = tunnel[c];
					tunnel[c] = tunnel[feeler[!tunnelLeftRight[c]]];
					tunnel[feeler[!tunnelLeftRight[c]]] = tmpV;
					bool tmpB = tunnelLeftRight[c];
					tunnelLeftRight[c] = tunnelLeftRight[feeler[!tunnelLeftRight[c]]];
					tunnelLeftRight[feeler[!tunnelLeftRight[c]]] = tmpB;
				}

				// Adjust the current vertex and the feelers.
				c = apex;
				feeler[0] = apex;
				feeler[1] = apex;
			}
		}
	}

	path.push_back(start);
	if (funnelIdx.size() < 2) {
		path.push_back(end);
		_linesToDraw.clear();
		_linesToDraw.push_back(start);
		_linesToDraw.push_back(end);
		return;
	}

	// FIXME: The next chunck of code is provided as there is now no dynamic walkmesh.
	// In a near future, this will be not needed anymore.
	if (halfWidth > 0.f) {
		// Rough algorithm to circumvent next-to-wall vertices.
		// The principle is to draw the upper part of a square around a vertex that we have to
		// circumvent. Ideally it should be a nice circle, i.e. a rounded corner.
		Common::Vector3 middlePoint, segment;
		Common::Vector3 middleSquare, firstSquare, secondSquare, orthoVec;

		funnelIdx.push_back(tunnel.size() - 1);
		for (uint32 point = 1; point < funnelIdx.size() - 1; ++point) {
			uint32 pos = funnelIdx[point];
			uint32 nextPos = funnelIdx[point + 1];
			// Check if curcumvention is still needed.
			if (point > stopVertex) {
				path.push_back(tunnel[pos]);
				continue;
			}
			// Check if it is wall free.
			if (tunnelFree[pos]) {
				path.push_back(tunnel[pos]);
				continue;
			}

			middlePoint = tunnel[pos];
			segment = tunnel[nextPos] - path.back();

			bool clockwise = tunnelLeftRight[pos];
			orthoVec = getOrthonormalVec(segment, clockwise) * halfWidth;
			middleSquare = middlePoint + orthoVec;
			// Check if we still are in a walkable surface (right side)
			if (!walkable(middleSquare))
				middleSquare = middlePoint - orthoVec;

			secondSquare = middleSquare + (segment.norm() * halfWidth);
			firstSquare = middleSquare + (segment.norm() * (-1) * halfWidth);
			if (!walkable(firstSquare) || !walkable(secondSquare)) {
				// TODO What is the good choice?
				warning("A square is not walkable");
				continue;
			}

			path.push_back(firstSquare);
			// If the two position are too close, avoid "z" path (going backward) except for last point.
			if ((tunnel[nextPos] - tunnel[pos]).length() > width
			    || point + 2 == funnelIdx.size())
				path.push_back(secondSquare);
		}
	} else {
		// Build the path from the funnel.
		for (uint32 point = 1; point < funnelIdx.size() - 1; ++point)
			path.push_back(tunnel[funnelIdx[point]]);
	}

	path.push_back(end);

	// Remove unnecessary step. This is very slow
	// 	minimizePath(finalPath, halfWidth);

	// Drawing part.
	_linesToDraw.clear();
	for (std::vector<Common::Vector3>::iterator f = path.begin(); f != path.end(); ++f)
		_linesToDraw.push_back(*f);

	_pointsToDraw.clear();
	_pointsToDraw.push_back(start);
	for (std::vector<uint32>::iterator f = funnelIdx.begin(); f != funnelIdx.end(); ++f)
		_pointsToDraw.push_back(tunnel[*f]);

	_pointsToDraw.push_back(end);
}

void Pathfinding::getVerticesTunnel(std::vector<uint32> &facePath, std::vector<Common::Vector3> &tunnel,
                                   std::vector<bool> &tunnelLeftRight) {
	if (facePath.size() < 2)
		return;

	std::vector<Common::Vector3> cVerts, pVerts;

	for (uint32 face = 1; face < facePath.size(); ++face) {
		getVertices(facePath[face], cVerts);
		getVertices(facePath[face - 1], pVerts);

		if (face == 1) {
			// Find the first left and right by comparing to the next face.
			uint8 startVert = _polygonEdges;
			for (uint8 pV = 0; pV < _polygonEdges; ++pV) {
				bool equal = false;
				for (uint8 cV = 0; cV < _polygonEdges; ++cV) {
					if (pVerts[pV] == cVerts[cV]) {
						equal = true;
						break;
					}
				}
				if (equal)
					continue;

				startVert = pV;
				break;
			}

			if (startVert == _polygonEdges)
				error("No different vertices found");

			tunnel.push_back(pVerts[(startVert + 1) % _polygonEdges]);
			tunnel.push_back(pVerts[(startVert + 2) % _polygonEdges]);

			// Check if it is clockwise (to the left).
			bool orderedVert = isToTheLeft(pVerts[(startVert + 1) % _polygonEdges], pVerts[(startVert + 2) % _polygonEdges], pVerts[startVert]);
			// We are looking backward.
			tunnelLeftRight.push_back(!orderedVert);
			tunnelLeftRight.push_back(orderedVert);
		} else {
			bool otherVertSide = true;
			for (uint8 cV = 0; cV < _polygonEdges; ++cV) {
				bool equal = false;
				for (uint8 pV = 0; pV < _polygonEdges; ++pV) {
					if (pVerts[pV] == cVerts[cV]) {
						equal = true;
						break;
					}
				}
				if (equal) {
					// Check if it is a new vertex or an already added vertex.
					bool alreadyThere = false;
					for (uint32 t = tunnel.size() - 1; t != UINT32_MAX;--t) {
						if (cVerts[cV] == tunnel[t]) {
							// The new vertex has to be the vertex from the previous face which we hadn't add last time.
							// So if we know on which side the vertex we previously added, we can deduce the side of the new added vertex.
							otherVertSide = tunnelLeftRight[t];
							alreadyThere = true;
							break;
						}
					}

					if (!alreadyThere)
						tunnel.push_back(cVerts[cV]);
				}
			}
			tunnelLeftRight.push_back(!otherVertSide);
		}
	}
}

void Pathfinding::getVertices(uint32 faceID, std::vector<Common::Vector3> &vertices, bool xyPlane) const {
	vertices.clear();
	vertices.resize(_polygonEdges);

	for (uint32 v = 0; v < _polygonEdges; ++v) {
		getVertex(_faces[faceID * _polygonEdges + v], vertices[v], xyPlane);
	}
}


void Pathfinding::getVertex(uint32 vertexID, Common::Vector3 &vertex, bool xyPlane) const {
	// Don't take the z component into account.
	vertex = Common::Vector3(_vertices[vertexID * _polygonEdges],
	                         _vertices[vertexID * _polygonEdges + 1],
	                         xyPlane ? 0.f : _vertices[vertexID * _polygonEdges + 2]);
}

bool Pathfinding::walkableAASquare(Common::Vector3 center, float halfWidth) {
	Common::Vector3 min(center[0] - halfWidth, center[1] - halfWidth, 0.f);
	Common::Vector3 max(center[0] + halfWidth, center[1] + halfWidth, 0.f);

	std::vector<Common::AABBNode *> nodesIn;
	for (std::vector<Common::AABBNode *>::iterator n = _AABBTrees.begin(); n != _AABBTrees.end(); ++n) {
		if (*n)
			(*n)->getNodesInAABox2D(min, max, nodesIn);
	}

	bg::model::box<Common::Vector3> box(min, max);
	std::vector<Common::Vector3> vertices;
	for (std::vector<Common::AABBNode *>::iterator n = nodesIn.begin(); n != nodesIn.end(); ++n) {
		uint32 face = (*n)->getProperty();
		getVertices(face, vertices);
		bg::model::polygon<Common::Vector3> boostFace;
		bg::assign_points(boostFace, vertices);

		if (!bg::intersects(box, boostFace))
			continue;

		if (!faceWalkable(face))
			return false;
	}
	return true;
}

bool Pathfinding::walkablePolygon(std::vector<Common::Vector3> &vertices) {
	std::vector<Common::AABBNode *> nodesIn;

	for (std::vector<Common::AABBNode *>::iterator n = _AABBTrees.begin(); n != _AABBTrees.end(); ++n) {
		if (*n)
			(*n)->getNodesInPolygon(vertices, nodesIn);
	}

	bg::model::polygon<Common::Vector3> polygon;
	bg::assign_points(polygon, vertices);

	std::vector<Common::Vector3> vertFace;
	for (std::vector<Common::AABBNode *>::iterator n = nodesIn.begin(); n != nodesIn.end(); ++n) {
		uint32 face = (*n)->getProperty();
		getVertices(face, vertFace);
		bg::model::polygon<Common::Vector3> testFace;
		bg::assign_points(testFace, vertices);

		if (!bg::intersects(polygon, testFace))
			continue;

		if (!faceWalkable(face))
			return false;
	}
	return true;
}

bool Pathfinding::walkableSegment(Common::Vector3 start, Common::Vector3 end) {
	std::vector<Common::AABBNode *> nodesIn;

	for (std::vector<Common::AABBNode *>::iterator n = _AABBTrees.begin(); n != _AABBTrees.end(); ++n) {
		if (*n)
			(*n)->getNodesInSegment(start, end, nodesIn);
	}

	bg::model::segment<Common::Vector3> line(start, end);
	std::vector<Common::Vector3> vertFace;
	for (std::vector<Common::AABBNode *>::iterator n = nodesIn.begin(); n != nodesIn.end(); ++n) {
		uint32 face = (*n)->getProperty();
		getVertices(face, vertFace);
		bg::model::polygon<Common::Vector3> testFace;
		bg::assign_points(testFace, vertFace);

		if (!bg::intersects(line, testFace))
			continue;

		if (!faceWalkable(face))
			return false;
	}
	return true;
}

void Pathfinding::drawWalkmesh() {
	if (_faces.empty())
		return;

//	glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
//	for (uint32 f = 0; f < _facesCount; ++f) {
//		if (!faceWalkable(f))
//			continue;
//		glBegin(GL_TRIANGLES);
//		glColor3f(0.5, 0.5, 0.5);
//		uint32 vI_1 = _faces[f * 3 + 0];
//		glVertex3f(_vertices[vI_1 * 3], _vertices[vI_1 * 3 + 1], _vertices[vI_1 * 3 + 2] + 0.01);
//		uint32 vI_2 = _faces[f * 3 + 1];
//		glVertex3f(_vertices[vI_2 * 3], _vertices[vI_2 * 3 + 1], _vertices[vI_2 * 3 + 2] + 0.01);
//		uint32 vI_3 = _faces[f * 3 + 2];
//		glVertex3f(_vertices[vI_3 * 3], _vertices[vI_3 * 3 + 1], _vertices[vI_3 * 3 + 2] + 0.01);
//		glEnd();
//	}
//	glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
//	for (uint32 f = 0; f < _facesCount; ++f) {
//// 		if (!walkable(f))
//// 			continue;
//		glBegin(GL_TRIANGLES);
//		if (faceWalkable(f)) {
//			glColor4f(0.5, 0, 0.5, 0.4);
//		} else {
//			glEnd();
//			continue;
//// 			glColor4f(0.1, 0.5, 0.5, 0.4);
//		}

//		uint32 vI_1 = _faces[f * 3 + 0];
//		glVertex3f(_vertices[vI_1 * 3], _vertices[vI_1 * 3 + 1], _vertices[vI_1 * 3 + 2] + 0.04);
//		uint32 vI_2 = _faces[f * 3 + 1];
//		glVertex3f(_vertices[vI_2 * 3], _vertices[vI_2 * 3 + 1], _vertices[vI_2 * 3 + 2] + 0.04);
//		uint32 vI_3 = _faces[f * 3 + 2];
//		glVertex3f(_vertices[vI_3 * 3], _vertices[vI_3 * 3 + 1], _vertices[vI_3 * 3 + 2] + 0.04);
//		glEnd();
//	}

	// Draw path.
	for (uint32 f = 0; f < _facesToDraw.size(); ++f) {
		glBegin(GL_TRIANGLES);
		glColor4f(0.5, 0.2, 0.5, 0.4);

		uint32 vI_1 = _faces[_facesToDraw[f] * 3 + 0];
		glVertex3f(_vertices[vI_1 * 3], _vertices[vI_1 * 3 + 1], _vertices[vI_1 * 3 + 2] + 0.04);
		uint32 vI_2 = _faces[_facesToDraw[f] * 3 + 1];
		glVertex3f(_vertices[vI_2 * 3], _vertices[vI_2 * 3 + 1], _vertices[vI_2 * 3 + 2] + 0.04);
		uint32 vI_3 = _faces[_facesToDraw[f] * 3 + 2];
		glVertex3f(_vertices[vI_3 * 3], _vertices[vI_3 * 3 + 1], _vertices[vI_3 * 3 + 2] + 0.04);
		glEnd();
	}

	if (_linesToDraw.size() < 1)
		return;

	for (uint32 p = 0; p < _linesToDraw.size() - 1; ++p) {
		glLineWidth(3.f);
		glBegin(GL_LINES);
		glColor4f(1.f, 0.0, 1.0, 0.5);
		glVertex3f(_linesToDraw[p]._x, _linesToDraw[p]._y, _linesToDraw[p]._z + 0.05);
		glVertex3f(_linesToDraw[p + 1]._x, _linesToDraw[p + 1]._y, _linesToDraw[p + 1]._z + 0.05);
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

	if (_pointsToDraw.size() > 0) {
		glColor3f(1.0f,1.0f,1.0f);
		glPointSize(10.0f);
		glBegin(GL_POINTS);
		for (std::vector<Common::Vector3>::iterator p = _pointsToDraw.begin(); p != _pointsToDraw.end(); ++p)
			glVertex3f((*p)._x, (*p)._y, (*p)._z + 0.05);
		glEnd();
	}
}

bool Pathfinding::walkable(Common::Vector3 point) {
	uint32 face = findFace(point[0], point[1]);
	if (face == UINT32_MAX) {
		warning("face not found");
		return false;
	}

	return faceWalkable(face);
}

uint32 Pathfinding::findFace(float x, float y, bool onlyWalkable) {
    for (std::vector<Common::AABBNode *>::iterator it = _AABBTrees.begin(); it != _AABBTrees.end(); ++it) {
        if (*it == 0)
            continue;

        if (!(*it)->isIn(x, y))
            continue;

        std::vector<Common::AABBNode *> nodes;
        (*it)->getNodes(x, y, nodes);
        for (uint n = 0; n < nodes.size(); ++n) {
            uint32 face = nodes[n]->getProperty();
            // Check walkability
            if (onlyWalkable && !faceWalkable(face))
                continue;

            if (!inFace(face, Common::Vector3(x, y, 0.f)))
                continue;

            return face;
        }
    }

    return UINT32_MAX;
}

bool Pathfinding::findIntersection(float x1, float y1, float z1, float x2, float y2, float z2,
                                 Common::Vector3 &intersect, bool onlyWalkable) {
	for (std::vector<Common::AABBNode *>::iterator it = _AABBTrees.begin(); it != _AABBTrees.end(); ++it) {
		if (*it == 0)
			continue;

		if (!(*it)->isIn(x1, y1, z1, x2, y2, z2))
			continue;

		std::vector<Common::AABBNode *> nodes;
		(*it)->getNodes(x1, y1, z1, x2, y2, z2, nodes);
		for (uint n = 0; n < nodes.size(); ++n) {
			uint32 face = nodes[n]->getProperty();
			if (!inFace(face, Common::Vector3(x1, y1, z1), Common::Vector3(x2, y2, z2), intersect))
				continue;

			if (onlyWalkable && !faceWalkable(face))
				continue;

			return true;
		}
	}

	// Face not found
	return false;
}

bool Pathfinding::goThrough(uint32 fromFace, uint32 toFace, float width) {
	if (width <= 0.f)
		return true;

	Common::Vector3 vec1, vec2;
	getSharedVertices(fromFace, toFace, vec1, vec2);
	// Check if the shared side is large enough
	if ((vec2 - vec1).length() > width)
		return true;

	// Test maximum three different cases:
	//  * A segment at the center of the shared side,
	//  * A segment that starts at the right vertex and go to the left,
	//  * A segment that starts at the left vertex and go to the right.
	// Obviously, it doesn't account all the possibilities, the test segment could be
	// a little bit to the left and also to the right at same time though it should
	// avoid all true negatives. Also it could be faster to test the first test at
	// the last position, it should be tested.

	Common::Vector3 center, side1, side2, intersect;
	center = vec1 + ((vec2 - vec1) * 0.5f);
	side1 = center + ((vec1 - vec2).norm() * (width / 2));
	side2 = center - ((vec1 - vec2).norm() * (width / 2));
	bool test1 = walkableSegment(side1, side2);
	if (test1)
		return true;

	side1 = vec1;
	side2 = vec1 + ((vec2 - vec1).norm() * width);
	bool test2 = walkableSegment(side1, side2);
	if (test2)
		return true;

	side1 = vec2;
	side2 = vec2 + ((vec1 - vec2).norm() * width);
	bool test3 = walkableSegment(side1, side2);
	if (test3)
		return true;

	return false;
}

void Pathfinding::getAdjacentFaces(uint32 face, std::vector<uint32> &adjFaces) {
	adjFaces.clear();

	// Get adjacent faces
	for (uint8 f = 0; f < _polygonEdges; ++f) {
		// Get adjacent face.
		uint32 adjFace = _adjFaces[face * _polygonEdges + f];

		// Check if it is a border
		if (adjFace == UINT32_MAX)
			continue;

		adjFaces.push_back(adjFace);
	}
}

void Pathfinding::getAdjacencyCenter(uint32 faceA, uint32 faceB, float &x, float &y) const {
	bool adjacent = false;
	// Get vertices from the closest edge to the face we are looking at.
	for (uint8 f = 0; f < _polygonEdges; ++f) {
		if (_adjFaces[faceA * _polygonEdges + f] != faceB)
			continue;

		adjacent = true;
		uint32 vert1 = _faces[faceA * _polygonEdges + f];
		uint32 vert2 = _faces[faceA * _polygonEdges + (f + 1) % _polygonEdges];

		// Compute the center of the edge.
		x = (_vertices[vert1 * 3] + _vertices[vert2 * 3]) / 2;
		y = (_vertices[vert1 * 3 + 1] + _vertices[vert2 * 3 + 1]) / 2;

		break;
	}

	// Ensure the two faces are adjacent
	if(!adjacent)
		error("The two faces are not adjacent");
}

bool Pathfinding::inFace(uint32 faceID, Common::Vector3 point) const {
	// Ensure we are in the XY plane.
	point[2] = 0.f;

	std::vector<Common::Vector3> vertices;
	getVertices(faceID, vertices);
	// Close the polygon.
	vertices.push_back(vertices.front());

	bg::model::polygon<Common::Vector3> polygon;
	bg::assign_points(polygon, vertices);

	return bg::intersects(point, polygon);
}

bool Pathfinding::inFace(uint32 faceID, Common::Vector3 lineStart, Common::Vector3 lineEnd, Common::Vector3 &intersect) const {
	std::vector<Common::Vector3> vertices;
	getVertices(faceID, vertices, false);

	// Boost intersection algorithm in 3 dimensions doesn't seem reliable enough.
	// This algorithm is taken from "Fast, minimum storage ray/triangle intersection" by Tomas Möller and Ben Trumbore.
	float epsilon = 0.000001;

	Common::Vector3 segment = lineEnd - lineStart;
	Common::Vector3 edgeA, edgeB;
	Common::Vector3 P, Q, T;
	float determinant, invDeterminant, u, v;
	float t;

	// Find vectors for two edges sharing vA.
	edgeA = vertices[1] - vertices[0];
	edgeB = vertices[2] - vertices[0];

	// Begin calculating determinant - also used to calculate u parameter.
	P = segment.cross(edgeB);

	// If determinant is near zero, ray lies in plane of triangle or ray is parallel to plane of triangle.
	determinant = edgeA.dot(P);
	// Not culling.
	if (determinant > - epsilon && determinant < epsilon)
		return false;

	invDeterminant = 1.f / determinant;

	// Calculate distance from vA to ray origin.
	T = lineStart - vertices[0];

	// Calculate u parameter and test bound.
	u = T.dot(P) * invDeterminant;
	// The intersection lies outside of the triangle.
	if (u < 0.f || u > 1.f)
		return false;

	// Prepare to test v parameter.
	Q = T.cross(edgeA);

	// Calculate v parameter and test bound.
	v = segment.dot(Q) * invDeterminant;
	// The intersection lies outside of the triangle.
	if (v < 0.f || u + v  > 1.f)
		return false;

	t = edgeB.dot(Q) * invDeterminant;

	// Ray intersection.
	if (t > epsilon) {
		intersect = lineStart + segment * t;
		return true;
	}

	// No hit, no win.
	return false;
}

bool Pathfinding::getSharedVertices(uint32 face1, uint32 face2, Common::Vector3 &vert1, Common::Vector3 &vert2) const {
	for (uint8 i = 0; i < _polygonEdges; ++i) {
		if (_adjFaces[face1 * _polygonEdges + i] == face2) {
			std::vector<Common::Vector3> vertices;
			getVertices(face1, vertices);
			vert1 = vertices[i];
			vert2 = vertices[(i + 1) % _polygonEdges];

			return true;
		}
	}

	// The faces are not adjacent.
	return false;
}

void Pathfinding::setAStarAlgorithm(AStar *aStarAlgorithm) {
	_aStarAlgorithm = aStarAlgorithm;
}

bool Pathfinding::faceWalkable(uint32 faceID) const {
	return _walkableProperties[_faceProperty[faceID]];
}

} // namespace Engines
