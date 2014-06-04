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

/** @file engines/nwn/creature.cpp
 *  NWN creature.
 */

#include "common/util.h"
#include "common/maths.h"
#include "common/file.h"
#include "common/configman.h"

#include "aurora/types.h"
#include "aurora/talkman.h"
#include "aurora/resman.h"
#include "aurora/gfffile.h"
#include "aurora/2dafile.h"
#include "aurora/2dareg.h"

#include "graphics/aurora/model.h"
#include "graphics/aurora/modelnode.h"
#include "graphics/aurora/pltfile.h"

#include "events/events.h"

#include "engines/aurora/util.h"
#include "engines/aurora/model.h"

#include "engines/nwn/creature.h"
#include "engines/nwn/item.h"

#include "engines/nwn/gui/widgets/tooltip.h"

static const uint32 kBICID = MKTAG('B', 'I', 'C', ' ');

namespace Engines {

namespace NWN {

Creature::Associate::Associate(AssociateType t, Creature *a) : type(t), associate(a) {
}


Creature::BodyPart::BodyPart() : id(Aurora::kFieldIDInvalid) {
}

Creature::Creature(): Object(kObjectTypeCreature) {
	init();

	_isPC = true;
	// Set default clothes.
	Item *cloth = new Item();
	Aurora::GFFFile *uti = new Aurora::GFFFile("NW_CLOTH001", Aurora::kFileTypeUTI, MKTAG('U', 'T', 'I', ' '));
	cloth->load(uti->getTopLevel(), &uti->getTopLevel());
	_equippedItems.push_back(*cloth);
	delete uti;
}


Creature::Creature(const Aurora::GFFStruct &creature) : Object(kObjectTypeCreature) {
	init();

	load(creature);
}

Creature::Creature(const Common::UString &bic, bool local) : Object(kObjectTypeCreature) {
	init();

	loadCharacter(bic, local);
}

Creature::~Creature() {
	if (_master)
		_master->removeAssociate(*this);

	for (std::list<Associate>::iterator a = _associates.begin(); a != _associates.end(); ++a)
		a->associate->setMaster(0);

	hide();

	delete _model;
	delete _tooltip;
}

void Creature::init() {
	_lastChangedGUIDisplay = 0;

	_gender 	= kGenderNone;
	_race   	= kRaceInvalid;
	_portrait 	= "gui_po_nwnlogo_";
	_startingPackage= _race;

	_name = "John Doe";

	_isPC = false;
	_isDM = false;

	_age = 0;

	_xp = 0;

	_baseHP    = 0;
	_bonusHP   = 0;
	_currentHP = 0;

	_baseAttackBonus = 0;

	_hitDice = 0;

	_goodEvil = 0;
	_lawChaos = 0;

	_skills.assign(28, 0);

	_appearanceID = 6;
	_phenotype    = 0;

	_colorSkin    = 1;
	_colorHair    = 1;
	_colorTattoo1 = 1;
	_colorTattoo2 = 1;

	_colorMetal1 = 1;
	_colorMetal2 = 1;
	_colorLeather1 = 1;
	_colorLeather2 = 1;
	_colorCloth1 = 1;
	_colorCloth2 = 1;

	_master = 0;

	_isCommandable = true;

	_model   = 0;
	_tooltip = 0;

	for (int i = 0; i < kAbilityMAX; i++)
		_abilities[i] = 0;

	_bodyParts.resize(kBodyPartMAX);
	for (uint it = 0; it < kBodyPartMAX; ++it) {
		_bodyParts.push_back(BodyPart());
		_bodyParts[it].id = it;
		_bodyParts[it].armor_id = 1;
	}
}

void Creature::show() {
	if (_model)
		_model->show();
}

void Creature::hide() {
	leave();

	hideTooltip();

	delete _tooltip;
	_tooltip = 0;

	if (_model)
		_model->hide();
}

void Creature::setPosition(float x, float y, float z) {
	Object::setPosition(x, y, z);
	Object::getPosition(x, y, z);

	if (_model)
		_model->setPosition(x, y, z);
}

void Creature::setOrientation(float x, float y, float z) {
	Object::setOrientation(x, y, z);
	Object::getOrientation(x, y, z);

	if (_model)
		_model->setRotation(x, z, -y);
}

uint32 Creature::lastChangedGUIDisplay() const {
	return _lastChangedGUIDisplay;
}

const Common::UString &Creature::getFirstName() const {
	return _firstName;
}

const Common::UString &Creature::getLastName() const {
	return _lastName;
}

uint32 Creature::getGender() const {
	return _gender;
}

void Creature::setGender(uint32 gender) {
	_gender = gender;
}

void Creature::setRace(uint32 race) {
	_race = race;
}

void Creature::setPortrait(Common::UString portrait) {
	_portrait = portrait;
}

void Creature::replaceArmor(const Common::UString &armor) {
	///TODO Remove every equip item as Item::isArmor is broken.

	  _equippedItems.clear();
	Item *armorItem = new Item();
	Aurora::GFFFile *uti = new Aurora::GFFFile(armor, Aurora::kFileTypeUTI, MKTAG('U', 'T', 'I', ' '));
	armorItem->load(uti->getTopLevel(), &uti->getTopLevel());
	_equippedItems.push_back(*armorItem);
	delete uti;
}

void Creature::setAppearance(uint32 appearance) {
	_appearanceID = appearance;
}

void Creature::setPhenotype(uint32 phenotype) {
	_phenotype = phenotype;
}

bool Creature::isFemale() const {
	// Male and female are hardcoded.  Other genders (none, both, other)
	// count as male when it comes to tokens in text strings.

	return _gender == Aurora::kGenderFemale;
}

uint32 Creature::getRace() const {
	return _race;
}

bool Creature::isPC() const {
	return _isPC;
}

bool Creature::isDM() const {
	return _isDM;
}

uint32 Creature::getAge() const {
	return _age;
}

uint32 Creature::getXP() const {
	return _xp;
}

int32 Creature::getCurrentHP() const {
	return _currentHP + _bonusHP;
}

int32 Creature::getMaxHP() const {
	return _baseHP + _bonusHP;
}

void Creature::addAssociate(Creature &henchman, AssociateType type) {
	removeAssociate(henchman);

	assert(!henchman.getMaster());

	_associates.push_back(Associate(type, &henchman));
	henchman.setMaster(this);
}

void Creature::removeAssociate(Creature &henchman) {
	for (std::list<Associate>::iterator a = _associates.begin(); a != _associates.end(); ++a) {
		if (a->associate == &henchman) {
			assert(a->associate->getMaster() == this);

			a->associate->setMaster(0);
			_associates.erase(a);
			break;
		}
	}
}

Creature *Creature::getAssociate(AssociateType type, int nth) const {
	if (_associates.empty())
		return 0;

	Creature *curAssociate = 0;

	std::list<Associate>::const_iterator associate = _associates.begin();
	while (nth-- > 0) {
		while ((associate != _associates.end()) && (associate->type != type))
			++associate;

		if (associate == _associates.end())
			return 0;

		curAssociate = associate->associate;
	}

	return curAssociate;
}

void Creature::setMaster(Creature *master) {
	_master = master;
}

Creature *Creature::getMaster() const {
	return _master;
}

bool Creature::isCommandable() const {
	return _isCommandable;
}

void Creature::setCommandable(bool commandable) {
	_isCommandable = commandable;
}

void Creature::constructPartName(const Common::UString &type, uint32 id,
		const Common::UString &gender, const Common::UString &race,
		const Common::UString &phenoType, Common::UString &part) {

	part = Common::UString::sprintf("p%s%s%s_%s%03d",
	       gender.c_str(), race.c_str(), phenoType.c_str(), type.c_str(), id);
}

void Creature::constructPartName(const Common::UString &type, uint32 id,
		const Common::UString &gender, const Common::UString &race,
		const Common::UString &phenoType, const Common::UString &phenoTypeAlt,
		Aurora::FileType fileType, Common::UString &part) {

	constructPartName(type, id, gender, race, phenoType, part);
	if ((fileType == Aurora::kFileTypeNone) || ResMan.hasResource(part, fileType))
		return;

	constructPartName(type, id, gender, race, phenoTypeAlt, part);
	if (!ResMan.hasResource(part, fileType))
		part.clear();
}

void Creature::constructModelName(const Common::UString &type, uint32 id,
		const Common::UString &gender, const Common::UString &race,
		const Common::UString &phenoType, const Common::UString &phenoTypeAlt,
		Common::UString &model, Common::UString &texture) {

	constructPartName(type, id, gender, race, phenoType, phenoTypeAlt, Aurora::kFileTypeMDL, model);

	constructPartName(type, id, gender, race, phenoType, phenoTypeAlt, Aurora::kFileTypePLT, texture);

	// PLT texture doesn't exist, try a generic human PLT
	if (texture.empty())
		constructPartName(type, id, gender, "H", phenoType, phenoTypeAlt, Aurora::kFileTypePLT, texture);

	// Human PLT texture doesn't exist either, assume it's a non-PLT texture
	if (texture.empty())
		constructPartName(type, id, gender, race, phenoType, phenoTypeAlt, Aurora::kFileTypeNone, texture);
}

// Based on filenames in model2.bif
// These should be read from MDLNAME, NODENAME in capart.2da (in 2da.bif)
static const char *kBodyPartModels[] = {
	"head"  ,
	"neck"  ,
	"chest" ,
	"pelvis",
	"belt"  ,
	"footr" , "footl" ,
	"shinr" , "shinl" ,
	"legl"  , "legr"  ,
	"forer" , "forel" ,
	"bicepr", "bicepl",
	"shor"  , "shol"  ,
	"handr" , "handl"
};

// Node names taken from pfa0.mdl
static const char *kBodyPartNodes[] = {
	"head_g"     ,
	"neck_g"     ,
	"torso_g"    ,
	"pelvis_g"   ,
	"belt_g"     ,
	"rfoot_g"    , "lfoot_g"    ,
	"rshin_g"    , "lshin_g"    ,
	"lthigh_g"   , "rthigh_g"   ,
	"rforearm_g" , "lforearm_g" ,
	"rbicep_g"   , "lbicep_g"   ,
	"rshoulder_g", "lshoulder_g",
	"rhand_g"    , "lhand_g"
};

void Creature::getPartModels() {
	const Aurora::TwoDAFile &appearance = TwoDAReg.get("appearance");

	const Aurora::TwoDARow &gender = TwoDAReg.get("gender").getRow(_gender);
	const Aurora::TwoDARow &race   = TwoDAReg.get("racialtypes").getRow(_race);
	const Aurora::TwoDARow &raceAp = appearance.getRow(race.getInt("Appearance"));
	const Aurora::TwoDARow &pheno  = TwoDAReg.get("phenotype").getRow(_phenotype);

	Common::UString genderChar   = gender.getString("GENDER");
	Common::UString raceChar     = raceAp.getString("RACE");
	Common::UString phenoChar    = Common::UString::sprintf("%d", _phenotype);
	Common::UString phenoAltChar = pheno.getString("DefaultPhenoType");

	// Important to capture the supermodel
	_partsSuperModelName = Common::UString::sprintf("p%s%s%s",
	                       genderChar.c_str(), raceChar.c_str(), phenoChar.c_str());

	// Fall back to the default phenotype if required
	if (!ResMan.hasResource(_partsSuperModelName, Aurora::kFileTypeMDL))
		_partsSuperModelName = Common::UString::sprintf("p%s%s%s",
		                       genderChar.c_str(), raceChar.c_str(), phenoAltChar.c_str());

	for (uint i = 0; i < kBodyPartMAX; i++)
		constructModelName(kBodyPartModels[i], _bodyParts[i].armor_id > 0 ? _bodyParts[i].armor_id : _bodyParts[i].id,
		                   genderChar, raceChar, phenoChar, phenoAltChar,
		                   _bodyParts[i].modelName, _bodyParts[i].texture);
}

void Creature::getArmorModels() {
	for (std::vector<Item>::iterator e = _equippedItems.begin(); e != _equippedItems.end(); ++e) {
		Item item = *e;
		if (!item.isArmor()) {
			continue;
		}
		status("Equipping armour \"%s\" on model \"%s\"", item.getName().c_str(), _tag.c_str());

		// Set the body part models
		for (uint i = 0; i < kBodyPartMAX; i++) {
			int id = item.getArmorPart(i);
			if (id > 0)
			_bodyParts[i].armor_id = id;
		}
		// Set the armour color channels
		_colorMetal1 = item._colorMetal1;
		_colorMetal2 = item._colorMetal2;
		_colorLeather1 = item._colorLeather1;
		_colorLeather2 = item._colorLeather2;
		_colorCloth1 = item._colorCloth1;
		_colorCloth2 = item._colorCloth2;
	}
}

void Creature::finishPLTs(std::list<Graphics::Aurora::PLTHandle> &plts) {
	for (std::list<Graphics::Aurora::PLTHandle>::iterator p = plts.begin();
	     p != plts.end(); ++p) {

		Graphics::Aurora::PLTFile &plt = p->getPLT();

		plt.setLayerColor(Graphics::Aurora::PLTFile::kLayerSkin   , _colorSkin);
		plt.setLayerColor(Graphics::Aurora::PLTFile::kLayerHair   , _colorHair);
		plt.setLayerColor(Graphics::Aurora::PLTFile::kLayerTattoo1, _colorTattoo1);
		plt.setLayerColor(Graphics::Aurora::PLTFile::kLayerTattoo2, _colorTattoo2);
		plt.setLayerColor(Graphics::Aurora::PLTFile::kLayerMetal1, _colorMetal1);
		plt.setLayerColor(Graphics::Aurora::PLTFile::kLayerMetal2, _colorMetal2);
		plt.setLayerColor(Graphics::Aurora::PLTFile::kLayerLeather1, _colorLeather1);
		plt.setLayerColor(Graphics::Aurora::PLTFile::kLayerLeather2, _colorLeather2);
		plt.setLayerColor(Graphics::Aurora::PLTFile::kLayerCloth1, _colorCloth1);
		plt.setLayerColor(Graphics::Aurora::PLTFile::kLayerCloth2, _colorCloth2);

		plt.rebuild();
	}
}

void Creature::loadModel() {
	if (_model)
		return;

	if (_appearanceID == Aurora::kFieldIDInvalid) {
		warning("Creature \"%s\" has no appearance", _tag.c_str());
		return;
	}

	const Aurora::TwoDARow &appearance = TwoDAReg.get("appearance").getRow(_appearanceID);

	if (_portrait.empty())
		_portrait = appearance.getString("PORTRAIT");

	if (appearance.getString("MODELTYPE") == "P") {
		getArmorModels();
		getPartModels();
		_model = loadModelObject(_partsSuperModelName);

		for (uint i = 0; i < kBodyPartMAX; i++) {
			if (_bodyParts[i].modelName.empty())
				continue;

			TextureMan.clearNewPLTs();

			// Try to load in the corresponding part model
			Graphics::Aurora::Model *part_model = loadModelObject(_bodyParts[i].modelName, _bodyParts[i].texture);
			if (!part_model)
				continue;

			// Add the loaded model to the appropriate part node
			Graphics::Aurora::ModelNode *part_node = _model->getNode(kBodyPartNodes[i]);
			if (part_node)
				part_node->addChild(part_model);

			TextureMan.getNewPLTs(_bodyParts[i].plts);

			finishPLTs(_bodyParts[i].plts);
		}

	} else
		_model = loadModelObject(appearance.getString("RACE"));

	// Positioning

	float x, y, z;

	getPosition(x, y, z);
	setPosition(x, y, z);

	getOrientation(x, y, z);
	setOrientation(x, y, z);

	// Clickable

	if (_model) {
		_model->setTag(_tag);
		_model->setClickable(isClickable());

		_ids.push_back(_model->getID());
	}
}

void Creature::unloadModel() {
	hide();

	delete _model;
	_model = 0;
}

void Creature::loadCharacter(const Common::UString &bic, bool local) {
	Aurora::GFFFile *gff = openPC(bic, local);

	try {
		load(gff->getTopLevel(), 0);
	} catch (...) {
		delete gff;
		throw;
	}

	// All BICs should be PCs.
	_isPC = true;

	// Set the PC tag to something recognizable for now.
	// Let's hope no script depends on it being "".

	_tag = Common::UString::sprintf("[PC: %s]", _name.c_str());

	_lastChangedGUIDisplay = EventMan.getTimestamp();
}

void Creature::load(const Aurora::GFFStruct &creature) {
	Common::UString temp = creature.getString("TemplateResRef");

	Aurora::GFFFile *utc = 0;
	if (!temp.empty()) {
		try {
			utc = new Aurora::GFFFile(temp, Aurora::kFileTypeUTC, MKTAG('U', 'T', 'C', ' '));
		} catch (...) {
			delete utc;
		}
	}

	load(creature, utc ? &utc->getTopLevel() : 0);

	delete utc;

	_lastChangedGUIDisplay = EventMan.getTimestamp();
}

void Creature::load(const Aurora::GFFStruct &instance, const Aurora::GFFStruct *blueprint) {
	// General properties

	if (blueprint)
		loadProperties(*blueprint); // Blueprint
	loadProperties(instance);    // Instance

	// Position

	setPosition(instance.getDouble("XPosition"),
	            instance.getDouble("YPosition"),
	            instance.getDouble("ZPosition"));

	// Orientation

	float bearingX = instance.getDouble("XOrientation");
	float bearingY = instance.getDouble("YOrientation");

	float o[3];
	Common::vector2orientation(bearingX, bearingY, o[0], o[1], o[2]);

	setOrientation(o[0], o[1], o[2]);
}

static const char *kBodyPartFields[] = {
	"Appearance_Head",
	"BodyPart_Neck"  ,
	"BodyPart_Torso" ,
	"BodyPart_Pelvis",
	"BodyPart_Belt"  ,
	"ArmorPart_RFoot", "BodyPart_LFoot" ,
	"BodyPart_RShin" , "BodyPart_LShin" ,
	"BodyPart_LThigh", "BodyPart_RThigh",
	"BodyPart_RFArm" , "BodyPart_LFArm" ,
	"BodyPart_RBicep", "BodyPart_LBicep",
	"BodyPart_RShoul", "BodyPart_LShoul",
	"BodyPart_RHand" , "BodyPart_LHand"
};

void Creature::loadProperties(const Aurora::GFFStruct &gff) {
	// Tag

	_tag = gff.getString("Tag", _tag);

	// Name

	if (gff.hasField("FirstName")) {
		Aurora::LocString firstName;
		gff.getLocString("FirstName", firstName);

		_firstName = firstName.getString();
	}

	if (gff.hasField("LastName")) {
		Aurora::LocString lastName;
		gff.getLocString("LastName", lastName);

		_lastName = lastName.getString();
	}

	_name = _firstName + " " + _lastName;
	_name.trim();

	// Description

	if (gff.hasField("Description")) {
		Aurora::LocString description;
		gff.getLocString("Description", description);

		_description = description.getString();
	}

	// Conversation

	_conversation = gff.getString("Conversation", _conversation);

	// Sound Set

	_soundSet = gff.getUint("SoundSetFile", Aurora::kFieldIDInvalid);

	// Portrait

	loadPortrait(gff, _portrait);

	// Gender
	_gender = gff.getUint("Gender", _gender);

	// Race
	_race = gff.getUint("Race", _race);

	// Subrace
	_subRace = gff.getString("Subrace", _subRace);

	// PC and DM
	_isPC = gff.getBool("IsPC", _isPC);
	_isDM = gff.getBool("IsDM", _isDM);

	// Age
	_age = gff.getUint("Age", _age);

	// Experience
	_xp = gff.getUint("Experience", _xp);

	// Starting package
	_startingPackage = gff.getUint("StartingPackage", _startingPackage);

	// Base attack bonus
	_baseAttackBonus = gff.getUint("BaseAttackBonus", _baseAttackBonus);

	// Abilities
	_abilities[kAbilityStrength]     = gff.getUint("Str", _abilities[kAbilityStrength]);
	_abilities[kAbilityDexterity]    = gff.getUint("Dex", _abilities[kAbilityDexterity]);
	_abilities[kAbilityConstitution] = gff.getUint("Con", _abilities[kAbilityConstitution]);
	_abilities[kAbilityIntelligence] = gff.getUint("Int", _abilities[kAbilityIntelligence]);
	_abilities[kAbilityWisdom]       = gff.getUint("Wis", _abilities[kAbilityWisdom]);
	_abilities[kAbilityCharisma]     = gff.getUint("Cha", _abilities[kAbilityCharisma]);

	// Classes
	loadClasses(gff, _classes, _hitDice);

	// Skills
	if (gff.hasField("SkillList")) {
		_skills.clear();

		const Aurora::GFFList &skills = gff.getList("SkillList");
		for (Aurora::GFFList::const_iterator s = skills.begin(); s != skills.end(); ++s) {
			const Aurora::GFFStruct &skill = **s;

			_skills.push_back(skill.getSint("Rank"));
		}
	}

	// Feats
	if (gff.hasField("FeatList")) {
		_feats.clear();

		const Aurora::GFFList &feats = gff.getList("FeatList");
		for (Aurora::GFFList::const_iterator f = feats.begin(); f != feats.end(); ++f) {
			const Aurora::GFFStruct &feat = **f;

			_feats.push_back(feat.getUint("Feat"));
		}
	}

	// Deity
	_deity = gff.getString("Deity", _deity);

	// Health
	if (gff.hasField("HitPoints")) {
		_baseHP    = gff.getSint("HitPoints");
		_bonusHP   = gff.getSint("MaxHitPoints", _baseHP) - _baseHP;
		_currentHP = gff.getSint("CurrentHitPoints", _baseHP);
	}

	// Alignment

	_goodEvil = gff.getUint("GoodEvil", _goodEvil);
	_lawChaos = gff.getUint("LawfulChaotic", _lawChaos);

	// Appearance

	_appearanceID = gff.getUint("Appearance_Type", _appearanceID);
	_phenotype    = gff.getUint("Phenotype"      , _phenotype);

	// Body parts
	for (uint i = 0; i < kBodyPartMAX; i++) {
		_bodyParts[i].id = gff.getUint(kBodyPartFields[i], _bodyParts[i].id);
		_bodyParts[i].armor_id = 0;
	}

	// Colors
	_colorSkin    = gff.getUint("Color_Skin", _colorSkin);
	_colorHair    = gff.getUint("Color_Hair", _colorHair);
	_colorTattoo1 = gff.getUint("Color_Tattoo1", _colorTattoo1);
	_colorTattoo2 = gff.getUint("Color_Tattoo2", _colorTattoo2);

	// Equipped Items
	loadEquippedItems(gff);

	// Scripts
	readScripts(gff);
}

void Creature::loadPortrait(const Aurora::GFFStruct &gff, Common::UString &portrait) {
	uint32 portraitID = gff.getUint("PortraitId");
	if (portraitID != 0) {
		const Aurora::TwoDAFile &twoda = TwoDAReg.get("portraits");

		Common::UString portrait2DA = twoda.getRow(portraitID).getString("BaseResRef");
		if (!portrait2DA.empty())
			portrait = "po_" + portrait2DA;
	}

	portrait = gff.getString("Portrait", portrait);
}

void Creature::loadEquippedItems(const Aurora::GFFStruct &gff) {
	if (!gff.hasField("Equip_ItemList"))
		return;

	const Aurora::GFFList &cEquipped = gff.getList("Equip_ItemList");
	for (Aurora::GFFList::const_iterator e = cEquipped.begin(); e != cEquipped.end(); ++e) {
		const Aurora::GFFStruct &cItem = **e;

		Common::UString itemref = cItem.getString("EquippedRes");
		if (itemref.empty())
			itemref = cItem.getString("TemplateResRef");

		Aurora::GFFFile *uti = 0;
		if (!itemref.empty()) {
			try {
				uti = new Aurora::GFFFile(itemref, Aurora::kFileTypeUTI, MKTAG('U', 'T', 'I', ' '));
			} catch (...) {
				delete uti;
			}
		}

		// Load the item and add it to the equipped list
		_equippedItems.push_back(Item());
		_equippedItems.back().load(cItem, uti ? &uti->getTopLevel() : 0);

		delete uti;
	}

}

void Creature::loadClasses(const Aurora::GFFStruct &gff,
                           std::vector<Class> &classes, uint8 &hitDice) {

	if (!gff.hasField("ClassList"))
		return;

	classes.clear();
	hitDice = 0;

	const Aurora::GFFList &cClasses = gff.getList("ClassList");
	for (Aurora::GFFList::const_iterator c = cClasses.begin(); c != cClasses.end(); ++c) {
		classes.push_back(Class());

		const Aurora::GFFStruct &cClass = **c;

		classes.back().classID = cClass.getUint("Class");
		classes.back().level   = cClass.getUint("ClassLevel");

		hitDice += classes.back().level;
	}
}

const Common::UString &Creature::getConvRace() const {
	const uint32 strRef = TwoDAReg.get("racialtypes").getRow(_race).getInt("ConverName");

	return TalkMan.getString(strRef);
}

const Common::UString &Creature::getConvrace() const {
	const uint32 strRef = TwoDAReg.get("racialtypes").getRow(_race).getInt("ConverNameLower");

	return TalkMan.getString(strRef);
}

const Common::UString &Creature::getConvRaces() const {
	const uint32 strRef = TwoDAReg.get("racialtypes").getRow(_race).getInt("NamePlural");

	return TalkMan.getString(strRef);
}

const Common::UString &Creature::getSubRace() const {
	return _subRace;
}

void Creature::getClass(uint32 position, uint32 &classID, uint16 &level) const {
	if (position >= _classes.size()) {
		classID = kClassInvalid;
		level   = 0;
		return;
	}

	classID = _classes[position].classID;
	level   = _classes[position].level;
}
 
Uint16 Creature::getLevel() const {
	Uint16 level = 0;
	for (std::vector<Class>::const_iterator it = _classes.begin(); it != _classes.end(); ++it)
		level += (*it).level;

	return level;
}

uint16 Creature::getClassLevel(uint32 classID) const {
	for (std::vector<Class>::const_iterator c = _classes.begin(); c != _classes.end(); ++c)
		if (c->classID == classID)
			return c->level;

	return 0;
}
 
uint32 Creature::getLastClass() const {
	if (getLevel() > 0)
		return _lastClass->classID;

	return kClassInvalid;
}

bool Creature::addLevel(uint32 className) {
	///TODO Check if we have reach the level limit
	for (std::vector<Class>::iterator it = _classes.begin(); it != _classes.end(); ++it) {
		if ((*it).classID == className) {
			(*it).level++;
			_lastClass = it;
			return true;
		}
	}
	_classes.push_back(Class(className,1));
	_lastClass = --(_classes.end());
	return true;
}

void Creature::removeLastLevel() {
	_lastClass->level--;
}

const Common::UString &Creature::getConvClass() const {
	const uint32 classID = _classes.front().classID;
	const uint32 strRef  = TwoDAReg.get("classes").getRow(classID).getInt("Name");

	return TalkMan.getString(strRef);
}

const Common::UString &Creature::getConvclass() const {
	const uint32 classID = _classes.front().classID;
	const uint32 strRef  = TwoDAReg.get("classes").getRow(classID).getInt("Lower");

	return TalkMan.getString(strRef);
}

const Common::UString &Creature::getConvClasses() const {
	const uint32 classID = _classes.front().classID;
	const uint32 strRef  = TwoDAReg.get("classes").getRow(classID).getInt("Plural");

	return TalkMan.getString(strRef);
}

const Common::UString &Creature::getDeity() const {
	return _deity;
}

uint8 Creature::getGoodEvil() const {
	return _goodEvil;
}
 
void Creature::setGoodEvil(uint8 point) {
	_goodEvil = point;
}

uint8 Creature::getLawChaos() const {
	return _lawChaos;
}
 
void Creature::setLawChaos(uint8 point) {
	_lawChaos = point;
}

Common::UString Creature::getClassString() const {
	Common::UString classString;

	getClassString(_classes, classString);

	return classString;
}

uint8 Creature::getHitDice() const {
	return _hitDice;
}

uint8 Creature::getAbility(Ability ability) const {
	assert((ability >= 0) && (ability < kAbilityMAX));

	return _abilities[ability];
}

int8 Creature::getAbilityModifier(Ability ability) const {
	const Aurora::TwoDAFile &twodaRace = TwoDAReg.get("racialtypes");
	const Aurora::TwoDAFile &twodaAbilities = TwoDAReg.get("iprp_abilities");
	int racialModifier = twodaRace.getRow(_race).getInt(twodaAbilities.getRow(_race).getString("Label") + "Adjust");

	uint realValue = _abilities[ability] + racialModifier;
	realValue -= 6;
	int8 modifier = (realValue - realValue % 2) / 2;
	modifier -= 2;

	return modifier;
}

void Creature::setAbility(int8 ability, uint8 score) {
	_abilities[ability] = score;
}

int8 Creature::getSkillRank(uint32 skill) const {
	if (skill >= _skills.size())
		return -1;

	return _skills[skill];
}

void Creature::setSkillRank(uint32 skill, int8 rank) {
	_skills[skill] = rank;
}

void Creature::setFeat(uint32 feat) {
	if (hasFeat(feat))
		return;

	_feats.push_back(feat);
}

bool Creature::hasFeat(uint32 feat) const {
	for (std::vector<uint32>::const_iterator f = _feats.begin(); f != _feats.end(); ++f)
		if (*f == feat)
			return true;

	return false;
}

void Creature::addSpellLevel(uint32 classID, uint8 spellType) {
	for (std::vector<Class>::iterator it = _classes.begin(); it != _classes.end(); ++it) {
		if (it->classID == classID) {
			std::vector<Spell> newSpellLevel;
			if (spellType & 0x01) {
				it->knownList.push_back(newSpellLevel);
			}
			if (spellType & 0x02) {
				it->memorizedList.push_back(newSpellLevel);
			}
		}
	}
}

void Creature::addSpell(uint32 classID, uint16 spell, uint16 spellLevel, uint8 spellType, uint8 spellFlag, uint8 metamagicType) {
	for (std::vector<Engines::NWN::Creature::Class>::iterator cl = _classes.begin(); cl != _classes.end(); ++cl) {
		if (cl->classID != classID)
			continue;

		if (spellType & 0x01) {
			Spell newSpell;
			newSpell.spell = spell;
			newSpell.spellFlags = spellFlag;
			newSpell.spellMetaMagic = metamagicType;

			for (std::vector<Class>::iterator it = _classes.begin(); it != _classes.end(); ++it) {
				if (it->classID != classID)
					continue;

				if (hasSpell(spell, spellLevel, 0x01, classID))
					continue;

				it->knownList[spellLevel].push_back(newSpell);
			}
		}

		if (spellType & 0x02) {
			Spell newSpell;
			newSpell.spell = spell;
			newSpell.spellFlags = spellFlag;
			newSpell.spellMetaMagic = metamagicType;

			for (std::vector<Class>::iterator it = _classes.begin(); it != _classes.end(); ++it) {
				if (it->classID != classID)
					continue;
				
				if (hasSpell(spell, spellLevel, 0x02, classID))
					continue;
				
				it->memorizedList[spellLevel].push_back(newSpell);
			}
		}
	}
}

bool Creature::hasSpell(uint16 spell, uint16 spellLevel,uint8 spellType, uint32 classID) {
	for (std::vector<Engines::NWN::Creature::Class>::iterator cl = _classes.begin(); cl != _classes.end(); ++cl) {
		if (classID == cl->classID || classID == kClassInvalid) {
			// Look into the memorized list.
			if (spellType & 0x02) {
				for (std::vector<Spell>::iterator sp = cl->memorizedList[spellLevel].begin(); sp != cl->memorizedList[spellLevel].end(); ++sp) {
					if (sp->spell == spell)
						return true;
				}
			}

			// Look into the known list.
			if (spellType & 0x01) {
				for (std::vector<Spell>::iterator sp = cl->knownList[spellLevel].begin(); sp != cl->knownList[spellLevel].end(); ++sp)
					if (sp->spell == spell)
						return true;
			}
		}
	}

	return false;
}

uint16 Creature::getMaxSpellLevel(uint32 specificClass, bool addLevelToClass) {
	uint16 levelMax = 0;

	// If we are level 0.
	if (getLevel() == 0 && addLevelToClass) {
		const Aurora::TwoDAFile &twodaClasses = TwoDAReg.get("classes");
		const Aurora::TwoDARow &rowClass = twodaClasses.getRow(specificClass);

		if (rowClass.isEmpty("SpellGainTable"))
			return levelMax;

		const Aurora::TwoDAFile &twodaClsSpell = TwoDAReg.get(rowClass.getString("SpellGainTable"));
		const Aurora::TwoDARow &rowSpellTable = twodaClsSpell.getRow(0);
		return rowSpellTable.getInt("NumSpellLevels") - 1;
	}

	for (std::vector<Class>::iterator it = _classes.begin(); it != _classes.end(); ++it) {
		uint16 spellLevel;
		if (it->classID == specificClass) {
			const Aurora::TwoDAFile &twodaClasses = TwoDAReg.get("classes");
			const Aurora::TwoDARow &rowClass = twodaClasses.getRow(specificClass);
			const Aurora::TwoDAFile &twodaClsSpell = TwoDAReg.get(rowClass.getString("SpellGainTable"));
			uint level = it->level;

			if (addLevelToClass)
				level++;

			const Aurora::TwoDARow &rowSpellTable = twodaClsSpell.getRow(level);

			return (rowSpellTable.getInt("NumSpellLevels") - 1);
		} else {
			spellLevel = MAX<uint16>(it->knownList.size(), it->memorizedList.size());
		}

		levelMax = MAX<uint16>(levelMax, spellLevel);
	}

	return levelMax;
}

void Creature::setPackage(uint8 package) {
	_startingPackage = package;
}

uint8 Creature::getPackage() const {
	return _startingPackage;
}

void Creature::setDomain(uint32 classID, uint8 firstDomain, uint8 secondDomain) {
	for (std::vector<Class>::iterator it = _classes.begin(); it != _classes.end(); ++it) {
		if (classID != it->classID)
			continue;

		it->domain1 = firstDomain;
		it->domain2 = secondDomain;
	}
	
}

uint8 Creature::getBaseAttackBonus() const {
	return _baseAttackBonus;
}

void Creature::setBaseAttackBonus(uint8 bab) {
	_baseAttackBonus = bab;
}

int8 Creature::getForSaveThrow() const {
	///TODO Add modifier from effects, such as from spells or items.
	return _forSaveThrow + getAbilityModifier(kAbilityConstitution);
}

int8 Creature::getWillSaveThrow() const {
	///TODO Add modifier from effects, such as from spells or items.
	return _willSaveThrow + getAbilityModifier(kAbilityWisdom);
}

int8 Creature::getRefSaveThrow() const {
	///TODO Add modifier from effects, such as from spells or items.
	return _refSaveThrow + getAbilityModifier(kAbilityDexterity);
}

void Creature::enter() {
	highlight(true);
}

void Creature::leave() {
	highlight(false);
}

void Creature::highlight(bool enabled) {
	if (_model)
		_model->drawBound(enabled);

	if (enabled)
		showTooltip();
	else
		hideTooltip();
}

bool Creature::click(Object *triggerer) {
	// Try the onDialog script first
	if (hasScript(kScriptDialogue))
		return runScript(kScriptDialogue, this, triggerer);

	// Next, look we have a generic onClick script
	if (hasScript(kScriptClick))
		return runScript(kScriptClick, this, triggerer);

	// Lastly, try to start a conversation directly
	return beginConversation(triggerer);
}

void Creature::createTooltip() {
	if (_tooltip)
		return;

	_tooltip = new Tooltip(Tooltip::kTypeFeedback, *_model);

	_tooltip->setAlign(0.5);
	_tooltip->addLine(_name, 0.5, 0.5, 1.0, 1.0);
	_tooltip->setPortrait(_portrait);
}

void Creature::showTooltip() {
	createTooltip();
	_tooltip->show();
}

void Creature::hideTooltip() {
	if (!_tooltip)
		return;

	_tooltip->hide();
}

void Creature::playAnimation(const Common::UString &animation, bool restart, int32 loopCount) {
	if (!_model)
		return;

	if (animation.empty()) {
		_model->playDefaultAnimation();
		return;
	}

	_model->playAnimation(animation, restart, loopCount);
}

void Creature::getPCListInfo(const Common::UString &bic, bool local,
                             Common::UString &name, Common::UString &classes,
                             Common::UString &portrait) {

	Aurora::GFFFile *gff = openPC(bic, local);

	try {
		const Aurora::GFFStruct &top = gff->getTopLevel();

		// Reading name
		Aurora::LocString firstName;
		top.getLocString("FirstName", firstName);

		Aurora::LocString lastName;
		top.getLocString("LastName", lastName);

		name = firstName.getString() + " " + lastName.getString();

		// Reading portrait (failure non-fatal)
		try {
			loadPortrait(top, portrait);
		} catch (Common::Exception &e) {
			portrait.clear();

			e.add("Can't read portrait for PC \"%s\"", bic.c_str());
			Common::printException(e, "WARNING: ");
		}

		// Reading classes
		std::vector<Class> classLevels;
		uint8 hitDice;

		loadClasses(top, classLevels, hitDice);
		getClassString(classLevels, classes);

		classes = "(" + classes + ")";

	} catch (...) {
		delete gff;
		throw;
	}

	delete gff;
}

Aurora::GFFFile *Creature::openPC(const Common::UString &bic, bool local) {
	Common::UString pcDir  = ConfigMan.getString(local ? "NWN_localPCDir" : "NWN_serverPCDir");
	Common::UString pcFile = pcDir + "/" + bic + ".bic";

	Common::File *pc = 0;
	try {
		pc = new Common::File(pcFile);
	} catch (...) {
		delete pc;
		throw;
	}

	Aurora::GFFFile *gff = 0;
	try {
		gff = new Aurora::GFFFile(pc, kBICID);
	} catch (...) {
		delete gff;
		throw;
	}

	return gff;
}

void Creature::getClassString(const std::vector<Class> &classes, Common::UString &str) {
	for (std::vector<Class>::const_iterator c = classes.begin(); c != classes.end(); ++c) {
		if (!str.empty())
			str += '/';

		uint32 strRef = TwoDAReg.get("classes").getRow(c->classID).getInt("Name");

		str += TalkMan.getString(strRef);
	}
}

} // End of namespace NWN

} // End of namespace Engines
