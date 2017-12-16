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
 *
 */

/** @file
 *  Pathfinding for NWN.
 */


#include <boost/geometry.hpp>
#include <boost/geometry/geometries/point_xy.hpp>
#include <boost/geometry/geometries/polygon.hpp>
#include <boost/geometry/algorithms/centroid.hpp>

#include <algorithm>

#include "src/common/ustring.h"
#include "src/common/streamtokenizer.h"
#include "src/common/strutil.h"
#include"src/common/aabbnode.h"

#include "src/aurora/resman.h"

#include "src/engines/aurora/astaralgorithm.h"
#include "src/engines/nwn/pathfinding.h"

typedef boost::geometry::model::d2::point_xy<float> boostPoint2d;

namespace Engines {

namespace NWN {

Pathfinding::Pathfinding(std::vector<bool> walkableProperties) : Engines::Pathfinding(walkableProperties) {
	AStar * aStarAlgorithm = new AStar(this);
	setAStarAlgorithm(aStarAlgorithm);
}

Pathfinding::~Pathfinding() {
}

void Pathfinding::addData(const Common::UString &wokFile, uint8 orientation, float *position) {
	Common::SeekableReadStream *stream = ResMan.getResource(wokFile, ::Aurora::kFileTypeWOK);
	warning("loading wok:  %s.wok", wokFile.c_str());
	if (!stream)
		throw Common::Exception("No such WOK \"%s\"", wokFile.c_str());

	stream->seek(0);

	Common::StreamTokenizer *tokenize = new Common::StreamTokenizer(Common::StreamTokenizer::kRuleIgnoreAll);
	tokenize->addSeparator(' ');
	tokenize->addChunkEnd('\n');
	tokenize->addIgnore('\r');

	float localPosition[3];

	Tile tile = Tile();
	tile.tileId = _tiles.size();
	_tiles.push_back(tile);
	while (!stream->eos()) {
		std::vector<Common::UString> line;
		size_t count = tokenize->getTokens(*stream, line, 3);
		tokenize->nextChunk(*stream);
		// Ignore empty lines and comments
		if ((count == 0) || line[0].empty() || (*line[0].begin() == '#'))
			continue;

		line[0].makeLower();

		if (line[0] == "position") {
			readFloats(line, localPosition, 3, 1);
// 			warning("Position: (%f, %f, %f)", position[0], position[1], position[2]);
		} else if (line[0] == "orientation") {
			float ori[4];
			readFloats(line, ori, 4, 1);
// 			warning("Ori: (%f, %f, %f, %f)", ori[0], ori[1], ori[2], ori[3]);
			orientation += (int8) roundf((2 * M_PI) / ori[3]) + 4;
			orientation %= 4;

			// Update position orientation
			changeOrientation(orientation, localPosition);
			for (uint8 i = 0; i < 3; ++i)
				position[i] += localPosition[i];
		} else if (line[0] == "verts") {
			size_t vertsCount;
			Common::parseString(line[1], vertsCount);
			readVerts(vertsCount, position, stream, tokenize, orientation);
		} else if (line[0] == "faces") {
			size_t facesCount;
			Common::parseString(line[1], facesCount);
			readFaces(facesCount, stream, tokenize);
		} else if (line[0] == "aabb") {
			float raw_min[3], raw_max[3], min[3], max[3];
			readFloats(line, raw_min, 3, 1);
			readFloats(line, raw_max, 3, 4);

			changeOrientation(orientation, raw_min);
			changeOrientation(orientation, raw_max);
			for (uint8 i = 0; i < 3; ++i) {
				raw_min[i] += position[i];
				raw_max[i] += position[i];
				min[i] = MIN(raw_min[i], raw_max[i]);
				max[i] = MAX(raw_min[i], raw_max[i]);
			}

			Common::AABBNode *rootNode = new Common::AABBNode(min, max);
// 			warning("AABB root: min(%f, %f, %f), max(%f, %f, %f)", min[0], min[1], min[2], max[0], max[1], max[2]);

			Common::AABBNode *leftChild = readAABB(position, orientation, stream, tokenize);
			Common::AABBNode *rightChild = readAABB(position, orientation, stream, tokenize);
			rootNode->setChildren(leftChild, rightChild);

			_AABBTrees.push_back(rootNode);
		} else if (line[0] == "endwalkmeshgeom") {
			break;
		}
	}

	// Find Adjacent tiles
// 	warning("Finding adjacents tiles...");
// 	warning("Current position (%f, %f, %f)", position[0], position[1], position[2]);
	float epsilon = 0.001;
	Common::Vector3 leftMax(position[0] - 5.f, position[1] + 5.f, 0.f);
	Common::Vector3 topMin(leftMax);
	Common::Vector3 bottomMax(position[0] + 5.f, position[1] - 5.f, 0.f);
	Common::Vector3 rightMin(bottomMax);

	for (uint32 n = 0; n < _AABBTrees.size(); ++n) {
		float x, y, z;
		_AABBTrees[n]->getMax(x, y, z);
// 		warning("Tile max (%f, %f, %f)", x, y, z);
		if (fabs(x - leftMax._x) < epsilon && fabs(y - leftMax._y) < epsilon) {
// 			warning("left tile found");
			connectTiles(n, _AABBTrees.size() - 1, false, x);
			continue;
		}
		if (fabs(x - bottomMax._x) < epsilon && fabs(y - bottomMax._y) < epsilon) {
// 			warning("bottom tile found");
			connectTiles(n, _AABBTrees.size() - 1, true, y);
			continue;
		}
	}

// 	connectInnerFaces(_tiles.size() - 1);
}

Common::AABBNode *Pathfinding::readAABB(float *position, uint8 orientation, Common::SeekableReadStream *stream, Common::StreamTokenizer *tokenize) {
	std::vector<Common::UString> line;
	tokenize->getTokens(*stream, line, 7);
	tokenize->nextChunk(*stream);

	float raw_min[3], raw_max[3], min[3], max[3];
	readFloats(line, raw_min, 3, 0);
	readFloats(line, raw_max, 3, 3);

	changeOrientation(orientation, raw_min);
	changeOrientation(orientation, raw_max);
	for (uint8 i = 0; i < 3; ++i) {
		raw_min[i] += position[i];
		raw_max[i] += position[i];
		min[i] = MIN(raw_min[i], raw_max[i]);
		max[i] = MAX(raw_min[i], raw_max[i]);
	}

	// If it's a child node, record the related face.
	int32 face;
	Common::parseString(line[6], face);
// 	if (face >= 0)
// 		face += _startFace.back();

// 	warning("AABB: min(%f, %f, %f), max(%f, %f, %f), face: %i", min[0], min[1], min[2], max[0], max[1], max[2], face);

	Common::AABBNode *node = new Common::AABBNode(min, max, face);
	if (face < 0) {
		Common::AABBNode *leftChild = readAABB(position, orientation, stream, tokenize);
		Common::AABBNode *rightChild = readAABB(position, orientation, stream, tokenize);
		node->setChildren(leftChild, rightChild);
	}

	return node;
}

void Pathfinding::finalize() {
	// Merge all faces, adjacency and property included.
	_faces.clear();
	std::vector<uint32> startIndex;

	// Connect faces inside the tile.
	for (uint32 t = 0; t < _tiles.size(); ++t)
		connectInnerFaces(t);

// 	warning("Describing tiles");
// 	for (uint32 t = 0; t < _tiles.size(); ++t) {
// 		warning("looking at tile %u that contains %zu", t, _tiles[t].faces.size());
// 		for (uint32 it = 0; it < _tiles[t].faces.size(); ++it) {
// 			warning("Adjacency for face %u (%u, %u, %u)", it, _tiles[t].adjFaces[it * 3 + 0], _tiles[t].adjFaces[it * 3 + 1], _tiles[t].adjFaces[it * 3 + 2]);
// 		}
// 	}

	// Adjust face indices.
	for (uint32 t = 0; t < _tiles.size(); ++t) {
		startIndex.push_back(_faceProperty.size());
		uint32 prevFacesCount = startIndex.back();
// 		warning("prevfacecount (tile %u): %u", prevFacesCount, t);

		// Append faces from the tiles.
		_faces.insert(_faces.end(), _tiles[t].faces.begin(), _tiles[t].faces.end());

		// Append adjacency. Adjust face index.
		_adjFaces.resize(_faces.size());
		for (uint32 aF = 0; aF < _tiles[t].adjFaces.size(); ++aF) {
			uint32 adjFaceTile = _tiles[t].adjFaces[aF];
			if (adjFaceTile != UINT32_MAX)
				_adjFaces[prevFacesCount * 3 + aF] = prevFacesCount + adjFaceTile;
		}

		// Append face property.
		_faceProperty.insert(_faceProperty.end(), _tiles[t].facesProperty.begin(), _tiles[t].facesProperty.end());

		// Adjust AABB.
		_AABBTrees[t]->adjustChildrenProperty(prevFacesCount);
	}

	// Set adjacency between tiles.
	for (uint32 t = 0; t < _tiles.size(); ++t) {
		for (uint32 f = 0; f < _tiles[t].borderBottom.size(); ++f) {
			// Check if there is an adjacent face.
			if (_tiles[t].borderBottom[f].adjacentFace == UINT32_MAX)
				continue;

			// Set adjacent face.
			Face &face = _tiles[t].borderBottom[f];
			uint32 faceID = face.faceId + startIndex[t];
			uint32 adjacentFaceID = face.adjacentFace + startIndex[face.adjacentTile];
			_adjFaces[faceID * 3 + getAdjPosition(face.minVert, face.maxVert)] = adjacentFaceID;

			// Do the same for the opposite face.
			Face &oppositeFace = _tiles[face.adjacentTile].borderTop[f];
			_adjFaces[adjacentFaceID * 3 + getAdjPosition(oppositeFace.minVert, oppositeFace.maxVert)] = faceID;
		}

		for (uint32 f = 0; f < _tiles[t].borderLeft.size(); ++f) {
			// Check if there is an adjacent face.
			if (_tiles[t].borderLeft[f].adjacentFace == UINT32_MAX)
				continue;

			// Set adjacent face.
			Face &face = _tiles[t].borderLeft[f];
			uint32 faceID = face.faceId + startIndex[t];
			uint32 adjacentFaceID = face.adjacentFace + startIndex[face.adjacentTile];
			_adjFaces[faceID * 3 + getAdjPosition(face.minVert, face.maxVert)] = adjacentFaceID;

			// Do the same for the opposite face.
			Face &oppositeFace = _tiles[face.adjacentTile].borderRight[f];
			_adjFaces[adjacentFaceID * 3 + getAdjPosition(oppositeFace.minVert, oppositeFace.maxVert)] = faceID;
		}
	}

}

void Pathfinding::readFloats(const std::vector<Common::UString> &strings,
                                float *floats, uint32 n, uint32 start) {

	if (strings.size() < (start + n))
		throw Common::Exception("Missing tokens");

	for (uint32 i = 0; i < n; i++)
		Common::parseString(strings[start + i], floats[i]);
}


void Pathfinding::readVerts(size_t n, float *position, Common::SeekableReadStream *stream,
							   Common::StreamTokenizer *tokenize, uint8 orientation) {

	_vertices.resize(3 * (_verticesCount + n));

	_startVertex.push_back(_verticesCount);

	for (size_t i = _verticesCount; i < _verticesCount + n; ++i) {
		std::vector<Common::UString> line;
		size_t count = tokenize->getTokens(*stream, line, 3);
		tokenize->nextChunk(*stream);

		// Ignore empty lines and comments
		if ((count == 0) || line[0].empty() || (*line[0].begin() == '#')) {
			--i;
			continue;
		}

		for (uint32 vi = 0; vi < 3; ++vi) {
			float val;
			Common::parseString(line[vi], val);
			_vertices[3 * i + vi] = val;
		}
		changeOrientation(orientation, &_vertices[3 * i]);
		for (uint32 vi = 0; vi < 3; ++vi)
			_vertices[3 * i + vi] += position[vi];

// 		warning("v(%u): %f, %f, %f", i, _vertices[3 * i], _vertices[3 * i + 1], _vertices[3 * i + 2]);
	}

	// Update total vertices count.
	_verticesCount += n;
}

void Pathfinding::readFaces(size_t n, Common::SeekableReadStream *stream, Common::StreamTokenizer *tokenize) {
// 	_faces.resize(3 * (_facesCount + n));
// 	_adjFaces.resize(3 * (_facesCount + n));
// 	_faceProperty.resize((_facesCount + n));
	_tiles.back().faces.resize(n * 3);
	_tiles.back().adjFaces.resize(n * 3);
	_tiles.back().facesProperty.resize(n);

// 	_startFace.push_back(_facesCount);

	for (size_t i = 0; i < n; ++i) {
		std::vector<Common::UString> line;
		size_t count = tokenize->getTokens(*stream, line, 3);
		tokenize->nextChunk(*stream);

		// Ignore empty lines and comments
		if ((count == 0) || line[0].empty() || (*line[0].begin() == '#')) {
			--i;
			continue;
		}

		for (uint32 vi = 0; vi < 3; ++vi) {
			float val;
			Common::parseString(line[vi], val);
			_tiles.back().faces[3 * i + vi] = val + _startVertex[_tiles.size() - 1];;
			_tiles.back().adjFaces[3 * i + vi] = UINT32_MAX;
		}

		// Walktype
		Common::parseString(line[7], _tiles.back().facesProperty[i]);

// 		warning("f(%u): %i, %i, %i with mat: %i", i,  _faces[3 * i + 0], _faces[3 * i + 1], _faces[3 * i + 2], _faceProperty[i]);
	}

	_facesCount += n;
}

void Pathfinding::changeOrientation(uint8 orientation, float *position) {
	for (uint8 o = 0; o < orientation; ++o) {
		float temp = position[0];
		position[0] = - position[1];
		position[1] = temp;
	}
}

void Pathfinding::connectInnerFaces(uint32 tile) {
	for (uint32 f = 0; f < _tiles[tile].faces.size() / 3; ++f) {
		for (uint32 i = 0; i < _tiles[tile].faces.size() / 3; ++i) {
			if (f == i)
				continue;

			// Avoid unwalkable face
			if (!walkable(tile, f) || !walkable(tile, i))
				continue;

			uint8 count = 0;
			std::vector<uint8> edges;
			for (uint32 c = 0; c < 3; ++c) {
				if (_tiles[tile].faces[f * 3] == _tiles[tile].faces[i * 3 + c]) {
					++count;
					edges.push_back(0);
				} else if (_tiles[tile].faces[f * 3 + 1] == _tiles[tile].faces[i * 3 + c]) {
					++count;
					edges.push_back(1);
				} else if (_tiles[tile].faces[f * 3 + 2] == _tiles[tile].faces[i * 3 + c]) {
					++count;
					edges.push_back(2);
				}
			}

			if (count > 1) {
				if ((edges[0] == 0 && edges[1] == 1) || (edges[1] == 0 && edges[0] == 1)) {
					_tiles[tile].adjFaces[f * 3] = i;
				} else if ((edges[0] == 1 && edges[1] == 2) || (edges[1] == 1 && edges[0] == 2)) {
					_tiles[tile].adjFaces[f * 3 + 1] = i;
				} else if ((edges[0] == 0 && edges[1] == 2) || (edges[1] == 0 && edges[0] == 2) ) {
					_tiles[tile].adjFaces[f * 3 + 2] = i;
				}

// 				if (tile == 15 || tile == 16)
// 					warning("In connectInnerFaces, tile(%u) face(%u) adjacency (%u, %u, %u)", tile, f, _tiles[tile].adjFaces[f * 3], _tiles[tile].adjFaces[f * 3 + 1], _tiles[tile].adjFaces[f * 3 + 2]);
			}

		}
	}
}

void Pathfinding::connectTiles(uint32 tileA, uint32 tileB, bool yAxis, float axisPosition) {
// 	warning("Connecting tiles (%u, %u), yAxis: %i, axisPosition: %f", tileA, tileB,yAxis, axisPosition);
	// First, collect faces on the border of the new tile (tileB) and the other tile (tileA).
	// Second, match them to the (almost) exact faces of the other tile (which should already have collect border faces).
	// Finally, split faces as needed.

	// Collect faces on the border.
	// How close two vertices should be to be stated as the same.
	float epsilon = 0.2;
	// If yAxis is true the border is bottom/top (tileB/tileA), left/right (tileB/tileA) otherwise.
	std::vector<Face> &borderB = yAxis ? _tiles[tileB].borderBottom : _tiles[tileB].borderLeft;
	std::vector<Face> &borderA = yAxis ? _tiles[tileA].borderTop : _tiles[tileA].borderRight;

	getBorderface(borderA, tileA, yAxis, axisPosition, epsilon);
	getBorderface(borderB, tileB, yAxis, axisPosition, epsilon);

// 	warning("Border size, A(%zu), B(%zu)", borderA.size(), borderB.size());

	// Sort faces along the axis.
	std::sort(borderA.begin(), borderA.end());
	std::sort(borderB.begin(), borderB.end());

// 	if (borderA.size() != borderB.size())
// 		warning("Faces from tileA: %zu, faces from tileB: %zu", borderA.size(), borderB.size());

	uint32 posA = 0;
	uint32 posB = 0;
	while (posA < borderA.size() && posB < borderB.size()) {
		// Ensure both faces start (the min) at the same point.
		if (posA == 0 && posB == 0) {
			if (fabs(borderA[posA].min - borderB[posB].min) > epsilon) {
				warning("The left vertex (min) from both faces should be the same");
			}
			// Adjust the vertex.
			uint32 vertexToMove = _tiles[tileB].faces[borderB[posB].faceId * 3 + borderB[posB].minVert];
			for (uint32 v = 0; v < 3; ++v)
				_vertices[vertexToMove * 3 + v] = borderA[posA].vert[borderA[posA].minVert][v];
		}

		// Check if the faces fit.
		if (fabs(borderA[posA].max - borderB[posB].max) < epsilon) {
			// Set adjacency properties.
			borderA[posA].adjacentTile = tileB;
			borderA[posA].adjacentFace = borderB[posB].faceId;
			borderB[posB].adjacentTile = tileA;
			borderB[posB].adjacentFace = borderA[posA].faceId;

			// Adjust vertex.
			uint32 vertexToMove = _tiles[tileB].faces[borderB[posB].faceId * 3 + borderB[posB].maxVert];
			for (uint32 v = 0; v < 3; ++v)
				_vertices[vertexToMove * 3 + v] = borderA[posA].vert[borderA[posA].maxVert][v];

			++posA; ++posB;
		} else {
			bool isBToCut = borderA[posA].max < borderB[posB].max;
			Face &faceToCut = isBToCut ? borderB[posB] : borderA[posA];
			Tile &tileToCut = isBToCut ? _tiles[tileB] : _tiles[tileA];
			Face &faceGood = isBToCut ? borderA[posA] : borderB[posB];
			Tile &tileGood = isBToCut ? _tiles[tileA] : _tiles[tileB];

			// Get center of the face to be cut.
			boostPoint2d v_1(faceToCut.vert[0][0], faceToCut.vert[0][1]);
			boostPoint2d v_2(faceToCut.vert[1][0], faceToCut.vert[1][1]);
			boostPoint2d v_3(faceToCut.vert[2][0], faceToCut.vert[2][1]);
			boost::geometry::model::polygon<boostPoint2d> boostFace;
			boost::geometry::append(boostFace.outer(), v_1);
			boost::geometry::append(boostFace.outer(), v_2);
			boost::geometry::append(boostFace.outer(), v_3);
			boostPoint2d center(0.f, 0.f);
			boost::geometry::centroid(boostFace, center);

			// Create a new face.
			Face newFace = faceToCut;
			newFace.faceId = tileToCut.facesProperty.size();
			newFace.min = faceGood.max;
			newFace.vert[newFace.minVert] = faceGood.vert[faceGood.maxVert];
			// Add vertex indices from the cut face.
			for (uint32 v = 0; v < 3; ++v) {
				tileToCut.faces.push_back(tileToCut.faces[faceToCut.faceId * 3 + v]);
				tileToCut.adjFaces.push_back(UINT32_MAX);
			}
			// Use the max vertex from the good face as the min vertex for the new face.
			tileToCut.faces[newFace.faceId * 3 + newFace.minVert] = tileGood.faces[faceGood.faceId * 3 + faceGood.maxVert];
			// Set face property.
			tileToCut.facesProperty.push_back(tileToCut.facesProperty[faceToCut.faceId]);

			// Change max vertex for the cut face.
			faceToCut.vert[faceToCut.maxVert] = faceGood.vert[faceGood.maxVert];
			faceToCut.max = faceGood.max;
			tileToCut.faces[faceToCut.faceId * 3 + faceToCut.maxVert] = tileGood.faces[faceGood.faceId * 3 + faceGood.maxVert];

			// Set adjacency properties.
			faceGood.adjacentTile = isBToCut ? tileB : tileA;
			faceGood.adjacentFace = faceToCut.faceId;
			faceToCut.adjacentTile = isBToCut ? tileA : tileB;
			faceToCut.adjacentFace = faceGood.faceId;

// 			warning("faceToCutID %u", faceToCut.faceId);
			// Create two new AABB nodes.
			float min[3], max[3];
			getMinMaxFromFace(faceToCut, min, max);
			Common::AABBNode *cutNode = new Common::AABBNode(min, max, faceToCut.faceId);
			getMinMaxFromFace(newFace, min, max);
			Common::AABBNode *newNode = new Common::AABBNode(min, max, newFace.faceId);

			// Get AABB from cut face.
			std::vector<Common::AABBNode *> nodes;
			_AABBTrees[isBToCut ? tileB : tileA]->getNodes(center.x(), center.y(), nodes);
			Common::AABBNode *parentNode = 0;
			for (std::vector<Common::AABBNode *>::iterator it = nodes.begin(); it != nodes.end(); ++it) {
// 				warning("node %i, faceID %u", (*it)->getProperty(), faceToCut.faceId);
				if ((uint32) (*it)->getProperty() == faceToCut.faceId) {
					parentNode = *it;
					break;
				}
			}

			if (!parentNode)
				error("Parent node not found");

			// Set property to -1 i.e. has children.
			parentNode->adjustChildrenProperty(-1 - (int32) faceToCut.faceId);
			parentNode->setChildren(cutNode, newNode);

			// Add the new face to the border after the current one.
			if (isBToCut)
				borderB.insert(borderB.begin() + posB + 1, newFace);
			else
				borderA.insert(borderA.begin() + posA + 1, newFace);

			++posA; ++posB;
		}
	}
// 	if ((tileA == 15 && tileB == 16 ) || (tileA == 16 && tileB == 15)) {
// 		warning("coucou");
// 	}
}

void Pathfinding::Face::computeMinOnAxis() {
	bool onAxis = false;
	for (uint32 v = 0; v < 3; ++v) {
		if (fabs(vert[v][(uint32) yAxis] - axisPosition) < epsilon) {
			axisVert.push_back(v);
			onAxis = true;
		} else {
			oppositeVert = v;
		}
	}

	if (!onAxis) {
		error("No vertex on the axis");
	}

	if (axisVert.size() > 1) {
		if (vert[axisVert[0]][(uint32) !yAxis] < vert[axisVert[1]][(uint32) !yAxis]) {
			minVert = axisVert[0];
			maxVert = axisVert[1];
		} else {
			minVert = axisVert[1];
			maxVert = axisVert[0];
		}
	} else {
		error("At least two vertices should be on the axis");
	}

	min = vert[minVert][(uint32) !yAxis];
	max = vert[maxVert][(uint32) !yAxis];
}

bool Pathfinding::Face::operator<(const Face &face) const {
	return this->min < face.min;
}

uint32 Pathfinding::getAdjPosition(uint32 vertA, uint32 vertB) const {
	uint32 pos = vertA + vertB;
	if (pos == 3) {
		return 1;
	} else if (pos == 1) {
		return 0;
	}
	return pos;
}

void Pathfinding::getBorderface(std::vector<Face> &border, uint32 tile, bool yAxis, float axisPosition, float epsilon) {
// 	warning("Getting border faces on tile %u", tile);
	Common::Vector3 vert[3];
	for (uint32 f = 0; f < _tiles[tile].facesProperty.size(); ++f) {
		// Avoid unwalkable face.
		if (!walkable(tile, f))
			continue;

		getVertex(_tiles[tile].faces[f * 3], vert[0]);
		getVertex(_tiles[tile].faces[f * 3 + 1], vert[1]);
		getVertex(_tiles[tile].faces[f * 3 + 2], vert[2]);
		for (uint32 v = 0; v < 3; ++v) {
			if (fabs(vert[v][(int32) yAxis] - axisPosition) < epsilon && fabs(vert[(v + 1) % 3][(int32) yAxis] - axisPosition) < epsilon) {
				Face face = Face();
				face.faceId = f;
				face.axisPosition = axisPosition;
				face.yAxis = yAxis;
				face.vert[0] = vert[0]; face.vert[1] = vert[1]; face.vert[2] = vert[2];
				face.epsilon = epsilon;
				face.computeMinOnAxis();

				border.push_back(face);

// 				warning("good face %u", f);
				break;
			}
		}
	}
}

void Pathfinding::getMinMaxFromFace(Face &face, float min[], float max[]) {
	min[0] = face.vert[0][0]; min[1] = face.vert[0][1]; min[2] = face.vert[0][2];
	max[0] = face.vert[0][0]; max[1] = face.vert[0][1]; max[2] = face.vert[0][2];

	for (uint32 i = 1; i < 3; ++i) {
		for (uint32 j = 0; j < 2; ++j) {
			if (face.vert[i][j] < min[j])
				min[j] = face.vert[i][j];

			if (face.vert[i][j] > max[j])
				max[j] = face.vert[i][j];
		}
	}
}

bool Pathfinding::walkable(uint32 tile, uint32 face) const {
	const uint32 &property = _tiles[tile].facesProperty[face];
	return property != 0 && property != 2 && property != 7 && property != 8;
}

bool Pathfinding::walkable(uint32 face) const {
	return Engines::Pathfinding::faceWalkable(face);
}


} // namespace NWN

} // namespace Engines
