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
 *  A* algorithm is used to find paths as fast as possible and as short as possible.
 */

#ifndef ENGINES_ASTARALGORITHM_H
#define ENGINES_ASTARALGORITHM_H

#include <vector>

#include "src/common/types.h"

namespace Engines {

class Pathfinding;

class AStar {
public:
	AStar(Pathfinding *pathfinding, uint32 polygonEdgesCount = 3);
	~AStar();
	/** Find a path of faces in the walkmesh.
	 *
	 *  If the width is set, it will discard paths that are not large enough.
	 *
	 *  @param startX       The x component of the starting point.
	 *  @param startY       The y component of the starting point.
	 *  @param endX         The x component of the ending point.
	 *  @param endY         The y component of the ending point.
	 *  @param facePath     The vector where the path will be stored.
	 *  @param width        The creature's width. Default is no width.
	 *  @param maxIteration The maximum number of iteration before the algorithm stop searching.
	 *  @return             Return true if a path is found. False otherwise.
	 */
	bool findPath(float startX, float startY, float endX, float endY,
	              std::vector<uint32> &facePath, float width = 0.f, uint32 maxIteration = 10000);

protected:
	/** A node in the walkmesh network and its relationship within the structure and the algorithm.
	 *
	 * Here the node is in fact the face. It could have been a vertex or a side of a face.
	 * The edges of the network is, here, the adjacency between faces. They are weighted by the "distance"
	 * between the faces: from the center of the adjacent side of the parent node and the center of
	 * the side of the next node).
	 */
	struct Node {
		uint32 face; ///< Actually the real node.
		float x; ///< The x position of the side center.
		float y; ///< The y position of the side center.
		float z; ///< The z position of the side center.
		uint32 parent; ///< Parent node, UINT32_MAX for no parent.

		float G;
		float H; ///< Heuristic value.

		Node();
		Node(uint32 faceID, float pX, float pY, float pZ, uint32 parent = UINT32_MAX);
		/** Compare the distance between two nodes. */
		bool operator<(const Node &node) const;
	};

private:
	Pathfinding *_pathfinding;
	uint32 _polygonEdgesCount;
};

} // namespace Engines

#endif // ENGINES_ASTARALGORITHM_H
