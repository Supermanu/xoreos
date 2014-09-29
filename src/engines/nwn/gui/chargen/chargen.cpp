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

/** @file engines/nwn/gui/chargen/chargen.cpp
 *  The new character generator.
 */

#include "engines/aurora/widget.h"

#include "engines/nwn/module.h"
#include "engines/nwn/gui/widgets/button.h"

#include "engines/nwn/gui/chargen/charsex.h"
#include "engines/nwn/gui/chargen/chargen.h"

namespace Engines {

namespace NWN {

CharGenMenu::CharGenMenu(Module &module) : _module(&module) {
	load("cg_main");

	// TODO: Character trait buttons
// 	getWidget("GenderButton"   , true)->setDisabled(true);
	getWidget("RaceButton"     , true)->setDisabled(true);
	getWidget("PortraitButton" , true)->setDisabled(true);
	getWidget("ClassButton"    , true)->setDisabled(true);
	getWidget("AlignButton"    , true)->setDisabled(true);
	getWidget("AbilitiesButton", true)->setDisabled(true);
	getWidget("PackagesButton" , true)->setDisabled(true);
	getWidget("CustomizeButton", true)->setDisabled(true);

	// TODO: Reset
	getWidget("ResetButton", true)->setDisabled(true);

	// TODO: Play
	getWidget("PlayButton" , true)->setDisabled(true);

	init();
}

CharGenMenu::~CharGenMenu() {
	for (uint it = 0; it < _chargenGuis.size(); ++it)
		delete _chargenGuis[it];
}

void CharGenMenu::reset() {
}

void CharGenMenu::callbackActive(Widget &widget) {
	for (uint it = 0; it < _chargenGuis.size(); ++it) {
		if (widget.getTag() == _charButtons[it]->getTag()) {
			if (sub(*_chargenGuis[it]) == 2) {
				if (it == _chargenGuis.size() - 1)
					return;
				_charButtons[it + 1]->setDisabled(false);
				_chargenGuis[it + 1]->reset();
				for (uint next = it + 2; next < _charButtons.size(); ++next) {
					_charButtons[next]->setDisabled(true);
					_chargenGuis[next]->reset();
				}
				return;
			}
		}
	}

	if (widget.getTag() == "CancelButton") {
		_returnCode = 1;
		return;
	}

}

void CharGenMenu::init() {
	_charButtons.push_back(getButton("GenderButton", true));
// 	_charButtons.push_back(getButton("RaceButton", true));
// 	_charButtons.push_back(getButton("PortraitButton", true));
// 	_charButtons.push_back(getButton("ClassButton", true));
// 	_charButtons.push_back(getButton("AlignButton", true));
// 	_charButtons.push_back(getButton("AbilitiesButton", true));
// 	_charButtons.push_back(getButton("PackagesButton", true));

	_chargenGuis.push_back(new CharSex());
}

} // End of namespace NWN

} // End of namespace Engines
