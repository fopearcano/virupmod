#include "methods/TreeMethodLOD.hpp"

TreeMethodLOD::TreeMethodLOD()
    : TreeMethodLOD("invsq")
{
}

TreeMethodLOD::TreeMethodLOD(std::string const& shadersCommonName)
    : TreeMethodLOD(shadersCommonName, shadersCommonName)
{
}

TreeMethodLOD::TreeMethodLOD(std::string const& vertexShaderPath,
                             std::string const& fragmentShaderPath)
    : Method(vertexShaderPath, fragmentShaderPath)
{
	showdm() = true;

	GLHandler::setPointSize(1);
}

void TreeMethodLOD::init(std::vector<float>& gasVertices,
                         std::vector<float>& starsVertices,
                         std::vector<float>& darkMatterVertices)
{
	if(!gasVertices.empty() && gasTree == nullptr)
	{
		gasTree = new OctreeLOD(shaderProgram);
		gasTree->init(gasVertices);
	}
	if(!starsVertices.empty() && starsTree == nullptr)
	{
		starsTree = new OctreeLOD(shaderProgram);
		starsTree->init(starsVertices);
	}
	if(!darkMatterVertices.empty() && darkMatterTree == nullptr)
	{
		darkMatterTree = new OctreeLOD(shaderProgram);
		darkMatterTree->init(darkMatterVertices);
	}
}

void TreeMethodLOD::init(std::string const& gasPath,
                         std::string const& starsPath,
                         std::string const& darkMatterPath)
{
	if(!gasPath.empty())
	{
		if(gasPath.find(".dat") == std::string::npos && gasTree == nullptr)
		{
			loadOctreeFromFile(gasPath, &gasTree, "Gas", shaderProgram);
		}
		else
		{
			dustModel = new VolumetricModel(gasPath.c_str());
		}
	}
	if(!starsPath.empty() && starsTree == nullptr)
	{
		loadOctreeFromFile(starsPath, &starsTree, "Stars", shaderProgram);
	}
	if(!darkMatterPath.empty())
	{
		if(darkMatterPath.find(".dat") == std::string::npos
		   && darkMatterTree == nullptr)
		{
			loadOctreeFromFile(darkMatterPath, &darkMatterTree, "Dark matter",
			                   shaderProgram);
		}
		else
		{
			hiiModel = new VolumetricModel(darkMatterPath.c_str());
			hiiModel->initMesh();
			hiiModel->setColor(darkMatterColor);
		}
	}
}

BBox TreeMethodLOD::getDataBoundingBox() const
{
	std::vector<BBox> bboxes;
	if(gasTree != nullptr)
	{
		bboxes.push_back(gasTree->getBoundingBox());
	}
	if(starsTree != nullptr)
	{
		bboxes.push_back(starsTree->getBoundingBox());
	}
	if(darkMatterTree != nullptr)
	{
		bboxes.push_back(darkMatterTree->getBoundingBox());
	}
	return globalBBox(bboxes);
}

uint64_t TreeMethodLOD::getOctreesTotalDataSize() const
{
	uint64_t result(0);
	if(gasTree != nullptr)
	{
		result += gasTree->getTotalDataSize();
	}
	if(starsTree != nullptr)
	{
		result += starsTree->getTotalDataSize();
	}
	if(darkMatterTree != nullptr)
	{
		result += darkMatterTree->getTotalDataSize();
	}
	return result;
}

bool TreeMethodLOD::preloadOctreesLevel(unsigned int level,
                                        QProgressDialog& progress)
{
	if(gasTree != nullptr)
	{
		if(!gasTree->preloadLevel(level))
		{
			return false;
		}
		QCoreApplication::processEvents();
		progress.setValue(OctreeLOD::getUsedMem());
	}
	if(starsTree != nullptr)
	{
		if(!starsTree->preloadLevel(level))
		{
			return false;
		}
		QCoreApplication::processEvents();
		progress.setValue(OctreeLOD::getUsedMem());
	}
	if(darkMatterTree != nullptr)
	{
		if(!darkMatterTree->preloadLevel(level))
		{
			return false;
		}
		QCoreApplication::processEvents();
		progress.setValue(OctreeLOD::getUsedMem());
	}
	return true;
}

void TreeMethodLOD::update(Camera const& camera)
{
	update(camera, camera.dataToWorldTransform(),
	       Utils::toQt(camera.getTruePosition()));
}

void TreeMethodLOD::update(Camera const& camera, QMatrix4x4 const& model,
                           QVector3D const& campos)
{
	if(gasTree != nullptr)
	{
		gasTree->update(camera, model, campos, getAlpha());
	}
	if(starsTree != nullptr)
	{
		starsTree->update(camera, model, campos, getAlpha());
	}
	if(darkMatterTree != nullptr && showdm())
	{
		darkMatterTree->update(camera, model, campos, getAlpha());
	}
	if(hiiModel != nullptr)
	{
		hiiModel->render(camera, model, campos, dustModel);
	}
}

void TreeMethodLOD::render(Camera const& camera)
{
	render(camera, camera.dataToWorldTransform(),
	       Utils::toQt(camera.getTruePosition()), 1.f);
}

