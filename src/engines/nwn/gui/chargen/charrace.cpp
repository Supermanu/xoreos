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

namespace Engines {

namespace NWN {


CharRace::CharRace(Module &module, Creature &character) : _module(&module), _character(&character) {
	load("cg_race");

	getWidget("Title"	, true)->setHorCentered();

	_raceWidgets = new raceButtonAssoc;

	_raceWidgets->insert(std::pair<Common::UString, std::pair<WidgetButton *, uint32> >("HumanButton", std::make_pair(((WidgetButton *) getWidget("HumanButton", true)), NWN::kRaceHuman)));
	_raceWidgets->insert(std::pair<Common::UString, std::pair<WidgetButton *, uint32> >("DwarfButton", std::make_pair(((WidgetButton *) getWidget("DwarfButton", true)), NWN::kRaceDwarf)));
	_raceWidgets->insert(std::pair<Common::UString, std::pair<WidgetButton *, uint32> >("ElfButton", std::make_pair(((WidgetButton *) getWidget("ElfButton", true)), NWN::kRaceElf)));
	_raceWidgets->insert(std::pair<Common::UString, std::pair<WidgetButton *, uint32> >("HalflingButton", std::make_pair(((WidgetButton *) getWidget("HalflingButton", true)), NWN::kRaceHalfling)));
	_raceWidgets->insert(std::pair<Common::UString, std::pair<WidgetButton *, uint32> >("GnomeButton", std::make_pair(((WidgetButton *) getWidget("GnomeButton", true)), NWN::kRaceGnome)));
	_raceWidgets->insert(std::pair<Common::UString, std::pair<WidgetButton *, uint32> >("HalfElfButton", std::make_pair(((WidgetButton *) getWidget("HalfElfButton", true)), NWN::kRaceHalfElf)));
	_raceWidgets->insert(std::pair<Common::UString, std::pair<WidgetButton *, uint32> >("HalfOrcButton", std::make_pair(((WidgetButton *) getWidget("HalfOrcButton", true)), NWN::kRaceHalfOrc)));

	for (raceButtonAssoc::iterator it = _raceWidgets->begin(); it != _raceWidgets->end(); ++it) {
		it->second.first->setStayPressed();
	}

	changeRaceTo(NWN::kRaceHuman);


}

CharRace::~CharRace() {
	delete _raceWidgets;
}

void CharRace::callbackActive(Widget &widget) {
	if (widget.getTag() == "CancelButton") {
		changeRaceTo(_character->getRace());
		_returnCode = 1;
		return;
	}

	if (widget.getTag() == "OkButton") {
		_character->setRace(_race);
		_returnCode = 1;
		return;
	}
	
	if (widget.getTag() == "RecommendButton") {
		changeRaceTo("HumanButton");
		return;
	}

	for (raceButtonAssoc::iterator it = _raceWidgets->begin(); it != _raceWidgets->end(); ++it) {
		if (widget.getTag() == it->first) {
			changeRaceTo(it->first);
			return;
		}
	}

}

void CharRace::changeRaceTo(Common::UString race) {
	if (_race == _raceWidgets->at(race).second)
		return;

	_raceWidgets->at(race).first->setPressed();
	for (raceButtonAssoc::iterator it = _raceWidgets->begin(); it != _raceWidgets->end(); ++it) {
		if (it->first == race)
			continue;

		it->second.first->setPressed(false);
	}
	_race = _raceWidgets->at(race).second;
}

void CharRace::changeRaceTo(uint32 race) {
	  if (_race == race) {
			return;
	  }
  
	for (raceButtonAssoc::iterator it = _raceWidgets->begin(); it != _raceWidgets->end(); ++it) {
		if (it->second.second == race) {
			changeRaceTo(it->first);
			return;
		}
	}
}



} // End of namespace NWN

} // End of namespace Engines
