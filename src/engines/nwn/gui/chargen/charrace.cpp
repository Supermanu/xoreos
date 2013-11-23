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
 *
 */

#include "engines/nwn/gui/chargen/charrace.h"

#include "aurora/talkman.h"

namespace Engines {

namespace NWN {


CharRace::CharRace(Module &module, Creature &character) : _module(&module), _character(&character) {
	load("cg_race");

	getWidget("Title"		, true)->setHorCentered();
	//TODO Implement subrace.
	getWidget("SubRaceButton"	, true)->setDisabled(true);

	initVectors();

	_helpBox = (WidgetEditBox *) getWidget("HelpBox", true);

	for (std::vector<WidgetButton *>::iterator it = _widgetList.begin(); it != _widgetList.end(); ++it) 
		(*it)->setStayPressed();
	
	if (_character->getRace() == kRaceInvalid) {
		changeRaceTo(kRaceHuman);
	} else {
		changeRaceTo(_character->getRace());
	}
	_helpBox->setTitle(_helpTexts[15]);
	_helpBox->setMainText(_helpTexts[7]);
	_race = kRaceInvalid;
}

CharRace::~CharRace() {
	delete _helpBox;
}

void CharRace::callbackActive(Widget &widget) {
	if (widget.getTag() == "CancelButton") {
		changeRaceTo(_character->getRace());
		_returnCode = 1;
		return;
	}

	if (widget.getTag() == "OkButton") {
		if (_race == kRaceInvalid)
			_race = kRaceHuman;

		_character->setRace(_race);
		_returnCode = 1;
		return;
	}
	
	if (widget.getTag() == "RecommendButton") {
		changeRaceTo(kRaceHuman);
		return;
	}

	for (Uint8 it = 0; it < 7; ++it) {
		if (widget.getTag() == _buttonList.at(it))
			changeRaceTo(_raceList[it]);
	}
}

void CharRace::changeRaceTo(Uint32 race) {
	if (_race == race)
		return;
 
	if (_race == kRaceInvalid) {
		_widgetList[0]->setPressed(false);
	}

	for (Uint8 it = 0; it < 7; ++it) {
		if (_raceList[it] == _race) {
			_widgetList[it]->setPressed(false);
		} else if (_raceList[it] == race) {
			_widgetList[it]->setPressed();
			_helpBox->setMainText(_helpTexts[it]);
			_helpBox->setTitle(_helpTexts[it+8]);
		}
	}
	_race = race;
}

void CharRace::initVectors() {
	_buttonList.push_back("HumanButton");
	_buttonList.push_back("DwarfButton");
	_buttonList.push_back("ElfButton");
	_buttonList.push_back("HalflingButton");
	_buttonList.push_back("GnomeButton");
	_buttonList.push_back("HalfElfButton");
	_buttonList.push_back("HalfOrcButton");

	_raceList.push_back(kRaceHuman);
	_raceList.push_back(kRaceDwarf);
	_raceList.push_back(kRaceElf);
	_raceList.push_back(kRaceHalfling);
	_raceList.push_back(kRaceGnome);
	_raceList.push_back(kRaceHalfElf);
	_raceList.push_back(kRaceHalfOrc);
	
	//Add main text
	_helpTexts.push_back(TalkMan.getString(257));
	_helpTexts.push_back(TalkMan.getString(251));
	_helpTexts.push_back(TalkMan.getString(252));
	_helpTexts.push_back(TalkMan.getString(254));
	_helpTexts.push_back(TalkMan.getString(253));
	_helpTexts.push_back(TalkMan.getString(255));
	_helpTexts.push_back(TalkMan.getString(256));
	_helpTexts.push_back(TalkMan.getString(485));

	//Add titles
	_helpTexts.push_back(TalkMan.getString(35));
	_helpTexts.push_back(TalkMan.getString(23));
	_helpTexts.push_back(TalkMan.getString(25));
	_helpTexts.push_back(TalkMan.getString(29));
	_helpTexts.push_back(TalkMan.getString(27));
	_helpTexts.push_back(TalkMan.getString(31));
	_helpTexts.push_back(TalkMan.getString(33));
	_helpTexts.push_back(TalkMan.getString(481));
	
	for (Uint8 it = 0; it < 7; ++it) {
		_widgetList.push_back((WidgetButton *) getWidget(_buttonList[it], true));
	}
}

void CharRace::reset() {
	changeRaceTo(kRaceHuman);
	_helpBox->setTitle(_helpTexts[15]);
	_helpBox->setMainText(_helpTexts[7]);
	_race = kRaceInvalid;
}



} // End of namespace NWN

} // End of namespace Engines
