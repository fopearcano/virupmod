#include "methods/BaseLineMethodTex.hpp"

BaseLineMethodTex::BaseLineMethodTex()
    : BaseLineMethod("gaz")
    , tex("data/virup/images/particle.png")
{
}

void BaseLineMethodTex::render(Camera const& camera)
{
	const GLStateSet glState({{GL_PROGRAM_POINT_SIZE, true}});
	shaderProgram.setUniform("scale", static_cast<float>(camera.scale));
	GLHandler::useTextures({&tex});
	BaseLineMethod::render(camera);
}
