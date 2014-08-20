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

#include "aurora/2dareg.h"
#include "aurora/2dafile.h"

#include "engines/nwn/gui/chargen/chargenabstract.h"

namespace Engines {

namespace NWN {

CharGenAbstract::CharacterAbilities CharGenAbstract::_characterChoices = CharacterAbilities();

CharGenAbstract::CharacterAbilities::CharacterAbilities() {
	classID = kClassInvalid;
	skills.assign(28, 0);
	character = 0;
	baseAttackBonusChange = 0;
}
CharGenAbstract::CharacterAbilities::CharacterAbilities(Creature *charac) {
	skills.assign(28, 0);
	for (uint it = 0; it < 28; ++it)
		skills[it] = (uint8) charac->getSkillRank(it);

	baseAttackBonusChange = 0;
	school = 0;
	spells.resize(10);
	character = charac;
}

void CharGenAbstract::CharacterAbilities::setRace(uint32 race) {
	character->setRace(race);

	// Add racial feats.
	racialFeats.clear();
	const Aurora::TwoDAFile &twodaRace     = TwoDAReg.get("racialtypes");
	const Aurora::TwoDAFile &twodaFeatRace = TwoDAReg.get(twodaRace.getRow(_characterChoices.character->getRace()).getString("FeatsTable"));

	for (uint8 it = 0; it < twodaFeatRace.getRowCount(); ++it) {
		const Aurora::TwoDARow &rowFeatRace = twodaFeatRace.getRow(it);
		racialFeats.push_back(rowFeatRace.getInt("FeatIndex"));
	}
}

void CharGenAbstract::CharacterAbilities::setClass(uint32 chosenClass) {
	classID = chosenClass;

	// Add granted class feats.
	classFeats.clear();
	const Aurora::TwoDAFile &twodaClasses = TwoDAReg.get("classes");
	const Aurora::TwoDAFile &twodaClsFeat = TwoDAReg.get(twodaClasses.getRow(chosenClass).getString("FeatsTable"));
	for (uint it = 0; it < twodaClsFeat.getRowCount(); ++it) {
		const Aurora::TwoDARow &rowFeat = twodaClsFeat.getRow(it);

		if (rowFeat.getInt("List") != 3)
			continue;

		if (rowFeat.getInt("GrantedOnLevel") != character->getLevel() + 1)
			continue;

		if (!hasFeat(rowFeat.getInt("FeatIndex")))
			classFeats.push_back(rowFeat.getInt("FeatIndex"));
	}

	// Init spell slots.
	spells.resize(character->getMaxSpellLevel(classID, true));
}

bool CharGenAbstract::CharacterAbilities::hasFeat(uint32 feat) {
	for (std::vector<uint32>::const_iterator f = normalFeats.begin(); f != normalFeats.end(); ++f)
		if (*f == feat)
			return true;

	for (std::vector<uint32>::const_iterator f = racialFeats.begin(); f != racialFeats.end(); ++f)
		if (*f == feat)
			return true;

	for (std::vector<uint32>::const_iterator f = classFeats.begin(); f != classFeats.end(); ++f)
		if (*f == feat)
			return true;

	return character->hasFeat(feat);
}

bool CharGenAbstract::CharacterAbilities::hasPrereqFeat(uint32 feat, bool isClassFeat) {
	const Aurora::TwoDAFile &twodaFeats = TwoDAReg.get("feat");
	const Aurora::TwoDARow  &row        = twodaFeats.getRow(feat);

	// Some feats have been removed. Check if it's the case.
	if (row.isEmpty("FEAT"))
		return false;

	if (!row.getInt("ALLCLASSESCANUSE") && !isClassFeat)
		return false;

	// Check abilities.
	if (row.getInt("MINSTR") > (int32) character->getAbility(kAbilityStrength))
		return false;
	if (row.getInt("MINDEX") > (int32) character->getAbility(kAbilityDexterity))
		return false;
	if (row.getInt("MININT") > (int32) character->getAbility(kAbilityIntelligence))
		return false;
	if (row.getInt("MINWIS") > (int32) character->getAbility(kAbilityWisdom))
		return false;
	if (row.getInt("MINCHA") > (int32) character->getAbility(kAbilityCharisma))
		return false;
	if (row.getInt("MINCON") > (int32) character->getAbility(kAbilityConstitution))
		return false;

	// Check if the character has the prerequisite feats.
	if (!row.isEmpty("PREREQFEAT1") && !hasFeat(row.getInt("PREREQFEAT1")))
		return false;
	if (!row.isEmpty("PREREQFEAT2") && !hasFeat(row.getInt("PREREQFEAT2")))
		return false;

	if (!row.isEmpty("OrReqFeat0")) {
		bool OrReqFeat = hasFeat(row.getInt("OrReqFeat0"));
		if (!row.isEmpty("OrReqFeat1")) {
			OrReqFeat = OrReqFeat || hasFeat(row.getInt("OrReqFeat1"));
			if (!row.isEmpty("OrReqFeat2")) {
				OrReqFeat = OrReqFeat || hasFeat(row.getInt("OrReqFeat2"));
				if (!row.isEmpty("OrReqFeat3")) {
					OrReqFeat = OrReqFeat || hasFeat(row.getInt("OrReqFeat3"));
					if (!row.isEmpty("OrReqFeat4"))
						OrReqFeat = OrReqFeat || hasFeat(row.getInt("OrReqFeat4"));
				}
			}
		}

		if (!OrReqFeat)
			return false;
	}

	// Check base bonus attack.
	uint8 newBAB = character->getBaseAttackBonus() + baseAttackBonusChange;
	if ((newBAB < row.getInt("MINATTACKBONUS")) && !row.isEmpty("MINATTACKBONUS"))
		return false;

	// Check epicness.
	if ((character->getLevel() < 20) && (row.getInt("PreReqEpic") == 1))
		return false;

	// Check minimun level.
	if ((character->getClassLevel(row.getInt("MinLevelClass")) < row.getInt("MinLevel")) && !row.isEmpty("MinLevel"))
		return false;

	// Check maximum level.
	if ((character->getLevel() >= row.getInt("MaxLevel")) && !row.isEmpty("MaxLevel"))
		return false;

	// Check skill rank.
	if (!row.isEmpty("REQSKILL")) {
		if (skills[row.getInt("REQSKILL")] == 0)
			return false;
		if ((row.getInt("ReqSkillMinRanks") > skills[row.getInt("REQSKILL")]) && !row.isEmpty("ReqSkillMinRanks"))
			return false;
	}
	if (!row.isEmpty("REQSKILL2")) {
		if (skills[row.getInt("REQSKILL2")] == 0)
			return false;
		if ((row.getInt("ReqSkillMinRanks2") > skills[row.getInt("REQSKILL2")]) && !row.isEmpty("ReqSkillMinRanks2"))
			return false;
	}

	// Check if the character already has the feat.
	if (hasFeat(feat)) {
		if (!row.getInt("GAINMULTIPLE"))
			return false;
	}

	// Check minimal spell level requirement.
	if (row.getInt("MINSPELLLVL") > character->getMaxSpellLevel(classID))
		return false;

	// Check the creature's fortitude saving throw.
	if (!row.isEmpty("MinFortSave")) {
		const Aurora::TwoDAFile &twodaClasses = TwoDAReg.get("classes");
		const Aurora::TwoDARow  &rowCls       = twodaClasses.getRow(classID);
		const Aurora::TwoDAFile &twodaSavTbl  = TwoDAReg.get(rowCls.getString("SavingThrowTable"));

		int8   diffFortSave;
		uint16 classLevel = character->getClassLevel(classID);
		if (!classLevel)
			diffFortSave = twodaSavTbl.getRow(0).getInt("FortSave");
		else
			diffFortSave = twodaSavTbl.getRow(classLevel).getInt("FortSave") - twodaSavTbl.getRow(classLevel - 1).getInt("FortSave");

		if (character->getForSaveThrow() + diffFortSave < row.getInt("MinFortSave"))
			return false;
	}


	return true;
}

void CharGenAbstract::CharacterAbilities::setFeat(uint32 feat) {
	if (hasFeat(feat))
		return;

	///TODO Handle succesor feats.
	normalFeats.push_back(feat);
}

void CharGenAbstract::CharacterAbilities::applyAbilities() {
	character->addLevel(classID);
	for (std::vector<uint32>::const_iterator f = normalFeats.begin(); f != normalFeats.end(); ++f)
		character->setFeat(*f);

	for (std::vector<uint32>::const_iterator f = racialFeats.begin(); f != racialFeats.end(); ++f)
		character->setFeat(*f);

	for (std::vector<uint32>::const_iterator f = classFeats.begin(); f != classFeats.end(); ++f)
		character->setFeat(*f);

	for (uint s = 0; s < skills.size(); ++s)
		character->setSkillRank(s, skills[s]);

	character->setBaseAttackBonus(character->getBaseAttackBonus() + baseAttackBonusChange);

	// Apply cleric domain.
	character->setDomain(classID, domain[0], domain[1]);

	///TODO apply spell slots and new spells.
}

void CharGenAbstract::CharacterAbilities::setPackageFeat() {
	///TODO Package feats.
}

int8 CharGenAbstract::CharacterAbilities::setPackageSkill(uint16 package) {
	const Aurora::TwoDAFile &twodaPck           = TwoDAReg.get("packages");
	const Aurora::TwoDAFile &twodaClasses       = TwoDAReg.get("classes");
	const Aurora::TwoDARow  &rowPck             = twodaPck.getRow(package);
	const Aurora::TwoDAFile &twodaSkillsPackage = TwoDAReg.get(rowPck.getString("SkillPref2DA"));
	const Aurora::TwoDARow  &rowClass           = twodaClasses.getRow(classID);
	const Aurora::TwoDAFile &twodaClassSkills   = TwoDAReg.get(rowClass.getString("SkillsTable"));

	// Construct a map of class skills.
	std::map<uint16, bool> classSkillMap;
	for (uint it = 0; it < twodaClassSkills.getRowCount(); ++it) {
		const Aurora::TwoDARow &rowClsSkills = twodaClassSkills.getRow(it);
		classSkillMap[rowClsSkills.getInt("SkillIndex")] = (bool) rowClsSkills.getInt("ClassSkill");
	}

	// Compute how many skill points the character has.
	uint skillPointBase = twodaClasses.getRow(classID).getInt("SkillPointBase");
	uint skillsCount    = (skillPointBase + character->getAbilityModifier(kAbilityIntelligence));

	// If the character is human, he has "Quick to Master" feat and can have an additional skill point.
	if (character->getRace() == kRaceHuman)
		skillsCount++;

	if (skillsCount < 0)
		skillsCount = 0;

	if (character->getLevel() == 0)
		skillsCount *= 4;

	uint8 skillLimit = character->getLevel() + 4;

	// We iterate on the skill package.
	for (uint r = 0; r < twodaSkillsPackage.getRowCount(); ++r) {
		const Aurora::TwoDARow &rowPckSkill = twodaSkillsPackage.getRow(r);
		if (skillsCount == 0)
			break;

		uint32 skillIndex = rowPckSkill.getInt("SKILLINDEX");
		int8 limit = skillLimit;
		uint8 gain = 1;
		if (!classSkillMap[skillIndex]) {
			if (skillsCount < 2)
				continue;

			limit = floor(skillLimit / 2);
			gain = 2;
		}

		while (skills[skillIndex] < limit && skillsCount >= gain) {
			skills[skillIndex] += gain;
			skillsCount -= gain;
		}
	}
 
	return skillsCount;
}

} // End of namespace NWN

} // End of namespace Engines
