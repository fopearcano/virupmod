/*
    Copyright (C) 2019 Florian Cabot <florian.cabot@hotmail.fr>

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

#include "AsyncTexture.hpp"

bool& AsyncTexture::forceSync()
{
	static bool forceSync(false);
	return forceSync;
}

std::list<std::pair<std::unique_ptr<at::WorkerThread>,
                    std::unique_ptr<GLPixelBufferObject>>>&
    AsyncTexture::waitingForDeletion()
{
	static std::list<std::pair<std::unique_ptr<at::WorkerThread>,
	                           std::unique_ptr<GLPixelBufferObject>>>
	    waitingForDeletion = {};
	return waitingForDeletion;
}

AsyncTexture::AsyncTexture(QString const& path, QColor const& defaultColor,
                           bool sRGB)
    : defaultTex(GLTexture::Tex2DProperties(1, 1, sRGB))
    , sRGB(sRGB)
    , averageColor(defaultColor)
{
	unsigned char color[4];
	color[0] = defaultColor.red();
	color[1] = defaultColor.green();
	color[2] = defaultColor.blue();
	color[3] = defaultColor.alpha();
	defaultTex.setData({&(color[0])});

	if(path.isEmpty())
	{
		emptyPath = true;
		return;
	}

	QImageReader imReader(path);
	QSize size(imReader.size());

	pbo = std::make_unique<GLPixelBufferObject>(size);

	thread = std::make_unique<at::WorkerThread>(path, pbo->getMappedData());
	thread->start();
}

AsyncTexture::AsyncTexture(QString const& path, unsigned int width,
                           unsigned int height, QColor const& defaultColor,
                           bool sRGB, bool forbidUpSample)
    : defaultTex(GLTexture::Tex2DProperties(1, 1, sRGB))
    , sRGB(sRGB)
    , averageColor(defaultColor)
{
	unsigned char color[4];
	color[0] = defaultColor.red();
	color[1] = defaultColor.green();
	color[2] = defaultColor.blue();
	color[3] = defaultColor.alpha();
	defaultTex.setData({&(color[0])});

	if(path.isEmpty())
	{
		emptyPath = true;
		return;
	}

	QImageReader imReader(path);
	QSize size(imReader.size());

	if(forbidUpSample
	   && (width > static_cast<unsigned int>(size.width())
	       || height > static_cast<unsigned int>(size.height())))
	{
		pbo = std::make_unique<GLPixelBufferObject>(size);

		thread = std::make_unique<at::WorkerThread>(path, pbo->getMappedData());
		thread->start();
	}
	else
	{
		pbo = std::make_unique<GLPixelBufferObject>(width, height);

		thread = std::make_unique<at::WorkerThread>(path, pbo->getMappedData(),
		                                            width, height);
		thread->start();
	}
}

GLTexture const& AsyncTexture::getTexture()
{
	if(emptyPath)
	{
		return defaultTex;
	}

	if(loaded)
	{
		return *tex;
	}

	if(!thread->isFinished())
	{
		if(forceSync())
		{
			thread->wait();
		}
		else
		{
			return defaultTex;
		}
	}

	tex = pbo->copyContentToNewTex(sRGB);
	pbo.reset();
	tex->generateMipmap();
	unsigned int lastMipmap(tex->getHighestMipmapLevel());
	averageColor = tex->getContentAsImage(lastMipmap).pixelColor(0, 0);
	thread.reset();
	loaded = true;

	return *tex;
}

AsyncTexture::~AsyncTexture()
{
	if(!emptyPath && !loaded && !thread->isFinished())
	{
		thread->setPriority(QThread::LowestPriority);
		waitingForDeletion().emplace_back(std::move(thread), std::move(pbo));
	}
}

void AsyncTexture::garbageCollect(bool force)
{
	// go in reverse because of possible deletions
	for(auto it(waitingForDeletion().begin());
	    it != waitingForDeletion().end();)
	{
		if(force)
		{
			it->first->wait();
		}
		if(it->first->isFinished())
		{
			it = waitingForDeletion().erase(it);
		}
		else
		{
			++it;
		}
	}
}
