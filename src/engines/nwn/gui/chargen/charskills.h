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

/** @file engines/nwn/gui/chargen/charskills.h
 *  The NWN skills selection for the character generator.
 */

#ifndef ENGINES_NWN_GUI_CHARGEN_CHARSKILLS_H
#define ENGINES_NWN_GUI_CHARGEN_CHARSKILLS_H

#include "engines/nwn/creature.h"

#include "engines/nwn/gui/widgets/listbox.h"
#include "engines/nwn/gui/widgets/listitembutton.h"
#include "engines/nwn/gui/widgets/panel.h"
#include "engines/nwn/gui/widgets/label.h"
#include "engines/nwn/gui/widgets/button.h"

#include "engines/nwn/gui/chargen/chargenabstract.h"

namespace Engines {

namespace NWN {

class WidgetListItemSkill : public WidgetListItemButton {
friend class CharSkills;
public:
	WidgetListItemSkill(::Engines::GUI &gui, const Common::UString &model, const Common::UString &text, const Common::UString &icon, const Common::UString &tag, uint32 skillIndex, uint16 initialSkill, bool classSkill);
	~WidgetListItemSkill();

	void subActive(Widget &widget);
	/** Change the skill value, default is +1. **/
	void changeSkill(int8 amount = 1);
	/** Return the skill value. **/
	uint8 getSkillRank() const;
	void setSkillRank(uint8 skillRank);
	uint32 getSkillIndex() const;

	void reset();

private:
	void updateText();
	WidgetButton *_upButton;
	WidgetButton *_downButton;
	WidgetPanel *_skillPointPanel;
	WidgetLabel *_skillPoint;

	uint8 _skillRank;
	uint8 _initialSkillValue;
	uint32 _skillIndex;
	bool _classSkill;
};

class CharSkills : public CharGenAbstract {
public:
	CharSkills(Creature &character, uint8 package = 200);
	~CharSkills();

	void show();
	void reset();

	bool changeSkillPoints(int8 amount, uint initialRank, bool isClassSkill);
	void setHelp(uint8 skillItem);

	void fixWidgetType(const Common::UString &tag, WidgetType &type);

private:
	void callbackActive(Widget &widget);
	Creature *_character;

	WidgetListBox *_skillList;
	std::vector<Common::UString> _helpTexts;
	std::vector<Common::UString> _skillNames;

	uint32 _skillPointsLeft;
	uint8 _package;
};

} // End of namespace NWN

} // End of namespace Engines

#endif // ENGINES_NWN_GUI_CHARGEN_CHARSKILLS_H
