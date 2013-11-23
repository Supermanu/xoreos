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

/** @file engines/nwn/gui/widgets/editbox.cpp
 *  A NWN editbox widget.
 */

#include "graphics/graphics.h"
#include "graphics/aurora/text.h"
#include "graphics/aurora/modelnode.h"
#include "graphics/aurora/model.h"

#include "engines/nwn/gui/widgets/editbox.h"
#include "engines/nwn/gui/widgets/button.h"
#include "engines/nwn/gui/widgets/scrollbar.h"


namespace Engines {

namespace NWN {

WidgetEditBox::WidgetEditBox(::Engines::GUI &gui, const Common::UString &tag,
                             const Common::UString &model, const Common::UString &font) :
	ModelWidget(gui, tag, model), _hasScrollbar(false), _firstLineToShow(0){

	_fontHandle = FontMan.get(font);
	
	_title = new Graphics::Aurora::Text(_fontHandle,"");
	_title->setPosition(15,192,-90);

	getProperties();
	createScrollbar();
	_model->setClickable(true);
}

WidgetEditBox::~WidgetEditBox() {
	delete _title;
}

void WidgetEditBox::setMainText(Common::UString &mainText) {
      if (!_mainText.empty()) {
		for (std::vector<Graphics::Aurora::Text *>::iterator it = _mainText.begin(); it != _mainText.end(); ++it) {
			(*it)->hide();
			delete (*it);
		}
		_mainText.clear();
      }

      std::vector<Common::UString> lines;
      _fontHandle.getFont().split(mainText, lines, getWidth() - 30);
      for (std::vector<Common::UString>::iterator it = lines.begin(); it != lines.end(); ++it) {
		Graphics::Aurora::Text * text = new Graphics::Aurora::Text(_fontHandle, *it);
		text->setPosition(15, 105 - (it - lines.begin()) *  text->getHeight(), -100);
		_mainText.push_back(text);
		if (this->isVisible() && (it - lines.begin()) < 18)
			text->show();
      }
      _firstLineToShow = 0;
      updateScrollbarLength();
      updateScrollbarPosition();
}

void WidgetEditBox::setTitle(Common::UString title) {
	_title->set(title);
}

void WidgetEditBox::show() {
	GfxMan.lockFrame();
	ModelWidget::show();

	_title->show();
	if (_hasScrollbar) {
		_up->show();
		_down->show();
		_scrollbar->show();
	}
	Uint8 counter = 0;
	for (std::vector<Graphics::Aurora::Text *>::iterator it = _mainText.begin(); it != _mainText.end(); ++it, ++counter) {
		if (counter < _firstLineToShow)
			continue;

		if (counter >= _firstLineToShow + 18)
			break;

		(*it)->setPosition(15, 105 - (counter - _firstLineToShow) *  (*it)->getHeight(), -100);
		(*it)->show();
	}
	setActive(true);
	GfxMan.unlockFrame();
}

void WidgetEditBox::hide() {
	ModelWidget::hide();

	_title->hide();
	if (_hasScrollbar) {
		_up->hide();
		_down->hide();
		_scrollbar->hide();
	}

// 	Uint8 counter = 0;
	for (std::vector<Graphics::Aurora::Text *>::iterator it = _mainText.begin(); it != _mainText.end(); ++it) {
		(*it)->hide();
	}
	setActive(false);
}

void WidgetEditBox::createScrollbar() {
	if (!_hasScrollbar)
		return;

	// Get top position
	float minX, minY, minZ;
	_model->getNode("scrollmin")->getPosition(minX, minY, minZ);
	//Adjustement
	minX += 9;
	minY -= 199;


	// Create the "up" button
	_up = new WidgetButton(*_gui, getTag() + "#Up", "pb_scrl_up", "gui_scroll");
	_up->setPosition(minX, minY, -100.0);
	addSub(*_up);

	// Get bottom position
	float maxX, maxY, maxZ;
	_model->getNode("scrollmax")->getPosition(maxX, maxY, maxZ);
	//Adjustement
	maxY -= 199;
	maxX += 9;

	// Create the "down" button
	_down = new WidgetButton(*_gui, getTag() + "#Down", "pb_scrl_down", "gui_scroll");
	_down->setPosition(maxX, maxY - 10, -100.0);
	addSub(*_down);

	// Scroll bar range (max length)
	float scrollRange = minY - (maxY - 10) - _up->getHeight() - 1;

	// Create the scrollbar
	_scrollbar = new WidgetScrollbar(*_gui, getTag() + "#Bar", Scrollbar::kTypeVertical, scrollRange);

	// Center the bar within the scrollbar area
	float scrollX = maxX + (_up->getWidth() - _scrollbar->getWidth()) / 2;
	// Move it to the base position
	float scrollY = maxY - 10 + _up->getHeight();

	_scrollbar->setPosition(scrollX, scrollY, -100.0);
	addSub(*_scrollbar);
}

void WidgetEditBox::updateScrollbarLength() {
	///TODO Add condition to ensure a minimal length
	if (!_scrollbar)
		return;

	if (_mainText.size() <= 18)
		_scrollbar->setLength(1.0);
	else
		_scrollbar->setLength(18.0 / _mainText.size());
}

void WidgetEditBox::updateScrollbarPosition() {
	if (!_scrollbar)
		return;

	float max = _mainText.size() - 18 + 1;
	if ( max == 1)
		return;

	_scrollbar->setState(_firstLineToShow / (max - 1));
}

void WidgetEditBox::scrollDown(uint n) {
	if (_mainText.size() <= 18 || (_mainText.size() - 18) == _firstLineToShow)
		return;

	hide();
	_firstLineToShow += n;
// 	updateScrollbarPosition();
	if (!_scrollbar->isActive())
		updateScrollbarPosition();

	show();
}

void WidgetEditBox::scrollUp(uint n) {
	if (_mainText.size() <= 18 || _firstLineToShow == 0)
		return;

	hide();
	_firstLineToShow -= n;
	if (!_scrollbar->isActive())
		updateScrollbarPosition();

	show();
}


void WidgetEditBox::getProperties() {
	// Do we have a scroll bar?
	_hasScrollbar = _model->hasNode("scrollmin") && _model->hasNode("scrollmax");
}

void WidgetEditBox::subActive(Widget &widget) {
	///TODO Add the ability to continue to scroll when the button Up and Down is still pressed
	if (widget.getTag().endsWith("#Up")) {
		scrollUp(1);
		return;
	}

	if (widget.getTag().endsWith("#Down")) {
		scrollDown(1);
		return;
	}

	if (widget.getTag().endsWith("#Bar")) {
		int futurePosition =  _firstLineToShow - (_scrollbar->getState() * (_mainText.size() - 18 + 1));
		if (futurePosition > 0) {
			scrollUp(futurePosition);
		} else {
			scrollDown(-1 * futurePosition);
		}
	}
}

void WidgetEditBox::mouseDown(uint8 state, float x, float y) {
	if (isDisabled())
		return;

	if (state == SDL_BUTTON_WHEELUP) {
		scrollUp(1);
		return;
	}
	if (state == SDL_BUTTON_WHEELDOWN) {
		scrollDown(1);
		return;
	}
}

} // End of namespace NWN

} // End of namespace Engines
