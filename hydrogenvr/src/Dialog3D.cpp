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
    : widget3d(*this)
    , shader("default")
{
	shader.setUniform("color", QColor(255, 0, 0));
	pointer.setVertexShaderMapping(shader, {{"position", 3}});
	pointer.setVertices({0.f, 0.f, 0.f, 0.f, -sqrt2over2, -sqrt2over2});
}

void Dialog3D::installEventFilters()
{
	for(auto* child : children())
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
	const auto inter(intersection(controller));
	const QPointF localPos2D(inter.x(), inter.y());
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
	const auto inter(intersection(controller));
	const QPointF localPos2D(inter.x(), inter.y());
	mouseRelease(localPos2D);
}

bool Dialog3D::intersects(VRHandler const& headset) const
{
	const auto inter(intersection(headset));
	return inter.x() >= 0.f && inter.y() >= 0.f && inter.x() <= 1.f
	       && inter.y() <= 1.f;
}

bool Dialog3D::intersects(Controller const& controller) const
{
	auto inter(intersection(controller));
	return inter.x() >= 0.f && inter.y() >= 0.f && inter.x() <= 1.f
	       && inter.y() <= 1.f;
}

void Dialog3D::click(VRHandler const& headset)
{
	if(!isVisible())
	{
		return;
	}
	const auto inter(intersection(headset));
	const QPointF localPos2D(inter.x(), inter.y());
	if(localPos2D.x() < 0.f || localPos2D.y() < 0.f || localPos2D.x() > 1.f
	   || localPos2D.y() > 1.f)
	{
		return;
	}
	mouseClick(localPos2D);
}

void Dialog3D::click(Controller const& controller)
{
	// this error is thrown by Qt's code
	// NOLINTBEGIN(clang-analyzer-cplusplus.NewDelete)
	if(controller.side != sidePriority || !isVisible())
	{
		return;
	}
	const auto inter(intersection(controller));
	const QPointF localPos2D(inter.x(), inter.y());
	if(localPos2D.x() < 0.f || localPos2D.y() < 0.f || localPos2D.x() > 1.f
	   || localPos2D.y() > 1.f)
	{
		return;
	}
	mouseClick(localPos2D);
	// NOLINTEND(clang-analyzer-cplusplus.NewDelete)
}

void Dialog3D::triggerWheelEvent(VRHandler const& headset, QWheelEvent* e)
{
	if(!isVisible())
	{
		return;
	}
	const auto inter(intersection(headset));
	const QPointF localPos2D(inter.x(), inter.y());
	if(localPos2D.x() < 0.f || localPos2D.y() < 0.f || localPos2D.x() > 1.f
	   || localPos2D.y() > 1.f)
	{
		return;
	}
	mouseWheel(localPos2D, e);
}

