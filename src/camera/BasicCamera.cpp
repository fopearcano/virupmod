/*
    Copyright (C) 2023 Florian Cabot <florian.cabot@hotmail.fr>

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

#include "camera/BasicCamera.hpp"

BasicCamera::BasicCamera(VRHandler const& vrHandler)
    : vrHandler(vrHandler)
    , eyeDistanceFactor(1.0f)
{
}

void BasicCamera::setView(QVector3D const& position,
                          QVector3D const& lookDirection, QVector3D const& up)
{
	view = QMatrix4x4();
	view.lookAt(position, position + lookDirection, up);
}

void BasicCamera::lookAt(QVector3D const& position, QVector3D const& center,
                         QVector3D const& up)
{
	view = QMatrix4x4();
	view.lookAt(position, center, up);
}

void BasicCamera::setPerspectiveProj(float fov, float aspectratio,
                                     float nearPlan, float farPlan)
{
	proj = QMatrix4x4();
	proj.perspective(fov, aspectratio, nearPlan, farPlan);
}

void BasicCamera::setEyeDistanceFactor(float eyeDistanceFactor)
{
	this->eyeDistanceFactor     = eyeDistanceFactor;
	eyeDistanceCorrection(0, 0) = eyeDistanceFactor;
	eyeDistanceCorrection(1, 1) = eyeDistanceFactor;
	eyeDistanceCorrection(2, 2) = eyeDistanceFactor;
}

QVector4D BasicCamera::project(QVector3D const& vertex) const
{
	return project(QVector4D(vertex, 1.f));
}

QVector4D BasicCamera::project(QVector4D const& vertex) const
{
	return fullTransform * vertex;
}

bool BasicCamera::shouldBeCulled(BoundingSphere const& boundingSphere) const
{
	for(auto const& angleShift : clippingPlanes.keys())
	{
		if(!shouldBeCulled(angleShift, boundingSphere))
		{
			return false;
		}
	}
	return true;
}

bool BasicCamera::shouldBeCulled(QString const& angleShift,
                                 BoundingSphere const& boundingSphere) const
{
	for(unsigned int i(0); i < 6; ++i)
	{
		if(QVector4D::dotProduct(clippingPlanes[angleShift].at(i),
		                         QVector4D(boundingSphere.position, 1.0))
		   < -boundingSphere.radius)
		{
			return true;
		}
	}
	return false;
}

void BasicCamera::update(QMatrix4x4 const& angleShiftMat)
{
	const QMatrix4x4 shiftedView(angleShiftMat * view);
	if(vrHandler.isEnabled())
	{
		// proj
		projLeft  = vrHandler.getProjectionMatrix(angleShiftMat, Side::LEFT,
		                                          0.1f * eyeDistanceFactor,
		                                          10000.f * eyeDistanceFactor);
		projRight = vrHandler.getProjectionMatrix(angleShiftMat, Side::RIGHT,
		                                          0.1f * eyeDistanceFactor,
		                                          10000.f * eyeDistanceFactor);

		Side currentRenderingEye(vrHandler.getCurrentRenderingEye());

		// fullEyeSpaceTransform
		fullEyeSpaceTransform
		    = (currentRenderingEye == Side::LEFT) ? projLeft : projRight;

		projLeft = projLeft
		           * eyeDist(vrHandler.getEyeViewMatrix(Side::LEFT),
		                     eyeDistanceFactor);
		projRight = projRight
		            * eyeDist(vrHandler.getEyeViewMatrix(Side::RIGHT),
		                      eyeDistanceFactor);

		QMatrix4x4* projEye
		    = (currentRenderingEye == Side::LEFT) ? &projLeft : &projRight;

		// prog * shiftedView
		QMatrix4x4 hmdMat(vrHandler.getHMDPosMatrix().inverted());

		fullSeatedTrackedSpaceTransform
		    = *projEye * eyeDistanceCorrection * hmdMat;
		fullStandingTrackedSpaceTransform
		    = fullSeatedTrackedSpaceTransform
		      * vrHandler.getSeatedToStandingAbsoluteTrackingPos().inverted();
		fullHmdSpaceTransform = *projEye * eyeDistanceCorrection;

		if(!seatedVROrigin)
		{
			hmdMat = hmdMat
			         * vrHandler.getSeatedToStandingAbsoluteTrackingPos()
			               .inverted();
		}

		hmdMat = eyeDist(hmdMat, eyeDistanceFactor);
		// not true ! it holds its inverse for now
		hmdScaledToWorld = hmdMat * shiftedView;
		fullTransform    = *projEye * hmdScaledToWorld;
		// now it's the right value :)
		hmdScaledToWorld = hmdScaledToWorld.inverted();

		fullCameraSpaceTransform = *projEye * hmdMat;

		fullSkyboxSpaceTransform
		    = vrHandler.getProjectionMatrix(angleShiftMat, currentRenderingEye,
		                                    0.1f, 10000.f)
		      * noTrans(vrHandler.getEyeViewMatrix(currentRenderingEye))
		      * noTrans(vrHandler.getHMDPosMatrix().inverted())
		      * noTrans(shiftedView);

		pixVertFOV
		    = atan(1.0f / projLeft.column(1)[1]) * 2.0 / windowSize.height();

		updateClippingPlanes(angleShiftMat);

		return;
	}

	update2D(angleShiftMat);
}

void BasicCamera::update2D(QMatrix4x4 const& angleShiftMat)
{
	const QMatrix4x4 shiftedView(angleShiftMat * view);
	fullTransform                     = proj * shiftedView;
	fullEyeSpaceTransform             = proj;
	fullCameraSpaceTransform          = proj;
	fullSeatedTrackedSpaceTransform   = proj * eyeDistanceCorrection;
	fullStandingTrackedSpaceTransform = fullSeatedTrackedSpaceTransform;
	fullHmdSpaceTransform             = fullSeatedTrackedSpaceTransform;
	fullSkyboxSpaceTransform          = proj * noTrans(shiftedView);

	pixVertFOV = atan(1.0f / proj.column(1)[1]) * 2.0 / windowSize.height();

	updateClippingPlanes(angleShiftMat);
}

void BasicCamera::uploadMatrices() const
{
	GLHandler::setUpTransforms(
	    fullTransform, fullEyeSpaceTransform, fullCameraSpaceTransform,
	    fullSeatedTrackedSpaceTransform, fullStandingTrackedSpaceTransform,
	    fullHmdSpaceTransform, fullSkyboxSpaceTransform);
}

void BasicCamera::updateClippingPlanes(QMatrix4x4 const& angleShiftMat)
{
	auto k = toStr(angleShiftMat);
	// update clipping planes
	// Gribb, G., & Hartmann, K. (2001). Fast extraction of viewing frustum
	// planes from the world-view-projection matrix.
	// http://www.cs.otago.ac.nz/postgrads/alexis/planeExtraction.pdf
	clippingPlanes[k][LEFT_PLANE]
	    = (fullTransform.row(3) + fullTransform.row(0));
	clippingPlanes[k][RIGHT_PLANE]
	    = (fullTransform.row(3) - fullTransform.row(0));

	clippingPlanes[k][BOTTOM_PLANE]
	    = (fullTransform.row(3) + fullTransform.row(1));
	clippingPlanes[k][TOP_PLANE]
	    = (fullTransform.row(3) - fullTransform.row(1));

	clippingPlanes[k][NEAR_PLANE]
	    = (fullTransform.row(3) + fullTransform.row(2));
	clippingPlanes[k][FAR_PLANE]
	    = (fullTransform.row(3) - fullTransform.row(2));

	// normalize
	for(auto& plane : clippingPlanes[k])
	{
		auto l = plane.toVector3D().length();
		plane /= l;
	}
}

QVector3D BasicCamera::getWorldSpacePosition() const
{
	QMatrix4x4 eyeViewMatrix;
	if(vrHandler.isEnabled())
	{
		eyeViewMatrix
		    = vrHandler.getEyeViewMatrix(vrHandler.getCurrentRenderingEye());
	}

	return view.inverted()
	       * QVector3D((hmdScaledToWorld * eyeViewMatrix.inverted()).column(3));
}

QMatrix4x4 BasicCamera::eyeSpaceToWorldTransform() const
{
	// see TRANSFORMS
	return fullTransform.inverted() * fullEyeSpaceTransform;
}

QMatrix4x4 BasicCamera::cameraSpaceToWorldTransform() const
{
	// see TRANSFORMS
	return view.inverted();
}

QMatrix4x4 BasicCamera::seatedTrackedSpaceToWorldTransform() const
{
	// see TRANSFORMS
	return fullTransform.inverted() * fullSeatedTrackedSpaceTransform;
}

QMatrix4x4 BasicCamera::standingTrackedSpaceToWorldTransform() const
{
	// see TRANSFORMS
	return fullTransform.inverted() * fullStandingTrackedSpaceTransform;
}

QMatrix4x4 BasicCamera::hmdSpaceToWorldTransform() const
{
	// see TRANSFORMS
	return fullTransform.inverted() * fullHmdSpaceTransform;
}

QMatrix4x4 BasicCamera::hmdScaledSpaceToWorldTransform() const
{
	if(vrHandler.isEnabled())
	{
		return hmdScaledToWorld;
	}
	return view.inverted();
}

QMatrix4x4 BasicCamera::skyboxSpaceToWorldTransform() const
{
	// see TRANSFORMS
	return fullTransform.inverted() * fullSkyboxSpaceTransform;
}

QMatrix4x4 BasicCamera::screenToWorldTransform() const
{
	return view.inverted() * proj.inverted();
}

float BasicCamera::pixelVertFOV() const
{
	return pixVertFOV;
}

float BasicCamera::pixelSolidAngle() const
{
	float radPerPix(pixelVertFOV());
	// https://en.wikipedia.org/wiki/Solid_angle#Pyramid
	return 4.0 * asin(sin(radPerPix / 2.0) * sin(radPerPix / 2.0));
}

QMatrix4x4 BasicCamera::hmdScreenToWorldTransform(Side side) const
{
	if(side == Side::LEFT)
	{
		return hmdScaledToWorld * projLeft.inverted();
	}
	return hmdScaledToWorld * projRight.inverted();
}

QMatrix4x4 BasicCamera::noTrans(QMatrix4x4 const& matrix)
{
	QMatrix4x4 result(matrix);
	result.setColumn(3, QVector4D(0.f, 0.f, 0.f, 1.f));

	return result;
}

QMatrix4x4 BasicCamera::eyeDist(QMatrix4x4 const& matrix,
                                float eyeDistanceFactor)
{
	QMatrix4x4 result(matrix);
	result.setColumn(
	    3, QVector4D(eyeDistanceFactor * QVector3D(matrix.column(3)), 1.0f));

	return result;
}
