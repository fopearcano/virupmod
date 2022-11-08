#include "methods/OctreeLOD.hpp"

int64_t& OctreeLOD::usedMem()
{
	static int64_t usedMem(0);
	return usedMem;
}

const int64_t& OctreeLOD::memLimit()
{
	static const int64_t memLimit(
	    static_cast<int64_t>(1000000)
	    * QSettings().value("misc/maxvramusagemb").toInt());
	return memLimit;
}

bool& OctreeLOD::forceMaxQuality()
{
	static bool forceMaxQuality(false);
	return forceMaxQuality;
}

int& OctreeLOD::forceQuality()
{
	static int forceQuality(-1);
	return forceQuality;
}

float& OctreeLOD::minTanAngleLimit()
{
	static float minTanAngleLimit(0.05f);
	return minTanAngleLimit;
}

float& OctreeLOD::tanAngleLimit()
{
	static float tanAngleLimit(1.2f);
	return tanAngleLimit;
}

bool& OctreeLOD::timerStarted()
{
	static bool timerStarted(false);
	return timerStarted;
}

QElapsedTimer& OctreeLOD::timer()
{
	static QElapsedTimer timer;
	return timer;
}

// TODO just draw nothing if vertices.size() == 0 (prevents nullptr tests when
// drawing)

OctreeLOD::OctreeLOD(GLShaderProgram const& shaderProgram)
    : shaderProgram(&shaderProgram)
{
	minTanAngleLimit() = QSettings().value("misc/mintanangle").toDouble();
}

OctreeLOD::OctreeLOD(GLShaderProgram const& shaderProgram,
                     Octree::CommonData& commonData, unsigned int lvl)
    : Octree(commonData)
    , shaderProgram(&shaderProgram)
    , lvl(lvl)
{
}

// NOLINTNEXTLINE(misc-unused-parameters)
void OctreeLOD::init(std::vector<float>& data)
{
	Octree::init(data);
	computeBBox();
	ramToVideo();
}

void OctreeLOD::init(std::istream& in)
{
	Octree::init(in);
}

void OctreeLOD::init(int64_t file_addr, std::istream& in)
{
	Octree::init(file_addr, in);
}

void OctreeLOD::readOwnData(std::istream& in)
{
	Octree::readOwnData(in);

	if((getFlags() & Flags::NORMALIZED_NODES) == Flags::NONE)
	{
		for(size_t i(0); i < data.size(); i += commonData.dimPerVertex)
		{
			for(unsigned int j(0); j < 3; ++j)
			{
				data[i + j] -= localTranslation[j];
			}
		}
	}
	else
	{
		double localScale;
		if((bbox.maxx - bbox.minx >= bbox.maxy - bbox.miny)
		   && (bbox.maxx - bbox.minx >= bbox.maxz - bbox.minz))
		{
			localScale = bbox.maxx - bbox.minx;
		}
		else if(bbox.maxy - bbox.miny >= bbox.maxz - bbox.minz)
		{
			localScale = bbox.maxy - bbox.miny;
		}
		else
		{
			localScale = bbox.maxz - bbox.minz;
		}
		for(size_t i(0); i < data.size(); i += commonData.dimPerVertex)
		{
			for(unsigned int j(0); j < 3; ++j)
			{
				data[i + j] *= localScale;
			}
		}
	}
}

void OctreeLOD::readBBox(std::istream& in)
{
	Octree::readBBox(in);
	computeBBox();
}

std::vector<float> OctreeLOD::getOwnData() const
{
	std::vector<float> result(data);
	for(size_t i(0); i < result.size(); i += commonData.dimPerVertex)
	{
		for(unsigned int j(0); j < 3; ++j)
		{
			result[i + j] += localTranslation[j];
		}
	}

	return result;
}

void OctreeLOD::unload()
{
	if(isLoaded)
	{
		usedMem() -= dataSize * sizeof(float);
		dataSize = 0;
		delete mesh;
		mesh = nullptr;
		for(Octree* oct : children)
		{
			if(oct != nullptr)
			{
				dynamic_cast<OctreeLOD*>(oct)->unload();
			}
		}
		isLoaded = false;
	}
}

