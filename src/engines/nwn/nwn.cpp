/* eos - A reimplementation of BioWare's Aurora engine
 * Copyright (c) 2010-2011 Sven Hesse (DrMcCoy), Matthew Hoops (clone2727)
 *
 * The Infinity, Aurora, Odyssey and Eclipse engines, Copyright (c) BioWare corp.
 * The Electron engine, Copyright (c) Obsidian Entertainment and BioWare corp.
 *
 * This file is part of eos and is distributed under the terms of
 * the GNU General Public Licence. See COPYING for more informations.
 */

/** @file engines/nwn/nwn.cpp
 *  Engine class handling Neverwinter Nights.
 */

#include "engines/nwn/nwn.h"
#include "engines/nwn/util.h"
#include "engines/nwn/menu/legal.h"
#include "engines/nwn/menu/main.h"

#include "engines/util.h"

#include "common/util.h"
#include "common/strutil.h"
#include "common/filelist.h"
#include "common/stream.h"

#include "graphics/graphics.h"

#include "graphics/aurora/cube.h"
#include "graphics/aurora/font.h"
#include "graphics/aurora/text.h"
#include "graphics/aurora/model.h"

#include "sound/sound.h"

#include "events/events.h"

#include "aurora/error.h"
#include "aurora/resman.h"
#include "aurora/talkman.h"

namespace Engines {

namespace NWN {

const NWNEngineProbe kNWNEngineProbe;

const Common::UString NWNEngineProbe::kGameName = "Neverwinter Nights";

Aurora::GameID NWNEngineProbe::getGameID() const {
	return Aurora::kGameIDNWN;
}

const Common::UString &NWNEngineProbe::getGameName() const {
	return kGameName;
}

bool NWNEngineProbe::probe(const Common::UString &directory, const Common::FileList &rootFiles) const {
	// Don't accidentally trigger on NWN2
	if (rootFiles.contains(".*/nwn2.ini", true))
		return false;
	if (rootFiles.contains(".*/nwn2main.exe", true))
		return false;

	// If either the ini file or a binary is found, this should be a valid path

	if (rootFiles.contains(".*/nwn.ini", true))
		return true;

	if (rootFiles.contains(".*/(nw|nwn)main.exe", true))
		return true;

	if (rootFiles.contains(".*/(nw|nwn)main", true))
		return true;

	return false;
}

bool NWNEngineProbe::probe(Common::SeekableReadStream &stream) const {
	return false;
}

Engines::Engine *NWNEngineProbe::createEngine() const {
	return new NWNEngine;
}


NWNEngine::NWNEngine() {
	_modelLoader = new NWNModelLoader;
}

NWNEngine::~NWNEngine() {
	delete _modelLoader;
}

void NWNEngine::run(const Common::UString &target) {
	_baseDirectory = target;

	init();

	status("Successfully initialized the engine");

	playVideo("atarilogo");
	playVideo("biowarelogo");
	playVideo("wotclogo");
	playVideo("fge_logo_black");
	playVideo("nwnintro");

	// Menu music
	Sound::ChannelHandle menuMusic = _hasXP2 ?
	playSound("mus_x2theme"   , Sound::kSoundTypeMusic, true) :
	playSound("mus_theme_main", Sound::kSoundTypeMusic, true);

	// Start sound
	playSound("gui_prompt", Sound::kSoundTypeSFX);

	Legal    *legal    = new Legal(*_modelLoader);
	MainMenu *mainMenu = new MainMenu(*_modelLoader, _hasXP1, _hasXP2);

	// Fade in the legal billboard
	legal->fadeIn();

	// Show the main menu (still hidden by the legal billboard)
	mainMenu->show();

	// Show the legal billboard, then fade it out
	legal->show();

	delete legal;

	// Handle the main menu
	mainMenu->handle();

	delete mainMenu;
}

void NWNEngine::init() {
	status("Setting base directory");
	ResMan.registerDataBaseDir(_baseDirectory);
	indexMandatoryDirectory("", 0, 0, 0);

	status("Adding extra archive directories");
	ResMan.addArchiveDir(Aurora::kArchiveBIF, "data");
	ResMan.addArchiveDir(Aurora::kArchiveERF, "nwm");
	ResMan.addArchiveDir(Aurora::kArchiveERF, "modules");
	ResMan.addArchiveDir(Aurora::kArchiveERF, "hak");
	ResMan.addArchiveDir(Aurora::kArchiveERF, "texturepacks");

	status("Loading main KEY");
	indexMandatoryArchive(Aurora::kArchiveKEY, "chitin.key", 0);

	status("Loading expansions and patch KEYs");

	// Base game patch
	indexOptionalArchive(Aurora::kArchiveKEY, "patch.key", 1);

	// Expansion 1: Shadows of Undrentide (SoU)
	_hasXP1 = indexOptionalArchive(Aurora::kArchiveKEY, "xp1.key", 2);
	indexOptionalArchive(Aurora::kArchiveKEY, "xp1patch.key", 3);

	// Expansion 2: Hordes of the Underdark (HotU)
	_hasXP2 = indexOptionalArchive(Aurora::kArchiveKEY, "xp2.key", 4);
	indexOptionalArchive(Aurora::kArchiveKEY, "xp2patch.key", 5);

	// Expansion 3: Kingmaker (resources also included in the final 1.69 patch)
	_hasXP3 = indexOptionalArchive(Aurora::kArchiveKEY, "xp3.key", 6);
	indexOptionalArchive(Aurora::kArchiveKEY, "xp3patch.key", 7);

	status("Loading high-res texture packs");
	indexMandatoryArchive(Aurora::kArchiveERF, "gui_32bit.erf"   , 10);
	indexOptionalArchive (Aurora::kArchiveERF, "xp1_gui.erf"     , 11);
	indexOptionalArchive (Aurora::kArchiveERF, "xp2_gui.erf"     , 12);
	indexMandatoryArchive(Aurora::kArchiveERF, "textures_tpa.erf", 13);
	indexMandatoryArchive(Aurora::kArchiveERF, "tiles_tpa.erf"   , 14);
	indexOptionalArchive (Aurora::kArchiveERF, "xp1_tex_tpa.erf" , 15);
	indexOptionalArchive (Aurora::kArchiveERF, "xp2_tex_tpa.erf" , 16);

	status("Indexing extra sound resources");
	indexMandatoryDirectory("ambient"   , 0, 0, 20);
	status("Indexing extra music resources");
	indexMandatoryDirectory("music"     , 0, 0, 21);
	status("Indexing extra movie resources");
	indexMandatoryDirectory("movies"    , 0, 0, 22);
	status("Indexing extra image resources");
	indexMandatoryDirectory("portraits" , 0, 0, 23);
	status("Indexing extra character resources");
	indexMandatoryDirectory("localvault", 0, 0, 24);
	status("Indexing extra talktables");
	indexOptionalDirectory ("tlk"       , 0, 0, 25);

	status("Indexing override files");
	indexOptionalDirectory("override", 0, 0, 30);

	status("Loading main talk table");
	TalkMan.addMainTable("dialog");
}

} // End of namespace NWN

} // End of namespace Engines
