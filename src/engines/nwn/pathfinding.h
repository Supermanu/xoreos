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

#ifndef ENGINES_NWN_PATHFINDING_H
#define ENGINES_NWN_PATHFINDING_H

#include "src/engines/aurora/pathfinding.h"

namespace Common {
class AABBNode;
class SeekableReadStream;
class StreamTokenizer;
}

namespace Engines {

namespace NWN {

class Pathfinding : public Engines::Pathfinding {
public:
	Pathfinding(std::vector<bool> walkableProperties);
	~Pathfinding();

	void addData(const Common::UString &wokFile, uint8 orientation, float *position);
	void finalize();
	bool walkable(uint32 face) const;

private:
	struct Face {
		uint32 faceId;
		uint32 adjacentTile;
		uint32 adjacentFace;
		Common::Vector3 vert[3];
		bool yAxis;
		float axisPosition;
		float epsilon;
		std::vector<uint8> axisVert;
		uint8 oppositeVert;
		uint8 minVert;
		uint8 maxVert;
		float min;
		float max;

		Face(): adjacentTile(UINT32_MAX), adjacentFace(UINT32_MAX) {};
		void computeMinOnAxis();
		bool operator<(const Face &face) const;
	};

	struct Tile {
		uint32 tileId;
		uint32 xPosition;
		uint32 yPosition;
		std::vector<uint32> faces;
		std::vector<uint32> adjFaces;
		std::vector<uint32> facesProperty;
		std::vector<Face> borderBottom;
		std::vector<Face> borderRight;
		std::vector<Face> borderLeft;
		std::vector<Face> borderTop;
	};

	void readVerts(size_t n, float *position, Common::SeekableReadStream *stream, Common::StreamTokenizer *tokenize, uint8 orientation);
	void readFaces(size_t n, Common::SeekableReadStream *stream, Common::StreamTokenizer *tokenize);
	void readFloats(const std::vector<Common::UString> &strings,
	                float *floats, uint32 n, uint32 start);
	Common::AABBNode *readAABB(float *position, uint8 orientation, Common::SeekableReadStream *stream, Common::StreamTokenizer *tokenize);
	void changeOrientation(uint8 orientation, float *position);
	void connectTiles(uint32 tileA, uint32 tileB, bool yAxis, float axisPosition);
	void connectInnerFaces(uint32 tile);
	uint32 getAdjPosition(uint32 vertA, uint32 vertB) const;
	void getBorderface(std::vector<Face> &border, uint32 tile, bool yAxis, float axisPosition, float epsilon);
	void getMinMaxFromFace(Face &face, float min[], float max[]);

	bool walkable(uint32 tile, uint32 face) const;

// 	std::vector<uint32> _startFace;
	std::vector<uint32> _startVertex;
	std::vector<Tile> _tiles;
};

}

}

#endif // ENGINES_NWN_PATHFINDING_H
