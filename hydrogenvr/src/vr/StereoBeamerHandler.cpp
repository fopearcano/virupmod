#include "vr/StereoBeamerHandler.hpp"

#include "Renderer.hpp"

#include <numbers>

bool StereoBeamerHandler::init(Renderer const& renderer,
                               ToneMappingModel const& tmm)
{
	VRHandler::init(renderer, tmm);
	hmdPosMatrix = QMatrix4x4();
	enabled      = true;
	return true;
}

QSize StereoBeamerHandler::getEyeRenderTargetSize() const
{
	auto fullRTSize(renderer->getSize(true));
	if(!forceLeft && !forceRight)
	{
		fullRTSize.setWidth(fullRTSize.width() / 2);
	}
	return fullRTSize;
}

float StereoBeamerHandler::getFrameTiming() const
{
	return -1.f;
}

const Controller* StereoBeamerHandler::getController(Side /*side*/) const
{
	return nullptr;
}

const Hand* StereoBeamerHandler::getHand(Side /*side*/) const
{
	return nullptr;
}

QMatrix4x4 StereoBeamerHandler::getSeatedToStandingAbsoluteTrackingPos() const
{
	return {};
}

QSizeF StereoBeamerHandler::getPlayAreaSize() const
{
	return {0.f, 0.f};
}

std::vector<QVector3D> StereoBeamerHandler::getPlayAreaQuad() const
{
	std::vector<QVector3D> result;
	result.emplace_back();
	result.emplace_back();
	result.emplace_back();
	result.emplace_back();

	return result;
}

void StereoBeamerHandler::prepareRendering(Side eye)
{
	currentRenderingEye = eye;
}

void StereoBeamerHandler::renderControllers() const {}

void StereoBeamerHandler::renderHands() const {}

void StereoBeamerHandler::submitRendering(GLFramebufferObject const& /*fbo*/) {}

bool StereoBeamerHandler::pollEvent(Event& /*e*/)
{
	return false;
}

void StereoBeamerHandler::close()
{
	enabled = false;
}

QMatrix4x4 StereoBeamerHandler::getEyeViewMatrix(Side eye) const
{
	QMatrix4x4 res;
	res.translate(stereoMultiplier
	              * QVector3D(eye == Side::LEFT ? eyeShiftDist : -eyeShiftDist,
	                          0.f, 0.f /*-0.015f*/));
	return res;
}

// https://stackoverflow.com/questions/8948493/visual-studio-doesnt-allow-me-to-use-certain-variable-names
#undef far
#undef near

QMatrix4x4
    StereoBeamerHandler::getProjectionMatrix(QMatrix4x4 const& angleShiftMat,
                                             Side eye, float nearPlan,
                                             float farPlan) const
{
	auto deltaRel(QSettings()
	                  .value("vr/virtualcamshift")
	                  .value<QVector3D>()); // move cam in height units

	// add eye displacement
	const double screenHeight
	    = QSettings().value("vr/screenheight").toDouble(); // m
	QVector3D eyeDisplacement(eye == Side::LEFT ? eyeShiftDist : -eyeShiftDist,
	                          0.f, 0.f /*-0.015f*/);
	eyeDisplacement /= screenHeight; // in height unit
	deltaRel -= eyeDisplacement;

	deltaRel = angleShiftMat.mapVector(deltaRel);

	const float vFOV(renderer->getVerticalFOV() * std::numbers::pi / 180.0);
	const float aspRat(renderer->getAspectRatioFromFOV());
	float left(-nearPlan * aspRat * tan(vFOV / 2.0));
	float top(nearPlan * tan(vFOV / 2.0));
	float right(-left);
	float bottom(-top);

	const float height(top - bottom);

	right -= deltaRel.x() * height;
	left -= deltaRel.x() * height;
	top -= deltaRel.y() * height;
	bottom -= deltaRel.y() * height;

	const float near(nearPlan + deltaRel.z() * height);
	right *= nearPlan / near;
	left *= nearPlan / near;
	top *= nearPlan / near;
	bottom *= nearPlan / near;

	QMatrix4x4 result;
	result.frustum(left, right, bottom, top, nearPlan, farPlan);
	return result;
}

void StereoBeamerHandler::resetPos() {}
