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

#include <boost/geometry.hpp>
#include <boost/geometry/geometries/point_xy.hpp>
#include <boost/geometry/geometries/box.hpp>
#include <boost/geometry/geometries/polygon.hpp>
#include <boost/geometry/algorithms/intersects.hpp>

#include "src/common/util.h"
#include "src/common/maths.h"
#include "src/common/vector3.h"
#include "src/common/vec3util.h"
#include "src/common/aabbnode.h"

#include "src/graphics/graphics.h"

#include "src/engines/aurora/astaralgorithm.h"
#include "src/engines/aurora/pathfinding.h"

typedef boost::geometry::model::d2::point_xy<float> boostPoint2d;

namespace Engines {

bool sortByLenght(Common::Vector3 vec1, Common::Vector3 vec2) {
	return vec1.length() < vec2.length();
}

Pathfinding::Pathfinding(uint32 polygonEdges) : _polygonEdges(polygonEdges), _verticesCount(0),
	_facesCount(0), _aStarAlgorithm(0) {
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
		Common::Vector3 rightBottomCorner = path[step] + shift * halfWidth;
		Common::Vector3 leftBottomCorner = path[step] - shift * halfWidth;
		Common::Vector3 rightTopCorner = path[step + 2] + shift * halfWidth;
		Common::Vector3 leftTopCorner = path[step + 2] - shift * halfWidth;
		Common::Vector3 rectangle[4] = {rightBottomCorner, leftBottomCorner, leftTopCorner, rightTopCorner};

		if (!walkablePolygon(rectangle, 4)) {
			newPath.push_back(path[step + 1]);
			continue;
		}

		// Bypass step + 1 point.
		newPath.push_back(path[step + 2]);
		++step;
	}

	newPath.push_back(path.back());
	if (path.size() == newPath.size()) {
		warning("Minimize: no step removed");
		return;
	}

	warning("Minimize: Some steps have been removed %lu", path.size() - newPath.size());
	path.clear();
	path.assign(newPath.begin(), newPath.end());
}

