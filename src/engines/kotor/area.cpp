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
 *  The context holding a Star Wars: Knights of the Old Republic area.
 */

#include <ctime>

#include "src/common/util.h"
#include "src/common/error.h"
#include "src/common/readstream.h"

#include "src/aurora/resman.h"
#include "src/aurora/gff3file.h"
#include "src/aurora/2dafile.h"
#include "src/aurora/2dareg.h"

#include "src/graphics/graphics.h"
#include "src/graphics/renderable.h"

#include "src/graphics/aurora/cursorman.h"

#include "src/sound/sound.h"

#include "src/engines/aurora/util.h"

#include "src/engines/kotor/area.h"
#include "src/engines/kotor/room.h"
#include "src/engines/kotor/module.h"
#include "src/engines/kotor/waypoint.h"
#include "src/engines/kotor/placeable.h"
#include "src/engines/kotor/door.h"
#include "src/engines/kotor/creature.h"
#include "src/engines/kotor/kotorpathfinding.h"

namespace Engines {

namespace KotOR {

Area::Area(Module &module, const Common::UString &resRef) : Object(kObjectTypeArea),
	_module(&module), _resRef(resRef), _visible(false), _activeObject(0), _highlightAll(false) {

	std::vector<bool> walkableProp;
	walkableProp.push_back(true);
	_pathfinding = new KotORPathfinding(walkableProp);
	_iter = 1;
	GfxMan.setPathfinding(_pathfinding);

	try {
		load();
	} catch (...) {
		clear();
		throw;
	}

	// Tell the module that we exist
	_module->addObject(*this);
}

Area::~Area() {
	_module->removeObject(*this);

	hide();

	removeFocus();

	clear();

	GfxMan.setPathfinding(0);
	delete _pathfinding;
}

void Area::load() {
	loadLYT(); // Room layout
	loadVIS(); // Room visibilities

	loadRooms();

	Aurora::GFF3File are(_resRef, Aurora::kFileTypeARE, MKTAG('A', 'R', 'E', ' '));
	loadARE(are.getTopLevel());

	Aurora::GFF3File git(_resRef, Aurora::kFileTypeGIT, MKTAG('G', 'I', 'T', ' '));
	loadGIT(git.getTopLevel());
}

void Area::clear() {
	for (ObjectList::iterator o = _objects.begin(); o != _objects.end(); ++o) {
		_module->removeObject(**o);

		delete *o;
	}

	for (RoomList::iterator r = _rooms.begin(); r != _rooms.end(); ++r)
		delete *r;

	_objects.clear();
	_rooms.clear();
}

uint32 Area::getMusicDayTrack() const {
	return _musicDayTrack;
}

uint32 Area::getMusicNightTrack() const {
	return _musicNightTrack;
}

uint32 Area::getMusicBattleTrack() const {
	return _musicBattleTrack;
}

void Area::setMusicDayTrack(uint32 track) {
	_musicDayTrack = track;
	_musicDay      = TwoDAReg.get2DA("ambientmusic").getRow(track).getString("Resource");
}

void Area::setMusicNightTrack(uint32 track) {
	_musicNightTrack = track;
	_musicNight      = TwoDAReg.get2DA("ambientmusic").getRow(track).getString("Resource");
}

void Area::setMusicBattleTrack(uint32 track) {
	_musicBattleTrack = track;

	if (_musicBattleTrack != Aurora::kStrRefInvalid) {
		const Aurora::TwoDAFile &ambientMusic = TwoDAReg.get2DA("ambientmusic");

		// Normal battle music
		_musicBattle = ambientMusic.getRow(_musicBattleTrack).getString("Resource");

		// Battle stingers
		Common::UString stinger[3];
		stinger[0] = ambientMusic.getRow(_musicBattleTrack).getString("Stinger1");
		stinger[1] = ambientMusic.getRow(_musicBattleTrack).getString("Stinger2");
		stinger[2] = ambientMusic.getRow(_musicBattleTrack).getString("Stinger3");

		_musicBattleStinger.clear();
		for (int i = 0; i < 3; i++)
			if (!stinger[i].empty())
				_musicBattleStinger.push_back(stinger[i]);
	}
}

void Area::stopSound() {
	stopAmbientMusic();
	stopAmbientSound();
}

void Area::stopAmbientMusic() {
	SoundMan.stopChannel(_ambientMusic);
}

void Area::stopAmbientSound() {
	SoundMan.stopChannel(_ambientSound);
}

void Area::playAmbientMusic(Common::UString music) {
	stopAmbientMusic();

	// TODO: Area::playAmbientMusic(): Day/Night
	if (music.empty())
		music = _musicDay;

	if (music.empty())
		return;

	_ambientMusic = ::Engines::playSound(music, Sound::kSoundTypeMusic, true);
}

void Area::playAmbientSound(Common::UString sound) {
	stopAmbientSound();

	// TODO: Area::playAmbientSound():  Day/Night
	if (sound.empty())
		sound = _ambientDay;

	if (sound.empty())
		return;

	_ambientSound = ::Engines::playSound(sound, Sound::kSoundTypeSFX, true, _ambientDayVol);
}

void Area::show() {
	if (_visible)
		return;

	GfxMan.lockFrame();

	// Show rooms
	for (RoomList::iterator r = _rooms.begin(); r != _rooms.end(); ++r)
		(*r)->show();

	// Show objects
	for (ObjectList::iterator o = _objects.begin(); o != _objects.end(); ++o)
		(*o)->show();

	GfxMan.unlockFrame();

	// Play music and sound
	playAmbientSound();
	playAmbientMusic();

	_visible = true;
}

void Area::hide() {
	if (!_visible)
		return;

	removeFocus();

	stopSound();

	GfxMan.lockFrame();

	// Hide objects
	for (ObjectList::iterator o = _objects.begin(); o != _objects.end(); ++o)
		(*o)->hide();

	// Hide rooms
	for (RoomList::iterator r = _rooms.begin(); r != _rooms.end(); ++r)
		(*r)->hide();

	GfxMan.unlockFrame();

	_visible = false;
}

void Area::loadLYT() {
	Common::SeekableReadStream *lyt = 0;
	try {
		if (!(lyt = ResMan.getResource(_resRef, Aurora::kFileTypeLYT)))
			throw Common::Exception("No such LYT");

		warning("loading lyt file %s", _resRef.c_str());
		_lyt.load(*lyt);

		delete lyt;
	} catch (Common::Exception &e) {
		delete lyt;
		e.add("Failed loading LYT \"%s\"", _resRef.c_str());
		throw;
	}
}

void Area::loadVIS() {
	Common::SeekableReadStream *vis = 0;
	try {
		if (!(vis = ResMan.getResource(_resRef, Aurora::kFileTypeVIS)))
			throw Common::Exception("No such VIS");

		_vis.load(*vis);

		delete vis;
	} catch (Common::Exception &e) {
		delete vis;
		e.add("Failed loading VIS \"%s\"", _resRef.c_str());
		throw;
	}
}

void Area::loadARE(const Aurora::GFF3Struct &are) {
	// Tag
	_tag = are.getString("Tag");

	// Name
	_name = are.getString("Name");

	// Scripts
	readScripts(are);
}

void Area::loadGIT(const Aurora::GFF3Struct &git) {
	if (git.hasField("AreaProperties"))
		loadProperties(git.getStruct("AreaProperties"));

	if (git.hasField("WaypointList"))
		loadWaypoints(git.getList("WaypointList"));

	if (git.hasField("Placeable List"))
		loadPlaceables(git.getList("Placeable List"));

	if (git.hasField("Door List"))
		loadDoors(git.getList("Door List"));

	if (git.hasField("Creature List"))
		loadCreatures(git.getList("Creature List"));
}

void Area::loadProperties(const Aurora::GFF3Struct &props) {
	// Ambient sound

	const Aurora::TwoDAFile &ambientSound = TwoDAReg.get2DA("ambientsound");

	uint32 ambientDay   = props.getUint("AmbientSndDay"  , Aurora::kStrRefInvalid);
	uint32 ambientNight = props.getUint("AmbientSndNight", Aurora::kStrRefInvalid);

	_ambientDay   = ambientSound.getRow(ambientDay  ).getString("Resource");
	_ambientNight = ambientSound.getRow(ambientNight).getString("Resource");

	uint32 ambientDayVol   = CLIP<uint32>(props.getUint("AmbientSndDayVol"  , 127), 0, 127);
	uint32 ambientNightVol = CLIP<uint32>(props.getUint("AmbientSndNightVol", 127), 0, 127);

	_ambientDayVol   = 1.25f * (1.0f - (1.0f / powf(5.0f, ambientDayVol   / 127.0f)));
	_ambientNightVol = 1.25f * (1.0f - (1.0f / powf(5.0f, ambientNightVol / 127.0f)));

	// TODO: PresetInstance0 - PresetInstance7


	// Ambient music

	setMusicDayTrack  (props.getUint("MusicDay"   , Aurora::kStrRefInvalid));
	setMusicNightTrack(props.getUint("MusicNight" , Aurora::kStrRefInvalid));

	// Battle music

	setMusicBattleTrack(props.getUint("MusicBattle", Aurora::kStrRefInvalid));
}

void Area::loadRooms() {
	const Aurora::LYTFile::RoomArray &rooms = _lyt.getRooms();
	for (Aurora::LYTFile::RoomArray::const_iterator r = rooms.begin(); r != rooms.end(); ++r) {
		_rooms.push_back(new Room(r->model, r->x, r->y, r->z));
		_pathfinding->addData(r->model);
	}

	_pathfinding->finalize();
}

void Area::loadObject(KotOR::Object &object) {
	_objects.push_back(&object);
	_module->addObject(object);

	if (!object.isStatic()) {
		const std::list<uint32> &ids = object.getIDs();

		for (std::list<uint32>::const_iterator id = ids.begin(); id != ids.end(); ++id)
			_objectMap.insert(std::make_pair(*id, &object));
	}
}

void Area::loadWaypoints(const Aurora::GFF3List &list) {
	for (Aurora::GFF3List::const_iterator w = list.begin(); w != list.end(); ++w) {
		Waypoint *waypoint = new Waypoint(**w);

		loadObject(*waypoint);
	}
}

void Area::loadPlaceables(const Aurora::GFF3List &list) {
	for (Aurora::GFF3List::const_iterator p = list.begin(); p != list.end(); ++p) {
		Placeable *placeable = new Placeable(**p);

		loadObject(*placeable);
	}
}

void Area::loadDoors(const Aurora::GFF3List &list) {
	for (Aurora::GFF3List::const_iterator d = list.begin(); d != list.end(); ++d) {
		Door *door = new Door(*_module, **d);

		loadObject(*door);
	}
}

void Area::loadCreatures(const Aurora::GFF3List &list) {
	for (Aurora::GFF3List::const_iterator c = list.begin(); c != list.end(); ++c) {
		Creature *creature = new Creature(**c);

		loadObject(*creature);
	}
}

void Area::addEvent(const Events::Event &event) {
	_eventQueue.push_back(event);
}

void Area::processEventQueue() {
	bool hasMove = false;
	for (std::list<Events::Event>::const_iterator e = _eventQueue.begin();
	     e != _eventQueue.end(); ++e) {

		if (e->type == Events::kEventKeyDown) {
			if (e->key.keysym.sym == SDLK_F10) {
// 				float x1, y1, z1, x2, y2, z2;
// 				int x, y;
// 				CursorMan.getPosition(x, y);
// 				warning("cursor (%f, %f)", (float) x, (float) y);
// 				GfxMan.unproject((float) x, (float) y, x1, y1, z1, x2, y2, z2);
// 				warning("line: (%f, %f, %f) (%f, %f, %f)", x1, y1, z1, x2, y2, z2);
// 				uint32 face = _pathfinding->findFace(x1, y1, z1, x2, y2, z2, Common::Vector3());
// 				warning("face %u found", face);
			}
		}

		if        (e->type == Events::kEventMouseMove) { // Moving the mouse
			hasMove = true;
		} else if (e->type == Events::kEventMouseDown) { // Clicking
			if (e->button.button == SDL_BUTTON_LMASK) {
				checkActive(e->button.x, e->button.y);
				click(e->button.x, e->button.y);

				float x1, y1, z1, x2, y2, z2;
				int x, y;
				CursorMan.getPosition(x, y);
				GfxMan.unproject((float) x, (float) y, x1, y1, z1, x2, y2, z2);
				Common::Vector3 intersect;
				uint32 face = _pathfinding->findFace(x1, y1, z1, x2, y2, z2, intersect);
				warning("intersect (%f, %f, %f)", intersect._x, intersect._y, intersect._z);

				float width = 1.f;
				if (face != UINT32_MAX && _pathfinding->walkable(face)) {
					if (_startEndPoints.size() < 2) {
						_startEndPoints.push_back(intersect);
					} else {
						_startEndPoints[0] = _startEndPoints[1];
						_startEndPoints[1] = intersect;
					}

					if (_startEndPoints.size() == 2) {
						std::vector<uint32> path;
						clock_t startFindPath = std::clock();
						bool out = _pathfinding->findPath(_startEndPoints[0]._x, _startEndPoints[0]._y,
														  _startEndPoints[1]._x, _startEndPoints[1]._y, path, width);
						clock_t endFindPath = std::clock();
						++_iter;
						warning("Out is %i", out);
						clock_t startSmooth = std::clock();
						if (out) {
							std::vector<Common::Vector3> smoothPath;
// 							_pathfinding->SSFA(_startEndPoints[0], _startEndPoints[1], path, smoothPath, width);
							_pathfinding->smoothPath(_startEndPoints[0][0],_startEndPoints[0][1],
							                         _startEndPoints[1][0], _startEndPoints[1][1], path, smoothPath, width);
						}
						clock_t endSmooth = std::clock();
						double findPath = double(endFindPath - startFindPath);
						double smoothing = double(endSmooth - startSmooth);
						warning("Time spent find path: %f ms", findPath / CLOCKS_PER_SEC * 1000);
						warning("Time spent smoothing: %f ms", smoothing / CLOCKS_PER_SEC * 1000);
						warning("Total time: %f ms", (findPath + smoothing) / CLOCKS_PER_SEC * 1000);
					}
				}
			}
		} else if (e->type == Events::kEventKeyDown) { // Holding down TAB
			if (e->key.keysym.sym == SDLK_TAB)
				highlightAll(true);
		} else if (e->type == Events::kEventKeyUp) {   // Releasing TAB
			if (e->key.keysym.sym == SDLK_TAB)
				highlightAll(false);
		}
	}

	_eventQueue.clear();

	if (hasMove)
		checkActive();
}

KotOR::Object *Area::getObjectAt(int x, int y) {
	const Graphics::Renderable *obj = GfxMan.getObjectAt(x, y);
	if (!obj)
		return 0;

	ObjectMap::iterator o = _objectMap.find(obj->getID());
	if (o == _objectMap.end())
		return 0;

	return o->second;
}

void Area::setActive(KotOR::Object *object) {
	if (object == _activeObject)
		return;

	if (_activeObject)
		_activeObject->leave();

	_activeObject = object;

	if (_activeObject)
		_activeObject->enter();
}

void Area::checkActive(int x, int y) {
	if (_highlightAll)
		return;

	Common::StackLock lock(_mutex);

	if ((x < 0) || (y < 0))
		CursorMan.getPosition(x, y);

	setActive(getObjectAt(x, y));
}

void Area::click(int x, int y) {
	Common::StackLock lock(_mutex);

	KotOR::Object *o = getObjectAt(x, y);
	if (!o)
		return;

	o->click(_module->getPC());
}

void Area::highlightAll(bool enabled) {
	if (_highlightAll == enabled)
		return;

	_highlightAll = enabled;

	for (ObjectMap::iterator o = _objectMap.begin(); o != _objectMap.end(); ++o)
		if (o->second->isClickable())
			o->second->highlight(enabled);
}

void Area::removeFocus() {
	if (_activeObject)
		_activeObject->leave();

	_activeObject = 0;
}

void Area::notifyCameraMoved() {
	checkActive();
}

} // End of namespace KotOR

} // End of namespace Engines
