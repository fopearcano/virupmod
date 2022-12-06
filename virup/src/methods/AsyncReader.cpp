/*
    Copyright (C) 2022 Florian Cabot <florian.cabot@hotmail.fr>

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

#include "methods/AsyncReader.hpp"

#include "methods/OctreeLOD.hpp"

#include <QThreadPool>

namespace AR
{

class Thread : public QThread
{
  public:
	virtual void run() override;

	std::deque<std::queue<OctreeLOD*>> queues;

	bool doRun = true;

  private:
	static std::mutex& lock();
	static void updateOctree(OctreeLOD* octree);
};

std::mutex& Thread::lock()
{
	static std::mutex lock;
	return lock;
}

void Thread::updateOctree(OctreeLOD* octree)
{
	if(octree->state == AsyncReader::State::CANCEL)
	{
		octree->unload();
		return;
	}
	lock().lock();
	octree->readOwnData(*octree->getFile());
	lock().unlock();
	if(octree->state == AsyncReader::State::CANCEL)
	{
		octree->unload();
	}
	else
	{
		octree->state = AsyncReader::State::READ;
	}
}

void Thread::run()
{
	while(doRun)
	{
		bool allEmpty(true);
		for(unsigned int i(0); i < queues.size(); ++i)
		{
			auto& queue(queues[i]);
			if(queue.empty())
			{
				continue;
			}

			allEmpty = false;
			auto oct = queue.front();
			queue.pop();
			updateOctree(oct);
			break;
		}
		if(allEmpty)
		{
			usleep(500);
		}
	}
}

} // namespace AR

AR::Thread& AsyncReader::mainThread()
{
	static AR::Thread mainThread;
	return mainThread;
}

void AsyncReader::load(OctreeLOD& octree)
{
	if(!mainThread().isRunning())
	{
		mainThread().start();
	}

	auto i(octree.getLevel());
	if(mainThread().queues.size() <= i)
	{
		mainThread().queues.resize(i + 1);
	}

	mainThread().queues[i].push(&octree);
}

void AsyncReader::clean()
{
	mainThread().doRun = false;
	mainThread().wait();
}
