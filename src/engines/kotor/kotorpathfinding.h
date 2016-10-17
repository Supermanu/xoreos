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

#ifndef ENGINES_KOTOR_KOTORPATHFINDING_H
#define ENGINES_KOTOR_KOTORPATHFINDING_H

#include <vector>

#include "src/engines/aurora/pathfinding.h"

namespace Common {
class AABBNode;
class SeekableReadStream;
}

namespace Engines {

namespace KotOR {

class KotORPathfinding : public Pathfinding {
public:
	KotORPathfinding();
	~KotORPathfinding();

	void addData(const Common::UString &wokFile);
	void finalize();

private:
	uint32 getFaceFromEdge(uint32 edge, uint32 room) const;
	Common::AABBNode *getAABB(Common::SeekableReadStream *stream, uint32 nodeOffset, uint32 AABBsOffset);

	std::vector<std::map<uint32, uint32> > _adjRooms;
	std::vector<uint32> _startFace;
};

} // namespace KotOR

} // namespace Engines

#endif // ENGINES_KOTOR_KOTORPATHFINDING_H
