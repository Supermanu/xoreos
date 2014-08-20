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

/** @file engines/nwn/gui/chargen/charspells.h
 *  The NWN spells selection for the character generator.
 */

#ifndef ENGINES_NWN_GUI_CHARGEN_CHARSPELLS_H
#define ENGINES_NWN_GUI_CHARGEN_CHARSPELLS_H

#include "engines/nwn/gui/widgets/listbox.h"
#include "engines/nwn/gui/widgets/listitembutton.h"

#include "engines/nwn/gui/chargen/charhelpinfo.h"

#include "engines/nwn/gui/chargen/chargenabstract.h"

namespace Engines {

namespace NWN {

class WidgetListItemSpell : public WidgetListItemExchange {
public:
	WidgetListItemSpell(GUI &gui, const Common::UString &text, const Common::UString &icon, const Common::UString &tag, const Common::UString &description, bool left, uint16 spellIndex);
	~WidgetListItemSpell();

	void helpActivated();
};

class CharSpells : public CharGenAbstract {
public:
	CharSpells();
	~CharSpells();

	void reset();
	void show();
	void showSpellLevel(uint16 spellLevel);
	void openSpellInfo(const Common::UString &spellName, const Common::UString &description, const Common::UString &icon);
	void moveSpell(Engines::NWN::WidgetListItemSpell * item, bool left, uint itemNumber);
	bool chooseSpells();

	void fixWidgetType(const Common::UString &tag, WidgetType &type);
private:
	void callbackActive(Widget &widget);
	void buildSpellList();
	void updateRemaining();

	std::vector<std::vector <uint32> > _spellAvailable;
	std::vector<std::vector <uint32> > _spellKnown;
	std::vector<uint > _newSpellSlot;
	uint _remainingSpells;
	bool _prepareSpell;
	uint16 _currentLevel;
	uint16 _maxLevel;
	Common::UString _casterName;

	WidgetListBox *_availableWidgetList;
	WidgetListBox *_knownWidgetList;
	CharHelpInfo *_spellInfo;
	
};

} // End of namespace NWN

} // End of namespace Engines

#endif // ENGINES_NWN_GUI_CHARGEN_CHARSPELLS_H
