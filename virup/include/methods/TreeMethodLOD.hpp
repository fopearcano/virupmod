#ifndef TREEMETHODLOD_H
#define TREEMETHODLOD_H

#include <QElapsedTimer>
#include <QProgressDialog>
#include <chrono>
#include <future>
#include <thread>

#include "Method.hpp"
#include "OctreeLOD.hpp"
#include "PIDController.hpp"
#include "VolumetricModel.hpp"

class TreeMethodLOD : public Method
{
	Q_OBJECT
  public:
	TreeMethodLOD();
	TreeMethodLOD(std::string const& shadersCommonName);
	TreeMethodLOD(std::string const& vertexShaderPath,
	              std::string const& fragmentShaderPath);
	virtual std::string getName() const override { return "Tree LOD"; };
	virtual void init(std::vector<float>& gasVertices,
	                  std::vector<float>& starsVertices,
	                  std::vector<float>& darkMatterVertices) override;
	virtual void init(std::string const& gasPath, std::string const& starsPath,
	                  std::string const& darkMatterPath) override;
	void init(QStringList const& gasFiles, QStringList const& starsFiles,
	          QStringList const& dmFiles);
	virtual BBox getDataBoundingBox() const override;
	uint64_t getOctreesTotalDataSize() const;
	bool preloadOctreesLevel(unsigned int level,
	                         QProgressDialog* progress = nullptr);
	void update(Camera const& camera);
	void update(Camera const& camera, QMatrix4x4 const& model,
	            QVector3D const& campos);
	virtual void render(Camera const& camera) override;
	void render(Camera const& camera, QMatrix4x4 const& model,
	            QVector3D const& campos, float unitInKpc);
	void dumpOctreesStates(QString const& dirPath,
	                       QString const& filePathPrefix);
	void unload();
	void cleanUp();
	virtual ~TreeMethodLOD();

	bool silent = false;

  protected:
	VolumetricModel* dustModel = nullptr;
	std::vector<OctreeLOD> gasTrees;
	std::vector<OctreeLOD> starsTrees;
	std::vector<OctreeLOD> darkMatterTrees;
	VolumetricModel* hiiModel = nullptr;

	// ugly fix for pointSize problems
	bool setPointSize = true;

	static void loadOctreeFromFile(std::string const& path,
	                               std::vector<OctreeLOD>& container,
	                               std::string const& name,
	                               GLShaderProgram const& shaderProgram,
	                               bool silent);
	static void initOctree(OctreeLOD* octree, std::istream* in);
	void setShaderColor(QColor const& color);
};

#endif // TREEMETHOD_H
