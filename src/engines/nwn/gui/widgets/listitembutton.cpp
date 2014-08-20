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

/** @file engines/nwn/gui/widgets/listitembutton.cpp
 *  Button widgets within a NWN listbox widget with at least a text and an icon.
 */

#include "graphics/aurora/model.h"
#include "graphics/aurora/text.h"

#include "graphics/aurora/modelnode.h"
#include "engines/aurora/model.h"
#include "engines/aurora/util.h"

#include "engines/nwn/gui/widgets/button.h"

#include "engines/nwn/gui/widgets/listitembutton.h"

namespace Engines {

namespace NWN {

WidgetListItemButton::WidgetListItemButton(::Engines::GUI &gui, const Common::UString &model, const Common::UString &font, const Common::UString &text, const Common::UString &icon): WidgetListItem(gui), _available(true) {
	_buttonItem = loadModelGUI(model);
	_buttonItem->setClickable(true);
	_text = new Graphics::Aurora::Text(FontMan.get(font), text);
// 	if (icon == "")
	if (true) {
		_icon = 0;
	} else {
		_icon = new PortraitWidget(gui, icon + "#icon", icon, Portrait::kSizeIcon);
		addChild(*_icon);
		addSub(*_icon);
	}
}

WidgetListItemButton::~WidgetListItemButton() {
	if (_icon)
		_icon->remove();

	delete _text;
	freeModel(_buttonItem);
}

void WidgetListItemButton::mouseUp(uint8 state, float x, float y) {
	if (isDisabled())
		return;
	
	if (state != SDL_BUTTON_LMASK)
		return;
}

void WidgetListItemButton::mouseDown(uint8 state, float x, float y) {
	WidgetListItem::mouseDown(state, x, y);
	
	activate();
}

void WidgetListItemButton::show() {
	Engines::Widget::show();
	_buttonItem->show();
	_text->show();
}
void WidgetListItemButton::hide() {
	Engines::NWN::NWNWidget::hide();
	_buttonItem->hide();
	_text->hide();
}

float WidgetListItemButton::getWidth() const {
	return _buttonItem->getWidth();
}

float WidgetListItemButton::getHeight() const {
	return _buttonItem->getHeight();
}

void WidgetListItemButton::setPosition(float x, float y, float z) {
	NWNWidget::setPosition(x, y, z);
	getPosition(x, y ,z);
	
	z -= 5;
	_buttonItem->setPosition(x, y, z);
	z -= 5;
	float pX, pY, pZ;
	if (_buttonItem->getNode("text0")) {
		_buttonItem->getNode("text0")->getPosition(pX, pY, pZ);
		pY -= _text->getHeight() * 1.5;
	} else if (_buttonItem->getNode("text")) {
		_buttonItem->getNode("text")->getPosition(pX, pY, pZ);
		pY -= _text->getHeight() / 2;
	}

	_text->setPosition(x + pX, y + pY, z);
	if (_icon == 0)
		return;
	
	_buttonItem->getNode("icon")->getPosition(pX, pY, pZ);
	_icon->setPosition(x + pX - _icon->getWidth() / 2, y + pY - _icon->getHeight() / 2, z - 5);
}

void WidgetListItemButton::setDisabled(bool disable) {
	if (_available != disable)
		return;
	
	if (disable)
		_text->setColor(0.5, 0.5, 0.5, 1.0);
	else
		_text->unsetColor();
}

void WidgetListItemButton::setTag(const Common::UString &tag) {
	Engines::Widget::setTag(tag);
	_buttonItem->setTag(tag);
}

bool WidgetListItemButton::activate() {
	if (_buttonItem->getState() == "down" && isVisible())
		playSound("gui_button", Sound::kSoundTypeSFX);
	
	if (!WidgetListItem::activate()) {
		return false;
	}
	
	_buttonItem->setState("down");
	return true;
}

bool WidgetListItemButton::deactivate() {
	if(!WidgetListItem::deactivate())
		return false;
	
	_buttonItem->setState("");
	return true;
}

bool WidgetListItemButton::isAvailable() {
	float r, g, b, a;
	_text->getColor(r, g, b, a);
	if (r == 0.5)
		return false;
	else
		return true;
}

WidgetListItemButtonWithHelp::WidgetListItemButtonWithHelp(Engines::GUI& gui, const Common::UString& model, const Common::UString& text, const Common::UString& icon, const Common::UString& tag): WidgetListItemButton(gui, model, "fnt_galahad14", text, icon) {
	setTag(tag);

	_helpButton = loadModelGUI("ctl_cg_btn_help");
}

WidgetListItemButtonWithHelp::~WidgetListItemButtonWithHelp() {
	std::cout << "Removing WidgetListItemButtonWithHelp " << std::endl;
	freeModel(_helpButton);
}

void WidgetListItemButtonWithHelp::enter() {
	_helpButton->show();
}

void WidgetListItemButtonWithHelp::leave() {
	_helpButton->hide();
}
void WidgetListItemButtonWithHelp::mouseDown(uint8 state, float x, float y) {
	Engines::NWN::WidgetListItemButton::mouseDown(state, x, y);
	
	if (_helpButton->isIn(x, y)) {
		_helpButton->setState("down");
	}
}
void WidgetListItemButtonWithHelp::mouseUp(uint8 state, float x, float y) {
	Engines::NWN::WidgetListItemButton::mouseUp(state, x, y);
	
	_helpButton->setState("up");
	if (_helpButton->isIn(x, y)) {
		helpActivated();
	}
}

void WidgetListItemButtonWithHelp::helpActivated() {
}

void WidgetListItemButtonWithHelp::setPosition(float x, float y, float z) {
	Engines::NWN::WidgetListItemButton::setPosition(x, y, z);
	
	getPosition(x, y, z);
	float nX, nY, nZ;
	if (_buttonItem->getNode("helpbutton")) {
		_buttonItem->getNode("helpbutton")->getPosition(nX, nY, nZ);
		_helpButton->setPosition(x + nX, y + nY, z - 10);
	}
}

WidgetListItemExchange::WidgetListItemExchange(Engines::GUI &gui, const Common::UString &model, const Common::UString &text, const Common::UString &icon, const Common::UString &tag, const Common::UString &description, bool left, uint16 abstractIndex) : WidgetListItemButtonWithHelp(gui, model, text, icon, tag) {
	_description = description;
	_isMovable = true;
	_left = left;
	_needToMove = false;
	_abstractIndex = abstractIndex;

	_addRemoveButton = new WidgetButton(gui, tag + "#AddRemove", left ? "ctl_cg_btn_left" : "ctl_cg_btn_right");
	addChild(*_addRemoveButton);
	addSub(*_addRemoveButton);
}

WidgetListItemExchange::~WidgetListItemExchange() {
	std::cout << "Removing WidgetListItemExchange" << std::endl;
	_addRemoveButton->remove();
	std::cout << "end removing WidgetListItemExchange" << std::endl;
}

void WidgetListItemExchange::setPosition(float x, float y, float z) {
	Engines::NWN::WidgetListItemButtonWithHelp::setPosition(x, y, z);
	
	getPosition(x, y, z);
	float nX, nY, nZ;
	_buttonItem->getNode("addremovebutton")->getPosition(nX, nY, nZ);
	_addRemoveButton->setPosition(x + nX, y + nY, z - 10);
}

void WidgetListItemExchange::subActive(Widget &widget) {
	if (!_isMovable)
		return;

	if (widget.getTag().endsWith("#AddRemove"))
		_needToMove = true;
}

void WidgetListItemExchange::getProperties(Common::UString &title, Common::UString &featDescription, Common::UString &icon) {
	title = _text->get();
	featDescription = _description;
	if (_icon)
		icon = _icon->getPortrait();
	else
		icon = "";
}

void WidgetListItemExchange::setProperties(const Common::UString &title, const Common::UString &description) {
	_text->set(title);
	_description = description;
}

void WidgetListItemExchange::setUnmovable() {
	_text->setColor(0.5, 0.5, 0.5, 1.0);
	_isMovable = false;
}

uint16 WidgetListItemExchange::getAbstractIndex() const {
	return _abstractIndex;
}

void WidgetListItemExchange::setAbstractIndex(uint16 abstractIndex) {
	_abstractIndex = abstractIndex;
}

void WidgetListItemExchange::changeOrientation() {
	_left = !_left;

	_addRemoveButton->remove();
	_addRemoveButton = new WidgetButton(*_gui, _tag + "#AddRemove", _left ? "ctl_cg_btn_left" : "ctl_cg_btn_right");
	addChild(*_addRemoveButton);
	addSub(*_addRemoveButton);
}

bool WidgetListItemExchange::hasToMove() const {
	return _needToMove;
}

void WidgetListItemExchange::setNeedToMove(bool needToMove) {
	_needToMove = needToMove;
}

bool WidgetListItemExchange::isLeft() const {
	return _left;
}

} // End of namespace NWN

} // End of namespace Engines
