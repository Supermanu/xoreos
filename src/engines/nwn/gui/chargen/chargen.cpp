/* xoreos - A reimplementation of BioWare's Aurora engine
 *
 * xoreos is the legal property of its developers, whose names can be
 * found in the AUTHORS file distributed with this source
 * distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 3
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 *
 * The Infinity, Aurora, Odyssey, Eclipse and Lycium engines, Copyright (c) BioWare corp.
 * The Electron engine, Copyright (c) Obsidian Entertainment and BioWare corp.
 */

/** @file engines/nwn/gui/chargen/chargen.cpp
 *  The new character generator.
 */

#include "engines/aurora/widget.h"

#include "engines/nwn/module.h"

#include "engines/nwn/gui/chargen/chargen.h"


namespace Engines {

namespace NWN {

CharGenMenu::CharGenMenu(Module &module) : _module(&module) {
	load("cg_main");
	
	_character = new Creature();
	
	_charSex	= new CharSex(*_module, *_character);
	_charRace	= new CharRace(*_module, *_character);
	_charPortrait	= new CharPortrait(*_module, *_character);

	// Move to half the parent widget
	//getWidget("TitleLabel"     , true)->movePosition(371,0,0);
	getWidget("TitleLabel"     , true)->setHorCentered();

	// TODO: Character trait buttons
	getWidget("ClassButton"    , true)->setDisabled(true);
	getWidget("AlignButton"    , true)->setDisabled(true);
	getWidget("AbilitiesButton", true)->setDisabled(true);
	getWidget("PackagesButton" , true)->setDisabled(true);
	getWidget("CustomizeButton", true)->setDisabled(true);

	// TODO: Play
	getWidget("PlayButton" , true)->setDisabled(true);

}

CharGenMenu::~CharGenMenu() {
	delete _character;
	delete _charSex;
	delete _charRace;
	delete _charPortrait;
}

void CharGenMenu::reset() {
	delete _character;
	_character = new Creature();
	_charSex->reset();
	_charRace->reset();
	_charPortrait->reset();
}


void CharGenMenu::callbackActive(Widget &widget) {
	if (widget.getTag() == "CancelButton") {
		reset();
		_returnCode = 1;
		return;
	}
	
	if (widget.getTag() == "GenderButton") {
		sub(*_charSex);
		return;
	}
	
	if (widget.getTag() == "RaceButton") {
		sub(*_charRace);
		return;
	}

	if (widget.getTag() == "PortraitButton") {
		sub(*_charPortrait);
		return;
	}
	
	if (widget.getTag() == "ResetButton") {
		reset();
		return;
	}


}

} // End of namespace NWN

} // End of namespace Engines