void OctreeLOD::setFile(std::istream* file)
{
	this->file = file;
	for(Octree* oct : children)
	{
		if(oct != nullptr)
		{
			dynamic_cast<OctreeLOD*>(oct)->setFile(file);
		}
	}
}

bool OctreeLOD::preloadLevel(unsigned int lvlToLoad)
{
	if(usedMem() >= memLimit())
	{
		return false;
	}
	if(lvlToLoad == 0)
	{
		readOwnData(*file);
		ramToVideo();
	}
	else
	{
		for(Octree* oct : children)
		{
			if(oct != nullptr)
			{
				if(!dynamic_cast<OctreeLOD*>(oct)->preloadLevel(lvlToLoad - 1))
				{
					return false;
				}
			}
		}
	}
	return true;
}

void OctreeLOD::update(Camera const& camera, QMatrix4x4 const& globalModel,
                       QVector3D const& globalCampos, float alpha)
{
	doRender = true;
	recurse  = false;
	if(camera.shouldBeCulled(bbox, globalModel, true) && lvl > 0)
	{
		if(usedMem() > (memLimit() * 80) / 100)
		{
			unload();
		}
		doRender = false;
		return;
	}

	if(!isLoaded)
	{
		/*if(usedMem() < memLimit())
		{*/
		readOwnData(*file);
		ramToVideo();
		/*}
		else
		{
		    doRender = false;
		    return;
		}*/
	}

	if(!isLeaf())
	{
		if(forceQuality() >= 0)
		{
			if(static_cast<int>(lvl) <= forceQuality())
			{
				recurse = true;
			}
		}
		else if(currentTanAngle(globalCampos) > tanAngleLimit())
		{
			recurse = true;
		}
	}

	if(recurse)
	{
		// UPDATE SUBTREES
		for(Octree* oct : children)
		{
			if(oct != nullptr)
			{
				dynamic_cast<OctreeLOD*>(oct)->update(camera, globalModel,
				                                      globalCampos, alpha);
			}
		}
		return;
	}

	if(!isLeaf() && usedMem() > (memLimit() * 80) / 100)
	{
		// unload children
		for(Octree* oct : children)
		{
			if(oct != nullptr)
			{
				dynamic_cast<OctreeLOD*>(oct)->unload();
			}
		}
	}

#ifdef IGNORE
	if(false && isLeaf())
	{
		Vector3 campos(Utils::fromQt(globalCampos));

		// see if useful for optimization or not... 100 is too much for Eagle
		// data (won't trigger until precision problems already appear)
		if(/*camera.scale > 100 &&*/ campos[0] > bbox.minx
		   && campos[0] < bbox.maxx && campos[1] > bbox.miny
		   && campos[1] < bbox.maxy && campos[2] > bbox.minz
		   && campos[2] < bbox.maxz)
		{
			Vector3 closest(DBL_MAX, DBL_MAX, DBL_MAX);
			if((campos - closestBackup).length() > neighborDist / 2.0)
			{
				if(absoluteData.empty())
				{
					readOwnData(*file);
					absoluteData = getOwnData();
					data.resize(0);
					data.shrink_to_fit();
				}

				double dist(FLT_MAX);
				for(unsigned int i(0); i < absoluteData.size();
				    i += commonData.dimPerVertex)
				{
					Vector3 x(absoluteData[i], absoluteData[i + 1],
					          absoluteData[i + 2]);
					double distx((campos - x).length());
					if(distx < dist)
					{
						closest = x;
						dist    = distx;
					}
				}
			}
			else
			{
				closest = closestBackup;
			}
			localTranslation = closest;
			if(closest != closestBackup)
			{
				closestChanged(closest);
				closestBackup = closest;

				Vector3 closestNeighbor(DBL_MAX, DBL_MAX, DBL_MAX);
				neighborDist = DBL_MAX;

				std::vector<float> vertexData(absoluteData);
				for(unsigned int i(0); i < vertexData.size();
				    i += commonData.dimPerVertex)
				{
					vertexData[i] -= closest[0];
					vertexData[i + 1] -= closest[1];
					vertexData[i + 2] -= closest[2];
					Vector3 x(vertexData[i], vertexData[i + 1],
					          vertexData[i + 2]);
					// it's closest itself !
					if(x.length() == 0.0)
					{
						continue;
					}

					if(x.length() < neighborDist)
					{
						closestNeighbor = x;
						neighborDist    = x.length();
					}
				}
				mesh->setVertices(vertexData);
			}
		}
		else
		{
			absoluteData.resize(0);
			absoluteData.shrink_to_fit();
			closestBackup = Vector3(DBL_MAX, DBL_MAX, DBL_MAX);
			neighborDist  = 0.0;
		}
	}
#endif

	QMatrix4x4 model;
	model.translate(Utils::toQt(localTranslation));
	update(camera, globalModel * model, model.inverted() * globalCampos);
}

