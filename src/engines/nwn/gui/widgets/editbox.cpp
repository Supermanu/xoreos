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
	ModelWidget(gui, tag, model), _cursor(0), _hasScrollbar(false), _hasTitle(false), _firstLineToShow(0), _hasFocus(false), _cursorPosition(0), _mode(kModeStatic) {

	_fontHandle = FontMan.get(font);

	getProperties();
	createScrollbar();
	createTitle();
	_model->setClickable(true);
}

WidgetEditBox::~WidgetEditBox() {
	delete _cursor;
}

void WidgetEditBox::setMainText(const Common::UString &mainText) {
	if (!_mainText.empty()) {
		for (std::vector<Graphics::Aurora::Text *>::iterator it = _mainText.begin(); it != _mainText.end(); ++it) {
			(*it)->hide();
			delete (*it);
		}
		_mainText.clear();
	}

	std::vector<Common::UString> lines;
	_fontHandle.getFont().split(mainText, lines, getWidth() - 30);

	float pX, pY, pZ;
	_model->getNode("text0")->getPosition(pX, pY, pZ);
	pY -= _fontHandle.getFont().getHeight();

	// Adjust position from the widget.
	float x, y, z;
	getPosition(x, y, z);

	for (std::vector<Common::UString>::iterator it = lines.begin(); it != lines.end(); ++it) {
		Graphics::Aurora::Text *text = new Graphics::Aurora::Text(_fontHandle, *it);
		text->setPosition(x + pX, y + pY - (it - lines.begin()) * text->getHeight(), -1500);
		_mainText.push_back(text);
		if (this->isVisible() && ((it - lines.begin()) < 18))
			text->show();
	}
	_firstLineToShow = 0;

	if (_hasScrollbar) {
		updateScrollbarLength();
		updateScrollbarPosition();
	}
}

void WidgetEditBox::setTitle(const Common::UString &title) {
	if (_hasTitle)
		_title->setText(title);
}

void WidgetEditBox::setMode(WidgetEditBox::Mode mode) {
	_mode = mode;
	if (mode == kModeStatic)
		return;

	_cursor = new Graphics::Aurora::GUIQuad("", 0, 1.0, 0, _fontHandle.getFont().getHeight() + 2);
	float pX, pY, pZ, x, y, z;
	_model->getNode("text0")->getPosition(pX, pY, pZ);
	getPosition(x, y, z);
	_cursor->setPosition(pX + x, pY + y - _fontHandle.getFont().getHeight() - 1);
	_cursor->setWidth(1);
	_cursor->setColor(1.0, 1.0, 1.0, 1.0);
}

void WidgetEditBox::setFocus(bool hasFocus) {
	_hasFocus = hasFocus;
}

void WidgetEditBox::show() {
	GfxMan.lockFrame();
	ModelWidget::show();

	if (_hasScrollbar) {
		_up->show();
		_down->show();
		_scrollbar->show();
	}

	if (_hasTitle)
		_title->show();

	if (_cursor && _hasFocus)
		_cursor->show();

	float pX, pY, pZ;
	if (_mainText.size())
		_mainText.front()->getPosition(pX, pY, pZ);

	uint8 counter = 0;
	for (std::vector<Graphics::Aurora::Text *>::iterator it = _mainText.begin(); it != _mainText.end(); ++it, ++counter) {
		if (counter < _firstLineToShow)
			continue;

		if (counter >= _firstLineToShow + 18)
			break;

		(*it)->setPosition(pX, pY - (counter - _firstLineToShow) * (*it)->getHeight(), pZ);
		(*it)->show();
	}

	setActive(true);
	GfxMan.unlockFrame();
}

void WidgetEditBox::hide() {
	ModelWidget::hide();

	if (_hasScrollbar) {
		_up->hide();
		_down->hide();
		_scrollbar->hide();
	}

	if (_hasTitle)
		_title->hide();

	if (_cursor)
		_cursor->hide();

	for (std::vector<Graphics::Aurora::Text *>::iterator it = _mainText.begin(); it != _mainText.end(); ++it)
		(*it)->hide();
	setActive(false);
}

void WidgetEditBox::createScrollbar() {
	if (!_hasScrollbar)
		return;

	// Get top position
	float minX, minY, minZ;
	_model->getNode("scrollmin")->getPosition(minX, minY, minZ);

	// Create the "up" button
	_up = new WidgetButton(*_gui, getTag() + "#Up", "pb_scrl_up", "gui_scroll");
	_up->setPosition(minX, minY, -100.0);
	addSub(*_up);
	addChild(*_up);

	// Get bottom position
	float maxX, maxY, maxZ;
	_model->getNode("scrollmax")->getPosition(maxX, maxY, maxZ);

	// Create the "down" button
	_down = new WidgetButton(*_gui, getTag() + "#Down", "pb_scrl_down", "gui_scroll");
	_down->setPosition(maxX, maxY - 10, -100.0);
	addSub(*_down);
	addChild(*_down);

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
	addChild(*_scrollbar);
}

