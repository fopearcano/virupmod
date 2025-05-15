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
	TreeMethodLOD(TreeMethodLOD const&)            = delete;
	TreeMethodLOD(TreeMethodLOD&&)                 = delete;
	TreeMethodLOD& operator=(TreeMethodLOD const&) = delete;
	TreeMethodLOD& operator=(TreeMethodLOD&&)      = delete;
	explicit TreeMethodLOD(std::string const& shadersCommonName);
	TreeMethodLOD(std::string const& vertexShaderPath,
	              std::string const& fragmentShaderPath);
	std::string getName() const override { return "Tree LOD"; };
	void init(std::vector<float>& gasVertices,
	          std::vector<float>& starsVertices,
	          std::vector<float>& darkMatterVertices) override;
	void init(std::string const& gasPath, std::string const& starsPath,
	          std::string const& darkMatterPath) override;
	void init(QStringList const& gasFiles, QStringList const& starsFiles,
	          QStringList const& dmFiles);
	BBox getDataBoundingBox() const override;
	uint64_t getOctreesTotalDataSize() const;
	bool preloadOctreesLevel(unsigned int level,
	                         QProgressDialog* progress = nullptr);
	void update(Camera const& camera);
	void update(Camera const& camera, QMatrix4x4 const& model,
	            QVector3D const& campos);
	void render(Camera const& camera) override;
	void render(Camera const& camera, QMatrix4x4 const& model,
	            QVector3D const& campos, float unitInKpc);
	void dumpOctreesStates(QString const& dirPath,
	                       QString const& filePathPrefix);
	void unload();
	void cleanUp();
	~TreeMethodLOD() override;

	bool silent = false;

  protected:
	QString dustModelPath;
	std::vector<std::unique_ptr<OctreeLOD>> gasTrees;
	std::vector<std::unique_ptr<OctreeLOD>> starsTrees;
	std::vector<std::unique_ptr<OctreeLOD>> darkMatterTrees;
	std::unique_ptr<VolumetricModel> hiiModel;

	static std::vector<std::pair<QString, std::unique_ptr<VolumetricModel>>>&
	    volModels();
	static bool modelExists(QString const& path);
	static VolumetricModel* getModel(QString const& path);

	// ugly fix for pointSize problems
	bool setPointSize = true;

	static void
	    loadOctreeFromFile(std::string const& path,
	                       std::vector<std::unique_ptr<OctreeLOD>>& container,
	                       std::string const& name,
	                       GLShaderProgram const& shaderProgram, bool silent);
	static void initOctree(OctreeLOD* octree,
	                       std::shared_ptr<std::istream> const& in);
	void setShaderColor(QColor const& color);
};

#endif // TREEMETHOD_H
