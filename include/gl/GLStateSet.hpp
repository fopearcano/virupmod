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

#ifndef GLSTATESET_HPP
#define GLSTATESET_HPP

#include <unordered_map>

/** @brief Represents an OpenGL state set using RAII for compile-time state
 * stack.
 *
 * This wraps glEnable/glDisable to make sure the OpenGL enabled features state
 * is clean. The class is constant, stack-allocated only, non-copyable and
 * non-movable to make sure the state sets scopes don't overlap in a "set
 * A; set B; revert A; revert B" way.
 */
class GLStateSet
{
	struct GlobalState
	{
		GlobalState();
		std::unordered_map<int, bool> map;
	};

  public:
	GLStateSet()                  = delete;
	GLStateSet(GLStateSet const&) = delete;
	GLStateSet(GLStateSet&&)      = delete;
	/** @brief Specify the desired state set
	 *
	 * There is no check against the global state, all the corresponding
	 * glEnable/glDisable will be issued even if they don't change anything.
	 *
	 * @param stateSet are pairs of GLenum of a feature and if it's enabled
	 * (true) or disabled (false).
	 */
	GLStateSet(std::unordered_map<int, bool> const& stateSet);
	GLStateSet& operator=(GLStateSet const&) = delete;
	/** @brief Restores global state to where it was before calling the
	 * constructor
	 */
	~GLStateSet();

	static void* operator new(std::size_t)   = delete;
	static void* operator new[](std::size_t) = delete;
	static void operator delete(void*)       = delete;
	static void operator delete[](void*)     = delete;

	/** @brief Returns the global state tracked by GLStateSet (which can
	 * differ from the true global state if glEnable/glDisable was used outside
	 * of GLStateSet).
	 */
	static std::unordered_map<int, bool> getGlobalState();
	/** @brief Retrieves the global state using glIsEnabled
	 */
	static std::unordered_map<int, bool> getTrueGlobalState();
	/** @brief Prints the difference between two global states
	 */
	static void
	    printDifferences(std::unordered_map<int, bool> const& globalState0,
	                     std::unordered_map<int, bool> const& globalState1);

  private:
	std::unordered_map<int, bool> revertSet;
	static std::unordered_map<int, bool>& globalState();
};

#endif // GLSTATESET_HPP