void OctreeLOD::render(Camera const& camera, QMatrix4x4 const& globalModel,
                       QVector3D const& globalCampos, float alpha,
                       QMatrix4x4 const& globalDustModel)
{
	if(!doRender)
	{
		return;
	}

	if(recurse)
	{
		// RENDER SUBTREES
		for(Octree* oct : children)
		{
			if(oct != nullptr)
			{
				dynamic_cast<OctreeLOD*>(oct)->render(
				    camera, globalModel, globalCampos, alpha, globalDustModel);
			}
		}
		return;
	}

	QMatrix4x4 model;
	model.translate(Utils::toQt(localTranslation));
	renderNode(camera, globalModel * model, model.inverted() * globalCampos,
	           alpha * totalDataSize / dataSize, globalDustModel * model);
}

void OctreeLOD::dumpState(QString const& filePath)
{
	if(!doRender)
	{
		return;
	}

	qDebug() << filePath;
	QFile file(filePath);
	if(!file.open(QIODevice::WriteOnly | QIODevice::Text))
	{
		qWarning() << "Error while trying to open \"" + filePath
		                  + "\" for writing.";
		return;
	}

	QTextStream out(&file);
	auto totalDumped(dumpRenderedPos(out));

	if(totalDumped == 0)
	{
		return;
	}

	// add fake faces for OBJ to be read everywhere
	for(unsigned int i(0); i < totalDumped / 3; ++i)
	{
		out << "f " << i * 3 + 1 << " " << i * 3 + 2 << " " << i * 3 + 3
		    << "\n";
	}
	out << "f " << totalDumped - 2 << " " << totalDumped - 1 << " "
	    << totalDumped << "\n";
}

unsigned int OctreeLOD::dumpRenderedPos(QTextStream& stream)
{
	if(!doRender)
	{
		return 0;
	}

	if(recurse)
	{
		unsigned int totalDumped(0);
		// RENDER SUBTREES
		for(Octree* oct : children)
		{
			if(oct != nullptr)
			{
				totalDumped
				    += dynamic_cast<OctreeLOD*>(oct)->dumpRenderedPos(stream);
			}
		}
		return totalDumped;
	}

	if(absoluteData.empty())
	{
		readOwnData(*file);
		absoluteData = getOwnData();
		data.resize(0);
		data.shrink_to_fit();
	}
	unsigned int i(0);
	for(; i < absoluteData.size() / commonData.dimPerVertex; ++i)
	{
		stream << "v ";
		stream << absoluteData[commonData.dimPerVertex * i] << " ";
		stream << absoluteData[commonData.dimPerVertex * i + 1] << " ";
		stream << absoluteData[commonData.dimPerVertex * i + 2] << "\n";
	}
	absoluteData.resize(0);
	absoluteData.shrink_to_fit();
	return i;
}

void OctreeLOD::renderNode(Camera const& /*camera*/,
                           QMatrix4x4 const& localToWorld,
                           QVector3D const& localCamPos, float compensatedAlpha,
                           QMatrix4x4 const& localDustModel)
{
	shaderProgram->setUniform("alpha", compensatedAlpha);
	shaderProgram->setUniform("campos", localCamPos);
	shaderProgram->setUniform("dusttransform", localDustModel);
	GLHandler::setUpRender(*shaderProgram, localToWorld);
	mesh->render();
}

