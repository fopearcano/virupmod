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

#ifndef ASYNCREADER_HPP
#define ASYNCREADER_HPP

#include <QRunnable>
#include <QThread>

class OctreeLOD;

namespace AR
{
class Thread;
} // namespace AR

class AsyncReader
{
  public:
	enum class State
	{
		IDLE,   // is not in the ticketQueues (for Octree means completly loaded
		        // or don't need to be loaded)
		WAIT,   // inside the ticketQueues, waiting for reading
		CANCEL, // inside the ticketQueues, waiting to just be popped and marked
		        // IDLE (! Octree needs to handle this case if wanting to queue
		        // itself once again)
		READ, // data has been read, and ticket popped from the ticketQueues by
		      // the reader
	};
	AsyncReader() = default;

	static void load(OctreeLOD& octree);
	static void clean();

  private:
	static AR::Thread& mainThread();
};

#endif // ASYNCREADER_HPP
