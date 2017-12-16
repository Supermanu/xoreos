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
 *  
 */

#ifndef ENGINES_KOTOR_PATHFINDING_H
#define ENGINES_KOTOR_PATHFINDING_H

#include <vector>

#include "src/engines/aurora/pathfinding.h"

namespace Common {
class AABBNode;
class SeekableReadStream;
}

namespace Engines {

namespace KotOR {

class Pathfinding : public Engines::Pathfinding {
public:
	/** Construct a pathfinding object for KotOR. */
	Pathfinding(std::vector<bool> walkableProperties);
	~Pathfinding();

	/** Add a wok data for a room. */
	void addData(const Common::UString &wokFile);
	/** Set adjacencies between all faces of the walkmesh. */
	void finalize();

private:
	/** Get face index from an adjacency position in the adjacency vector. */
	uint32 getFaceFromEdge(uint32 edge, uint32 room) const;
	/** Read AABB data from a stream (the wok file). */
	Common::AABBNode *getAABB(Common::SeekableReadStream *stream, uint32 nodeOffset, uint32 AABBsOffset);

	std::vector<std::map<uint32, uint32> > _adjRooms; ///< Adjacency between rooms.
	std::vector<uint32> _startFace;                   ///< Cumulative face count for rooms.
};

} // namespace KotOR

} // namespace Engines

#endif // ENGINES_KOTOR_PATHFINDING_H
