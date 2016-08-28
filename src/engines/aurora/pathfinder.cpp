#include "src/aurora/resman.h"

#include "src/common/strutil.h"

#include "src/graphics/detour/DetourNavMesh.h"
#include "src/graphics/detour/DetourNavMeshBuilder.h"
#include "src/graphics/detour/DetourNavMeshQuery.h"
#include "src/graphics/detour/DetourDebugDraw.h"
#include "src/graphics/detour/DebugDraw.h"

#include "src/engines/aurora/pathfinder.h"
#include "src/graphics/graphics.h"


class GLCheckerTexture
{
	unsigned int m_texId;
public:
	GLCheckerTexture() : m_texId(0)
	{
	}
	
	~GLCheckerTexture()
	{
		if (m_texId != 0)
			glDeleteTextures(1, &m_texId);
	}
	void bind()
	{
		if (m_texId == 0)
		{
			// Create checker pattern.
			const unsigned int col0 = duRGBA(215,215,215,255);
			const unsigned int col1 = duRGBA(255,255,255,255);
			static const int TSIZE = 64;
			unsigned int data[TSIZE*TSIZE];
			
			glGenTextures(1, &m_texId);
			glBindTexture(GL_TEXTURE_2D, m_texId);

			int level = 0;
			int size = TSIZE;
			while (size > 0)
			{
				for (int y = 0; y < size; ++y)
					for (int x = 0; x < size; ++x)
						data[x+y*size] = (x==0 || y==0) ? col0 : col1;
				glTexImage2D(GL_TEXTURE_2D, level, GL_RGBA, size,size, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
				size /= 2;
				level++;
			}
			
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		}
		else
		{
			glBindTexture(GL_TEXTURE_2D, m_texId);
		}
	}
};
GLCheckerTexture g_tex;

void DebugDrawGL::depthMask(bool state)
{
	glDepthMask(state ? GL_TRUE : GL_FALSE);
}

void DebugDrawGL::texture(bool state)
{
	if (state)
	{
		glEnable(GL_TEXTURE_2D);
		g_tex.bind();
	}
	else
	{
		glDisable(GL_TEXTURE_2D);
	}
}

void DebugDrawGL::begin(duDebugDrawPrimitives prim, float size)
{
	switch (prim)
	{
		case DU_DRAW_POINTS:
			glPointSize(size);
			glBegin(GL_POINTS);
			break;
		case DU_DRAW_LINES:
			glLineWidth(size);
			glBegin(GL_LINES);
			break;
		case DU_DRAW_TRIS:
			glBegin(GL_TRIANGLES);
			break;
		case DU_DRAW_QUADS:
			glBegin(GL_QUADS);
			break;
	};
}

void DebugDrawGL::vertex(const float* pos, unsigned int color)
{
//	warning("vertex: %f, %f, %f", pos[0], pos[1], pos[2]);
	glColor4ubv((GLubyte*)&color);
	float posChange[3] = { pos[0], pos[2], pos[1] };
	glVertex3fv(posChange);
}

void DebugDrawGL::vertex(const float x, const float y, const float z, unsigned int color)
{
	glColor4ubv((GLubyte*)&color);
	glVertex3f(x,z,y);
}

void DebugDrawGL::vertex(const float* pos, unsigned int color, const float* uv)
{
	glColor4ubv((GLubyte*)&color);
	glTexCoord2fv(uv);
	float posChange[3] = { pos[0], pos[2], pos[1] };
	glVertex3fv(posChange);
}

void DebugDrawGL::vertex(const float x, const float y, const float z, unsigned int color, const float u, const float v)
{
	glColor4ubv((GLubyte*)&color);
	glTexCoord2f(u,v);
	glVertex3f(x,z,y);
}

void DebugDrawGL::end()
{
	glEnd();
	glLineWidth(1.0f);
	glPointSize(1.0f);
}

namespace Engines {

KotORWokFile::KotORWokFile(const Common::UString &name) {
	_stream = ResMan.getResource(name, ::Aurora::kFileTypeWOK);
	warning("loading wok file: %s", name.c_str());
	if (!_stream)
		throw Common::Exception("No such WOK \"%s\"", name.c_str());

	_stream->seek(0);
	_stream->skip(8); // Text: WokVersion
	_stream->readUint32LE(); // Walkmesh type
	_stream->skip(48); // Reserved

	float position[3];
	position[0] = _stream->readIEEEFloatLE();
	position[1] = _stream->readIEEEFloatLE();
	position[2] = _stream->readIEEEFloatLE();
	position[0] = 0.f;
	position[1] = 0.f;
	position[2] = 0.f;

	uint32 verticesCount = _stream->readUint32LE();
	warning("Vertices count: %u", verticesCount);
	if (verticesCount == 0)
		return;

	uint32 verticesOffset = _stream->readUint32LE();
	uint32 facesCount = _stream->readUint32LE();
	uint32 facesOffset = _stream->readUint32LE();
	uint32 walktypesOffset = _stream->readUint32LE();
	uint32 normalizedInvertedNormalsOffset = _stream->readUint32LE();
	uint32 facePlanesCoefficientOffset = _stream->readUint32LE();
	uint32 AABBsCount = _stream->readUint32LE();
	uint32 AABBsOffset = _stream->readUint32LE();
	_stream->skip(4); // Unknown
	uint32 walkableFacesAdjacencyEdgesCount = _stream->readUint32LE();
	uint32 walkableFacesAdjacencyEdgesOffset = _stream->readUint32LE();
	uint32 perimetricEdgesCount = _stream->readUint32LE();
	uint32 perimetricEdgesOffset = _stream->readUint32LE();
	uint32 perimetricCount = _stream->readUint32LE();
	uint32 perimetricOffset = _stream->readUint32LE();

	// Vertices
	_walkmeshPart.verts.resize(verticesCount);
	_stream->seek(verticesOffset);
	for (uint32 v = 0; v < verticesCount; ++v) {
		for (uint8 i = 0; i < 3; ++i)
			_walkmeshPart.verts[v].push_back(_stream->readIEEEFloatLE() + position[i]);

		warning("Vert %u: (%f, %f, %f)", v, _walkmeshPart.verts[v][0], _walkmeshPart.verts[v][1], _walkmeshPart.verts[v][2]);
	}

	// Faces
	_walkmeshPart.faces.resize(facesCount);
	_stream->seek(facesOffset);
	for (uint32 f = 0; f < facesCount; ++f) {
		for (uint8 i = 0; i < 3; ++i)
			_walkmeshPart.faces[f].push_back(_stream->readUint32LE());

		warning("Face %u: (%i, %i, %i)", f, _walkmeshPart.faces[f][0], _walkmeshPart.faces[f][1], _walkmeshPart.faces[f][2]);
	}

	// Walkmesh type
	_walkmeshPart.type.resize(facesCount);
	_stream->seek(walktypesOffset);
	for (uint32 w = 0; w < facesCount; ++w) {
		_walkmeshPart.type[w] = _stream->readUint32LE();
		warning("Walktype %u: %u", w, _walkmeshPart.type[w]);
	}

	// Adjacency
	_walkmeshPart.adjFaces.resize(walkableFacesAdjacencyEdgesCount);
	_stream->seek(walkableFacesAdjacencyEdgesOffset);
	for (uint32 a = 0; a < walkableFacesAdjacencyEdgesCount; ++a) {
		for (uint8 i = 0; i < 3; ++i) {
			int32 adjFace = _stream->readSint32LE();
			if (adjFace == -1)
				adjFace = 0xffff;

			_walkmeshPart.adjFaces[a].push_back(adjFace);
		}
	
		warning("AdjFace %u: (%i, %i, %i)", a, _walkmeshPart.adjFaces[a][0], _walkmeshPart.adjFaces[a][1], _walkmeshPart.adjFaces[a][2]);
	}

	// Perimetric edges
	_walkmeshPart.adjRooms.resize(perimetricEdgesCount);
	_stream->seek(perimetricEdgesOffset);
	for (uint32 pe = 0; pe < perimetricEdgesCount; ++pe) {
		uint32 perimetricEdge = _stream->readUint32LE();
		int32 roomId = _stream->readSint32LE();
		_walkmeshPart.adjRooms[pe].push_back(perimetricEdge);
		_walkmeshPart.adjRooms[pe].push_back(roomId);
		warning("Perimetric edge %u: (%u, %i)", pe, perimetricEdge, roomId);
	}

	// Perimetric
	_stream->seek(perimetricOffset);
	for (uint32 p = 0; p < perimetricCount; ++p) {
		uint32 periOffset = _stream->readUint32LE();
		warning("Perimetric %u: %u", p, periOffset);
	}
}

KotORWokFile::~KotORWokFile() {
}

NWNWokFile::NWNWokFile(const Common::UString &name) {
	_stream = ResMan.getResource(name, ::Aurora::kFileTypeWOK);
	warning("loading %s", name.c_str());
	if (!_stream)
		throw Common::Exception("No such WOK \"%s\"", name.c_str());

	_stream->seek(0);

	_tokenize = new Common::StreamTokenizer(Common::StreamTokenizer::kRuleIgnoreAll);
	_tokenize->addSeparator(' ');
	_tokenize->addChunkEnd('\n');
	_tokenize->addIgnore('\r');

	float position[3];

	while (!_stream->eos()) {
		std::vector<Common::UString> line;
		size_t count = _tokenize->getTokens(*_stream, line, 3);
		_tokenize->nextChunk(*_stream);
		// Ignore empty lines and comments
		if ((count == 0) || line[0].empty() || (*line[0].begin() == '#'))
			continue;

		line[0].makeLower();

		if (line[0] == "position") {
			readFloats(line, position, 3, 1);
			for (uint8 i = 0; i < 3; ++i)
				position[i] = roundf(position[i] * 1000) / 1000;
		} else if (line[0] == "verts") {
			size_t vertsCount;
			Common::parseString(line[1], vertsCount);
			readVerts(vertsCount, position);
		} else if (line[0] == "faces") {
			size_t facesCount;
			Common::parseString(line[1], facesCount);
			readFaces(facesCount);
		} else if (line[0] == "endwalkmeshgeom") {
			break;
		}

	}

	computeAdjFaces();
}

NWNWokFile::~NWNWokFile() {
	delete _stream;
}

void NWNWokFile::readVerts(size_t n, float *position) {
	_verts.resize(n);
	for (size_t i = 0; i < n; ++i) {
		std::vector<Common::UString> line;
		size_t count = _tokenize->getTokens(*_stream, line, 3);
		_tokenize->nextChunk(*_stream);

		// Ignore empty lines and comments
		if ((count == 0) || line[0].empty() || (*line[0].begin() == '#')) {
			--i;
			continue;
		}

		for (uint32 vi = 0; vi < 3; ++vi) {
			float val;
			Common::parseString(line[vi], val);
			val = roundf(val * 100) / 100;
			if (val < 0) {
				val = MAX<float>(-5.f, val);
			} else {
				val = MIN<float>(5.f, val);
			}
			_verts[i].push_back(val + position[vi]);
		}

//		warning("v(%i): %f, %f, %f", i, _verts[i][0], _verts[i][1], _verts[i][2]);
	}
}

void NWNWokFile::readFaces(size_t n) {
	_faces.resize(n);
	_mat.resize(n);
	for (size_t i = 0; i < n; ++i) {
		std::vector<Common::UString> line;
		size_t count = _tokenize->getTokens(*_stream, line, 3);
		_tokenize->nextChunk(*_stream);

		// Ignore empty lines and comments
		if ((count == 0) || line[0].empty() || (*line[0].begin() == '#')) {
			--i;
			continue;
		}

		for (uint32 vi = 0; vi < 3; ++vi) {
			float val;
			Common::parseString(line[vi], val);
			_faces[i].push_back(val);
		}

//		uint8 mat;
		Common::parseString(line[7], _mat[i]);

//		warning("f: %i, %i, %i with mat: %i", _faces[i][0], _faces[i][1], _faces[i][2], _mat[i]);
	}
}

void NWNWokFile::readFloats(const std::vector<Common::UString> &strings,
                                     float *floats, uint32 n, uint32 start) {

	if (strings.size() < (start + n))
		throw Common::Exception("Missing tokens");

	for (uint32 i = 0; i < n; i++)
		Common::parseString(strings[start + i], floats[i]);
}

void NWNWokFile::computeAdjFaces() {
	_adjFaces.resize(_faces.size());

	for (size_t f = 0; f < _faces.size(); ++f) {
		std::vector<short> adjency(3, 0xffff);
		for (uint32 i = 0; i < _faces.size(); ++i) {
			if (f == i)
				continue;

			uint8 count = 0;
			std::vector<uint8> edges;
			for (uint8 c = 0; c < 3; ++c) {
				if (_faces[f][0] == _faces[i][c]) {
					++count;
					edges.push_back(0);
				} else if (_faces[f][1] == _faces[i][c]) {
					++count;
					edges.push_back(1);
				} else if (_faces[f][2] == _faces[i][c]) {
					++count;
					edges.push_back(2);
				}
			}

			if (count > 1) {
				if ((edges[0] == 0 && edges[1] == 1) || (edges[1] == 0 && edges[0] == 1)) {
					adjency[0] = i;
				} else if ((edges[0] == 1 && edges[1] == 2) || (edges[1] == 1 && edges[0] == 2)) {
					adjency[1] = i;
				} else if ((edges[0] == 0 && edges[1] == 2) || (edges[1] == 0 && edges[0] == 2) ) {
					adjency[2] = i;
				}
			}

		}
		_adjFaces[f] = adjency;

		if (_verts[_faces[f][0]][0] == _verts[_faces[f][1]][0] && fabs(_verts[_faces[f][1]][0]) == 5.f) {
			_adjFaces[f][0] = DT_EXT_LINK;
		} else if (_verts[_faces[f][0]][1] == _verts[_faces[f][1]][1] && fabs(_verts[_faces[f][1]][1]) == 5.f) {
			_adjFaces[f][0] = DT_EXT_LINK;
		} else if (_verts[_faces[f][1]][0] == _verts[_faces[f][2]][0] && fabs(_verts[_faces[f][1]][0]) == 5.f) {
			_adjFaces[f][1] = DT_EXT_LINK;
		} else if (_verts[_faces[f][1]][1] == _verts[_faces[f][2]][1] && fabs(_verts[_faces[f][1]][1]) == 5.f) {
			_adjFaces[f][1] = DT_EXT_LINK;
		} else if (_verts[_faces[f][0]][0] == _verts[_faces[f][2]][0] && fabs(_verts[_faces[f][0]][0]) == 5.f) {
			_adjFaces[f][2] = DT_EXT_LINK;
		} else if (_verts[_faces[f][0]][1] == _verts[_faces[f][2]][1] && fabs(_verts[_faces[f][0]][1]) == 5.f) {
			_adjFaces[f][2] = DT_EXT_LINK;
		}
//		warning("For face %i, adj (%i, %i, %i)", f, adjency[0], adjency[1], adjency[2]);
	}
}

Path::Path() {
	polysCount = 0;
}

Pathfinder::Pathfinder(bool tiled) {
//	dtFreeNavMesh(_navMesh);
	_navMesh = 0;
	_navQuery = 0;
	_path = Path();
//	_navMesh = dtAllocNavMesh();
//	if (!_navMesh) {
//		error("buildTiledNavigation: Could not allocate navmesh.");
//	}

//	if (tiled) {
//		dtNavMeshParams params;
//		params.orig[0] = 0.f;
//		params.orig[1] = 0.f;
//		params.orig[2] = 0.f;
		
//		params.tileWidth = 200.f;
//		params.tileHeight = 15.f;
		
//		params.maxTiles = 100;
//		params.maxPolys = 10000;
		
//		dtStatus status;
		
//		status = _navMesh->init(&params);
//		if (dtStatusFailed(status)) {
//			error("buildTiledNavigation: Could not init navmesh.");
//			return;
//		}


//		_navQuery = new dtNavMeshQuery();
//		status = _navQuery->init(_navMesh, 2048);
//		if (dtStatusFailed(status)) {
//			error("buildTiledNavigation: Could not init Detour navmesh query");
//			return;
//		}
//	}
}

Pathfinder::~Pathfinder() {
	delete _navMesh;
	delete _navQuery;
}

void Pathfinder::loadData() {
	
}

void Pathfinder::setOrientation(std::vector<float> &vec, uint8 orientation) {
	for (uint8 o = 0; o < orientation; ++o) {
		float temp = vec[0];
		vec[0] = - vec[1];
		vec[1] = temp;
	}
}

void Pathfinder::addPart(const Common::UString &partName, float pX, float pY, float pZ) {
	KotORWokFile *wok = new KotORWokFile(partName);
	wok->_walkmeshPart.position.push_back(pX);
	wok->_walkmeshPart.position.push_back(pY);
	wok->_walkmeshPart.position.push_back(pZ);

	_walkmeshParts.push_back(wok->_walkmeshPart);

	delete wok;
}

void Pathfinder::mergeParts() {
	uint32 totalVerticesCount = 0;
	uint32 totalFacesCount = 0;
	for (std::vector<WalkmeshPart>::iterator it = _walkmeshParts.begin(); it != _walkmeshParts.end(); ++it) {
		totalVerticesCount += (*it).verts.size();
		totalFacesCount += (*it).faces.size();
	}

	warning("Total vertices = %u", totalVerticesCount);
	warning("Total faces = %u", totalFacesCount);

//	totalVerticesCount = 11;
//	totalFacesCount = 4;

	unsigned short verts[totalVerticesCount * 3];
	uint32 counter = 0;
	for (std::vector<WalkmeshPart>::iterator it = _walkmeshParts.begin(); it != _walkmeshParts.end(); ++it) {
		for (uint8 v = 0; v < (*it).verts.size(); ++v) {
			verts[(counter + v) * 3 + 0] = (unsigned short) (roundf(((*it).verts[v][0]) * 100.f));
			verts[(counter + v) * 3 + 2] = (unsigned short) (roundf(((*it).verts[v][1]) * 100.f));
			verts[(counter + v) * 3 + 1] = (unsigned short) (roundf(((*it).verts[v][2] + (*it).position[2] + 5.f) * 100.f));

			warning("vert %u: (%u, %u, %u)", v + counter, verts[(counter + v) * 3 + 0], verts[(counter + v) * 3 + 1], verts[(counter + v) * 3 + 2]);
		}
		counter += (*it).verts.size();
	}

//	// Adjust faces
	unsigned short polys[totalFacesCount * 2 * 3];
	unsigned short polyFlags[totalFacesCount];
	unsigned char polyAreas[totalFacesCount];
	uint32 counterFaces = 0;
	counter = 0;

	std::vector< std::vector<short> > walkableFaces;
	std::vector<short> startIndex;

	for (std::vector<WalkmeshPart>::iterator it = _walkmeshParts.begin(); it != _walkmeshParts.end(); ++it) {
		warning("new part with %u faces", (*it).faces.size());
		startIndex.push_back(counterFaces);
		std::vector<short> walkableFacesPart;
		for (uint32 f = 0; f < (*it).faces.size(); ++f) {
			// Vertices indices
			polys[(counterFaces + f) * 6 + 0 + 0] = (*it).faces[f][0] + counter;
			polys[(counterFaces + f) * 6 + 0 + 1] = (*it).faces[f][1] + counter;
			polys[(counterFaces + f) * 6 + 0 + 2] = (*it).faces[f][2] + counter;
			warning("face %u: (%u, %u, %u)", counterFaces + f, polys[(counterFaces + f) * 6 + 0 + 0], polys[(counterFaces + f) * 6 + 0 + 1] , polys[(counterFaces + f) * 6 + 0 + 2]);

			// Walktypes
			unsigned short flag = 0x01;
			if ((*it).type[f] != 7 && (*it).type[f] != 2) {
				flag = 0x02;
				walkableFacesPart.push_back(f);
			}

			polyFlags[(counterFaces + f)] = flag;

			// Init adjacent faces
			polys[(counterFaces + f) * 6 + 3 + 0] = 0xffff;
			polys[(counterFaces + f) * 6 + 3 + 1] = 0xffff;
			polys[(counterFaces + f) * 6 + 3 + 2] = 0xffff;

//			if (walkable)
//				warning("adjFace %u (face %u): (%u, %u, %u)", walkableFacesPart.size() - 1, f,polys[(counterFaces + f) * 6 + 3 + 0], polys[(counterFaces + f) * 6 + 3 + 1], polys[(counterFaces + f) * 6 + 3 + 2]);

			// Areas
			polyAreas[(counterFaces + f)] = 1;
		}

		// Adjacent faces
		for (uint8 af = 0; af < walkableFacesPart.size(); ++af) {
			for (uint8 j = 0; j < 3; ++j) {
				unsigned short adjFace = 0xffff;
				if ((*it).adjFaces[af][j] != 0xffff)
					adjFace = ((*it).adjFaces[af][j] + (2 - (*it).adjFaces[af][j] % 3)) / 3 + counterFaces;

				polys[(counterFaces + walkableFacesPart[af]) * 6 + 3 + j] = adjFace;
			}
			warning("adjface %u (face %u): (%u, %u, %u)", af, counterFaces + walkableFacesPart[af], polys[(counterFaces + walkableFacesPart[af]) * 6 + 3 + 0],
			        polys[(counterFaces + walkableFacesPart[af]) * 6 + 3 + 1], polys[(counterFaces + walkableFacesPart[af]) * 6 + 3 + 2]);
		}

		counter += (*it).verts.size();
		counterFaces += (*it).faces.size();

		walkableFaces.push_back(walkableFacesPart);
//		warning("counter faces: %u", counterFaces);
	}

	dtNavMeshCreateParams params;
	memset(&params, 0, sizeof(params));
	params.verts = verts;
	params.vertCount = totalVerticesCount;
	params.polys = polys;
	params.polyAreas = polyAreas;
	params.polyFlags = polyFlags;
	params.polyCount = totalFacesCount;
	params.nvp = 3;
//		params.detailMeshes = m_dmesh->meshes;
//		params.detailVerts = verts;
//		params.detailVertsCount = vertexCount;
//		params.detailTris = m_dmesh->tris;
//		params.detailTriCount = m_dmesh->ntris;
//		params.offMeshConVerts = m_geom->getOffMeshConnectionVerts();
//		params.offMeshConRad = m_geom->getOffMeshConnectionRads();
//		params.offMeshConDir = m_geom->getOffMeshConnectionDirs();
//		params.offMeshConAreas = m_geom->getOffMeshConnectionAreas();
//		params.offMeshConFlags = m_geom->getOffMeshConnectionFlags();
//		params.offMeshConUserID = m_geom->getOffMeshConnectionId();
//		params.offMeshConCount = m_geom->getOffMeshConnectionCount();
//		params.walkableHeight = m_agentHeight;
		params.walkableRadius = 0.2f;
		params.walkableClimb = 1.f;
	params.bmin[0] = 0.0f;
	params.bmin[2] = 0.0f;
	params.bmin[1] = -5.f;
	params.bmax[0] = 300.0f;
	params.bmax[2] = 300.0f;
	params.bmax[1] = 10.f;
//	params.tileX = 0;
//	params.tileY = 0;
	params.cs = 0.01;
	params.ch = 0.01;
	params.buildBvTree = true;

	unsigned char* navData = 0;
	int navDataSize = 0;

//	if (!dtCreateNavMeshData(&params, &navData, &navDataSize)) {
//		error("Could not build Detour navmesh.");
//		return;
//	}

//	// Add tile, or leave the location empty.
//	if (navData) {
//		// Let the navmesh own the data.
//		warning("adding tile");
//		dtStatus status = _navMesh->addTile(navData, navDataSize, DT_TILE_FREE_DATA,0,0);
//		if (dtStatusFailed(status))
//			dtFree(navData);
//	}
	
//	dtNavMeshCreateParams params;
//	memset(&params, 0, sizeof(params));
//	params.verts = verts;
//	params.vertCount = totalVerticesCount;
//	params.polys = polys;
//	params.polyAreas = polyAreas;
//	params.polyFlags = polyFlags;
//	params.polyCount = totalFacesCount;
//	params.nvp = 3;

//	params.bmin[0] = 0.f;
//	params.bmin[1] = -5.f;
//	params.bmin[2] = 0.f;
//	params.bmax[0] = 100.f;
//	params.bmax[2] = 100.f;
//	params.bmax[1] = 10.f;

//	params.cs = 0.1;
//	params.ch = 0.1;
//	params.buildBvTree = true;

//	unsigned char* navData = 0;
//	int navDataSize = 0;
	if (!dtCreateNavMeshData(&params, &navData, &navDataSize)) {
		error("Could not build Detour navmesh.");
	}

	_navMesh = dtAllocNavMesh();
	if (!_navMesh)
	{
		dtFree(navData);
		error("Could not create Detour navmesh");
	}
	
	dtStatus status;

	status = _navMesh->init(navData, navDataSize, DT_TILE_FREE_DATA);
	if (dtStatusFailed(status))
	{
		dtFree(navData);
		error("Could not init Detour navmesh");
	}

	_navQuery = dtAllocNavMeshQuery();
	status = _navQuery->init(_navMesh, 2048);
	if (dtStatusFailed(status))
	{
		error("Could not init Detour navmesh query");
	}
}

void Pathfinder::addTile(const Common::UString &modelName, uint8 tX, uint8 tY, uint8 orientation) {
	NWNWokFile *wok = new NWNWokFile(modelName);

	unsigned char* navData = 0;
	int navDataSize = 0;

//	warning("ori %i", orientation);
	// Adjust vertices.
	unsigned short verts[wok->_verts.size() * 3];
	for (size_t i = 0; i < wok->_verts.size(); ++i) {
		setOrientation(wok->_verts[i], orientation);
		verts[i * 3 + 0] = (short) roundf((wok->_verts[i][0] + 5.f) * 100.f);
		verts[i * 3 + 2] = (short) roundf((wok->_verts[i][1] + 5.f) * 100.f);
		verts[i * 3 + 1] = (short) roundf((wok->_verts[i][2] + 5.f) * 100.f);
		float test[3];
		test[0] = ((float) verts[i*3]) / 100.f + tX * 10.f;
		test[1] = ((float) verts[i*3 + 2]) / 100.f + tY * 10.f;
		test[2] = ((float) verts[i*3 + 1]) / 100.f - 5.f;
//		warning("test vert v(%i) : %f, %f, %f", i, test[0], test[1], test[2]);
	}

	// Adjust faces
	unsigned short polys[wok->_faces.size() * 2 * 3];
	unsigned short polyFlags[wok->_faces.size()];
	unsigned char polyAreas[wok->_faces.size()];
	for (size_t i = 0; i < wok->_faces.size(); ++i) {
		std::vector<uint8> face = wok->_faces[i];
		// Vertices.
		polys[i * 6 + 0 + 0] = face[0];
		polys[i * 6 + 0 + 1] = face[1];
		polys[i * 6 + 0 + 2] = face[2];
//			warning("face %i: %i, %i, %i", i, polys[i*6], polys[i*6+1], polys[i*6+2]);
		// Adjacent faces.
		polys[i * 6 + 3 + 0] = wok->_adjFaces[i][0];
		polys[i * 6 + 3 + 1] = wok->_adjFaces[i][1];
		polys[i * 6 + 3 + 2] = wok->_adjFaces[i][2];
//		warning("adjface %i: %i, %i, %i", i, polys[i * 6 + 3], polys[i * 6 + 3 + 1], polys[i * 6 + 3 + 2]);

		unsigned short flag = 0x01;
//		if (wok->_mat[i] == 2 || wok->_mat[i] == 7) {
		if ((polys[i * 6 + 3 + 0] == DT_EXT_LINK || polys[i * 6 + 3 + 1] == DT_EXT_LINK || polys[i * 6 + 3 + 2] == DT_EXT_LINK)
		        && wok->_mat[i] != 7 && wok->_mat[i] != 2) {
			flag = 0x02;
		}
		polyFlags[i] = flag;

		polyAreas[i] = 0x01;

	}

	dtNavMeshCreateParams params;
	memset(&params, 0, sizeof(params));
	params.verts = verts;
	params.vertCount = wok->_verts.size();
	params.polys = polys;
	params.polyAreas = polyAreas;
	params.polyFlags = polyFlags;
	params.polyCount = wok->_faces.size();
	params.nvp = 3;
//		params.detailMeshes = m_dmesh->meshes;
//		params.detailVerts = verts;
//		params.detailVertsCount = vertexCount;
//		params.detailTris = m_dmesh->tris;
//		params.detailTriCount = m_dmesh->ntris;
//		params.offMeshConVerts = m_geom->getOffMeshConnectionVerts();
//		params.offMeshConRad = m_geom->getOffMeshConnectionRads();
//		params.offMeshConDir = m_geom->getOffMeshConnectionDirs();
//		params.offMeshConAreas = m_geom->getOffMeshConnectionAreas();
//		params.offMeshConFlags = m_geom->getOffMeshConnectionFlags();
//		params.offMeshConUserID = m_geom->getOffMeshConnectionId();
//		params.offMeshConCount = m_geom->getOffMeshConnectionCount();
//		params.walkableHeight = m_agentHeight;
//		params.walkableRadius = m_agentRadius;
//		params.walkableClimb = m_agentMaxClimb;
	params.bmin[0] = tX * 10.0f;
	params.bmin[2] = tY * 10.0f;
	params.bmin[1] = -5.f;
	params.bmax[0] = tX * 10.0f + 10.0f;
	params.bmax[2] = tY * 10.0f + 10.0f;
	params.bmax[1] = 5.f;
	params.tileX = tX;
	params.tileY = tY;
	params.cs = 0.01;
	params.ch = 0.01;
	params.buildBvTree = true;

	delete wok;

	if (!dtCreateNavMeshData(&params, &navData, &navDataSize)) {
		error("Could not build Detour navmesh.");
		return;
	}

	// Add tile, or leave the location empty.
	if (navData) {
		// Let the navmesh own the data.
		warning("adding tile");
		dtStatus status = _navMesh->addTile(navData, navDataSize, DT_TILE_FREE_DATA,0,0);
		if (dtStatusFailed(status))
			dtFree(navData);
	}

}

void Pathfinder::drawNavMesh() {
	if (!_navMesh)
		return;

	duDebugDrawNavMeshPolysWithFlags(&_dd, *_navMesh, 0x01, duRGBA(128,128,128,200));
//	duDebugDrawNavMeshPolysWithFlags(&_dd, *_navMesh, 0x02, duRGBA(228,100,228,255));
	duDebugDrawNavMesh(&_dd, *_navMesh, 0);

	for (std::vector<dtPolyRef>::iterator it = _polysToDraw.begin(); it != _polysToDraw.end(); ++it) {
		duDebugDrawNavMeshPoly(&_dd, *_navMesh, *it, duRGBA(200,200,200,255));
	}
}

void Pathfinder::drawPath() {
	if (_path.polysCount == 0)
		return;

	for (uint8 p = 0; p < _path.polysCount; ++p) {
		duDebugDrawNavMeshPoly(&_dd, *_navMesh, _path.polys[p], duRGBA(200,200,200,255));
	}

//	duDebugDrawCylinder(&_dd, _path.start[0] - 0.2, _path.start[1] - 0.2, _path.start[2] - 0.2,
//	        _path.start[0] + 0.2, _path.start[1] + 0.2, _path.start[2] + 0.2, duRGBA(250,200,200,255));

//	duDebugDrawCylinder(&_dd, _path.end[0] - 0.2, _path.end[1] - 0.2, _path.end[2] - 0.2,
//	        _path.end[0] + 0.2, _path.end[1] + 0.2, _path.end[2] + 0.2, duRGBA(250,200,200,255));

	_dd.depthMask(false);
	_dd.begin(DU_DRAW_LINES, 4.0f);
	for (uint8 p = 0; p < *_path.straightPathCount - 1; ++p) {
		_dd.vertex(_path.straightPath[p*3], _path.straightPath[p*3 + 1], _path.straightPath[p*3 + 2], duRGBA(0,250,250,255));
		_dd.vertex(_path.straightPath[(p + 1)*3], _path.straightPath[(p+1)*3 + 1], _path.straightPath[(p+1)*3 + 2], duRGBA(0,250,250,255));
	}
//	_dd.vertex(_path.start[0], _path.start[1], _path.start[2], duRGBA(0,250,250,255));
//	_dd.vertex(_path.end[0], _path.end[1], _path.end[2], duRGBA(0,250,250,255));
	_dd.end();
}

void Pathfinder::computeSmoothPath(float startPoint[3], float endPoint[3], dtPolyRef corridor[], uint32 corridorSize, std::vector<float> &smoothPath) {
	smoothPath.push_back(startPoint);
	for (uint32 p = 0; p < corridorSize; ++p) {
		
	}
}

void Pathfinder::addPoly(dtPolyRef poly) {
	_polysToDraw.push_back(poly);
}

void Pathfinder::addPath(float *start, float *end, dtPolyRef *path, uint8 polysCount, float *straightPath, int *straightPathCount) {
	_path.start = start;
	_path.end = end;
	_path.polys = path;
	_path.polysCount = polysCount;
	_path.straightPath = straightPath;
	_path.straightPathCount = straightPathCount;
}

void Pathfinder::clearPoly() {
	_polysToDraw.clear();
}

} // End of namespace Engines
