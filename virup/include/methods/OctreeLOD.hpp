#ifndef OCTREELOD_H
#define OCTREELOD_H

#include <QElapsedTimer>
#include <liboctree/Octree.hpp>
#include <random>

#include "graphics/renderers/OrbitalSystemRenderer.hpp"

#include "Camera.hpp"
#include "Primitives.hpp"
#include "gl/GLHandler.hpp"
#include "math/Vector3.hpp"
#include "physics/blackbody.hpp"
#include "utils.hpp"

#define MAX_LEAVES_PER_NODE 16000

class OctreeLOD : public Octree
{
  public:
	OctreeLOD(GLShaderProgram const& shaderProgram);
	virtual void init(std::vector<float>& data) override;
	virtual void init(std::istream& in) override;
	virtual void init(int64_t file_addr, std::istream& in) override;
	BBox getBoundingBox() const { return bbox; };
	virtual void readOwnData(std::istream& in) override;
	virtual void readBBox(std::istream& in) override;
	virtual std::vector<float> getOwnData() const override;
	void unload();
	void setFile(std::istream* file);
	std::istream* getFile() { return file; };
	bool preloadLevel(unsigned int lvlToLoad);
	void update(Camera const& camera, QMatrix4x4 const& globalModel,
	            QVector3D const& globalCampos, float alpha);
	void render(QMatrix4x4 const& globalModel, QVector3D const& globalCampos,
	            float alpha, QMatrix4x4 const& globalDustModel);
	~OctreeLOD();

	static void updateTanAngleLimit(Camera const& camera);

	static int64_t getUsedMem() { return usedMem(); };
	static int64_t getMemLimit() { return memLimit(); };

  protected:
	OctreeLOD(GLShaderProgram const& shaderProgram,
	          Octree::CommonData& commonData, unsigned int lvl = 0);
	virtual Octree* newChild() const override;

	// in Octree space
	virtual void closestChanged(Vector3 /*closest*/){};
	virtual void update(QMatrix4x4 const& /*localToWorld*/,
	                    QVector3D const& /*localCamPos*/){};
	virtual void renderNode(QMatrix4x4 const& localToWorld,
	                        QVector3D const& localCamPos,
	                        float compensatedAlpha,
	                        QMatrix4x4 const& localDustModel);

	GLMesh* mesh = nullptr;
	GLShaderProgram const* shaderProgram;

  private:
	unsigned int lvl = 0;
	BBox bbox;

	std::istream* file    = nullptr;
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
	static float& tanAngleLimit();
	// PIDController ctrl;

	// used to detect too long frames
	static bool& timerStarted();
	static QElapsedTimer& timer();

	bool doRender = false;
	bool recurse  = false;
};

#endif // OCTREELOD_H