void TreeMethodLOD::render(Camera const& camera, QMatrix4x4 const& model,
                           QVector3D const& campos, float unitInKpc)
{
	if(setPointSize)
	{
		GLHandler::setPointSize(1);
	}
	GLHandler::beginTransparent(GL_ONE, GL_ONE);
	shaderProgram.setUnusedAttributesValues(
	    {{"color", std::vector<float>{1.0f, 1.0f, 1.0f}}});
	shaderProgram.setUniform("useDust", dustModel == nullptr ? 0.f : 1.f);
	if(dustModel != nullptr)
	{
		GLHandler::useTextures({&dustModel->getTexture()});
	}
	GLHandler::setUpRender(shaderProgram, model);
	shaderProgram.setUniform("pixelSolidAngle", camera.pixelSolidAngle());
	shaderProgram.setUniform("unitInKpc", unitInKpc);
	shaderProgram.setUniform("useDust", 1.f);
	QMatrix4x4 dustTransform;
	if(dustModel != nullptr)
	{
		dustTransform = dustModel->getPosToTexCoord();
	}

	if(gasTree != nullptr)
	{
		if((gasTree->getFlags() & Octree::Flags::STORE_COLOR)
		   == Octree::Flags::NONE)
		{
			setShaderColor(gasColor);
		}
		gasTree->render(camera, model, campos, getAlpha(), dustTransform);
	}
	if(starsTree != nullptr)
	{
		if((starsTree->getFlags() & Octree::Flags::STORE_COLOR)
		   == Octree::Flags::NONE)
		{
			setShaderColor(starsColor);
		}
		starsTree->render(camera, model, campos, getAlpha(), dustTransform);
	}
	if(darkMatterTree != nullptr && showdm())
	{
		if((darkMatterTree->getFlags() & Octree::Flags::STORE_COLOR)
		   == Octree::Flags::NONE)
		{
			setShaderColor(darkMatterColor);
		}
		darkMatterTree->render(camera, model, campos, getAlpha(),
		                       dustTransform);
	}
	GLHandler::endTransparent();
	if(hiiModel != nullptr)
	{
		hiiModel->render(camera, model, campos, dustModel);
	}
}

void TreeMethodLOD::cleanUp()
{
	delete dustModel;
	dustModel = nullptr;
	delete hiiModel;
	hiiModel = nullptr;
	if(gasTree != nullptr)
	{
		if(gasTree->getFile() != nullptr)
		{
			delete gasTree->getFile();
		}
		delete gasTree;
	}
	gasTree = nullptr;
	if(starsTree != nullptr)
	{
		if(starsTree->getFile() != nullptr)
		{
			delete starsTree->getFile();
		}
		delete starsTree;
	}
	starsTree = nullptr;
	if(darkMatterTree != nullptr)
	{
		if(darkMatterTree->getFile() != nullptr)
		{
			delete darkMatterTree->getFile();
		}
		delete darkMatterTree;
	}
	darkMatterTree = nullptr;
}

void TreeMethodLOD::loadOctreeFromFile(std::string const& path,
                                       OctreeLOD** octree,
                                       std::string const& name,
                                       GLShaderProgram const& shaderProgram)
{
	std::cout << "Loading " + name + " octree..." << std::endl;
	auto file = new std::ifstream();
	file->open(path, std::fstream::in | std::fstream::binary);
	*octree = new OctreeLOD(shaderProgram);

	// Init tree with progress bar
	int64_t cursor(file->tellg());
	int64_t size;
	brw::read(*file, size);
	file->seekg(cursor);
	size *= -1;

	QProgressDialog progress(tr("Loading %1 tree structure").arg(name.c_str()),
	                         QString(), 0, size);
	progress.setMinimumDuration(0);
	progress.setValue(0);

	auto future = std::async(std::launch::async, &initOctree, *octree, file);

	float p(0.f);
	Octree::showProgress(p);
	while(future.wait_for(std::chrono::duration<int, std::milli>(100))
	      != std::future_status::ready)
	{
		QCoreApplication::processEvents();
		p = static_cast<float>(file->tellg()) / size;
		if(0.f <= p && p <= 1.f)
		{
			progress.setValue(file->tellg());
			Octree::showProgress(p);
		}
	}
	Octree::showProgress(1.f);

	(*octree)->setFile(file);
	// update bbox
	progress.setLabelText(
	    tr("Loading %1 tree bounding boxes...").arg(name.c_str()));
	QCoreApplication::processEvents();
	(*octree)->readBBoxes(*file);
	// (*octree)->readData(*file);
	std::cout << name << " loaded..." << std::endl;
}

void TreeMethodLOD::initOctree(OctreeLOD* octree, std::istream* in)
{
	octree->init(*in);
}

void TreeMethodLOD::setShaderColor(QColor const& color)
{
	shaderProgram.setUnusedAttributesValues(
	    {{"color",
	      {float(color.redF()), float(color.greenF()), float(color.blueF())}}});
}

TreeMethodLOD::~TreeMethodLOD()
{
	cleanUp();
}
