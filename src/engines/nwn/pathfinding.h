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
	/** Construct a pathfinding object for nwn. */
	Pathfinding(std::vector<bool> walkableProperties);
	~Pathfinding();

	/** Add wok tile data. */
	void addData(const Common::UString &wokFile, uint8 orientation, float *position);
	/** Connect all tiles together. Should be called before any path request. */
	void finalize();

private:
	/** Informations about adjacency and the position along the border of the tile. */
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

		Face();
		void computeMinOnAxis();
		bool operator<(const Face &face) const;
	};

	/** Structure used to connect the tiles together. */
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

	/** Read the vertices positions from an ASCII stream. */
	void readVerts(size_t n, float *position, Common::SeekableReadStream *stream,
	               Common::StreamTokenizer *tokenize, uint8 orientation);
	/** Read the faces vertices from an ASCII stream. */
	void readFaces(size_t n, Common::SeekableReadStream *stream, Common::StreamTokenizer *tokenize);
	/** Read floats number from lines of Common::UString. */
	void readFloats(const std::vector<Common::UString> &strings,
	                float *floats, uint32 n, uint32 start);
	/** Read and construct an AABB tree/node from an ASCII stream. */
	Common::AABBNode *readAABB(float *position, uint8 orientation, Common::SeekableReadStream *stream,
	                           Common::StreamTokenizer *tokenize);
	/** Change the axis orientation of coordonates. */
	void changeOrientation(uint8 orientation, float *position) const;
	/** Find face adjacencies between two tiles and make all border faces match an other face. */
	void connectTiles(uint32 tileA, uint32 tileB, bool yAxis, float axisPosition);
	/** Find face adjacencies within a tile. */
	void connectInnerFaces(uint32 tile);
	/** Get the position in the adjacent face vector from the vertices positions. */
	uint32 getAdjPosition(uint32 vertA, uint32 vertB) const;
	/** Find the faces on the border of a tile. */
	void getBorderface(std::vector<Face> &border, uint32 tile, bool yAxis, float axisPosition, float epsilon) const;
	/** Find out the order of the vertices of a face along the border of a tile. */
	void getMinMaxFromFace(Face &face, float min[], float max[]) const;
	/** Is a face, in a given tile, walkable? */
	bool faceInTileWalkable(uint32 tile, uint32 face) const;

	std::vector<uint32> _startVertex; ///< Starting index of the vertex for each tiles.
	std::vector<Tile> _tiles;         ///< Tiles of the area.
};

}

}

#endif // ENGINES_NWN_PATHFINDING_H
