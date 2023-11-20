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

#ifndef GLBLENDSET_HPP
#define GLBLENDSET_HPP

#include "GLStateSet.hpp"

/** @brief Represents an OpenGL blend state set using RAII for compile-time
 * state stack.
 *
 * This wraps calls to OpenGL to make sure the OpenGL enabled features and set
 * states are clean. The class is constant, stack-allocated only, non-copyable
 * and non-movable to make sure the blend set scopes don't overlap in a " set A;
 * set B; revert A; revert B" way.
 */
class GLBlendSet
{
  public:
	struct BlendState
	{
		/** by default :
		 blendfuncSfactor = GL_SRC_ALPHA
		 blendfuncDfactor = GL_ONE_MINUX_SRC_ALPHA
		 depthMask        = GL_FALSE
		 */
		BlendState();
		BlendState(int blendfuncSfactor);
		BlendState(int blendfuncSfactor, int blendfuncDfactor,
		           bool depthMask = false)
		    : blendfuncSfactor(blendfuncSfactor)
		    , blendfuncDfactor(blendfuncDfactor)
		    , depthMask(depthMask)
		{
		}
		int blendfuncSfactor;
		int blendfuncDfactor;
		bool depthMask;
	};

	GLBlendSet()                  = delete;
	GLBlendSet(GLBlendSet const&) = delete;
	GLBlendSet(GLBlendSet&&)      = delete;
	/** @brief Specify the desired blend set
	 *
	 * This implies GL_BLEND should be set as there's no point setting a blend
	 * set without using GL_BLEND.
	 *
	 * There is no check against the global state, all the corresponding
	 * GL functions will be issued even if they don't change anything.
	 *
	 * Parameters are passed to the glBlendFunc function as is.
	 *
	 * @param stateSet is the new BlendState to set.
	 */
	GLBlendSet(BlendState const& stateSet);
	GLBlendSet& operator=(GLBlendSet const&) = delete;
	/** @brief Restores global state to where it was before calling the
	 * constructor
	 */
	~GLBlendSet();

	static void* operator new(std::size_t)   = delete;
	static void* operator new[](std::size_t) = delete;
	static void operator delete(void*)       = delete;
	static void operator delete[](void*)     = delete;

	/** @brief Returns the global state tracked by GLBlendSet (which can
	 * differ from the true global state if GL functions were used outside
	 * of GLBlendSet).
	 */
	static BlendState getGlobalState();
	/** @brief Retrieves the global state using GL get functions
	 */
	static BlendState getTrueGlobalState();
	/** @brief Prints the difference between two global states
	 */
	static void printDifferences(BlendState const& globalState0,
	                             BlendState const& globalState1);

  private:
	BlendState revertSet;
	static BlendState& globalState();

	GLStateSet enableBlendStateSet;
};

#endif // GLBLENDSET_HPP
