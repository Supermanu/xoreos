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

/** @file engines/nwn/gui/widgets/button.cpp
 *  A NWN button widget.
 */

#include "graphics/aurora/model.h"

#include "engines/aurora/util.h"

#include "engines/nwn/gui/widgets/button.h"

namespace Engines {

namespace NWN {

WidgetButton::WidgetButton(::Engines::GUI &gui, const Common::UString &tag,
                           const Common::UString &model, const Common::UString &sound) :
	ModelWidget(gui, tag, model), _stayPressed(false), _pressed(false) {

	_model->setClickable(true);
	_model->setState("up");

	_sound = sound;
}

WidgetButton::~WidgetButton() {
}

void WidgetButton::enter() {
	ModelWidget::enter();

	if (isDisabled())
		return;

	if (_stayPressed) 
		return;

	_model->setState("hilite");
}

void WidgetButton::leave() {
	ModelWidget::leave();

	if (isDisabled())
		return;
	if (_stayPressed)
		return;

	_model->setState("up");
}

void WidgetButton::setDisabled(bool disabled) {
	///TODO When the button is disabled then unabled the text stay grey.
	NWNWidget::setDisabled(disabled);

	if (isDisabled())
		_model->setState("disabled");
	else
		_model->setState("up");
}

void WidgetButton::mouseDown(uint8 state, float x, float y) {
	if (isDisabled())
		return;

	if (state != SDL_BUTTON_LMASK)
		return;

	///TODO Add an animation when pressed: the text should move a little to the bottom.
	if (_stayPressed) {
		return;
	}
	_model->setState("down");

	playSound(_sound, Sound::kSoundTypeSFX);
}

void WidgetButton::mouseUp(uint8 state, float x, float y) {
	if (isDisabled())
		return;

	if (!_stayPressed) {
		_model->setState("hilite");
		setActive(true);
		return;
	} else {
		playSound(_sound, Sound::kSoundTypeSFX);
		_pressed = ! _pressed;

		if (_pressed) {
			setPressed(true);
		}
	}

}

void WidgetButton::setStayPressed(bool stay) {
	_stayPressed = stay;
}

void WidgetButton::setPressed(bool pressed) {
	if (!_stayPressed)
		return;


	if (pressed) {
		_model->setState("down");
		_pressed = true;
		setActive(true);
	} else {
		_model->setState("up");
		_pressed = false;
		setActive(false);
	}
}

bool WidgetButton::isPressed() {
	return _pressed;
}




} // End of namespace NWN

} // End of namespace Engines
