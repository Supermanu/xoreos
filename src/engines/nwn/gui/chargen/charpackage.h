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

/** @file engines/nwn/gui/chargen/charpackage.h
 *  The NWN package selection for the character generator.
 */

#ifndef ENGINES_NWN_GUI_CHARGEN_CHARPACKAGE_H
#define ENGINES_NWN_GUI_CHARGEN_CHARPACKAGE_H

#include "engines/nwn/gui/chargen/chargenabstract.h"
#include "engines/nwn/gui/widgets/listbox.h"

#include "engines/nwn/gui/chargen/charskills.h"

#include "engines/nwn/creature.h"

namespace Engines {

namespace NWN {

class Module;

class CharPackage : public CharGenAbstract {
public:
	CharPackage(Engines::NWN::Creature & character);
	~CharPackage();

	void show();
	void fixWidgetType(const Common::UString &tag, WidgetType &type);
	void reset();
	void applyPackage();
	bool hasTakenPackage();

private:
	void callbackActive(Widget &widget);

	Creature *_character;

	WidgetEditBox *_helpBox;
	WidgetListBox *_packageList;
	std::vector<Common::UString> _helpTexts;
	std::vector<Common::UString> _packageNames;
	bool _hasTakenPackage;

	CharSkills *_charSkills;
};

} // End of namespace NWN

} // End of namespace Engines

#endif // ENGINES_NWN_GUI_CHARGEN_CHARPACKAGE_H
