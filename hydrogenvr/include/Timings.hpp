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

#ifndef TIMINGS_HPP
#define TIMINGS_HPP

#include "gl/GLHandler.hpp"

#include <QElapsedTimer>
#include <unordered_map>

class AbstractMainWin;

class Timings
{
	struct Timer
	{
		GLQuery startQuery;
		GLQuery endQuery;
		bool started = false;
		bool ended   = false;
		QElapsedTimer cpuTimer;
		uint64_t cpuTime;
		bool persistent = false;
	};

	struct CPUGPUTiming
	{
		QString name;
		uint64_t cpu;
		uint64_t gpu;
		int depth;
	};

	struct QStringHash
	{
		std::size_t operator()(const QString& key) const
		{
			return qHash(key); // Utilize Qt's built-in hashing function
		}
	};

  public:
	Timings() = delete;
	static void start(QString const& timerName, bool persistent = false);
	static void end(QString const& timerName);

  private:
	friend AbstractMainWin;
	// pair(name, pair(cpuTiming, gpuTiming))
	static QList<CPUGPUTiming> getTimingsNanosecond();
	static void cleanUp();

	// true private
	static std::unordered_map<QString, Timer, QStringHash>& timers();

	static std::list<std::pair<QString, int>>& orderedNames();
	static int& currentDepth();
};

#endif // TIMINGS_HPP