void Pathfinding::smoothPath(Common::Vector3 start, Common::Vector3 end, std::vector<uint32> &facePath, std::vector<Common::Vector3> &path, float width) {
	warning("SmoothPath");
	std::vector<Common::Vector3> tunnel;
	std::vector<Common::Vector3> smoothedPath, finalPath;
	std::vector<bool> tunnelLeftRight, tunnelFree;
	std::vector<uint32> funnelIdx;
	float halfWidth = width / 2.f;

	tunnel.push_back(start);
	tunnelLeftRight.push_back(true);

	getVerticesTunnel(facePath, tunnel, tunnelLeftRight);
	tunnel.push_back(end);
	tunnel.push_back(end);
	tunnelLeftRight.push_back(true);
	tunnelLeftRight.push_back(false);
	tunnelFree.resize(tunnel.size());

	uint32 apex = 0;
	// Ignore the start point
	uint32 feeler[ 2 ] = { 0, 0 }; //ie left/right

	Common::Vector3 feeler_v[ 2 ]; //We store the vector from the apex to the feelers,
	//which is a simple optimization that stops us from having to recalculate
	//them each step

	_pointsToDraw.clear();
	for (uint32 c = 1; c < tunnel.size(); c++ ) {
		Common::Vector3 v = tunnel[c] - tunnel [apex];

		bool wB = walkableAASquare(tunnel[c], halfWidth);
// 		warning("Is left ? %i | walkable in box ? %i", (bool) tunnelLeftRight[c], wB);

// 		warning("iteration %u", c);
// 		bool isLeft = tunnelLeftRight[c];
// 		warning("Is Left ? %u", isLeft);
// 		warning("v before any treatment (%f, %f, %f)", v[0], v[1], v[2]);
// 		warning("apex: %u", apex);
// 		warning("feeler (%u, %u)", feeler[0], feeler[1]);
// 		warning("feeler_v (%f, %f, %f) and (%f, %f, %f)", feeler_v[0][0], feeler_v[0][1], feeler_v[0][2], feeler_v[1][0], feeler_v[1][1], feeler_v[1][2]);

		bool nextToWall = !wB; //!walkableCircle(tunnel[c], halfWidth);
		tunnelFree[c] = !nextToWall;
// 		warning("Next to wall %i", nextToWall);

		if(xyLength(v) > halfWidth && nextToWall) { //if v.length is below halfwidth, then the vertices are too close together
			//to form meaningful tangent calculations. This is not the case for vertices on the same side of funnel,
			//but they get straight-line calculations anyway, and no two vertices on opposite sides of funnel should be this close
			//because that would make path invalid (only start and end)

			if (apex == 0) { //Apex is start point
				if (c < tunnel.size() - 2) { //If not true, the current
					//element is the end point, so we actually want straight line between the apex and it after all
					float len = xyLength(v);
					float fsin = halfWidth / len * (tunnelLeftRight[c] ? -1.f : 1.0f);
					float fcos = sqrt(len * len - halfWidth * halfWidth) / len;
					float vX = v._x * fcos - v._y * fsin;
					float vY = v._x * fsin + v._y * fcos;
					v._x = vX;
					v._y = vY;
				}
			} else if ( c >= tunnel.size() - 2 ) { //Current point is end point
				float len = xyLength(v);
				float fsin = halfWidth / len * (tunnelLeftRight[apex] ? 1.0f : -1.f);
				float fcos = sqrt(abs(len * len - halfWidth * halfWidth)) / len;
				float vX = v._x * fcos - v._y * fsin;
				float vY = v._x * fsin + v._y * fcos;
				v._x = vX;
				v._y = vY;
			} else if ( tunnelLeftRight[c] != tunnelLeftRight[apex]) { // Opposite sides of list
// 				warning("Opposite sides of list");
				float len = xyLength(v) * 0.5f;
				float fsin = halfWidth / len * (tunnelLeftRight[c] ? -1.f : 1.0f);
				float fcos = sqrt(abs(len * len - halfWidth * halfWidth)) / len;
				float vX = v._x * fcos - v._y * fsin;
				float vY = v._x * fsin + v._y * fcos;
				v._x = vX;
				v._y = vY;
// 				warning("len %f, fsin %f, fcos %f", len, fsin, fcos);
			}
		}

// 		warning("v after (%f, %f, %f)", v[0], v[1], v[2]);

		_pointsToDraw.push_back(v + tunnel[apex]);
		//Is in on the ‘outside’ of the corresponding feeler? (or is the first
		//iteration after advancing the apex)?
		if (apex == feeler[tunnelLeftRight[c]] || (v.cross(feeler_v[tunnelLeftRight[c]])._z < 0.0f ) != tunnelLeftRight[c]) {
// 			warning("It's inside or is first iteration (%i)", apex == feeler[tunnelLeftRight[c]]);
			feeler[tunnelLeftRight[c]] = c;
			feeler_v[tunnelLeftRight[c]] = v;

			//Does it cross the opposite feeler?
			//Here we must first establish that the opposite feeler is not the apex, else there will be no meaningful calculation
			//Also, we need to account for our end position, which is the last two elements on the tunnel list
			if (apex != feeler[!tunnelLeftRight[c]] && (tunnel[c] == tunnel[feeler[!tunnelLeftRight[c]]]
			   || (v.cross(feeler_v[!tunnelLeftRight[ c ] ] )._z < 0.0f) != tunnelLeftRight[c])) {
// 				warning("It's crossing opposite feeler");
				funnelIdx.push_back(apex);

				apex = feeler[!tunnelLeftRight[c]];


				//There is occasionaly an instance whereby the current vertex is actually closer
				//than the one on the oposite left/right list
				//If this is the case, SWAP them, because we want to move to apex to the closest one
				if (xyLength(v) < xyLength(feeler_v[ !tunnelLeftRight[ c ] ])) {
					Common::Vector3 tmpV = tunnel[c];
					tunnel[c] = tunnel[feeler[!tunnelLeftRight[c]]];
					tunnel[feeler[!tunnelLeftRight[c]]] = tmpV;
					bool tmpB = tunnelLeftRight[c];
					tunnelLeftRight[c] = tunnelLeftRight[feeler[!tunnelLeftRight[c]]];
					tunnelLeftRight[feeler[!tunnelLeftRight[c]]] = tmpB;
				}

				c = apex; //ie apex + 1 on next itteration
				feeler[0] = apex;
				feeler[1] = apex;
			}
		}
	}

	finalPath.push_back(start);
	if (funnelIdx.size() < 2) {
		finalPath.push_back(end);
		_linesToDraw.clear();
		_linesToDraw.push_back(start);
		_linesToDraw.push_back(end);
		return;
	}

	Common::Vector3 middlePoint, segment;
	Common::Vector3 middleSquare, firstSquare, secondSquare, orthoVec;
	// 	warning("point 0 (%f, %f, %f)", smoothedPath[0][0], smoothedPath[0][1], smoothedPath[0][2]);

	funnelIdx.push_back(tunnel.size() - 1);
	warning("funnelIdx size %zu", funnelIdx.size());
	for (uint32 point = 1; point < funnelIdx.size() - 1; ++point) {
		// 		warning("point %u (%f, %f, %f)", point + 1, smoothedPath[point + 1][0], smoothedPath[point + 1][1], smoothedPath[point + 1][2]);
// 		if (smoothedPath[point] == smoothedPath[point + 1])
// 			continue;

		uint32 pos = funnelIdx[point];
		uint32 nextPos = funnelIdx[point + 1];
		if (tunnelFree[pos]) {
			finalPath.push_back(tunnel[pos]);
			continue;
		}


		middlePoint = tunnel[pos];
		segment = tunnel[nextPos] - finalPath.back();

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

        finalPath.push_back(firstSquare);
        // If the two position are too close, avoid "z" path (going backward) except for last point.
		if (xyLength(tunnel[nextPos], tunnel[pos]) > width || (xyLength(tunnel[nextPos], tunnel[pos]) <= width && point + 2 == funnelIdx.size())) {
            finalPath.push_back(secondSquare);
		} else {
// 			warning("Don't add last square. FunnelIdx position %i", point);
		}

	}

	// 	warning("point %lu (%f, %f, %f)", smoothedPath.size() - 1, smoothedPath[smoothedPath.size() - 1][0], smoothedPath[smoothedPath.size() - 1][1], smoothedPath[smoothedPath.size() - 1][2]);
	finalPath.push_back(end);

	// Remove unnecessary step. This is very slow!
// 	minimizePath(finalPath, halfWidth);

	// Drawing part.
	_linesToDraw.clear();
	for (std::vector<Common::Vector3>::iterator f = finalPath.begin(); f != finalPath.end(); ++f)
		_linesToDraw.push_back(*f);

// 	warning("size : %zu", funnelIdx.size());

	_pointsToDraw.clear();
	_pointsToDraw.push_back(start);
	for (std::vector<uint32>::iterator f = funnelIdx.begin(); f != funnelIdx.end(); ++f)
		_pointsToDraw.push_back(tunnel[*f]);

	_pointsToDraw.push_back(end);

    for (uint32 p = 0; p < _pointsToDraw.size() - 1; ++p) {
        float l = xyLength(_pointsToDraw[p + 1], _pointsToDraw[p]);
//         warning("Length between %i and %i : %f", p, p+1, l);
    }
}

