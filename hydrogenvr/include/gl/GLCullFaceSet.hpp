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

#ifndef GLCULLFACESET_HPP
#define GLCULLFACESET_HPP

#include "GLStateSet.hpp"

/** @brief Represents an OpenGL cull face state set using RAII for compile-time
 * state stack.
 *
 * This wraps calls to OpenGL to make sure the OpenGL enabled features and set
 * states are clean. The class is constant, stack-allocated only, non-copyable
 * and non-movable to make sure the cull face set scopes don't overlap in a "
 * set A; set B; revert A; revert B" way.
 */
class GLCullFaceSet
{
  public:
	struct CullFaceState
	{
		/** by default :
		 faceToCull            = GL_BACK
		 frontFaceWindingOrder = GL_CCW
		 */
		CullFaceState();
		CullFaceState(int faceToCull);
		CullFaceState(int faceToCull, int frontFaceWindingOrder)
		    : faceToCull(faceToCull)
		    , frontFaceWindingOrder(frontFaceWindingOrder)
		{
		}
		int faceToCull;
		int frontFaceWindingOrder;
	};

	GLCullFaceSet()                     = delete;
	GLCullFaceSet(GLCullFaceSet const&) = delete;
	GLCullFaceSet(GLCullFaceSet&&)      = delete;
	/** @brief Specify the desired cull face set
	 *
	 * As GLHandler enables GL_CULL_FACE by default, there is no need to set it.
	 * If you want to disable face culling, you should use GLStateSet to disable
	 * it.
	 *
	 * There is no check against the global state, all the corresponding
	 * GL functions will be issued even if they don't change anything.
	 *
	 * Parameters are passed to the glCullFace and glFrontFace functions as is.
	 *
	 * @param stateSet is the new CullFaceState to set.
	 */
	GLCullFaceSet(CullFaceState const& stateSet);
	GLCullFaceSet& operator=(GLCullFaceSet const&) = delete;
	/** @brief Restores global state to where it was before calling the
	 * constructor
	 */
	~GLCullFaceSet();

	static void* operator new(std::size_t)   = delete;
	static void* operator new[](std::size_t) = delete;
	static void operator delete(void*)       = delete;
	static void operator delete[](void*)     = delete;

	/** @brief Returns the global state tracked by GLCullFaceSet (which can
	 * differ from the true global state if GL functions were used outside
	 * of GLCullFaceSet).
	 */
	static CullFaceState getGlobalState();
	/** @brief Retrieves the global state using GL get functions
	 */
	static CullFaceState getTrueGlobalState();
	/** @brief Prints the difference between two global states
	 */
	static void printDifferences(CullFaceState const& globalState0,
	                             CullFaceState const& globalState1);

  private:
	CullFaceState revertSet;
	static CullFaceState& globalState();
};

#endif // GLCULLFACESET_HPP
