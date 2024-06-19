/*
    Copyright (C) 2024 Florian Cabot <florian.cabot@hotmail.fr>

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

#include "ZSTD.hpp"

#include <QtDebug>
#ifdef LIBZSTD
#include <zstd.h>
#endif

bool ZSTD::isEnabled()
{
#ifdef LIBZSTD
	return true;
#else
	return false;
#endif
}

QByteArray ZSTD::compress(QByteArray const& source, int compressionLevel)
{
#ifdef LIBZSTD
	auto const destMaxSize = ZSTD_compressBound(source.size());
	QByteArray destination;
	destination.resize(destMaxSize);
	auto const actualDestSize
	    = ZSTD_compress(destination.data(), destMaxSize, source.data(),
	                    source.size(), compressionLevel);
	destination.resize(actualDestSize);
	return destination;
#else
	qCritical() << "Attempt to compress using libstd, which hasn't been "
	               "enabled in this HydrogenVR build.";
	return {};
#endif
}

QByteArray ZSTD::decompress(QByteArray const& source)
{
#ifdef LIBZSTD
	auto const destSize
	    = ZSTD_getFrameContentSize(source.data(), source.size());

	QByteArray destination;
	destination.resize(destSize);
	ZSTD_decompress(destination.data(), destSize, source.data(), source.size());

	return destination;
#else
	qCritical() << "Attempt to decompress using libstd, which hasn't been "
	               "enabled in this HydrogenVR build.";
	return {};
#endif
}
