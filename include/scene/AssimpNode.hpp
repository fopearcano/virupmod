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

#ifndef ASSIMPNODE_HPP
#define ASSIMPNODE_HPP

#include "Model.hpp"
#include "scene/Node.hpp"

class AssimpNode : public Node
{
  public:
	AssimpNode(QString const& src, std::map<QString, Node*>& nodesDict,
	           QColor const& defaultDiffuseColor = {0xff, 0x09, 0xf7});
	virtual std::vector<std::pair<GLMesh const&, QMatrix4x4>>
	    getShadowCastingMeshes() const override;

  protected:
	virtual QMatrix4x4 preMultiplyTransform() const override;
	virtual void doRender(BasicCamera const& cam,
	                      std::vector<Light const*> const& lights,
	                      GLTexture const& brdfLUT) override;

  private:
	Model model;
};

#endif // ASSIMPNODE_HPP
