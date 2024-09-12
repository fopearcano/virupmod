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

#include "Timings.hpp"

std::unordered_map<QString, Timings::Timer, Timings::QStringHash>&
    Timings::timers()
{
	static std::unordered_map<QString, Timer, QStringHash> timers;
	return timers;
}

std::list<std::pair<QString, int>>& Timings::orderedNames()
{
	static std::list<std::pair<QString, int>> orderedNames;
	return orderedNames;
}

int& Timings::currentDepth()
{
	static int currentDepth = 0;
	return currentDepth;
}

void Timings::start(QString const& timerName, bool persistent)
{
	if(!timers().contains(timerName))
	{
		timers()[timerName] = {};
	}
	auto& timer(timers()[timerName]);
	if(timer.started)
	{
		qWarning() << "Attempt to start GPU timer" << timerName
		           << "multiple times.";
		return;
	}
	if(timer.ended)
	{
		qWarning() << "Attempt to start GPU timer" << timerName
		           << "that was already ended.";
		return;
	}

	orderedNames().emplace_back(timerName, currentDepth());
	++currentDepth();

	timer.startQuery.queryCounter();
	timer.cpuTimer.restart();
	timer.started    = true;
	timer.persistent = persistent;
}

void Timings::end(QString const& timerName)
{
	if(!timers().contains(timerName))
	{
		qWarning() << "Attempt to end GPU timer" << timerName
		           << "that never was started.";
		return;
	}
	auto& timer(timers()[timerName]);
	if(!timer.started)
	{
		qWarning() << "Attempt to end GPU timer" << timerName
		           << "that was started before but not in the current frame.";
		return;
	}
	if(timer.ended)
	{
		qWarning() << "Attempt to end GPU timer" << timerName
		           << "that was already ended.";
		return;
	}
	timer.cpuTime = timer.cpuTimer.nsecsElapsed();
	timer.endQuery.queryCounter();
	timer.ended = true;

	--currentDepth();
}

QList<Timings::CPUGPUTiming> Timings::getTimingsNanosecond()
{
	QList<CPUGPUTiming> result;
	std::vector<QString> toRemove;

	for(auto& pair : orderedNames())
	{
		auto const& name = pair.first;
		auto depth       = pair.second;
		auto& timer      = timers().at(name);
		if((!timer.started || !timer.ended) && !timer.persistent)
		{
			// badly used timer or ignored timer, remove it
			toRemove.push_back(name);
			continue;
		}

		result.push_back(
		    {name, timer.cpuTime,
		     timer.endQuery.getResult() - timer.startQuery.getResult(), depth});
		// reset checks
		timer.started = false;
		timer.ended   = false;
	}

	for(auto const& name : toRemove)
	{
		timers().erase(name);
	}

	orderedNames().clear();
	currentDepth() = 0;

	return result;
}

void Timings::cleanUp()
{
	timers().clear();
}