void Pathfinding::getVerticesTunnel(std::vector<uint32> &facePath, std::vector<Common::Vector3> &tunnel, std::vector<bool> &tunnelLeftRight) {
	if (facePath.size() < 2)
		return;

	Common::Vector3 cVert[3], pVert[3];


	for (uint32 face = 1; face < facePath.size(); ++face) {
		getVertices(facePath[face], cVert[0], cVert[1], cVert[2]);
		getVertices(facePath[face - 1], pVert[0], pVert[1], pVert[2]);
// 		warning("vertices from face %u (%f, %f, %f), (%f, %f, %f), (%f, %f, %f)", face,
// 		  cVert[0][0], cVert[0][1], cVert[0][2],
// 		  cVert[1][0], cVert[1][1], cVert[1][2],
// 		  cVert[2][0], cVert[2][1], cVert[2][2]
// 		);
//
// 		warning("vertices from face %u (%f, %f, %f), (%f, %f, %f), (%f, %f, %f)", face - 1,
// 				pVert[0][0], pVert[0][1], pVert[0][2],
// 		  pVert[1][0], pVert[1][1], pVert[1][2],
// 		  pVert[2][0], pVert[2][1], pVert[2][2]
// 		);

// 		warning("face %u", face);
		if (face == 1) {
			// Find the first left and right by comparing to the next face.
			uint8 startVert = 3;
			for (uint8 v = 0; v < 3; ++v) {
				if (pVert[v] == cVert[0] || pVert[v] == cVert[1] || pVert[v] == cVert[2])
					continue;

				startVert = v;
				break;
			}

			if (startVert == 3)
				error("No different vertices found");

			tunnel.push_back(pVert[(startVert + 1) % 3]);
			tunnel.push_back(pVert[(startVert + 2) % 3]);

			// Check if it is clockwise (to the left).
			bool orderedVert = triangleArea2(pVert[startVert], pVert[(startVert + 1) % 3], pVert[(startVert + 2) % 3]) > 0;
			// We are looking backward.
			tunnelLeftRight.push_back(!orderedVert);
			tunnelLeftRight.push_back(orderedVert);
		} else {
			bool otherVertSide = true;
			for (uint8 v = 0; v < 3; ++v) {
				if (cVert[v] == pVert[0] || cVert[v] == pVert[1] || cVert[v] == pVert[2]) {
					// Check if it is a new vertex or an already added vertex.
					bool alreadyThere = false;
					for (uint32 t = tunnel.size() - 1; t != UINT32_MAX;--t) {
						if (cVert[v] == tunnel[t]) {
							// The new vertex has to be the vertex from the previous face which we hadn't added last time.
							// So if we know on which side the vertex we previously added, we can deduce the side of the new added vertex.
							otherVertSide = tunnelLeftRight[t];
							alreadyThere = true;
							break;
						}
					}

					if (!alreadyThere)
						tunnel.push_back(cVert[v]);
				}
			}
			tunnelLeftRight.push_back(!otherVertSide);
		}
	}
// 	for (uint32 t = 0; t < tunnel.size(); ++t) {
// 		if (tunnelLeftRight[t])
// 			warning("Left (%f, %f, %f)", tunnel[t][0], tunnel[t][1], tunnel[t][2]);
// 		else
// 			warning("Right (%f, %f, %f)", tunnel[t][0], tunnel[t][1], tunnel[t][2]);
// 	}
}

