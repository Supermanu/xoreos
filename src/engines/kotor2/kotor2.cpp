/* eos - A reimplementation of BioWare's Aurora engine
 * Copyright (c) 2010-2011 Sven Hesse (DrMcCoy), Matthew Hoops (clone2727)
 *
 * The Infinity, Aurora, Odyssey and Eclipse engines, Copyright (c) BioWare corp.
 * The Electron engine, Copyright (c) Obsidian Entertainment and BioWare corp.
 *
 * This file is part of eos and is distributed under the terms of
 * the GNU General Public Licence. See COPYING for more informations.
 */

/** @file engines/kotor2/kotor2.cpp
 *  Engine class handling Star Wars: Knights of the Old Republic II - The Sith Lords
 */

#include "engines/kotor2/kotor2.h"
#include "engines/kotor2/util.h"
#include "engines/kotor2/module.h"

#include "engines/util.h"

#include "common/util.h"
#include "common/strutil.h"
#include "common/filelist.h"
#include "common/stream.h"

#include "graphics/graphics.h"

#include "graphics/aurora/cube.h"
#include "graphics/aurora/font.h"
#include "graphics/aurora/fontman.h"
#include "graphics/aurora/text.h"
#include "graphics/aurora/model_kotor.h"

#include "sound/sound.h"

#include "events/events.h"

#include "aurora/lytfile.h"
#include "aurora/resman.h"
#include "aurora/error.h"