void OctreeLOD::computeBBox()
{
	bbox.minx     = minX;
	bbox.maxx     = maxX;
	bbox.miny     = minY;
	bbox.maxy     = maxY;
	bbox.minz     = minZ;
	bbox.maxz     = maxZ;
	bbox.diameter = sqrtf((bbox.maxx - bbox.minx) * (bbox.maxx - bbox.minx)
	                      + (bbox.maxy - bbox.miny) * (bbox.maxy - bbox.miny)
	                      + (bbox.maxz - bbox.minz) * (bbox.maxz - bbox.minz));

	bbox.mid.setX((bbox.maxx + bbox.minx) / 2.0f);
	bbox.mid.setY((bbox.maxy + bbox.miny) / 2.0f);
	bbox.mid.setZ((bbox.maxz + bbox.minz) / 2.0f);

	localTranslation = Vector3(bbox.minx, bbox.miny, bbox.minz);
}

float OctreeLOD::currentTanAngle(QVector3D const& campos) const
{
	return bbox.diameter / campos.distanceToPoint(bbox.mid);
}

void OctreeLOD::ramToVideo()
{
	mesh                                                  = new GLMesh;
	std::vector<QPair<const char*, unsigned int>> mapping = {{"position", 3}};
	std::vector<QPair<const char*, std::vector<float>>> unused;
	if((getFlags() & Flags::STORE_RADIUS) != Flags::NONE)
	{
		mapping.emplace_back("radius", 1);
	}
	else
	{
		unused.emplace_back("radius", std::vector<float>{1.f});
	}
	if((getFlags() & Flags::STORE_LUMINOSITY) != Flags::NONE)
	{
		mapping.emplace_back("luminosity", 1);
	}
	else
	{
		unused.emplace_back("luminosity", std::vector<float>{1.f});
	}
	if((getFlags() & Flags::STORE_COLOR) != Flags::NONE)
	{
		mapping.emplace_back("color", 3);
	}
	shaderProgram->setUnusedAttributesValues(unused);
	mesh->setVertexShaderMapping(*shaderProgram, mapping);
	mesh->setVertices(data);
	dataSize = data.size();
	usedMem() += dataSize * sizeof(float);
	data.resize(0);
	data.shrink_to_fit();
	isLoaded = true;
}

Octree* OctreeLOD::newChild() const
{
	return new OctreeLOD(*shaderProgram, commonData, lvl + 1);
}

OctreeLOD::~OctreeLOD()
{
	unload();
}

void OctreeLOD::updateTanAngleLimit(Camera const& camera)
{
	if(forceMaxQuality())
	{
		tanAngleLimit() = minTanAngleLimit(); // for dome min 0.2f comfortable
		                                      // 0.4f | 2D : 0.05
		return;
	}

	if(!timerStarted())
	{
		timer().start();
		// init chrono
		// gettimeofday(&t0, NULL);

		// setting PID controller
		/*ctrl.Kp = -0.000001f;
		ctrl.Ki = -0.0000001f;
		ctrl.Kd = 0.00001f;

		ctrl.controlVariable = &tanAngleLimit;

		ctrl.tol = 500;*/
		timerStarted() = true;
		return;
	}

	/*struct timeval tf;
	gettimeofday(&tf, NULL);
	uint64_t dt = (tf.tv_sec * 1000000) + tf.tv_usec - t0.tv_usec
	              - (t0.tv_sec * 1000000);
	gettimeofday(&t0, NULL);

	float dtf = dt;
	if(camera.currentFrameTiming != 0)*/
	float dtf = camera.currentFrameTiming * 1000000.f;
	/*ctrl.targetMeasure = &dtf;
	ctrl.setPoint          = 1000000.0f / camera.targetFPS;
	ctrl.update(dt);

	// we don't want points to discard others on depth test, because with
	// transparency they should all be drawn; but we still want to be occluded
	// by solid materials (like controllers for example) so depth test is still
	// enabled*/

	// old way
	float coeff((dtf - 1000000.0f / camera.targetFPS) / 5000000.0f);
	coeff = coeff > 1.f / 90.f ? 1.f / 90.f : coeff;
	tanAngleLimit() += coeff;
	tanAngleLimit() = tanAngleLimit() > 1.2f ? 1.2f : tanAngleLimit();
	tanAngleLimit() = tanAngleLimit() < minTanAngleLimit() ? minTanAngleLimit()
	                                                       : tanAngleLimit();

	// if something very bad happened regarding last frame rendering
	if(timer().restart() > 200)
	{
		tanAngleLimit() = 1.2f;
	}
}