void Pathfinding::getVertices(uint32 faceID, Common::Vector3 &vA, Common::Vector3 &vB, Common::Vector3 &vC) const {
	getVertex(_faces[faceID * 3], vA);
	getVertex(_faces[faceID * 3 + 1], vB);
	getVertex(_faces[faceID * 3 + 2], vC);
}

void Pathfinding::getVertex(uint32 vertexID, Common::Vector3 &vertex) const {
	vertex = Common::Vector3(_vertices[vertexID * 3], _vertices[vertexID * 3 + 1], _vertices[vertexID * 3 + 2]);
}

bool Pathfinding::walkableAASquare(Common::Vector3 center, float halfWidth) {
	Common::Vector3 min(center[0] - halfWidth, center[1] - halfWidth, 0.f);
	Common::Vector3 max(center[0] + halfWidth, center[1] + halfWidth, 0.f);

	std::vector<Common::AABBNode *> nodesIn;
	for (std::vector<Common::AABBNode *>::iterator n = _AABBTrees.begin(); n != _AABBTrees.end(); ++n) {
		if (*n)
			(*n)->getNodesInAABox2D(min, max, nodesIn);
	}

	boost::geometry::model::box<boostPoint2d> box(boostPoint2d(min[0], min[1]), boostPoint2d(max[0], max[1]));
	Common::Vector3 vertices[3];
	for (std::vector<Common::AABBNode *>::iterator n = nodesIn.begin(); n != nodesIn.end(); ++n) {
		uint32 face = (*n)->getProperty();
		getVertices(face, vertices[0], vertices[1], vertices[2]);
		boost::geometry::model::polygon<boostPoint2d> boostFace;
		for (uint32 v = 0; v < 3; ++v) {
			boostPoint2d vert(vertices[v][0], vertices[v][1]);
			boost::geometry::append(boostFace.outer(), vert);
		}

		if (!boost::geometry::intersects(box, boostFace))
			continue;

		if (!walkable(face))
			return false;
	}
	return true;
}

