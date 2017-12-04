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

#include "src/engines/aurora/pathfinding.h"
#include "src/engines/aurora/astaralgorithm.h"

namespace Engines {

AStar::AStar(Engines::Pathfinding* pathfinding, uint32 polygonEdgesCount) :
	_pathfinding(pathfinding), _polygonEdgesCount(polygonEdgesCount) {
}

AStar::~AStar() {
}

bool AStar::findPath(float startX, float startY, float endX, float endY,
                     std::vector<uint32> &facePath, float width, uint32 maxIteration) {

	// Cleaning the futur path.
	facePath.clear();

	// Find faces start and end points belong.
	uint32 startFace = _pathfinding->findFace(startX, startY);
	uint32 endFace = _pathfinding->findFace(endX, endY);

	// Check if start and end points are in the walkmesh.
	if (startFace == UINT32_MAX || endFace == UINT32_MAX)
		return false;

	// Check if start and end points are in the same face.
	if (startFace == endFace) {
		facePath.push_back(startFace);
		return true;
	}

	// Init nodes and lists.
	Node startNode = Node(startFace, startX, startY, startZ);
	Node endNode   = Node(endFace, endX, endY, endZ);

	startNode.G = 0.f;
	startNode.H = getHeuristic(startNode, endNode);

	std::vector<Node> openList;
	std::vector<Node> closedList;
	openList.push_back(startNode);

	// Searching...
	for (uint32 it = 0; it < maxIteration; ++it) {
		if (openList.empty())
			break;

		Node current = openList.front();

		if (current.face == endNode.face) {
			reconstructPath(current, closedList, facePath);
			return true;
		}

		openList.erase(openList.begin());
		closedList.push_back(current);

		std::vector<uint32> adjNodes;
		getAdjacentNodes(current, adjNodes);
		for (std::vector<uint32>::iterator a = adjNodes.begin(); a != adjNodes.end(); ++a) {
			// Check if it has been already evaluated.
			if (hasNode(*a, closedList))
				continue;

			// Check if the creature can go through to the adjacent face.
			if (!goThrough(current.face, *a, width))
				continue;

			// Distance from start point to this node.
			float x, y, z;
			float gScore = current.G + getDistance(current, *a, x, y, z);

			// Check if it is a new node.
			Node *adjNode = getNode(*a, openList);
			bool isThere = adjNode != 0;
			if (!isThere) {
				adjNode = new Node(*a, x, y, z);
				adjNode->parent = current.face;
			} else if (gScore >= adjNode->G) {
				continue;
			}

			// adjNode is the best node up to now, update/add.
			adjNode->parent = current.face;
			adjNode->G = gScore;
			adjNode->H = getHeuristic(*adjNode, endNode);
			if (!isThere)
				openList.push_back(*adjNode);

			std::sort(openList.begin(), openList.end());
		}
	}

	return false;
}

} // namespace Engines
