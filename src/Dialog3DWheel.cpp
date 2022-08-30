/*
    Copyright (C) 2021 Florian Cabot <florian.cabot@hotmail.fr>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#include "Dialog3DWheel.hpp"

Dialog3DWheel::Dialog3DWheel(VRHandler const& vrHandler,
                             ToneMappingModel const& tmm)
    : vrHandler(vrHandler)
    , tmm(tmm)
    , layout(this)
{
}

void Dialog3DWheel::addDialog3D(QString const& name, Dialog3D& dialog3D)
{
	auto index(dialog3Ds.size());
	auto b = new QPushButton(this);
	b->setText(name);
	b->setAutoDefault(false);
	connect(b, &QPushButton::pressed,
	        [this, index]()
	        {
		        if(dialog3Ds[index]->isHidden())
		        {
			        dialog3Ds[index]->showFromHeadset(vrHandler);
		        }
		        else
		        {
			        dialog3Ds[index]->hide();
		        }
	        });

	dialog3Ds.push_back(&dialog3D);

	layout.QLayout::addWidget(b);
}

void Dialog3DWheel::vrEvent(VRHandler::Event const& e)
{
	switch(e.type)
	{
		case VRHandler::EventType::BUTTON_PRESSED:
			switch(e.button)
			{
				case VRHandler::Button::TRIGGER:
					for(auto d3d : dialog3Ds)
					{
						d3d->triggerPressed(*vrHandler.getController(e.side));
					}
					break;
				default:
					break;
			}
			break;
		case VRHandler::EventType::BUTTON_UNPRESSED:
			switch(e.button)
			{
				case VRHandler::Button::TRIGGER:
					for(auto d3d : dialog3Ds)
					{
						d3d->triggerReleased(*vrHandler.getController(e.side));
					}
					break;
				default:
					break;
			}
			break;
		default:
			break;
	}
}

void Dialog3DWheel::render()
{
	Dialog3D::render(vrHandler, tmm);
	for(auto d3d : dialog3Ds)
	{
		d3d->render(vrHandler, tmm);
	}
}
