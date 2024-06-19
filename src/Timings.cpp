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

void Timings::start(QString const& timerName, bool persistent)
{
	if(timers().count(timerName) == 0)
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
	timer.startQuery.queryCounter();
	timer.cpuTimer.restart();
	timer.started    = true;
	timer.persistent = persistent;
}

void Timings::end(QString const& timerName)
{
	if(timers().count(timerName) == 0)
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
}

QList<QPair<QString, QPair<uint64_t, uint64_t>>> Timings::getTimingsNanosecond()
{
	QList<QPair<QString, QPair<uint64_t, uint64_t>>> result;
	std::vector<QString> toRemove;

	for(auto& pair : timers())
	{
		if((!pair.second.started || !pair.second.ended)
		   && !pair.second.persistent)
		{
			// badly used timer or ignored timer, remove it
			toRemove.push_back(pair.first);
			continue;
		}

		result.push_back(
		    {pair.first,
		     {pair.second.cpuTime, pair.second.endQuery.getResult()
		                               - pair.second.startQuery.getResult()}});
		// reset checks
		pair.second.started = false;
		pair.second.ended   = false;
	}

	for(auto const& name : toRemove)
	{
		timers().erase(name);
	}

	return result;
}

void Timings::cleanUp()
{
	timers().clear();
}
