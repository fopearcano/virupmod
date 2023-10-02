/*
    Copyright (C) 2020 Florian Cabot <florian.cabot@hotmail.fr>

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

#include "gl/GLHandler.hpp"

#include "gl/GLTexture.hpp"

#include <QMap>

#ifdef LIBKTX
#include "ktx.h"
#endif

float GLTexture::getMaxAnisotropicFilterSamples()
{
	GLfloat result;
	GLHandler::glf().glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &result);
	return result;
}

unsigned int& GLTexture::instancesCount()
{
	static unsigned int instancesCount = 0;
	return instancesCount;
}

QList<GLTexture*>& GLTexture::allTextures()
{
	static QList<GLTexture*> allTextures = {};
	return allTextures;
}

GLTexture::GLTexture(GLTexture&& other) noexcept
    : type(other.type)
    , glTexture(other.glTexture)
    , glTarget(other.glTarget)
    , internalFormat(other.internalFormat)
    , size(other.size)
    , samples(other.samples)
    , name(other.name)
    , metadata(other.metadata)
    , doClean(other.doClean)
{
	// prevent other from cleaning shader if it destroys itself
	other.doClean     = false;
	auto id           = allTextures().indexOf(&other);
	allTextures()[id] = this;
}

GLTexture& GLTexture::operator=(GLTexture&& other) noexcept
{
	if(this == &other)
	{
		return *this;
	}
	cleanUp();

	type           = other.type;
	glTexture      = other.glTexture;
	glTarget       = other.glTarget;
	internalFormat = other.internalFormat;
	size           = other.size;
	samples        = other.samples;
	name           = other.name;
	metadata       = other.metadata;
	doClean        = other.doClean;

	other.doClean     = false;
	auto id           = allTextures().indexOf(&other);
	allTextures()[id] = this;
	return *this;
}

GLTexture::GLTexture(Tex1DProperties const& properties, Sampler const& sampler,
                     Data const& data)
{
	++instancesCount();
	allTextures().append(this);
	type           = Type::TEX1D;
	glTarget       = properties.target;
	internalFormat = properties.internalFormat;
	size[0]        = properties.width;

	GLHandler::glf().glGenTextures(1, &glTexture);
	initData(data);
	setSampler(sampler);
}

GLTexture::GLTexture(Tex2DProperties const& properties, Sampler const& sampler,
                     Data const& data)
{
	++instancesCount();
	allTextures().append(this);
	type           = Type::TEX2D;
	glTarget       = properties.target;
	internalFormat = properties.internalFormat;
	size[0]        = properties.width;
	size[1]        = properties.height;

	GLHandler::glf().glGenTextures(1, &glTexture);
	initData(data);
	setSampler(sampler);
}

GLTexture::GLTexture(TexMultisampleProperties const& properties,
                     Sampler const& sampler)
{
	++instancesCount();
	allTextures().append(this);
	type           = Type::TEXMULTISAMPLE;
	glTarget       = GL_TEXTURE_2D_MULTISAMPLE;
	internalFormat = properties.internalFormat;
	size[0]        = properties.width;
	size[1]        = properties.height;
	samples        = properties.samples;

	GLHandler::glf().glGenTextures(1, &glTexture);
	GLHandler::glf().glBindTexture(glTarget, glTexture);
	GLHandler::glf().glTexImage2DMultisample(glTarget, properties.samples,
	                                         internalFormat, properties.width,
	                                         properties.height, GL_TRUE);
	// glGenerateMipmap(target);
	GLHandler::glf().glBindTexture(glTarget, 0);
	setSampler(sampler);
}

GLTexture::GLTexture(Tex3DProperties const& properties, Sampler const& sampler,
                     Data const& data)
{
	++instancesCount();
	allTextures().append(this);
	type           = Type::TEX3D;
	glTarget       = properties.target;
	internalFormat = properties.internalFormat;
	size[0]        = properties.width;
	size[1]        = properties.height;
	size[2]        = properties.depth;

	GLHandler::glf().glGenTextures(1, &glTexture);
	initData(data);
	setSampler(sampler);
}

GLTexture::GLTexture(TexCubemapProperties const& properties,
                     Sampler const& sampler, DataArray<6> const& data)
{
	++instancesCount();
	allTextures().append(this);
	type           = Type::TEXCUBEMAP;
	glTarget       = properties.target;
	internalFormat = properties.internalFormat;
	size[0]        = properties.side;
	size[1]        = properties.side;

	GLHandler::glf().glGenTextures(1, &glTexture);
	initData(data);
	setSampler(sampler);
}

GLTexture::GLTexture(QImage const& image, bool sRGB)
    : GLTexture(GLTexture::Tex2DProperties(image.width(), image.height(), sRGB),
                GLTexture::Sampler(GL_LINEAR, GL_REPEAT))
{
	QImage img_data = image.convertToFormat(QImage::Format_RGBA8888);
	setData({img_data.bits()});
}

GLTexture::GLTexture(QString const& texturePath, bool sRGB)
{
	++instancesCount();
	allTextures().append(this);
	auto i = texturePath.lastIndexOf('.');
	if((texturePath.size() == i + 4 && texturePath[i + 1] == 'k'
	    && texturePath[i + 2] == 't' && texturePath[i + 3] == 'x')
	   || (texturePath.size() == i + 5 && texturePath[i + 1] == 'k'
	       && texturePath[i + 2] == 't' && texturePath[i + 3] == 'x'
	       && texturePath[i + 4] == '2'))
	{
		// LOAD KTX
#ifdef LIBKTX
		ktxTexture* kTexture;
		KTX_error_code result;
		GLenum glerror;

		result = ktxTexture_CreateFromNamedFile(texturePath.toLatin1().data(),
		                                        KTX_TEXTURE_CREATE_NO_FLAGS,
		                                        &kTexture);
		if(result != KTX_SUCCESS)
		{
			qWarning() << "Can't load texture" << texturePath
			           << ": libktx returned error code" << result;
		}

		if(kTexture->isArray)
		{
			qWarning()
			    << "Texture" << texturePath
			    << "is an array which is unsupported by HydrogenVR for now."
			    << result;
		}

		GLHandler::glf().glGenTextures(1, &glTexture);
		// Fill metadata
		for(auto entry = kTexture->kvDataHead; entry != nullptr;
		    entry      = ktxHashList_Next(entry))
		{
			unsigned int len;
			char *bKey, *bVal;
			ktxHashListEntry_GetKey(entry, &len, &bKey);
			ktxHashListEntry_GetValue(
			    entry, &len,
			    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
			    reinterpret_cast<void**>(&bVal));
			QString key(bKey);
			QString prefix(key.left(4)), suffix(key.mid(4));
			if(prefix == "i32_")
			{
				// NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
				metadata[suffix] = *reinterpret_cast<int*>(bVal);
			}
			else if(prefix == "f32_")
			{
				// NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
				metadata[suffix] = *reinterpret_cast<float*>(bVal);
			}
			else
			{
				metadata[key] = QString(bVal);
			}
		}

		// Upload to OpenGL
		// Upload TEXTURE_3D manually
		if(kTexture->numDimensions == 3 && !kTexture->isArray
		   && kTexture->numFaces == 1 && metadata.contains("glinternalformat"))
		{
			// reload with data
			ktxTexture_Destroy(kTexture);
			ktxTexture_CreateFromNamedFile(
			    texturePath.toLatin1().data(),
			    KTX_TEXTURE_CREATE_LOAD_IMAGE_DATA_BIT, &kTexture);

			glTarget        = GL_TEXTURE_3D;
			type            = Type::TEX3D;
			GLenum glType   = GL_FLOAT;
			GLenum glFormat = GL_RGBA;

			if(metadata["glinternalformat"] == "GL_R32F")
			{
				internalFormat = GL_R32F;
				glType         = GL_FLOAT;
				glFormat       = GL_RED;
			}
			else if(metadata["glinternalformat"] == "GL_RG32F")
			{
				internalFormat = GL_RG32F;
				glType         = GL_FLOAT;
				glFormat       = GL_RG;
			}
			else if(metadata["glinternalformat"] == "GL_RGB32F")
			{
				internalFormat = GL_RGB32F;
				glType         = GL_FLOAT;
				glFormat       = GL_RGB;
			}
			else if(metadata["glinternalformat"] == "GL_RGBA32F")
			{
				internalFormat = GL_RGBA32F;
				glType         = GL_FLOAT;
				glFormat       = GL_RGBA;
			}
			else
			{
				qCritical() << "Unrecognized \"glinternalformat\" key value :"
				            << metadata["glinternalformat"];
			}

			size[0] = kTexture->baseWidth;
			size[1] = kTexture->baseHeight;
			size[2] = kTexture->baseDepth;

			GLHandler::glf().glGenTextures(1, &glTexture);
			use();
			GLHandler::glf().glTexParameteri(glTarget, GL_TEXTURE_MAX_LEVEL, 0);
			initData({kTexture->pData, glType, glFormat});
		}
		else
		{
			result = ktxTexture_GLUpload(kTexture, &glTexture, &glTarget,
			                             &glerror);
			if(result != KTX_SUCCESS)
			{
				if(result == KTX_INVALID_VALUE)
				{
					qCritical()
					    << "Can't load texture" << texturePath
					    << ": libktx error : This or target is NULL or the "
					       "size of a mip level is greater than the size of "
					       "the preceding level.";
				}
				else if(result == KTX_GL_ERROR)
				{
					qCritical()
					    << "Can't load texture" << texturePath
					    << ": libktx error : A GL error was raised by "
					       "glBindTexture, glGenTextures or gl*TexImage*. "
					       "The GL error will be returned in *glerror, if "
					       "glerror is not NULL.";
					qCritical() << "glError ==" << glerror;
				}
				else if(result == KTX_UNSUPPORTED_TEXTURE_TYPE)
				{
					qCritical()
					    << "Can't load texture" << texturePath
					    << ": libktx error : The type of texture is not "
					       "supported by the current OpenGL context.";
				}
				else
				{
					qCritical() << "Can't load texture" << texturePath
					            << ": libktx error : Unknown error.";
				}
			}
		}

		ktxTexture_Destroy(kTexture);

		switch(glTarget)
		{
			case GL_TEXTURE_1D:
				type = Type::TEX1D;
				break;
			case GL_TEXTURE_2D:
				type = Type::TEX2D;
				break;
			case GL_TEXTURE_3D:
				type = Type::TEX3D;
				break;
			case GL_TEXTURE_CUBE_MAP:
				type = Type::TEXCUBEMAP;
				break;
			case GL_TEXTURE_2D_MULTISAMPLE:
				type = Type::TEXMULTISAMPLE;
				break;
			default:
				qCritical()
				    << "Cannot recognize texture glTarget :" << glTarget;
				break;
		}
		size[0] = kTexture->baseWidth;
		size[1] = kTexture->baseHeight;
		size[2] = kTexture->baseDepth;

		GLHandler::glf().glBindTexture(glTarget, glTexture);
		GLHandler::glf().glGetTexLevelParameteriv(
		    glTarget, 0, GL_TEXTURE_INTERNAL_FORMAT, &internalFormat);
#else
		qWarning() << "Can't load texture" << texturePath
		           << ": HydrogenVR wasn't build with libktx support.";
#endif
	}
	else
	{
		// LOAD QIMAGE
		auto image(getImage(texturePath));
		Tex2DProperties properties(image.width(), image.height(), sRGB);
		type           = Type::TEX2D;
		glTarget       = properties.target;
		internalFormat = properties.internalFormat;
		size[0]        = properties.width;
		size[1]        = properties.height;

		GLHandler::glf().glGenTextures(1, &glTexture);
		QImage img_data = image.convertToFormat(QImage::Format_RGBA8888);
		initData({img_data.bits()});
		setSampler(Sampler(GL_LINEAR, GL_REPEAT));
	}
}

GLTexture::GLTexture(std::array<QImage, 6> const& images, bool sRGB)
    : GLTexture(GLTexture::TexCubemapProperties(images.at(0).width(), sRGB))
{
	std::array<GLvoid const*, 6> data = {};

	std::array<QImage, 6> img_data = {};

	for(unsigned int i(0); i < 6; ++i)
	{
		switch(i)
		{
			case static_cast<int>(CubemapFace::FRONT):
			case static_cast<int>(CubemapFace::TOP):
			case static_cast<int>(CubemapFace::BOTTOM):
			{
				QImage im(images.at(i).height(), images.at(i).width(),
				          QImage::Format_RGBA8888);
				for(int j(0); j < im.height(); ++j)
				{
					for(int k(0); k < im.width(); ++k)
					{
						im.setPixel(k, j, images.at(i).pixel(j, k));
					}
				}
				img_data.at(i) = im;
			}
			break;
			case static_cast<int>(CubemapFace::BACK):
			{
				QImage im(images.at(i).height(), images.at(i).width(),
				          QImage::Format_RGBA8888);
				for(int j(0); j < im.height(); ++j)
				{
					for(int k(0); k < im.width(); ++k)
					{
						im.setPixel(im.width() - k - 1, im.height() - j - 1,
						            images.at(i).pixel(j, k));
					}
				}
				img_data.at(i) = im;
			}
			break;
			case static_cast<int>(CubemapFace::LEFT):
				img_data.at(i) = images.at(i)
				                     .mirrored(false, true)
				                     .convertToFormat(QImage::Format_RGBA8888);
				break;
			case static_cast<int>(CubemapFace::RIGHT):
				img_data.at(i) = images.at(i)
				                     .mirrored(true, false)
				                     .convertToFormat(QImage::Format_RGBA8888);
				break;
			default:
				break;
		}
	}
	for(unsigned int i(0); i < 6; ++i)
	{
		data.at(i) = img_data.at(i).bits();
	}

	setData({data});
}

GLTexture::GLTexture(std::array<QString, 6> const& texturesPaths, bool sRGB)
    : GLTexture(getImages(texturesPaths), sRGB)
{
}

QSize GLTexture::getSize(unsigned int level) const
{
	GLint width, height;
	GLHandler::glf().glBindTexture(glTarget, glTexture);
	GLHandler::glf().glGetTexLevelParameteriv(glTarget, level, GL_TEXTURE_WIDTH,
	                                          &width);
	GLHandler::glf().glGetTexLevelParameteriv(glTarget, level,
	                                          GL_TEXTURE_HEIGHT, &height);
	GLHandler::glf().glBindTexture(glTarget, 0);

	return {width, height};
}

QString GLTexture::getTypeStr() const
{
	switch(type)
	{
		case Type::TEX1D:
			return "TEX1D";
		case Type::TEX2D:
			return "TEX2D";
		case Type::TEX3D:
			return "TEX3D";
		case Type::TEXMULTISAMPLE:
			return "TEXMULTISAMPLE";
		case Type::TEXCUBEMAP:
			return "TEXCUBEMAP";
		default:
			return "UNKNOWN";
	}
}

void GLTexture::generateMipmap(unsigned int baseLevel,
                               unsigned int maxLevel) const
{
	GLHandler::glf().glHint(GL_GENERATE_MIPMAP_HINT, GL_NICEST);
	GLHandler::glf().glBindTexture(glTarget, glTexture);
	GLHandler::glf().glTexParameteri(glTarget, GL_TEXTURE_MIN_FILTER,
	                                 GL_LINEAR_MIPMAP_LINEAR);
	GLHandler::glf().glTexParameteri(glTarget, GL_TEXTURE_BASE_LEVEL,
	                                 baseLevel);
	GLHandler::glf().glTexParameteri(glTarget, GL_TEXTURE_MAX_LEVEL, maxLevel);
	GLHandler::glf().glGenerateMipmap(glTarget);
	GLHandler::glf().glBindTexture(glTarget, 0);
}

unsigned int GLTexture::getHighestMipmapLevel() const
{
	QSize size(getSize());
	return static_cast<unsigned int>(
	    log2(size.width() > size.height() ? size.width() : size.height()));
}

QImage GLTexture::getContentAsImage(unsigned int level) const
{
	QSize size(getSize(level));

	GLint internalFormat;
	GLenum target(glTarget);
	if(target == GL_TEXTURE_CUBE_MAP)
	{
		target = GL_TEXTURE_CUBE_MAP_POSITIVE_X;
	}
	GLHandler::glf().glBindTexture(glTarget, glTexture);
	GLHandler::glf().glGetTexLevelParameteriv(
	    glTarget, level, GL_TEXTURE_INTERNAL_FORMAT,
	    &internalFormat);  // get internal format type of GL texture
	switch(internalFormat) // determine what type GL texture has...
	{
		case GL_RGB:
		{
			QImage result(size, QImage::Format::Format_RGB888);
			GLHandler::glf().glGetTexImage(target, level, GL_RGBA,
			                               GL_UNSIGNED_BYTE, result.bits());
			return result;
		}
		break;
		case GL_RGBA:
		{
			QImage result(size, QImage::Format::Format_RGBA8888);
			GLHandler::glf().glGetTexImage(target, level, GL_RGBA,
			                               GL_UNSIGNED_BYTE, result.bits());
			return result;
		}
		break;
		case GL_SRGB8_ALPHA8:
		{
			QImage result(size, QImage::Format::Format_RGBA8888);
			GLHandler::glf().glGetTexImage(target, level, GL_RGBA,
			                               GL_UNSIGNED_BYTE, result.bits());
			return result;
		}
		default: // unsupported type for now
			break;
	}

	return {};
}

std::vector<GLfloat> GLTexture::getContentAsData(unsigned int level) const
{
	QSize size(getSize(level));

	GLint internalFormat;
	GLHandler::glf().glBindTexture(glTarget, glTexture);
	GLHandler::glf().glGetTexLevelParameteriv(
	    glTarget, level, GL_TEXTURE_INTERNAL_FORMAT,
	    &internalFormat); // get internal format type of GL texture
	std::vector<GLfloat> result;
	if(internalFormat == GL_RGBA32F) // determine what type GL texture has...
	{
		result.resize(size.width() * size.height() * 4);
		GLHandler::glf().glGetTexImage(glTarget, level, GL_RGBA, GL_FLOAT,
		                               result.data());
	}
	return result;
}

float GLTexture::getAverageLuminance() const
{
	generateMipmap();
	unsigned int lvl = getHighestMipmapLevel() - 3;
	auto size        = getSize(lvl);
	auto buff(getContentAsData(lvl));
	float lastFrameAverageLuminance = 0.f;
	if(!buff.empty())
	{
		float coeffSum = 0.f;
		float halfWidth((size.width() - 1) / 2.f);
		float halfHeight((size.height() - 1) / 2.f);
		for(int i(0); i < size.width(); ++i)
		{
			for(int j(0); j < size.height(); ++j)
			{
				unsigned int id(j * size.width() + i);
				float lum(0.2126 * buff[4 * id] + 0.7152 * buff[4 * id + 1]
				          + 0.0722 * buff[4 * id + 2]);
				float coeff
				    = exp(-1 * pow((i - halfWidth) * 4.5 / halfWidth, 2));
				coeff *= exp(-1 * pow((j - halfHeight) * 4.5 / halfHeight, 2));
				coeffSum += coeff;
				lastFrameAverageLuminance += coeff * lum;
			}
		}
		lastFrameAverageLuminance /= coeffSum;
	}
	return lastFrameAverageLuminance;
}

void GLTexture::setSampler(Sampler const& sampler) const
{
	GLHandler::glf().glBindTexture(glTarget, glTexture);
	GLHandler::glf().glTexParameteri(glTarget, GL_TEXTURE_MIN_FILTER,
	                                 sampler.filter);
	GLHandler::glf().glTexParameteri(glTarget, GL_TEXTURE_MAG_FILTER,
	                                 sampler.filter);
	GLHandler::glf().glTexParameteri(glTarget, GL_TEXTURE_WRAP_S,
	                                 sampler.wraps);
	GLHandler::glf().glTexParameteri(glTarget, GL_TEXTURE_WRAP_T,
	                                 sampler.wrapt);
	GLHandler::glf().glTexParameteri(glTarget, GL_TEXTURE_WRAP_R,
	                                 sampler.wrapr);
	GLHandler::glf().glTexParameterf(glTarget, GL_TEXTURE_MAX_ANISOTROPY_EXT,
	                                 sampler.anisotropicFilterSamples);
	GLHandler::glf().glBindTexture(glTarget, 0);
}

void GLTexture::setData(Data const& data) const
{
	GLHandler::glf().glBindTexture(glTarget, glTexture);
	switch(type)
	{
		case Type::TEX1D:
			GLHandler::glf().glTexSubImage1D(glTarget, 0, 0, size[0],
			                                 data.format, data.type, data.ptr);
			break;
		case Type::TEX2D:
			GLHandler::glf().glTexSubImage2D(glTarget, 0, 0, 0, size[0],
			                                 size[1], data.format, data.type,
			                                 data.ptr);
			break;
		case Type::TEXMULTISAMPLE:
			qWarning() << "Attempt to set data of a multisampled texture.";
			break;
		case Type::TEX3D:
			GLHandler::glf().glTexSubImage3D(glTarget, 0, 0, 0, 0, size[0],
			                                 size[1], size[2], data.format,
			                                 data.type, data.ptr);
			break;
		case Type::TEXCUBEMAP:
			qWarning()
			    << "Attempt to set data of cubemap texture for only one face.";
			break;
	}
	// glGenerateMipmap(format);
	GLHandler::glf().glBindTexture(glTarget, 0);
}

void GLTexture::setData(DataArray<6> const& data) const
{
	if(type != Type::TEXCUBEMAP)
	{
		qWarning() << "Attempt to set cubemap data on another texture type.";
		return;
	}
	GLHandler::glf().glBindTexture(glTarget, glTexture);
	for(unsigned int i(0); i < 6; ++i)
	{
		GLHandler::glf().glTexSubImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0,
		                                 0, 0, size[0], size[1], data.format,
		                                 data.type, data.ptrs.at(i));
	}
	// glGenerateMipmap(format);
	GLHandler::glf().glBindTexture(glTarget, 0);
}

void GLTexture::setData(const unsigned char* red, const unsigned char* green,
                        const unsigned char* blue, const unsigned char* alpha)
{
	const unsigned int nPix(size[0] * size[1] * size[2]);
	std::vector<GLubyte> image(nPix * 4);
	for(unsigned int i(0); i < nPix; ++i)
	{
		image[4 * i]     = red[i];
		image[4 * i + 1] = green[i];
		image[4 * i + 2] = blue[i];
		image[4 * i + 3] = alpha != nullptr ? alpha[i] : 255;
	}
	setData({image.data()});
}

void GLTexture::use(GLenum textureUnit) const
{
	GLHandler::glf().glActiveTexture(textureUnit);
	GLHandler::glf().glBindTexture(glTarget, glTexture);
}

void GLTexture::cleanUp()
{
	if(!doClean)
	{
		return;
	}
	--instancesCount();
	allTextures().removeOne(this);
	GLHandler::glf().glDeleteTextures(1, &glTexture);
	doClean = false;
}

void GLTexture::initData(Data const& data) const
{
	GLHandler::glf().glBindTexture(glTarget, glTexture);
	switch(type)
	{
		case Type::TEX1D:
			GLHandler::glf().glTexImage1D(glTarget, 0, internalFormat, size[0],
			                              0, data.format, data.type, data.ptr);
			break;
		case Type::TEX2D:
			GLHandler::glf().glTexImage2D(glTarget, 0, internalFormat, size[0],
			                              size[1], 0, data.format, data.type,
			                              data.ptr);
			break;
		case Type::TEXMULTISAMPLE:
			qWarning() << "Attempt to set data of a multisampled texture.";
			break;
		case Type::TEX3D:
			GLHandler::glf().glTexImage3D(glTarget, 0, internalFormat, size[0],
			                              size[1], size[2], 0, data.format,
			                              data.type, data.ptr);
			break;
		case Type::TEXCUBEMAP:
			qWarning()
			    << "Attempt to set data of cubemap texture for only one face.";
			break;
	}
	// glGenerateMipmap(format);
	GLHandler::glf().glBindTexture(glTarget, 0);
}

void GLTexture::initData(DataArray<6> const& data) const
{
	if(type != Type::TEXCUBEMAP)
	{
		qWarning() << "Attempt to set cubemap data on another texture type.";
		return;
	}
	GLHandler::glf().glBindTexture(glTarget, glTexture);
	for(unsigned int i(0); i < 6; ++i)
	{
		GLHandler::glf().glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0,
		                              internalFormat, size[0], size[1], 0,
		                              data.format, data.type, data.ptrs.at(i));
	}
	// glGenerateMipmap(format);
	GLHandler::glf().glBindTexture(glTarget, 0);
}

QImage GLTexture::getImage(QString const& path)
{
	QImage img_data;
	if(!img_data.load(path))
	{
		qWarning() << "Could not load Texture \"" << path << "\"" << '\n';
		return {};
	}
	return img_data;
}

std::array<QImage, 6> GLTexture::getImages(std::array<QString, 6> const& paths)
{
	std::array<QImage, 6> images;
	for(unsigned int i(0); i < 6; ++i)
	{
		if(!images.at(i).load(paths.at(i)))
		{
			qWarning() << "Could not load Texture \"" << paths.at(i) << "\""
			           << '\n';
			return {};
		}
	}

	return images;
}
