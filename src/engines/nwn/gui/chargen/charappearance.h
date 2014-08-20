/*
 * xoreos - A reimplementation of BioWare's Aurora engine
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

/** @file engines/nwn/gui/chargen/charappearance.h
 *  The NWN appearance selection for the character generator.
 */

#ifndef ENGINES_NWN_GUI_CHARGEN_CHARAPPEARANCE_H
#define ENGINES_NWN_GUI_CHARGEN_CHARAPPEARANCE_H

#include "engines/nwn/gui/chargen/chargenabstract.h"

#include "engines/nwn/creature.h"

#include "engines/nwn/gui/chargen/charinfo.h"

namespace Engines {

namespace NWN {

class CharAppearance : public CharGenAbstract {
public:
	CharAppearance(Creature &character);
	~CharAppearance();

	void show();
	void hide();
	void reset();

private:
	void callbackActive(Widget &widget);
	void init();
	void setArrowButton(const Common::UString &buttonName);
	void changeCloth(uint8 cloth);
	Creature *_character;

	CharInfo *_charInfo;
	std::vector<Common::UString> _clothes;
	uint8 _cloth;
};

#endif // CHARAPPEARANCE_H

} // End of namespace NWN

} // End of namespace Engines
