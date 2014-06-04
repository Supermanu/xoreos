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


#include "aurora/talkman.h"
#include "aurora/2dareg.h"
#include "aurora/2dafile.h"

#include "engines/aurora/widget.h"

#include "engines/nwn/module.h"

#include "engines/nwn/gui/chargen/chargen.h"


namespace Engines {

namespace NWN {

CharGenMenu::CharGenMenu(Module &module) : _module(&module) {
	load("cg_main");

	_character = new Creature();

	// Move to half the parent widget
	// getWidget("TitleLabel"     , true)->movePosition(371,0,0);
	getWidget("TitleLabel", true)->setHorCentered();

	// TODO: Play
	// getWidget("PlayButton" , true)->setDisabled(true);
	init();

}

CharGenMenu::~CharGenMenu() {
	for (uint it = 0; it < _choiceGui.size(); ++it)
		delete _choiceGui[it];
}

void CharGenMenu::reset() {
	delete _character;
	_character = new Creature();
	for (uint it = 0; it < _choiceGui.size(); ++it)
		_choiceGui[it]->reset();
}

void CharGenMenu::init() {
	_choiceButtons.push_back(getButton("GenderButton", true));
	_choiceButtons.push_back(getButton("RaceButton", true));
	_choiceButtons.push_back(getButton("PortraitButton", true));
	// _choiceButtons.push_back(getButton("ClassButton", true));
	// _choiceButtons.push_back(getButton("AlignButton", true));
	// _choiceButtons.push_back(getButton("AbilitiesButton", true));
	// _choiceButtons.push_back(getButton("PackagesButton", true));
	// _choiceButtons.push_back(getButton("CustomizeButton", true));

	_choiceGui.push_back(new CharSex(*_character));
	_choiceGui.push_back(new CharRace(*_character));
	_choiceGui.push_back(new CharPortrait(*_character));
	// _choiceGui.push_back(new CharClass(*_character));
	// _choiceGui.push_back(new CharAlignment(*_character));
	// _choiceGui.push_back(new CharAttributes(*_character));
	// _choiceGui.push_back(new CharPackage(*_character));
	// _choiceGui.push_back(new CharAppearance(*_character));
}

void CharGenMenu::callbackActive(Widget &widget) {
	if (widget.getTag() == "CancelButton") {
		reset();
		_returnCode = 1;
		return;
	}

	if (widget.getTag() == "PlayButton") {
		_characterChoices.applyAbilities();
		_module->setPC(_character);
		_returnCode = 2;
		return;
	}

	for (uint it = 0; it < _choiceGui.size(); ++it) {
		if (widget.getTag() == _choiceButtons[it]->getTag()) {
			if (sub(*_choiceGui[it]) == 2) {
				if (it == _choiceGui.size() - 1)
					return;

				_choiceButtons[it + 1]->setDisabled(false);
				_choiceGui[it + 1]->reset();
				for (uint next = it + 2; next < _choiceButtons.size(); ++next) {
					_choiceButtons[next]->setDisabled(true);
					_choiceGui[next]->reset();
				}
				return;
			}
		}
	}

	if (widget.getTag() == "ResetButton") {
		reset();
		return;
	}


}

} // End of namespace NWN

} // End of namespace Engines
