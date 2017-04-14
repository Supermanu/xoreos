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
class AABBNode;
}

namespace Engines {

class Pathfinding {
public:
	Pathfinding();
	~Pathfinding();

	bool findPath(float startX, float startY, float startZ, float endX, float endY, float endZ, std::vector<uint32> &facePath, float width = 0.01, uint32 nbrIt = 100000);
	void smoothPath(Common::Vector3 start, Common::Vector3 end, std::vector<uint32> &facePath, std::vector<Common::Vector3> &path, float width = 0.01);
	void SSFA(Common::Vector3 start, Common::Vector3 end, std::vector<uint32> &facePath, std::vector<Common::Vector3> &path, float width);
	uint32 findFace(float x1, float y1, float z1, float x2, float y2, float z2, Common::Vector3 &intersect);
	void drawWalkmesh();

	bool walkable(uint32 faceIndex) const;
	bool walkable(Common::Vector3 point);

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
	std::vector<uint32> _faceProperty;

	std::vector<Common::AABBNode *> _AABBTrees;

	uint32 findFace(float x, float y, float z, bool onlyWalkable = true);
    uint32 findFace(float x, float y, bool onlyWalkable = true);
	bool hasNode(uint32 face, std::vector<Node> &nodes) const;
	bool hasNode(uint32 face, std::vector<Node> &nodes, Node &node) const;
	Node *getNode(uint32 face, std::vector<Node> &nodes) const;

	void getAdjacentNodes(Node &node, std::vector<uint32> &adjNodes);
	float getDistance(Node &fromNode, uint32 toFace, float &toX, float &toY, float &toZ) const;
	float getDistance(float fX, float fY, float fZ, float tX, float tY, float tZ) const;
	float getHeuristic(Node &node, Node &endNode) const;
	void getVertices(uint32 faceID, Common::Vector3 &vA, Common::Vector3 &vB, Common::Vector3 &vC) const;
	bool walkableCircle(Common::Vector3 center, float radius);
	bool walkableBox(Common::Vector3 center, float halfWidth);

private:
	bool inFace(uint32 faceID, Common::Vector3 point) const;
	bool inFace(uint32 faceID, Common::Vector3 lineStart, Common::Vector3 lineEnd, Common::Vector3 &intersect);
	bool hasVertex(uint32 face, Common::Vector3 vertex) const;
	bool getSharedVertices(uint32 face1, uint32 face2, Common::Vector3 &vert1, Common::Vector3 &vert2) const;
	bool segmentInFace(uint32 faceID, Common::Vector3 segStart, Common::Vector3 segEnd);
	bool goThrough(uint32 fromFace, uint32 toFace, float width);
	void reconstructPath(Node &endNode, std::vector<Node> &closedList, std::vector<uint32> &path);
	void getIntersections(Common::Vector3 &start, Common::Vector3 &end, uint32 face, std::vector<Common::Vector3> &intersects);
	void getClosestIntersection(Common::Vector3 &start, Common::Vector3 &end, uint32 face, Common::Vector3 &intersect);
	Common::Vector3 getCreatureSizePoint(Common::Vector3 &from, Common::Vector3 &left, Common::Vector3 &right, float halfWidth, bool alongLeft);
	Common::Vector3 getOrthonormalVec(Common::Vector3 segment, bool clockwise = true) const;
	void manageCreatureSize(std::vector<Common::Vector3> &smoothedPath, float halfWidth, std::vector<Common::Vector3> &ignoredPoints,
	                        std::vector<Common::Vector3> &finalPath);
	bool isToTheLeft(Common::Vector3 startSegment, Common::Vector3 endSegment, Common::Vector3 Point) const;
	void getVerticesTunnel(std::vector<uint32> &facePath, std::vector<Common::Vector3> &tunnel, std::vector<bool> &tunnelLeftRight);

	std::vector<uint32> _facesToDraw;
	std::vector<Common::Vector3> _linesToDraw;
	std::vector<Common::Vector3> _pointsToDraw;
	Common::Vector3 _creaturePos;
	float _creatureWidth;
};

} // namespace Engines

#endif // ENGINES_PATHFINDING_H