bool Pathfinding::walkablePolygon(Common::Vector3 vertices[], uint32 vertexCount) {
	std::vector<Common::AABBNode *> nodesIn;

	for (std::vector<Common::AABBNode *>::iterator n = _AABBTrees.begin(); n != _AABBTrees.end(); ++n) {
		if (*n)
			(*n)->getNodesInPolygon(vertices, vertexCount, nodesIn);
	}

	boost::geometry::model::polygon<boostPoint2d> polygon;
	for (uint32 v = 0; v < vertexCount; ++v) {
		boostPoint2d vert(vertices[v][0], vertices[v][1]);
		boost::geometry::append(polygon.outer(), vert);
	}

	Common::Vector3 vertFace[3];
	for (std::vector<Common::AABBNode *>::iterator n = nodesIn.begin(); n != nodesIn.end(); ++n) {
		uint32 face = (*n)->getProperty();
		getVertices(face, vertFace[0], vertFace[1], vertFace[2]);
		boost::geometry::model::polygon<boostPoint2d> boostFace;
		for (uint32 v = 0; v < 3; ++v) {
			boostPoint2d vert(vertFace[v][0], vertFace[v][1]);
			boost::geometry::append(boostFace.outer(), vert);
		}

		if (!boost::geometry::intersects(polygon, boostFace))
			continue;

		if (!walkable(face))
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

	boost::geometry::model::segment<boostPoint2d> line(boostPoint2d(start[0], start[1]), boostPoint2d(end[0], end[1]));
	Common::Vector3 vertFace[3];
	for (std::vector<Common::AABBNode *>::iterator n = nodesIn.begin(); n != nodesIn.end(); ++n) {
		uint32 face = (*n)->getProperty();
		getVertices(face, vertFace[0], vertFace[1], vertFace[2]);
		boost::geometry::model::polygon<boostPoint2d> boostFace;
		for (uint32 v = 0; v < 3; ++v) {
			boostPoint2d vert(vertFace[v][0], vertFace[v][1]);
			boost::geometry::append(boostFace.outer(), vert);
		}

		if (!boost::geometry::intersects(line, boostFace))
			continue;

		if (!walkable(face))
			return false;
	}
	return true;
}

void Pathfinding::drawWalkmesh() {
	if (_faces.empty())
		return;

	glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
	for (uint32 f = 0; f < _facesCount; ++f) {
		if (!walkable(f))
			continue;
		glBegin(GL_TRIANGLES);
		glColor3f(0.5, 0.5, 0.5);
		uint32 vI_1 = _faces[f * 3 + 0];
		glVertex3f(_vertices[vI_1 * 3], _vertices[vI_1 * 3 + 1], _vertices[vI_1 * 3 + 2] + 0.01);
		uint32 vI_2 = _faces[f * 3 + 1];
		glVertex3f(_vertices[vI_2 * 3], _vertices[vI_2 * 3 + 1], _vertices[vI_2 * 3 + 2] + 0.01);
		uint32 vI_3 = _faces[f * 3 + 2];
		glVertex3f(_vertices[vI_3 * 3], _vertices[vI_3 * 3 + 1], _vertices[vI_3 * 3 + 2] + 0.01);
		glEnd();
	}
	glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
	for (uint32 f = 0; f < _facesCount; ++f) {
// 		if (!walkable(f))
// 			continue;
		glBegin(GL_TRIANGLES);
		if (walkable(f)) {
			glColor4f(0.5, 0, 0.5, 0.4);
		} else {
			glEnd();
			continue;
// 			glColor4f(0.1, 0.5, 0.5, 0.4);
		}

		uint32 vI_1 = _faces[f * 3 + 0];
		glVertex3f(_vertices[vI_1 * 3], _vertices[vI_1 * 3 + 1], _vertices[vI_1 * 3 + 2] + 0.04);
		uint32 vI_2 = _faces[f * 3 + 1];
		glVertex3f(_vertices[vI_2 * 3], _vertices[vI_2 * 3 + 1], _vertices[vI_2 * 3 + 2] + 0.04);
		uint32 vI_3 = _faces[f * 3 + 2];
		glVertex3f(_vertices[vI_3 * 3], _vertices[vI_3 * 3 + 1], _vertices[vI_3 * 3 + 2] + 0.04);
		glEnd();
	}

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

// 	float start[3] = { 11.5f, 27.f, -1.27 };
// 	float end[3]   = { 28.828587, 20.333252, -1.27 };

// 	glLineWidth(2.f);
// 	glBegin(GL_LINES);
// 	glColor4f(1.f, 1.0, 1.0, 0.5);
// 	glVertex3f(start[0], start[1], start[2]);
// 	glVertex3f(end[0], end[1], end[2]);
// 	glEnd();
// 	glLineWidth(1.f);

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

bool Pathfinding::walkable(uint32 faceIndex) const {
	if (faceIndex == UINT32_MAX)
		return false;

	// Hardcoded values.
	return _faceProperty[faceIndex] != 2 && _faceProperty[faceIndex] != 7
	&& _faceProperty[faceIndex] != 0 && _faceProperty[faceIndex] != 8;
}

bool Pathfinding::walkable(Common::Vector3 point) {
	uint32 face = findFace(point[0], point[1]);
	if (face == UINT32_MAX)
		warning("face not found");

	return walkable(face);
}

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
            if (onlyWalkable && !walkable(face))
                continue;

            if (!inFace(face, Common::Vector3(x, y, 0.f)))
                continue;

            return face;
        }
    }

    return UINT32_MAX;
}

