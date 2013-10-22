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

/** @file engines/nwn/gui/chargen/charportrait.h
 *  The portrait character chooser.
 */

#ifndef ENGINES_NWN_GUI_CHARGEN_CHARPORTRAIT_H
#define ENGINES_NWN_GUI_CHARGEN_CHARPORTRAIT_H

#include "engines/nwn/gui/gui.h"

#include "engines/nwn/creature.h"

#include "engines/nwn/gui/widgets/portrait.h"
#include "engines/nwn/gui/widgets/panel.h"
#include "engines/nwn/gui/widgets/listbox.h"

namespace Engines {

namespace NWN {

class Module;

class WidgetListItemPortrait : public WidgetListItem {
public:
	WidgetListItemPortrait(::Engines::GUI &gui, const Common::UString &portrait, float hSpacing, float vSpacing);
	~WidgetListItemPortrait();

	void show();
	void hide();

	float getHeight() const;
	float getWidth() const;
	void setPosition(float x, float y, float z);

	void mouseDown(uint8 state, float x, float y);

private:
	bool activate();
	bool deactivate();
	PortraitWidget *_portrait;
	float _hSpacing;
	float _vSpacing;
	float _border;
};

class CharPortrait : public GUI {
public:
	CharPortrait(Module &module, Creature &character);
	~CharPortrait();

	void reset() const;
	void show();
	void hide();

	void setMainPortrait(Common::UString portrait);

private:
	void callbackActive(Widget &widget);
	void initPortraitList();
	void buildPortraitList();

	Module *_module;
	Creature *_character;
	PortraitWidget *_mainPortrait;
	WidgetPanel *_bigPortraitPanel;
	WidgetListBox *_portraitListBox;

	std::vector<Common::UString> _humanMalePortraits;
	std::vector<Common::UString> _dwarfMalePortraits;
	std::vector<Common::UString> _elfMalePortraits;
	std::vector<Common::UString> _halflingMalePortraits;
	std::vector<Common::UString> _halforcMalePortraits;
	std::vector<Common::UString> _gnomeMalePortraits;
	std::vector<Common::UString> _humanFemalePortraits;
	std::vector<Common::UString> _dwarfFemalePortraits;
	std::vector<Common::UString> _elfFemalePortraits;
	std::vector<Common::UString> _halflingFemalePortraits;
	std::vector<Common::UString> _halforcFemalePortraits;
	std::vector<Common::UString> _gnomeFemalePortraits;
	std::vector<std::vector<Common::UString> > _portraitsMale;
	std::vector<std::vector<Common::UString> > _portraitsFemale;

	Common::UString _selected;
};

} // End of namespace NWN

} // End of namespace Engines

#endif // ENGINES_NWN_GUI_CHARGEN_CHARPORTRAIT_H
