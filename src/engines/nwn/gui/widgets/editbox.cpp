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

#include "graphics/aurora/text.h"

#include "engines/nwn/gui/widgets/editbox.h"


namespace Engines {

namespace NWN {

WidgetEditBox::WidgetEditBox(::Engines::GUI &gui, const Common::UString &tag,
                             const Common::UString &model, const Common::UString &font) :
	ModelWidget(gui, tag, model) {

	float pX,pY,pZ;
	this->getPosition(pX,pY,pZ);
	
	_fontHandle = FontMan.get(font);
	
	_title = new Graphics::Aurora::Text(_fontHandle,"");
	_title->setPosition(15,192,pZ-100);

	///TODO Add scrollbar
}

WidgetEditBox::~WidgetEditBox() {
}

void WidgetEditBox::setMainText(Common::UString mainText) {
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
		if (this->isVisible())
			text->show();
      }
}

void WidgetEditBox::setTitle(Common::UString title) {
	_title->set(title);
}

void WidgetEditBox::show() {
	Engines::NWN::ModelWidget::show();
	_title->show();

	for (std::vector<Graphics::Aurora::Text *>::iterator it = _mainText.begin(); it != _mainText.end(); ++it) {
		(*it)->show();
	}
}

void WidgetEditBox::hide() {
	Engines::NWN::ModelWidget::hide();
	_title->hide();

	for (std::vector<Graphics::Aurora::Text *>::iterator it = _mainText.begin(); it != _mainText.end(); ++it) {
		(*it)->hide();
	}
}

} // End of namespace NWN

} // End of namespace Engines
