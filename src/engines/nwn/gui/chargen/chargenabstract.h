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

/** @file engines/nwn/gui/chargen/chargenabstract.h
 *  The abstract class for all the classes of the NWN character generator.
 */

#ifndef ENGINES_NWN_GUI_CHARGEN_CHARGENABSTRACT_H
#define ENGINES_NWN_GUI_CHARGEN_CHARGENABSTRACT_H

#include "engines/nwn/creature.h"

#include "engines/nwn/gui/gui.h"

namespace Engines {

namespace NWN {

class CharGenAbstract : public GUI {
public:
	virtual void reset() = 0;

protected:
	struct CharacterAbilities {
		uint32 classID;
		std::vector<uint32> normalFeats;
		std::vector<uint32> racialFeats;
		std::vector<uint32> classFeats;
		std::vector<uint8>  skills;
		std::vector<std::vector<uint32> > spells;
		uint8 domain[2];
		uint8 school;
		std::vector<uint8> spellsPerDay;
		uint8    baseAttackBonusChange;
		Creature *character;

		CharacterAbilities();
		CharacterAbilities(Creature * charac);
		bool hasFeat(uint32 feat);
		bool hasPrereqFeat(uint32 feat, bool isClassFeat = false);
		void setRace(uint32 race);
		void setClass(uint32 classID);
		void setFeat(uint32 feat);
		/** Set predefined skills from package. Return remaining skill point if any. */
		int8 setPackageSkill(uint16 package);
		void setPackageFeat();
		void setPackageSpell();
		void applyAbilities();
	};

	static CharacterAbilities _characterChoices;
};

} // End of namespace NWN

} // End of namespace Engines

#endif // ENGINES_NWN_GUI_CHARGEN_CHARGENABSTRACT_H