void WidgetEditBox::createTitle() {
	if (!_hasTitle) {
		_title = 0;
		return;
	}

	_title = new TextWidget(*_gui, getTag() + "#Title", _fontHandle.getFontName(), "");

	float pX, pY, pZ;
	_model->getNode("title0")->getPosition(pX, pY, pZ);
	pY -= _fontHandle.getFont().getHeight();

	_title->setPosition(pX, pY, -90);
	addChild(*_title);
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
	if (max == 1)
		return;

	_scrollbar->setState(_firstLineToShow / (max - 1));
}

void WidgetEditBox::scrollDown(uint n) {
	if ((_mainText.size() <= 18) || ((_mainText.size() - 18) == _firstLineToShow))
		return;

	hide();
	_firstLineToShow += n;
	if (!_scrollbar->isActive())
		updateScrollbarPosition();

	show();
}

void WidgetEditBox::scrollUp(uint n) {
	if ((_mainText.size() <= 18) || (_firstLineToShow == 0))
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
	_hasTitle     = _model->hasNode("title0");
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
		int futurePosition = _firstLineToShow - (_scrollbar->getState() * (_mainText.size() - 18 + 1));
		if (futurePosition > 0)
			scrollUp(futurePosition);
		else
			scrollDown(-1 * futurePosition);
	}
}

void WidgetEditBox::mouseDown(uint8 state, float x, float y) {
	if (isDisabled())
		return;

	if (!_hasFocus && _cursor) {
		_hasFocus = true;
		_cursor->show();
	}

	float wX, wY, wZ;
	getPosition(wX, wY, wZ);

	// Check if we clicked on the scrollbar area
	if (_hasScrollbar) {
		if (x > (wX + getWidth() - 20)) {
			if (y > _scrollbar->getBarPosition())
				scrollUp(1);
			else
				scrollDown(1);

			return;
		}
	}

	if (!_cursor)
		return;

	// Move the cursor to the mouse click position.
	uint32 position = 0;
	for (uint32 it = 0; it < _mainText.size(); ++it) {
		float pX, pY, pZ;
		_mainText[it]->getPosition(pX, pY, pZ);
		position += _mainText[it]->get().size();

		if (!_mainText[it]->isIn(pX, y))
			continue;


		Common::UString line = _mainText[it]->get();
		float lineWidth      = _mainText[it]->getWidth();

		if (!_mainText[it]->isIn(x, y)) {
			_cursor->setPosition(pX + lineWidth, pY);
			break;
		}

		for (uint32 trunc = line.size() - 1; trunc >= 0; --trunc) {
			position--;
			line.erase(line.getPosition(trunc));
			lineWidth = _fontHandle.getFont().getWidth(line);
			if (lineWidth <= (x - pX))
				break;
		}
		_cursor->setPosition(pX + lineWidth, pY);
	}
	_cursorPosition = position;
}

void WidgetEditBox::mouseWheel(uint8 state, int x, int y) {
	if (isDisabled())
		return;

	if (y > 0)
		scrollUp(1);
	else if (y < 0)
		scrollDown(1);
}

void WidgetEditBox::moveCursorCharacter(bool next) {
	if (_mode == kModeStatic)
		return;

	uint32 totalSize = -1;
	for (uint it = 0; it < _mainText.size(); ++it)
		totalSize += _mainText[it]->get().size();

	if (next && (_cursorPosition < totalSize))
		_cursorPosition++;
	else if (!next && (_cursorPosition > 0))
		_cursorPosition--;

	updateCursor();
}

void WidgetEditBox::moveCursorLine(bool down) {
	uint32 count = 0;
	for (uint it = 0; it < _mainText.size(); ++it) {
		count += _mainText[it]->get().size();
		if (_cursorPosition < count)
			continue;

		float pX, pY, pZ;
		_mainText[it]->getPosition(pX, pY, pZ);
		float height = _fontHandle.getFont().getHeight();
		if (down)
			mouseDown(0, pX, pY - height);
		else
			mouseDown(0, pX, pY + height);

		break;
	}
}

void WidgetEditBox::updateCursor() {
	uint32 count = 0;
	for (uint it = 0; it < _mainText.size(); ++it) {
		count += _mainText[it]->get().size();
		if (_cursorPosition < count)
			continue;

		float pX, pY, pZ;
		_mainText[it]->getPosition(pX, pY, pZ);
		Common::UString line = _mainText[it]->get();
		uint32          cursorPositionLine = _cursorPosition - count + line.size();
		line.truncate(line.size() - cursorPositionLine);
		float x = pX + _fontHandle.getFont().getWidth(line);

		_cursor->setPosition(x, pY, pZ);
		break;
	}
}

float WidgetEditBox::getWidth() const {
	///WORKAROUND Sometimes the model has no dimension.
	if (!Engines::NWN::ModelWidget::getWidth())
		return _parent->getWidth() - 75;

	return Engines::NWN::ModelWidget::getWidth();
}

}   // End of namespace NWN

} // End of namespace Engines
