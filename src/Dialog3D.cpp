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

#include "Dialog3D.hpp"

Dialog3D::Dialog3D()
    : widget3d(this)
    , shader("default")
{
	shader.setUniform("color", QColor(255, 0, 0));
	pointer.setVertexShaderMapping(shader, {{"position", 3}});
	pointer.setVertices({0.f, 0.f, 0.f, 0.f, -sqrt2over2, -sqrt2over2});
}

void Dialog3D::installEventFilters()
{
	for(auto child : children())
	{
		child->installEventFilter(this);
		installEventFilters(child);
	}
}

void Dialog3D::showFromHeadset(VRHandler const& vrHandler)
{
	show();

	widget3d.getModel() = vrHandler.getHMDPosMatrix();
	widget3d.getModel().translate(0.f, 0.f, -0.7f);

	float factor(0.001f);
	if(width() > height())
	{
		factor *= size().width();
	}
	else
	{
		factor *= size().height();
	}
	widget3d.getModel().scale(factor);
}

void Dialog3D::showFromController(Controller const& controller)
{
	show();

	widget3d.getModel() = controller.getModel();
	widget3d.getModel().rotate(-45.f, QVector3D(1.f, 0.f, 0.f));
	widget3d.getModel().translate(0.f, 0.f, -0.2f);

	float factor(0.001f);
	if(width() > height())
	{
		factor *= size().width();
	}
	else
	{
		factor *= size().height();
	}
	widget3d.getModel().scale(factor);
}

void Dialog3D::triggerPressed(Controller const& controller)
{
	if(controller.side != sidePriority || !isVisible())
	{
		return;
	}
	auto inter(intersection(controller));
	QPointF localPos2D(inter.x(), inter.y());
	if(localPos2D.x() < 0.f || localPos2D.y() < 0.f || localPos2D.x() > 1.f
	   || localPos2D.y() > 1.f)
	{
		return;
	}
	mousePress(localPos2D);
}

void Dialog3D::triggerReleased(Controller const& controller)
{
	if(controller.side != sidePriority || !isVisible())
	{
		return;
	}
	auto inter(intersection(controller));
	QPointF localPos2D(inter.x(), inter.y());
	mouseRelease(localPos2D);
}

void Dialog3D::click(Controller const& controller)
{
	if(controller.side != sidePriority || !isVisible())
	{
		return;
	}
	auto inter(intersection(controller));
	QPointF localPos2D(inter.x(), inter.y());
	if(localPos2D.x() < 0.f || localPos2D.y() < 0.f || localPos2D.x() > 1.f
	   || localPos2D.y() > 1.f)
	{
		return;
	}
	mouseClick(localPos2D);
}

void Dialog3D::render(VRHandler const& vrHandler, ToneMappingModel const& tmm)
{
	if(!isVisible() || !vrHandler.isEnabled())
	{
		return;
	}
	shader.setUniform("exposure", tmm.exposure);
	shader.setUniform("dynamicrange", tmm.dynamicrange);
	widget3d.render(tmm, GLHandler::GeometricSpace::SEATEDTRACKED);
	if(vrHandler.getController(Side::LEFT) != nullptr
	   && sidePriority == Side::LEFT)
	{
		auto const& c(*vrHandler.getController(Side::LEFT));
		auto inter(intersection(c));
		if(inter.x() >= 0.f && inter.y() >= 0.f && inter.x() <= 1.f
		   && inter.y() <= 1.f)
		{
			mouseMove({inter.x(), inter.y()});
			QMatrix4x4 scale;
			scale.scale(inter.z());
			GLHandler::setUpRender(shader, c.getModel() * scale,
			                       GLHandler::GeometricSpace::SEATEDTRACKED);
			pointer.render(PrimitiveType::LINES);
		}
		// let the other side do its thing, we left the dialog3d
		else
		{
			sidePriority = Side::RIGHT;
		}
	}
	else
	{
		sidePriority = Side::RIGHT;
	}
	if(vrHandler.getController(Side::RIGHT) != nullptr
	   && sidePriority == Side::RIGHT)
	{
		auto const& c(*vrHandler.getController(Side::RIGHT));
		auto inter(intersection(c));
		if(inter.x() >= 0.f && inter.y() >= 0.f && inter.x() <= 1.f
		   && inter.y() <= 1.f)
		{
			mouseMove({inter.x(), inter.y()});
			QMatrix4x4 scale;
			scale.scale(inter.z());
			GLHandler::setUpRender(shader, c.getModel() * scale,
			                       GLHandler::GeometricSpace::SEATEDTRACKED);
			pointer.render(PrimitiveType::LINES);
			sidePriority = Side::RIGHT;
		}
		else
		{
			sidePriority = Side::LEFT;
		}
	}
	else
	{
		sidePriority = Side::LEFT;
	}
}

