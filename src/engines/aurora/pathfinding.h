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

	/** Get the intersection between a line and the walkmesh.
	 *
	 *  @param x1            The x component of the first point in the line.
	 *  @param y1            The y component of the first point in the line.
	 *  @param z1            The x component of the first point in the line.
	 *  @param x2            The x component of the second point in the line.
	 *  @param y2            The y component of the second point in the line.
	 *  @param z2            The x component of the second point in the line.
	 *  @param intersect     The returned intersection.
	 *  @param onlyWalkable  Specify if the intersection must be with a walkable face.
	 */
	bool findIntersection(float x1, float y1, float z1, float x2, float y2, float z2,
	                     Common::Vector3 &intersect, bool onlyWalkable = false);
	/** Set the A* algorithm object. */
	void setAStarAlgorithm(AStar *aStarAlgorithm);

	void drawWalkmesh();

	/** State if the point, in the XY plane, is walkable. */
	bool walkable(Common::Vector3 point);

protected:
	/** Find a face according to a point in the XY plane. */
	uint32 findFace(float x, float y, bool onlyWalkable = true);
	/** Is the walkmesh face walkable? */
	bool faceWalkable(uint32 faceID) const;
	/** Get the adjacent faces of a specific face. */
	void getAdjacentFaces(uint32 face, std::vector<uint32> &adjFaces);
	/** Get the vertices of a face. */
	void getVertices(uint32 faceID, std::vector<Common::Vector3> &vertices, bool xyPlane = true) const;
	/** The vertex position from the vertex id. */
	void getVertex(uint32 vertexID, Common::Vector3 &vertex, bool xyPlane = true) const;
	/** Check if a given align-axis square is walkable. */
	bool walkableAASquare(Common::Vector3 center, float halfWidth);
	/** Check if a given polygon is walkable. */
	bool walkablePolygon(std::vector<Common::Vector3> &vertices);
	/** Check if a given segment is walkable. */
	bool walkableSegment(Common::Vector3 start, Common::Vector3 end);

	uint32 _polygonEdges;  ///< The number of edge a walkmesh face has.
	uint32 _verticesCount; ///< The total number of vertices in the walkmesh.
	uint32 _facesCount;    ///< The total number of faces in the walkmesh.

	std::vector<float> _vertices;       ///< The vertices of the walkmesh. Each vertex has 3 components.
	std::vector<uint32> _faces;        ///< The faces of the walkmesh.
	std::vector<uint32> _adjFaces;     ///< The adjacent faces in the walkmesh.
	std::vector<uint32> _faceProperty; ///< The property of each faces. Usually used to state the walkability.

	std::vector<Common::AABBNode *> _AABBTrees; ///< The set of AABB trees in the walkmesh.

private:
	/** Is a point in a specific face? */
	bool inFace(uint32 faceID, Common::Vector3 point) const;
	/** Is a line in a specific face? */
	bool inFace(uint32 faceID, Common::Vector3 lineStart, Common::Vector3 lineEnd,
	            Common::Vector3 &intersect) const;
	/** Get the vertices shared by two faces. */
	bool getSharedVertices(uint32 face1, uint32 face2,
	                       Common::Vector3 &vert1, Common::Vector3 &vert2) const;
	/* Can a creature go from one face to an other one? */
	bool goThrough(uint32 fromFace, uint32 toFace, float width);
	/** Remove unecessary point in a path.
	 *
	 *  It's very slow and might be used carefully. It's currently not used.
	 */
	void minimizePath(std::vector<Common::Vector3> &path, float halfWidth);
	/** Get the orthonornmal vector of a segment. */
	Common::Vector3 getOrthonormalVec(Common::Vector3 segment, bool clockwise = true) const;
	/** Is a point to the left from a given segment? */
	bool isToTheLeft(Common::Vector3 startSegment, Common::Vector3 endSegment, Common::Vector3 Point) const;
	/** Get the vertices along a path of faces. */
	void getVerticesTunnel(std::vector<uint32> &facePath, std::vector<Common::Vector3> &tunnel,
	                       std::vector<bool> &tunnelLeftRight);
	/** Get the center of the adjacency edge from two faces. */
	void getAdjacencyCenter(uint32 faceA, uint32 faceB, float &x, float &y) const;

	std::vector<uint32> _facesToDraw;
	std::vector<Common::Vector3> _linesToDraw;
	std::vector<Common::Vector3> _pointsToDraw;
	Common::Vector3 _creaturePos;
	float _creatureWidth;

	std::vector<bool> _walkableProperties; ///< Mapping between surface property and walkability.
	AStar *_aStarAlgorithm; ///< A* algorithm used.

friend class AStar;
};

} // namespace Engines

#endif // ENGINES_PATHFINDING_H
