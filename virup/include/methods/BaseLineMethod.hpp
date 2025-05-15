#ifndef BASELINEMETHOD_H
#define BASELINEMETHOD_H

#include "Method.hpp"
#include "gl/GLHandler.hpp"
#include "methods/OctreeLOD.hpp"

class BaseLineMethod : public Method
{
	Q_OBJECT
  public:
	BaseLineMethod();
	explicit BaseLineMethod(std::string const& shadersCommonName);
	BaseLineMethod(std::string const& vertexShaderPath,
	               std::string const& fragmentShaderPath);
	BBox getDataBoundingBox() const override { return dataBBox; };
	void render(Camera const& camera) override;

  public slots:
	std::string getName() const override { return "Base Line"; };
	void init(std::vector<float>& gazVertices,
	          std::vector<float>& starsVertices,
	          std::vector<float>& darkMatterVertices) override;
	void init(std::string const& gazPath, std::string const& starsPath,
	          std::string const& darkMatterPath) override;

  private:
	GLMesh gazMesh;
	GLMesh starsMesh;
	GLMesh darkMatterMesh;

	BBox dataBBox = {};
};

#endif // BASELINEMETHOD_H