namespace Engines {

namespace KotOR2 {

const KotOR2EngineProbe kKotOR2EngineProbe;

const Common::UString KotOR2EngineProbe::kGameName = "Star Wars: Knights of the Old Republic II - The Sith Lords";

Aurora::GameID KotOR2EngineProbe::getGameID() const {
	return Aurora::kGameIDKotOR2;
}

const Common::UString &KotOR2EngineProbe::getGameName() const {
	return kGameName;
}

bool KotOR2EngineProbe::probe(const Common::UString &directory, const Common::FileList &rootFiles) const {
	// If either swkotor2.ini or swkotor2.exe exists, this should be a valid path
	return rootFiles.contains(".*/swkotor2.(exe|ini)", true);
}

bool KotOR2EngineProbe::probe(Common::SeekableReadStream &stream) const {
	return false;
}

Engines::Engine *KotOR2EngineProbe::createEngine() const {
	return new KotOR2Engine;
}


KotOR2Engine::KotOR2Engine() {
	_modelLoader = new KotOR2ModelLoader;
}

KotOR2Engine::~KotOR2Engine() {
	delete _modelLoader;
}

void KotOR2Engine::run(const Common::UString &target) {
	_baseDirectory = target;

	init();

	status("Successfully initialized the engine");

	playVideo("leclogo");
	playVideo("obsidianent");
	playVideo("legal");

	playVideo("permov01");

	Sound::ChannelHandle channel;

	Common::SeekableReadStream *wav = ResMan.getResource(Aurora::kResourceSound, "298hk50mun003");
	if (wav) {
		status("Found a wav. Trying to play it. Turn up your speakers");
		channel = SoundMan.playSoundFile(wav, Sound::kSoundTypeVoice);

		SoundMan.startChannel(channel);
	}

	// Test load up Nar Shaddaa

	Module *narShaddaa = new Module(*_modelLoader);

	narShaddaa->load("301NAR");
	narShaddaa->enter();

	Graphics::Aurora::FontHandle font = FontMan.get("dialogfont32x32b");

	float textX = (-(GfxMan.getScreenWidth()  / 2.0))                             / 100.0;
	float textY = ( (GfxMan.getScreenHeight() / 2.0) - font.getFont().getScale()) / 100.0;

	Graphics::Aurora::Text *text = new Graphics::Aurora::Text(font, "");
	text->setPosition(textX, textY);
	text->show();

	EventMan.enableKeyRepeat();

	status("Entering event loop");

	while (!EventMan.quitRequested()) {
		Events::Event event;
		while (EventMan.pollEvent(event)) {
			if (event.type == Events::kEventKeyDown) {
				if      (event.key.keysym.sym == SDLK_UP)
					narShaddaa->move  ( 0.5);
				else if (event.key.keysym.sym == SDLK_DOWN)
					narShaddaa->move  (-0.5);
				else if (event.key.keysym.sym == SDLK_LEFT)
					narShaddaa->turn  ( 0.0, -5.0,  0.0);
				else if (event.key.keysym.sym == SDLK_RIGHT)
					narShaddaa->turn  ( 0.0,  5.0,  0.0);
				else if (event.key.keysym.sym == SDLK_PAGEUP)
					narShaddaa->turn  (-5.0,  0.0,  0.0);
				else if (event.key.keysym.sym == SDLK_PAGEDOWN)
					narShaddaa->turn  ( 5.0,  0.0,  0.0);
				else if (event.key.keysym.sym == SDLK_INSERT)
					narShaddaa->move  ( 0.0,  0.0,  0.5);
				else if (event.key.keysym.sym == SDLK_DELETE)
					narShaddaa->move  ( 0.0,  0.0, -0.5);
				else if (event.key.keysym.sym == SDLK_w)
					narShaddaa->move  ( 0.5);
				else if (event.key.keysym.sym == SDLK_s)
					narShaddaa->move  (-0.5);
				else if (event.key.keysym.sym == SDLK_a)
					narShaddaa->strafe(-0.5);
				else if (event.key.keysym.sym == SDLK_d)
					narShaddaa->strafe( 0.5);
				else if (event.key.keysym.sym == SDLK_q)
					narShaddaa->turn  ( 0.0, -5.0,  0.0);
				else if (event.key.keysym.sym == SDLK_e)
					narShaddaa->turn  ( 0.0,  5.0,  0.0);
				else if (event.key.keysym.sym == SDLK_END) {
					const float *orient = narShaddaa->getOrientation();

					narShaddaa->setOrientation(0.0, orient[1], orient[2]);
				} else if (event.key.keysym.sym == SDLK_t) {
					const float *pos    = narShaddaa->getPosition();
					const float *orient = narShaddaa->getOrientation();

					warning("%+8.3f %+8.3f %+8.3f; %+8.3f %+8.3f %+8.3f", pos[0], pos[1], pos[2], orient[0], orient[1], orient[2]);
				}
			}
		}

		EventMan.delay(10);

		text->set(Common::UString::sprintf("%d fps", GfxMan.getFPS()));
	}

	narShaddaa->leave();

	delete narShaddaa;

	delete text;
}

void KotOR2Engine::init() {
	status("Setting base directory");
	ResMan.registerDataBaseDir(_baseDirectory);

	status("Adding extra archive directories");
	ResMan.addArchiveDir(Aurora::kArchiveBIF, "data");
	ResMan.addArchiveDir(Aurora::kArchiveERF, "lips");
	ResMan.addArchiveDir(Aurora::kArchiveERF, "texturepacks");
	ResMan.addArchiveDir(Aurora::kArchiveERF, "modules");
	ResMan.addArchiveDir(Aurora::kArchiveRIM, "rims");
	ResMan.addArchiveDir(Aurora::kArchiveRIM, "modules");

	status("Loading main KEY");
	indexMandatoryArchive(Aurora::kArchiveKEY, "chitin.key", 0);

	status("Loading high-res texture packs");
	indexMandatoryArchive(Aurora::kArchiveERF, "swpc_tex_gui.erf", 10);
	indexMandatoryArchive(Aurora::kArchiveERF, "swpc_tex_tpa.erf", 11);

	status("Indexing extra sound resources");
	indexMandatoryDirectory("streamsounds", 0, -1, 20);
	status("Indexing extra voice resources");
	indexMandatoryDirectory("streamvoice" , 0, -1, 21);
	status("Indexing extra music resources");
	indexMandatoryDirectory("streammusic" , 0, -1, 22);
	status("Indexing extra movie resources");
	indexMandatoryDirectory("movies"      , 0, -1, 23);

	status("Indexing override files");
	indexOptionalDirectory("override", 0, 0, 30);
}

} // End of namespace KotOR2

} // End of namespace Engines
