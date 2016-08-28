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
 *  Pathfinding handling.
 */

#ifndef ENGINES_AURORA_PATHFINDER_H
#define ENGINES_AURORA_PATHFINDER_H

#include "src/common/ustring.h"
#include "src/common/readstream.h"
#include "src/common/streamtokenizer.h"

#include "src/graphics/detour/DebugDraw.h"
#include "src/graphics/detour/DetourNavMesh.h"

class dtNavMesh;
class dtNavMeshQuery;

class DebugDrawGL : public duDebugDraw {
public:
	virtual void depthMask(bool state);
	virtual void texture(bool state);
	virtual void begin(duDebugDrawPrimitives prim, float size = 1.0f);
	virtual void vertex(const float* pos, unsigned int color);
	virtual void vertex(const float x, const float y, const float z, unsigned int color);
	virtual void vertex(const float* pos, unsigned int color, const float* uv);
	virtual void vertex(const float x, const float y, const float z, unsigned int color, const float u, const float v);
	virtual void end();
};

namespace Engines {

struct WalkmeshPart {
	std::vector<std::vector<float> > verts;
	std::vector<std::vector<uint8> > faces;
	std::vector<std::vector<unsigned short> > adjFaces;
	std::vector<uint8> type;
	std::vector<float> position;
	std::vector<std::vector<int8> > adjRooms;
};

class KotORWokFile {
public:
	KotORWokFile(const Common::UString &name);
	~KotORWokFile();
private:
	Common::SeekableReadStream *_stream;
	WalkmeshPart _walkmeshPart;

friend class Pathfinder;
};

class NWNWokFile {
public:
	NWNWokFile(const Common::UString &name);
	~NWNWokFile();

	void readVerts(size_t n, float *position);
	void readFaces(size_t n);
	void readFloats(const std::vector<Common::UString> &strings,
	                                     float *floats, uint32 n, uint32 start);

private:
	void computeAdjFaces();

	Common::SeekableReadStream *_stream;
	Common::StreamTokenizer *_tokenize;
	std::vector<std::vector<float> > _verts;
	std::vector<std::vector<uint8> > _faces;
	std::vector<std::vector<short> > _adjFaces;
	std::vector<uint8> _mat;

friend class Pathfinder;
};

struct Path {
	float *start;
	float *end;
	dtPolyRef *polys;
	uint8 polysCount;
	float *straightPath;
	int *straightPathCount;
	Path();
};

class Pathfinder {
public:
	Pathfinder(bool tiled = false);
	~Pathfinder();

	void loadData();
	void addPart(const Common::UString &partName, float pX, float pY, float pZ);
	void mergeParts();
	void addTile(const Common::UString &modelName, uint8 tX, uint8 tY, uint8 orientation);
	void drawNavMesh();
	void drawPath();
	void computeSmoothPath(dtPolyRef corridor[], uint32 corridorSize, std::vector<float> smoothPath);
	void addPoly(dtPolyRef poly);
	void addPath(float *start, float *end, dtPolyRef *path, uint8 polysCount, float *straightPath, int *straightPathCount);
	void clearPoly();

	dtNavMeshQuery *_navQuery;
	dtNavMesh *_navMesh;

private:
	void setOrientation(std::vector<float> &vec, uint8 orientation);
	
	std::vector<dtPolyRef> _polysToDraw;
	std::vector<WalkmeshPart> _walkmeshParts;
	Path _path;
	DebugDrawGL _dd;
};

} // End of namespace Engines

#endif // ENGINES_AURORA_PATHFINDER_H
