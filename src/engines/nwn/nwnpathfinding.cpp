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

#include "src/common/ustring.h"
#include "src/common/streamtokenizer.h"
#include "src/common/strutil.h"
#include"src/common/aabbnode.h"

#include "src/aurora/resman.h"

#include "src/engines/nwn/nwnpathfinding.h"

namespace Engines {

namespace NWN {

NWNPathfinding::NWNPathfinding() : Pathfinding() {
}

NWNPathfinding::~NWNPathfinding() {
}

void NWNPathfinding::addData(const Common::UString &wokFile, uint8 orientation, float *position) {
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
			changeOrientation(orientation, localPosition);
			for (uint8 i = 0; i < 3; ++i)
				position[i] += localPosition[i];
			warning("Position: (%f, %f, %f)", position[0], position[1], position[2]);
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
}

Common::AABBNode *NWNPathfinding::readAABB(float *position, uint8 orientation, Common::SeekableReadStream *stream, Common::StreamTokenizer *tokenize) {
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

	int32 face;
	Common::parseString(line[6], face);
	if (face >= 0)
		face += _startFace.back();

	warning("AABB: min(%f, %f, %f), max(%f, %f, %f), face: %i", min[0], min[1], min[2], max[0], max[1], max[2], face);

	Common::AABBNode *node = new Common::AABBNode(min, max, face);
	if (face < 0) {
		Common::AABBNode *leftChild = readAABB(position, orientation, stream, tokenize);
		Common::AABBNode *rightChild = readAABB(position, orientation, stream, tokenize);
		node->setChildren(leftChild, rightChild);
	}

	return node;
}

void NWNPathfinding::finalize() {
	_adjFaces.resize(_faces.size());

	_startFace.push_back(_facesCount);

	for (uint32 t = 0; t < _startFace.size() - 1; ++t) {
		for (size_t f = _startFace[t]; f < _startFace[t + 1]; ++f) {
			for (uint32 i = _startFace[t]; i < _startFace[t + 1]; ++i) {
				if (f == i)
					continue;

				// Avoid unwalkable face
				if (!walkable(f) || !walkable(i))
					continue;

				uint8 count = 0;
				std::vector<uint8> edges;
				for (uint32 c = 0; c < 3; ++c) {
					if (_faces[f * 3] == _faces[i * 3 + c]) {
						++count;
						edges.push_back(0);
					} else if (_faces[f * 3 + 1] == _faces[i * 3 + c]) {
						++count;
						edges.push_back(1);
					} else if (_faces[f * 3 + 2] == _faces[i * 3 + c]) {
						++count;
						edges.push_back(2);
					}
				}

				if (count > 1) {
					if ((edges[0] == 0 && edges[1] == 1) || (edges[1] == 0 && edges[0] == 1)) {
						_adjFaces[f * 3] = i;
					} else if ((edges[0] == 1 && edges[1] == 2) || (edges[1] == 1 && edges[0] == 2)) {
						_adjFaces[f * 3 + 1] = i;
					} else if ((edges[0] == 0 && edges[1] == 2) || (edges[1] == 0 && edges[0] == 2) ) {
						_adjFaces[f * 3 + 2] = i;
					}
				}

			}
			warning("For face %u, adj (%i, %i, %i)", f, _adjFaces[f * 3], _adjFaces[f * 3 + 1], _adjFaces[f * 3 + 2]);
		}
	}
}

void NWNPathfinding::readFloats(const std::vector<Common::UString> &strings,
                                float *floats, uint32 n, uint32 start) {

	if (strings.size() < (start + n))
		throw Common::Exception("Missing tokens");

	for (uint32 i = 0; i < n; i++)
		Common::parseString(strings[start + i], floats[i]);
}


void NWNPathfinding::readVerts(size_t n, float *position, Common::SeekableReadStream *stream,
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

		warning("v(%u): %f, %f, %f", i, _vertices[3 * i], _vertices[3 * i + 1], _vertices[3 * i + 2]);
	}

	// Update total vertices count.
	_verticesCount += n;
}

void NWNPathfinding::readFaces(size_t n, Common::SeekableReadStream *stream, Common::StreamTokenizer *tokenize) {
	_faces.resize(3 * (_facesCount + n));
	_adjFaces.resize(3 * (_facesCount + n));
	_faceProperty.resize((_facesCount + n));

	_startFace.push_back(_facesCount);

	for (size_t i = _facesCount; i < _facesCount + n; ++i) {
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
			_faces[3 * i + vi] = val + _startVertex[_startFace.size() - 1];
			_adjFaces[3 * i + vi] = UINT32_MAX;
		}

		// Walktype
		Common::parseString(line[7], _faceProperty[i]);

		warning("f(%u): %i, %i, %i with mat: %i", i,  _faces[3 * i + 0], _faces[3 * i + 1], _faces[3 * i + 2], _faceProperty[i]);
	}

	_facesCount += n;
}

void NWNPathfinding::changeOrientation(uint8 orientation, float *position) {
	for (uint8 o = 0; o < orientation; ++o) {
		float temp = position[0];
		position[0] = - position[1];
		position[1] = temp;
	}
}

} // namespace NWN

} // namespace Engines
