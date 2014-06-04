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

/** @file engines/nwn/gui/chargen/chargen.h
 *  The NWN character generator.
 */

#ifndef ENGINES_NWN_GUI_CHARGEN_CHARGEN_H
#define ENGINES_NWN_GUI_CHARGEN_CHARGEN_H

#include "engines/nwn/gui/chargen/charsex.h"
#include "engines/nwn/gui/chargen/charrace.h"
#include "engines/nwn/gui/chargen/charportrait.h"
// #include "engines/nwn/gui/chargen/charclass.h"
// #include "engines/nwn/gui/chargen/charalignment.h"
// #include "engines/nwn/gui/chargen/charattributes.h"
// #include "engines/nwn/gui/chargen/charpackage.h"
// #include "engines/nwn/gui/chargen/charappearance.h"

namespace Engines {

namespace NWN {

class Module;

/** The NWN character generator. */
class CharGenMenu : public CharGenAbstract {
public:
	CharGenMenu(Module &module);
	~CharGenMenu();
	void reset();

protected:
	void callbackActive(Widget &widget);

private:
	void init();
	Module *_module;
	Creature *_character;

	std::vector<WidgetButton *> _choiceButtons;
	std::vector<CharGenAbstract *> _choiceGui;
};

} // End of namespace NWN

} // End of namespace Engines

#endif // ENGINES_NWN_GUI_CHARGEN_CHARGEN_H
