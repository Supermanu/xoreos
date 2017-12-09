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
#include "src/common/vector3.h"

namespace Common {
class AABBNode;
}

namespace Engines {

class AStar;

class Pathfinding {
public:
	Pathfinding(std::vector<bool> walkableProperties, uint32 polygonEdges = 3);
	~Pathfinding();

	/** Find a path of face between two points.
	 *
	 *  It will use an A* algorithm to find as fast and as best as possible a path.
	 *  The algorithm used can be tuned from the bare class Engines::AStar and must be
	 *  set in the constructor thanks to setAStarAlgorithm() method.
	 */
	bool findPath  (float startX, float startY, float endX, float endY,
                    std::vector<uint32> &facePath, float width = 0.f, uint32 nbrIt = 100000);
	/** Compute a smooth line from a path of face.
	 *
	 *  @param startX       The x component of the starting point.
	 *  @param startY       The y component of the starting point.
	 *  @param endX         The x component of the ending point.
	 *  @param endY         The y component of the ending point.
	 *  @param facePath     The path of face.
	 *  @param width        The creature's width. Default is no width.
	 *  @param stopLength   If set, will stop smoothing and taking width into account after the
	 *                      specified length.
	 */
	void smoothPath(float startX, float startY, float endX, float endY, std::vector<uint32> &facePath,
                    std::vector<Common::Vector3> &path, float width = 0.f, float stopLength = 0.f);
	uint32 findFace(float x1, float y1, float z1, float x2, float y2, float z2, Common::Vector3 &intersect);
// 	uint32 findFace(float x, float y, float z, bool onlyWalkable = true);
	uint32 findFace(float x, float y, bool onlyWalkable = true);

	void setAStarAlgorithm(AStar *aStarAlgorithm);
	void drawWalkmesh();

	bool walkable(uint32 faceIndex) const;
	bool walkable(Common::Vector3 point);

protected:
	uint32 _polygonEdges;
	uint32 _verticesCount;
	uint32 _facesCount;

	std::vector<float> _vertices;
	std::vector<uint32> _faces;
	std::vector<uint32> _adjFaces;
	std::vector<uint32> _faceProperty;

	std::vector<Common::AABBNode *> _AABBTrees;
	void getAdjacentFaces(uint32 face, std::vector<uint32> &adjFaces);
	void getVertices(uint32 faceID, Common::Vector3 &vA, Common::Vector3 &vB, Common::Vector3 &vC) const;
	void getVertex(uint32 vertexID, Common::Vector3 &vertex) const;
	bool walkableAASquare(Common::Vector3 center, float halfWidth);
	bool walkablePolygon(Common::Vector3 vertices[], uint32 vertexCount);
	bool walkableSegment(Common::Vector3 start, Common::Vector3 end);

private:
	bool inFace(uint32 faceID, Common::Vector3 point) const;
	bool inFace(uint32 faceID, Common::Vector3 lineStart, Common::Vector3 lineEnd, Common::Vector3 &intersect);
	bool hasVertex(uint32 face, Common::Vector3 vertex) const;
	bool getSharedVertices(uint32 face1, uint32 face2, Common::Vector3 &vert1, Common::Vector3 &vert2) const;
	bool goThrough(uint32 fromFace, uint32 toFace, float width);
	void minimizePath(std::vector<Common::Vector3> &path, float halfWidth);
	Common::Vector3 getOrthonormalVec(Common::Vector3 segment, bool clockwise = true) const;
	void manageCreatureSize(std::vector<Common::Vector3> &smoothedPath, float halfWidth, std::vector<Common::Vector3> &ignoredPoints,
	                        std::vector<Common::Vector3> &finalPath);
	bool isToTheLeft(Common::Vector3 startSegment, Common::Vector3 endSegment, Common::Vector3 Point) const;
	void getVerticesTunnel(std::vector<uint32> &facePath, std::vector<Common::Vector3> &tunnel, std::vector<bool> &tunnelLeftRight);
	float xyLength(Common::Vector3 &vec) const;
	float xyLength(Common::Vector3 &vecA, Common::Vector3 &vecB) const;
	void getAdjacencyCenter(uint32 faceA, uint32 faceB, float &x, float &y) const;

	std::vector<uint32> _facesToDraw;
	std::vector<Common::Vector3> _linesToDraw;
	std::vector<Common::Vector3> _pointsToDraw;
	Common::Vector3 _creaturePos;
	float _creatureWidth;

	std::vector<bool> _walkableProperties;
	AStar *_aStarAlgorithm;

friend class AStar;
};

} // namespace Engines

#endif // ENGINES_PATHFINDING_H
