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

/** @file engines/nwn/gui/chargen/charsex.h
 *  The NWN gender selection for the character generator.
 */

#ifndef ENGINES_NWN_GUI_CHARGEN_CHARSEX_H
#define ENGINES_NWN_GUI_CHARGEN_CHARSEX_H

#include <map>

#include "engines/nwn/gui/chargen/chargenabstract.h"
#include "engines/nwn/gui/widgets/button.h"
#include "engines/nwn/gui/widgets/editbox.h"

#include "engines/nwn/creature.h"

namespace Engines {

namespace NWN {

class CharSex : public CharGenAbstract {
public:
	CharSex(Engines::NWN::Creature & character);
	~CharSex();

	void swapGender();
	void reset();

private:
	void callbackActive(Widget &widget);

	Creature *_character;
	uint32 _gender;
	std::map<Common::UString,WidgetButton*> *_genderWidgets;
	WidgetEditBox *_helpBox;
	std::vector<Common::UString> _helpTexts;
};

} // End of namespace NWN

} // End of namespace Engines

#endif // ENGINES_NWN_GUI_CHARGEN_CHARSEX_H