void Dialog3D::paintEvent(QPaintEvent* event)
{
	widget3d.triggerRepaint();
	QDialog::paintEvent(event);
}

void Dialog3D::mouseMove(QPointF const& relativePosition)
{
	QPoint p(static_cast<int>(relativePosition.x() * size().width()),
	         static_cast<int>(relativePosition.y() * size().height()));
	QTest::mouseMove(windowHandle(), p, -1);
}

void Dialog3D::mousePress(QPointF const& relativePosition)
{
	QPoint p(static_cast<int>(relativePosition.x() * size().width()),
	         static_cast<int>(relativePosition.y() * size().height()));
	QTest::mousePress(windowHandle(), Qt::LeftButton, nullptr, p, -1);
}

void Dialog3D::mouseRelease(QPointF const& relativePosition)
{
	QPoint p(static_cast<int>(relativePosition.x() * size().width()),
	         static_cast<int>(relativePosition.y() * size().height()));
	QTest::mouseRelease(windowHandle(), Qt::LeftButton, nullptr, p, -1);
}

void Dialog3D::mouseClick(QPointF const& relativePosition)
{
	QPoint p(static_cast<int>(relativePosition.x() * size().width()),
	         static_cast<int>(relativePosition.y() * size().height()));
	QTest::mouseClick(windowHandle(), Qt::LeftButton, nullptr, p, -1);
}

bool Dialog3D::eventFilter(QObject* obj, QEvent* event)
{
	if(event->type() == QEvent::Paint)
	{
		widget3d.triggerRepaint();
		return false;
	}
	return QDialog::eventFilter(obj, event);
}

void Dialog3D::installEventFilters(QObject* obj)
{
	for(auto child : obj->children())
	{
		child->installEventFilter(this);
		installEventFilters(child);
	}
}

QVector3D Dialog3D::intersection(Controller const& controller) const
{
	QVector3D pos(widget3d.getAspectRatioMatrix().inverted()
	              * widget3d.getModel().inverted() * controller.getPosition());
	QVector3D dir(widget3d.getAspectRatioMatrix().inverted()
	              * widget3d.getModel().inverted() * controller.getModel()
	              * QVector4D(0.f, -sqrt2over2, -sqrt2over2, 0.f));
	dir.normalize();

	if((pos.z() > 0.f && dir.z() >= 0.f) || (pos.z() < 0.f && dir.z() <= 0.f))
	{
		float inf(std::numeric_limits<float>::infinity());
		return {inf, inf, inf};
	}

	float distLocal(-pos.z() / dir.z());
	auto result(pos + distLocal * dir);
	result.setX(result.x() + 0.5f);
	result.setY(0.5f - result.y());

	auto intersectLocal(pos + distLocal * dir);
	auto intersectGlobal(widget3d.getModel() * widget3d.getAspectRatioMatrix()
	                     * intersectLocal);
	float distGlobal(intersectGlobal.distanceToPoint(controller.getPosition()));
	result.setZ(distGlobal);

	return result;
}