uint32 Pathfinding::findFace(float x1, float y1, float z1, float x2, float y2, float z2, Common::Vector3 &intersect) {
// 	warning("Looking for a face");
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
// 			warning("face found : %u", face);
			if (_adjFaces[face * _polygonEdges] != UINT32_MAX)
				_facesToDraw.push_back(_adjFaces[face * 3]);
			if (_adjFaces[face * _polygonEdges + 1] != UINT32_MAX)
				_facesToDraw.push_back(_adjFaces[face * 3 + 1]);
			if (_adjFaces[face * _polygonEdges + 2] != UINT32_MAX)
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
	if (xyLength(vec2, vec1) > width)
		return true;


	// Test maximum three different cases:
	//  * A segment at the center of the shared side,
	//  * A segment that starts at the right vertex and go to the left,
	//  * A segment that starts at the left vertex and go to the right.
	// Obviously, it doesn't account all the possibilities, the test segment could be
	// a little bit to the left and also to the right at same time though it should
	// avoid all true negatives. Also it could be faster to test the first test at
	// the last position, it should be tested.
// 	warning("want to go there");
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

// 	warning("check (%u -> %u) neighbour %i, %i, %i", fromFace, toFace, test1, test2, test3);

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
	Common::Vector3 vert[_polygonEdges];
	getVertices(face, vert[0], vert[1], vert[2]);

	for (uint8 i = 0; i < _polygonEdges; ++i) {
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
	for (uint8 i = 0; i < _polygonEdges; ++i) {
		if (_adjFaces[face1 * _polygonEdges + i] == face2) {
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

float Pathfinding::xyLength(Common::Vector3 &vec) const {
	return sqrt(pow(vec._x, 2.f) + pow(vec._y, 2.f));
}

float Pathfinding::xyLength(Common::Vector3 &vecA, Common::Vector3 &vecB) const {
	Common::Vector3 vecDiff = vecB - vecA;
	return xyLength(vecDiff);
}

void Pathfinding::setAStarAlgorithm(AStar *aStarAlgorithm) {
	_aStarAlgorithm = aStarAlgorithm;
}

} // namespace Engines
