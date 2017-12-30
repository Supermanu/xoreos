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
 *  Pathfinding for KotOR.
 */

#include "src/common/vector3.h"
#include "src/common/aabbnode.h"

#include "src/aurora/resman.h"

#include "src/engines/aurora/astaralgorithm.h"
#include "src/engines/kotor/pathfinding.h"

namespace Engines {

namespace KotOR {

Pathfinding::Pathfinding(std::vector<bool> walkableProperties) :
    Engines::Pathfinding(walkableProperties, 3) {
	AStar * aStarAlgorithm = new AStar(this);
	setAStarAlgorithm(aStarAlgorithm);
}

Pathfinding::~Pathfinding() {
}

void Pathfinding::addData(const Common::UString &wokFile) {
	Common::SeekableReadStream *stream = ResMan.getResource(wokFile, ::Aurora::kFileTypeWOK);

	if (!stream)
		throw Common::Exception("No such WOK \"%s\"", wokFile.c_str());

	stream->seek(0);
	stream->skip(8); // Text: WokVersion
	stream->readUint32LE(); // Walkmesh type
	stream->skip(48); // Reserved

	// Skip position.
	stream->readIEEEFloatLE();
	stream->readIEEEFloatLE();
	stream->readIEEEFloatLE();


	uint32 verticesCount = stream->readUint32LE();
	// Check if there is walkmesh.
	if (verticesCount == 0) {
		if (_startFace.empty())
			_startFace.push_back(0);
		else
			_startFace.push_back(_startFace.back());

		_adjRooms.push_back(std::map<uint32, uint32>());
		_AABBTrees.push_back(0);
		return;
	}

	uint32 verticesOffset = stream->readUint32LE();
	uint32 facesCount = stream->readUint32LE();
	uint32 facesOffset = stream->readUint32LE();
	uint32 walktypesOffset = stream->readUint32LE();
	stream->readUint32LE(); // normalizedInvertedNormalsOffset.
	stream->readUint32LE(); // facePlanesCoefficientOffset.
	stream->readUint32LE(); // AABBsCount
	uint32 AABBsOffset = stream->readUint32LE();
	stream->skip(4); // Unknown
	uint32 walkableFacesAdjacencyEdgesCount = stream->readUint32LE();
	uint32 walkableFacesAdjacencyEdgesOffset = stream->readUint32LE();
	uint32 perimetricEdgesCount = stream->readUint32LE();
	uint32 perimetricEdgesOffset = stream->readUint32LE();
	stream->readUint32LE(); // perimetricCount
	stream->readUint32LE(); // perimetricOffset

	// Vertices
	stream->seek(verticesOffset);
	_vertices.resize(3 * (_verticesCount + verticesCount));

	for (uint32 v = _verticesCount; v < _verticesCount + verticesCount; ++v) {
		for (uint32 i = 0; i < 3; ++i)
			_vertices[3 * v + i] = stream->readIEEEFloatLE();
	}
	_verticesCount += verticesCount;

	// Faces
	_startFace.push_back(_facesCount);
	stream->seek(facesOffset);
	_faces.resize(3 * (_facesCount + facesCount));

	for (uint32 f = _facesCount; f < _facesCount + facesCount; ++f) {
		for (uint32 i = 0; i < 3; ++i)
			_faces[3 * f + i] = stream->readUint32LE() + _verticesCount - verticesCount;
	}

	// Walkmesh type
	_faceProperty.resize(_facesCount + facesCount);
	stream->seek(walktypesOffset);
	std::vector<uint32> walkableFaces;
	for (uint32 w = _facesCount; w < _facesCount + facesCount; ++w) {
		_faceProperty[w] = stream->readUint32LE();

		// A map between walkable faces and actual faces.
		if (faceWalkable(w))
			walkableFaces.push_back(w);
	}

	// Adjacency
	_adjFaces.resize(3 * (_facesCount + facesCount));
	stream->seek(walkableFacesAdjacencyEdgesOffset);
	// Fill with empty values
	for (uint32 f = _facesCount; f < _facesCount + facesCount; ++f) {
		for (uint32 i = 0; i < 3; ++i)
			_adjFaces[3 * f + i] = UINT32_MAX;
	}

	for (uint32 a = 0; a < walkableFacesAdjacencyEdgesCount; ++a) {
		for (uint32 i = 0; i < 3; ++i) {
			uint32 edge = stream->readSint32LE();
			// Map edge to face.
			if (edge < UINT32_MAX)
				_adjFaces[walkableFaces[a] * 3 + i] = (edge + (2 - edge % 3)) / 3 + _facesCount;
		}
	}

	// Perimetric edges
	stream->seek(perimetricEdgesOffset);
	std::map<uint32, uint32> adjRooms;
	for (uint32 pe = 0; pe < perimetricEdgesCount; ++pe) {
		uint32 perimetricEdge = stream->readUint32LE();
		adjRooms[perimetricEdge] = stream->readUint32LE();
	}
	_adjRooms.push_back(adjRooms);

	_facesCount += facesCount;

	// AABB tree.
	Common::AABBNode * rootNode = getAABB(stream, AABBsOffset, AABBsOffset);
	_AABBTrees.push_back(rootNode);
}

void Pathfinding::finalize() {
	for (size_t r = 0; r < _adjRooms.size(); ++r) {
		for (std::map<uint32, uint32>::iterator ar = _adjRooms[r].begin(); ar != _adjRooms[r].end(); ++ar) {
			if (ar->second == UINT32_MAX)
				continue;

			uint32 currFace = getFaceFromEdge(ar->first, r);
			// Get adjacent face from the other room.
			uint32 otherRoom = ar->second;
			for (std::map<uint32, uint32>::iterator oF = _adjRooms[otherRoom].begin(); oF != _adjRooms[otherRoom].end(); ++oF) {
				// Get only faces adjacent to the current room (r).
				if (oF->second != r)
					continue;

				uint32 otherFace = getFaceFromEdge(oF->first, otherRoom);
				// Check if at least two vertices are the same.
				std::vector<Common::Vector3> currVerts;
				std::vector<Common::Vector3> othVerts;
				getVertices(currFace, currVerts);
				getVertices(otherFace, othVerts);

				uint32 cEdge = ar->first % 3;
				uint32 oEdge = oF->first % 3;
				Common::Vector3 &cVert1 = currVerts[cEdge];
				Common::Vector3 &cVert2 = currVerts[(cEdge + 1) % 3];
				Common::Vector3 &oVert1 = othVerts[oEdge];
				Common::Vector3 &oVert2 = othVerts[(oEdge + 1) % 3];

				if ((cVert1._x == oVert1._x && cVert2._x == oVert2._x && cVert1._y == oVert1._y && cVert2._y == oVert2._y)
					|| (cVert1._x == oVert2._x && cVert2._x == oVert1._x && cVert1._y == oVert2._y && cVert2._y == oVert1._y)) {

					_adjFaces[currFace * 3 + cEdge % 3] = otherFace;
					_adjFaces[otherFace * 3 + oEdge % 3] = currFace;

					break;
				}
			}
		}
	}
}

uint32 Pathfinding::getFaceFromEdge(uint32 edge, uint32 room) const {
	if (edge == UINT32_MAX)
		error("Edge is not valid");

	return (edge + (2 - edge % 3)) / 3 +  _startFace[room];
}

Common::AABBNode *Pathfinding::getAABB(Common::SeekableReadStream *stream, uint32 nodeOffset, uint32 AABBsOffset) {
	stream->seek(nodeOffset);

	float min[3], max[3];
	for (uint8 m = 0; m < 3; ++m)
		min[m] = stream->readIEEEFloatLE();
	for (uint8 m = 0; m < 3; ++m)
		max[m] = stream->readIEEEFloatLE();

	int32 relatedFace = stream->readSint32LE();
	stream->skip(4); // Unknown
	stream->readUint32LE(); // Plane
	uint32 leftOffset = stream->readUint32LE();
	uint32 rightOffset = stream->readUint32LE();

	// Children always come as pair.
	if (relatedFace >= 0)
		return new Common::AABBNode(min, max, relatedFace + _startFace.back());

	// 44 is the size of an AABBNode.
	Common::AABBNode *leftNode = getAABB(stream, leftOffset * 44 + AABBsOffset, AABBsOffset);
	Common::AABBNode *rightNode = getAABB(stream, rightOffset * 44 + AABBsOffset, AABBsOffset);
	Common::AABBNode *aabb = new Common::AABBNode(min, max);
	aabb->setChildren(leftNode, rightNode);

	return aabb;
}

} // namespace KotOR

} // namespace Engines