void Dialog3D::triggerWheelEvent(Controller const& controller, QWheelEvent* e)
{
	if(controller.side != sidePriority || !isVisible())
	{
		return;
	}
	const auto inter(intersection(controller));
	const QPointF localPos2D(inter.x(), inter.y());
	if(localPos2D.x() < 0.f || localPos2D.y() < 0.f || localPos2D.x() > 1.f
	   || localPos2D.y() > 1.f)
	{
		return;
	}
	mouseWheel(localPos2D, e);
}
void Dialog3D::render(VRHandler const& vrHandler, ToneMappingModel const& tmm)
{
	if(!isVisible() || !vrHandler.isEnabled())
	{
		return;
	}
	shader.setUniform("exposure", tmm.exposure);
	shader.setUniform("dynamicrange", tmm.dynamicrange);

	QVector2D cursor(-1.f, -1.f);
	if(vrHandler.getController(Side::LEFT) != nullptr
	   && sidePriority == Side::LEFT)
	{
		auto const& c(*vrHandler.getController(Side::LEFT));
		const auto inter(intersection(c));
		if(inter.x() >= 0.f && inter.y() >= 0.f && inter.x() <= 1.f
		   && inter.y() <= 1.f)
		{
			cursor = inter.toVector2D();
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
		const auto inter(intersection(c));
		if(inter.x() >= 0.f && inter.y() >= 0.f && inter.x() <= 1.f
		   && inter.y() <= 1.f)
		{
			cursor = inter.toVector2D();
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

	// if no controller, try headset
	if(vrHandler.getController(Side::LEFT) == nullptr
	   && vrHandler.getController(Side::RIGHT) == nullptr)
	{
		auto inter(intersection(vrHandler));
		if(inter.x() >= 0.f && inter.y() >= 0.f && inter.x() <= 1.f
		   && inter.y() <= 1.f)
		{
			cursor = inter.toVector2D();
			mouseMove({inter.x(), inter.y()});
			QMatrix4x4 scale;
			scale.scale(inter.z());
			GLHandler::setUpRender(shader, vrHandler.getHMDPosMatrix() * scale,
			                       GLHandler::GeometricSpace::SEATEDTRACKED);
			pointer.render(PrimitiveType::LINES);
			sidePriority = Side::RIGHT;
		}
	}

	widget3d.render(tmm, GLHandler::GeometricSpace::SEATEDTRACKED, cursor);
}

void Dialog3D::paintEvent(QPaintEvent* event)
{
	widget3d.triggerRepaint();
	QDialog::paintEvent(event);
}

void Dialog3D::mouseMove(QPointF const& relativePosition)
{
	const QPoint p(static_cast<int>(relativePosition.x() * size().width()),
	               static_cast<int>(relativePosition.y() * size().height()));
	QTest::mouseMove(windowHandle(), p, -1);
}

void Dialog3D::mousePress(QPointF const& relativePosition)
{
	const QPoint p(static_cast<int>(relativePosition.x() * size().width()),
	               static_cast<int>(relativePosition.y() * size().height()));
	QTest::mousePress(windowHandle(), Qt::LeftButton, {}, p, -1);
}

void Dialog3D::mouseRelease(QPointF const& relativePosition)
{
	const QPoint p(static_cast<int>(relativePosition.x() * size().width()),
	               static_cast<int>(relativePosition.y() * size().height()));
	QTest::mouseRelease(windowHandle(), Qt::LeftButton, {}, p, -1);
}

void Dialog3D::mouseClick(QPointF const& relativePosition)
{
	const QPoint p(static_cast<int>(relativePosition.x() * size().width()),
	               static_cast<int>(relativePosition.y() * size().height()));
	QTest::mouseClick(windowHandle(), Qt::LeftButton, {}, p, -1);
}

void Dialog3D::mouseWheel(QPointF const& relativePosition, QWheelEvent* e)
{
	const QPoint p(static_cast<int>(relativePosition.x() * size().width()),
	               static_cast<int>(relativePosition.y() * size().height()));
	mouseWheelTurn(this, e, p);
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
	for(auto* child : obj->children())
	{
		child->installEventFilter(this);
		installEventFilters(child);
	}
}

QVector3D Dialog3D::intersection(Controller const& controller) const
{
	const auto transform(widget3d.getModel() * widget3d.getAspectRatioMatrix());
	const auto invTransform(transform.inverted());

	const auto pos(
	    utils::transformPosition(invTransform, controller.getPosition()));
	QVector3D dir(widget3d.getAspectRatioMatrix().inverted()
	              * widget3d.getModel().inverted() * controller.getModel()
	              * QVector4D(0.f, -sqrt2over2, -sqrt2over2, 0.f));
	dir.normalize();

	if((pos.z() > 0.f && dir.z() >= 0.f) || (pos.z() < 0.f && dir.z() <= 0.f))
	{
		const float inf(std::numeric_limits<float>::infinity());
		return {inf, inf, inf};
	}

	const float distLocal(-pos.z() / dir.z());
	auto result(pos + distLocal * dir);
	result.setX(result.x() + 0.5f);
	result.setY(0.5f - result.y());

	const auto intersectLocal(pos + distLocal * dir);
	const auto intersectGlobal(
	    utils::transformPosition(transform, intersectLocal));
	const float distGlobal(
	    intersectGlobal.distanceToPoint(controller.getPosition()));
	result.setZ(distGlobal);

	return result;
}

QVector3D Dialog3D::intersection(VRHandler const& headset) const
{
	const auto transform(widget3d.getModel() * widget3d.getAspectRatioMatrix());
	const auto invTransform(transform.inverted());

	const QVector3D hmdPos(headset.getHMDPosMatrix().column(3).toVector3D());
	const QMatrix4x4 hmdModel(headset.getHMDPosMatrix());

	const auto pos(utils::transformPosition(invTransform, hmdPos));
	QVector3D dir(widget3d.getAspectRatioMatrix().inverted()
	              * widget3d.getModel().inverted() * hmdModel
	              * QVector4D(0.f, 0.f, -1.f, 0.f));
	dir.normalize();

	if((pos.z() > 0.f && dir.z() >= 0.f) || (pos.z() < 0.f && dir.z() <= 0.f))
	{
		const float inf(std::numeric_limits<float>::infinity());
		return {inf, inf, inf};
	}

	const float distLocal(-pos.z() / dir.z());
	auto result(pos + distLocal * dir);
	result.setX(result.x() + 0.5f);
	result.setY(0.5f - result.y());

	const auto intersectLocal(pos + distLocal * dir);
	const auto intersectGlobal(
	    utils::transformPosition(transform, intersectLocal));
	const float distGlobal(intersectGlobal.distanceToPoint(hmdPos));
	result.setZ(distGlobal);

	return result;
}
