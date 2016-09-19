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
 *  Base class for pathfinding
 */

#ifndef ENGINES_PATHFINDING_H
#define ENGINES_PATHFINDING_H

#include "src/common/ustring.h"

namespace Common {
class Vector3;
}

namespace Engines {

class Pathfinding {
public:
	Pathfinding();
	~Pathfinding();

	bool findPath(float startX, float startY, float startZ, float endX, float endY, float endZ, std::vector<uint32> &facePath, float width = 0.01, uint32 nbrIt = 100000);
	void smoothPath(Common::Vector3 start, Common::Vector3 end, std::vector<uint32> &facePath, std::vector<Common::Vector3> &path);
	void drawWalkmesh();

protected:
	struct Node {
		uint32 face;
		float x;
		float y;
		float z;
		uint32 parent; ///< Parent node, UINT32_MAX for no parent.

		float G;
		float H;

		Node();
		Node(uint32 faceID, float pX, float pY, float pZ, uint32 par = UINT32_MAX);
		bool operator<(const Node &node) const;
	};

	uint32 _verticesCount;
	uint32 _facesCount;

	std::vector<float> _vertices;
	std::vector<uint32> _faces;
	std::vector<uint32> _adjFaces;
	std::vector<uint8> _faceProperty;

	bool walkable(uint32 faceIndex) const;
	uint32 findFace(float x, float y, float z) const;
	bool hasNode(uint32 face, std::vector<Node> &nodes) const;
	bool hasNode(uint32 face, std::vector<Node> &nodes, Node &node) const;

	void getAdjacentNodes(Node &node, std::vector<uint32> &adjNodes);
	float getDistance(Node &fromNode, uint32 toFace, float &toX, float &toY, float &toZ) const;
	float getDistance(float fX, float fY, float fZ, float tX, float tY, float tZ) const;
	float getHeuristic(Node &node, Node &endNode) const;
	void getVertices(uint32 faceID, Common::Vector3 &vA, Common::Vector3 &vB, Common::Vector3 &vC);

private:
	bool inFace(uint32 faceID, float x, float y, float z) const;
	bool inFace(uint32 faceID, Common::Vector3 point) const;
	bool segmentInFace(uint32 faceID, Common::Vector3 segStart, Common::Vector3 segEnd);
	bool getIntersection(Common::Vector3 segStart1, Common::Vector3 segEnd1, Common::Vector3 segStart2, Common::Vector3 segEnd2, Common::Vector3 &intersect);
	void reconstructPath(Node &endNode, std::vector<Node> &closedList, std::vector<uint32> &path);
	void getIntersections(Common::Vector3 &start, Common::Vector3 &end, uint32 face, std::vector<Common::Vector3> &intersects);
	void getClosestIntersection(Common::Vector3 &start, Common::Vector3 &end, uint32 face, Common::Vector3 &intersect);

	std::vector<uint32> _facesToDraw;
	std::vector<Common::Vector3> _pointsToDraw;
};

} // namespace Engines

#endif // ENGINES_PATHFINDING_H
