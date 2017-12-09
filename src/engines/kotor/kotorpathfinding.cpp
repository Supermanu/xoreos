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

#include "src/engines/kotor/kotorpathfinding.h"


namespace Engines {

namespace KotOR {

KotORPathfinding::KotORPathfinding(std::vector<bool> walkableProperties) : Pathfinding(walkableProperties) {
}

KotORPathfinding::~KotORPathfinding() {
}

void KotORPathfinding::addData(const Common::UString &wokFile) {
	warning("Reading %s.wok", wokFile.c_str());
	Common::SeekableReadStream *stream = ResMan.getResource(wokFile, ::Aurora::kFileTypeWOK);

	if (!stream)
		throw Common::Exception("No such WOK \"%s\"", wokFile.c_str());

	stream->seek(0);
	stream->skip(8); // Text: WokVersion
	stream->readUint32LE(); // Walkmesh type
	stream->skip(48); // Reserved

//	_stream->skip(32 + 32 + 32); // Skip position.
	stream->readIEEEFloatLE();
	stream->readIEEEFloatLE();
	stream->readIEEEFloatLE();


	uint32 verticesCount = stream->readUint32LE();
	warning("Vertices count: %u", verticesCount);
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
	uint32 normalizedInvertedNormalsOffset = stream->readUint32LE();
	uint32 facePlanesCoefficientOffset = stream->readUint32LE();
	uint32 AABBsCount = stream->readUint32LE();
	uint32 AABBsOffset = stream->readUint32LE();
	stream->skip(4); // Unknown
	uint32 walkableFacesAdjacencyEdgesCount = stream->readUint32LE();
	uint32 walkableFacesAdjacencyEdgesOffset = stream->readUint32LE();
	uint32 perimetricEdgesCount = stream->readUint32LE();
	uint32 perimetricEdgesOffset = stream->readUint32LE();
	uint32 perimetricCount = stream->readUint32LE();
	uint32 perimetricOffset = stream->readUint32LE();
	
	// Vertices
	stream->seek(verticesOffset);
	_vertices.resize(3 * (_verticesCount + verticesCount));

	for (uint32 v = _verticesCount; v < _verticesCount + verticesCount; ++v) {
		for (uint32 i = 0; i < 3; ++i)
			_vertices[3 * v + i] = stream->readIEEEFloatLE();

// 		warning("Vert %u: (%f, %f, %f)", v, _vertices[3 * v], _vertices[3 * v + 1], _vertices[3 * v + 2]);
	}
	_verticesCount += verticesCount;

	// Faces
	_startFace.push_back(_facesCount);
// 	warning("Current room: %lu", _startFace.size() - 1);
	stream->seek(facesOffset);
	_faces.resize(3 * (_facesCount + facesCount));

	for (uint32 f = _facesCount; f < _facesCount + facesCount; ++f) {
		for (uint32 i = 0; i < 3; ++i)
			_faces[3 * f + i] = stream->readUint32LE() + _verticesCount - verticesCount;

// 		warning("Face %u: (%i, %i, %i)", f, _faces[3 * f], _faces[3 * f + 1], _faces[3 * f + 2]);
	}

	// Walkmesh type
	_faceProperty.resize(_facesCount + facesCount);
	stream->seek(walktypesOffset);
	std::vector<uint32> walkableFaces;
	for (uint32 w = _facesCount; w < _facesCount + facesCount; ++w) {
		_faceProperty[w] = stream->readUint32LE();

		// A map betwent walkable faces and actual faces.
		if (_faceProperty[w] != 2 && _faceProperty[w] != 7)
			walkableFaces.push_back(w);

// 		warning("Walktype %u: %u", w, _faceProperty[w]);
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

// 		warning("AdjFace %u: (%i, %i, %i)", walkableFaces[a], _adjFaces[walkableFaces[a] * 3], _adjFaces[walkableFaces[a] * 3 + 1], _adjFaces[walkableFaces[a] * 3 + 2]);
	}

	// Perimetric edges
	stream->seek(perimetricEdgesOffset);
	std::map<uint32, uint32> adjRooms;
	for (uint32 pe = 0; pe < perimetricEdgesCount; ++pe) {
		uint32 perimetricEdge = stream->readUint32LE();
		adjRooms[perimetricEdge] = stream->readUint32LE();

// 		warning("Perimetric edge %u: (%u, %i)", pe, perimetricEdge, adjRooms[perimetricEdge]);
	}
	_adjRooms.push_back(adjRooms);

	_facesCount += facesCount;

	// AABB tree.
	Common::AABBNode * rootNode = getAABB(stream, AABBsOffset, AABBsOffset);
	_AABBTrees.push_back(rootNode);

//	// Perimetric
//	_stream->seek(perimetricOffset);
//	for (uint32 p = 0; p < perimetricCount; ++p) {
//		uint32 periOffset = _stream->readUint32LE();
//		warning("Perimetric %u: %u", p, periOffset);
//	}
}

void KotORPathfinding::finalize() {
	for (uint32 r = 0; r < _adjRooms.size(); ++r) {
// 		warning("Looking for perimetric edge in room %u", r);
		for (std::map<uint32, uint32>::iterator ar = _adjRooms[r].begin(); ar != _adjRooms[r].end(); ++ar) {
			if (ar->second == UINT32_MAX)
				continue;

			uint32 currFace = getFaceFromEdge(ar->first, r);
// 			warning("Perimetric face found: %u", currFace);
			// Get adjacent face from the other room.
			uint32 otherRoom = ar->second;
// 			warning("Looking for adjacent face in room: %u", otherRoom);
			for (std::map<uint32, uint32>::iterator oF = _adjRooms[otherRoom].begin(); oF != _adjRooms[otherRoom].end(); ++oF) {
				// Get only faces adjacent to the current room (r).
				if (oF->second != r)
					continue;

				uint32 otherFace = getFaceFromEdge(oF->first, otherRoom);
// 				warning("Face next to original face found: %u", otherFace);
				// Check if at least two vertices are the same.
				Common::Vector3 cVertA, cVertB, cVertC, oVertA, oVertB, oVertC;
				Common::Vector3 currVerts[3] = { cVertA, cVertB, cVertC };
				Common::Vector3 othVerts[3]  = { oVertA, oVertB, oVertC };
				getVertices(currFace, currVerts[0], currVerts[1], currVerts[2]);
				getVertices(otherFace, othVerts[0], othVerts[1], othVerts[2]);

				uint32 cEdge = ar->first % 3;
				uint32 oEdge = oF->first % 3;
				Common::Vector3 &cVert1 = currVerts[cEdge];
				Common::Vector3 &cVert2 = currVerts[(cEdge + 1) % 3];
				Common::Vector3 &oVert1 = othVerts[oEdge];
				Common::Vector3 &oVert2 = othVerts[(oEdge + 1) % 3];

// 				warning("Original vertices: (%f, %f, %f), (%f, %f, %f), (%f, %f, %f)", currVerts[0]._x, currVerts[0]._y, currVerts[0]._z, currVerts[1]._x, currVerts[1]._y, currVerts[1]._z,  currVerts[2]._x, currVerts[2]._y, currVerts[2]._z);
// 				warning("Other vertices: (%f, %f, %f), (%f, %f, %f), (%f, %f, %f)", othVerts[0]._x, othVerts[0]._y, othVerts[0]._z, othVerts[1]._x, othVerts[1]._y, othVerts[1]._z,  othVerts[2]._x, othVerts[2]._y, othVerts[2]._z);


				if ((cVert1._x == oVert1._x && cVert2._x == oVert2._x && cVert1._y == oVert1._y && cVert2._y == oVert2._y)
					|| (cVert1._x == oVert2._x && cVert2._x == oVert1._x && cVert1._y == oVert2._y && cVert2._y == oVert1._y)) {

					_adjFaces[currFace * 3 + cEdge % 3] = otherFace;
					_adjFaces[otherFace * 3 + oEdge % 3] = currFace;

					warning("Adjacent face to %u: %u", currFace, otherFace);
// 					// Remove the other edge.
// 					_adjRooms[otherRoom].erase(oF);
					break;
				}
			}
		}
	}
}

uint32 KotORPathfinding::getFaceFromEdge(uint32 edge, uint32 room) const {
	if (edge == UINT32_MAX)
		error("Edge is not valid");

	return (edge + (2 - edge % 3)) / 3 +  _startFace[room];
}

Common::AABBNode *KotORPathfinding::getAABB(Common::SeekableReadStream *stream, uint32 nodeOffset, uint32 AABBsOffset) {
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

// 	warning("AABB node : min(%f, %f, %f) max(%f, %f, %f)", min[0], min[1], min[2], max[0], max[1], max[2]);
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
