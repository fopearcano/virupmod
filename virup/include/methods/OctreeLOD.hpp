#ifndef OCTREELOD_H
#define OCTREELOD_H

#include <QElapsedTimer>
#include <liboctree/Octree.hpp>
#include <random>

#include "graphics/renderers/OrbitalSystemRenderer.hpp"

#include "AsyncReader.hpp"
#include "Camera.hpp"
#include "Primitives.hpp"
#include "gl/GLHandler.hpp"
#include "math/Vector3.hpp"
#include "utils.hpp"

#define MAX_LEAVES_PER_NODE 16000

class OctreeLOD : public Octree
{
  public:
	explicit OctreeLOD(GLShaderProgram const& shaderProgram);
	OctreeLOD(OctreeLOD const&)            = delete;
	OctreeLOD(OctreeLOD&&)                 = delete;
	OctreeLOD& operator=(OctreeLOD const&) = delete;
	OctreeLOD& operator=(OctreeLOD&&)      = delete;
	bool isReady() const;
	unsigned int getLevel() const { return lvl; };
	void init(std::vector<float>& data, unsigned int maxLeafSize) override;
	void init(std::istream& in) override;
	void init(int64_t file_addr, std::istream& in) override;
	BBox getBoundingBox() const { return bbox; };
	void readOwnData(std::istream& in) override;
	void readBBox(std::istream& in) override;
	std::vector<float> getOwnData() const override;
	void unload();
	void waitOnAsyncLoader();
	void setFile(std::shared_ptr<std::istream> const& file);
	std::istream* getFile() { return file.get(); };
	bool preloadLevel(unsigned int lvlToLoad);
	void update(Camera const& camera, QMatrix4x4 const& globalModel,
	            QVector3D const& globalCampos, float alpha);
	void render(Camera const& camera, QMatrix4x4 const& globalModel,
	            QVector3D const& globalCampos, float alpha,
	            QMatrix4x4 const& globalDustModel);
	void dumpState(QString const& filePath);
	~OctreeLOD() override;

	static void updateTanAngleLimit(Camera const& camera);

	static int64_t getUsedMem() { return usedMem(); };
	static int64_t getMemLimit() { return memLimit(); };
	static bool& forceMaxQuality();
	static int& forceQuality();

	static float getCurrentTanAngleLimit() { return tanAngleLimit(); };
	static void setCurrentTanAngleLimit(float tanAngleLimit)
	{
		forceMaxQuality()  = true;
		minTanAngleLimit() = tanAngleLimit;
	};
	static void unsetCurrentTanAngleLimit()
	{
		forceMaxQuality()  = false;
		minTanAngleLimit() = QSettings().value("misc/mintanangle").toDouble();
	}

	volatile AsyncReader::State state = AsyncReader::State::IDLE;

  protected:
	OctreeLOD(GLShaderProgram const& shaderProgram,
	          Octree::CommonData& commonData, unsigned int lvl = 0);
	std::unique_ptr<Octree> newChild() const override;

	// in Octree space
	virtual void closestChanged(Vector3 /*closest*/){};
	virtual void update(Camera const& /*camera*/,
	                    QMatrix4x4 const& /*localToWorld*/,
	                    QVector3D const& /*localCamPos*/) {};
	virtual void renderNode(Camera const& camera,
	                        QMatrix4x4 const& localToWorld,
	                        QVector3D const& localCamPos,
	                        float compensatedAlpha,
	                        QMatrix4x4 const& localDustModel);

	std::unique_ptr<GLMesh> mesh;
	GLShaderProgram const* shaderProgram;

  private:
	unsigned int lvl = 0;
	BBox bbox;

	std::shared_ptr<std::istream> file;
	bool isLoaded         = false;
	unsigned int dataSize = 0;
	// total used memory across all instances
	static int64_t& usedMem();
	static const int64_t& memLimit();

	void computeBBox();
	float currentTanAngle(QVector3D const& campos) const;
	void ramToVideo();

	/* PRECISION ENHANCEMENT */
	std::vector<float> absoluteData; // backup data from file
	double neighborDist      = 0.0;
	Vector3 localTranslation = Vector3(0.f, 0.f, 0.f);

	/* PERFORMANCE */
	Vector3 closestBackup = Vector3(DBL_MAX, DBL_MAX, DBL_MAX);

	/* LOD DETERMINATION */

	// struct timeval t0;
	static float& minTanAngleLimit();
	static float& tanAngleLimit();
	// PIDController ctrl;

	// used to detect too long frames
	static bool& timerStarted();
	static QElapsedTimer& timer();

	bool doRender = false;
	bool recurse  = false;

	// returns number of particles dumped
	unsigned int dumpRenderedPos(QTextStream& stream);
};

#endif // OCTREELOD_H
