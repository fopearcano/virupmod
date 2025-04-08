#include "methods/TreeMethodLOD.hpp"

std::vector<std::pair<QString, std::unique_ptr<VolumetricModel>>>&
    TreeMethodLOD::volModels()
{
	static std::vector<std::pair<QString, std::unique_ptr<VolumetricModel>>>
	    volModels;
	return volModels;
}

bool TreeMethodLOD::modelExists(QString const& path)
{
	for(auto const& pair : volModels())
	{
		if(pair.first == path)
		{
			return pair.second != nullptr;
		}
	}
	return false;
}

VolumetricModel* TreeMethodLOD::getModel(QString const& path)
{
	for(auto const& pair : volModels())
	{
		if(pair.first == path)
		{
			return pair.second.get();
		}
	}
	return nullptr;
}

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
}

// NOLINTBEGIN(misc-unused-parameters)
void TreeMethodLOD::init(std::vector<float>& gasVertices,
                         std::vector<float>& starsVertices,
                         std::vector<float>& darkMatterVertices)
// NOLINTEND(misc-unused-parameters)
{
	if(!gasVertices.empty() && gasTrees.empty())
	{
		gasTrees.emplace_back(std::make_unique<OctreeLOD>(shaderProgram));
		gasTrees[0]->init(gasVertices, 16000);
	}
	if(!starsVertices.empty() && starsTrees.empty())
	{
		starsTrees.emplace_back(std::make_unique<OctreeLOD>(shaderProgram));
		starsTrees[0]->init(starsVertices, 16000);
	}
	if(!darkMatterVertices.empty() && darkMatterTrees.empty())
	{
		darkMatterTrees.emplace_back(
		    std::make_unique<OctreeLOD>(shaderProgram));
		darkMatterTrees[0]->init(darkMatterVertices, 16000);
	}
}

void TreeMethodLOD::init(std::string const& gasPath,
                         std::string const& starsPath,
                         std::string const& darkMatterPath)
{
	if(!gasPath.empty())
	{
		if(gasPath.find(".dat") == std::string::npos && gasTrees.empty())
		{
			loadOctreeFromFile(gasPath, gasTrees, "Gas", shaderProgram, silent);
		}
		else
		{
			dustModelPath = gasPath.c_str();
			if(!modelExists(dustModelPath))
			{
				volModels().emplace_back(dustModelPath,
				                         std::make_unique<VolumetricModel>(
				                             dustModelPath, "dustModel"));
			}
		}
	}
	if(!starsPath.empty() && starsTrees.empty())
	{
		loadOctreeFromFile(starsPath, starsTrees, "Stars", shaderProgram,
		                   silent);
	}
	if(!darkMatterPath.empty())
	{
		if(darkMatterPath.find(".dat") == std::string::npos
		   && darkMatterTrees.empty())
		{
			loadOctreeFromFile(darkMatterPath, darkMatterTrees, "Dark matter",
			                   shaderProgram, silent);
		}
		else
		{
			hiiModel = std::make_unique<VolumetricModel>(darkMatterPath.c_str(),
			                                             "hiiModel");
			hiiModel->initMesh();
			hiiModel->setColor(darkMatterColor);
		}
	}
}

void TreeMethodLOD::init(QStringList const& gasFiles,
                         QStringList const& starsFiles,
                         QStringList const& dmFiles)
{
	gasTrees.reserve(gasFiles.size());
	for(auto const& gasFile : gasFiles)
	{
		if(!gasFile.isEmpty())
		{
			loadOctreeFromFile(gasFile.toStdString(), gasTrees,
			                   "Gas (" + gasFile.toStdString() + ")",
			                   shaderProgram, silent);
		}
	}
	starsTrees.reserve(starsFiles.size());
	for(auto const& starsFile : starsFiles)
	{
		if(!starsFile.isEmpty())
		{
			loadOctreeFromFile(starsFile.toStdString(), starsTrees,
			                   "Stars (" + starsFile.toStdString() + ")",
			                   shaderProgram, silent);
		}
	}
	darkMatterTrees.reserve(dmFiles.size());
	for(auto const& dmFile : dmFiles)
	{
		if(!dmFile.isEmpty())
		{
			loadOctreeFromFile(dmFile.toStdString(), darkMatterTrees,
			                   "Dark matter (" + dmFile.toStdString() + ")",
			                   shaderProgram, silent);
		}
	}
}

BBox TreeMethodLOD::getDataBoundingBox() const
{
	std::vector<BBox> bboxes;
	bboxes.reserve(gasTrees.size() + starsTrees.size()
	               + darkMatterTrees.size());
	for(auto const& gasTree : gasTrees)
	{
		bboxes.push_back(gasTree->getBoundingBox());
	}
	for(auto const& starsTree : starsTrees)
	{
		bboxes.push_back(starsTree->getBoundingBox());
	}
	for(auto const& darkMatterTree : darkMatterTrees)
	{
		bboxes.push_back(darkMatterTree->getBoundingBox());
	}
	return globalBBox(bboxes);
}

uint64_t TreeMethodLOD::getOctreesTotalDataSize() const
{
	uint64_t result(0);
	for(auto const& gasTree : gasTrees)
	{
		result += gasTree->getTotalDataSize();
	}
	for(auto const& starsTree : starsTrees)
	{
		result += starsTree->getTotalDataSize();
	}
	for(auto const& darkMatterTree : darkMatterTrees)
	{
		result += darkMatterTree->getTotalDataSize();
	}
	return result;
}

bool TreeMethodLOD::preloadOctreesLevel(unsigned int level,
                                        QProgressDialog* progress)
{
	for(auto& gasTree : gasTrees)
	{
		if(!gasTree->preloadLevel(level))
		{
			return false;
		}
		QCoreApplication::processEvents();
		if(progress != nullptr)
		{
			progress->setValue(OctreeLOD::getUsedMem());
		}
	}
	for(auto& starsTree : starsTrees)
	{
		if(!starsTree->preloadLevel(level))
		{
			return false;
		}
		QCoreApplication::processEvents();
		if(progress != nullptr)
		{
			progress->setValue(OctreeLOD::getUsedMem());
		}
	}
	for(auto& darkMatterTree : darkMatterTrees)
	{
		if(!darkMatterTree->preloadLevel(level))
		{
			return false;
		}
		QCoreApplication::processEvents();
		if(progress != nullptr)
		{
			progress->setValue(OctreeLOD::getUsedMem());
		}
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
	for(auto& gasTree : gasTrees)
	{
		gasTree->update(camera, model, campos, getAlpha());
	}
	for(auto& starsTree : starsTrees)
	{
		starsTree->update(camera, model, campos, getAlpha());
	}
	if(showdm())
	{
		for(auto& darkMatterTree : darkMatterTrees)
		{
			darkMatterTree->update(camera, model, campos, getAlpha());
		}
	}
	if(hiiModel != nullptr)
	{
		hiiModel->render(camera, model, campos, getModel(dustModelPath));
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
	const GLStateSet glState({{GL_PROGRAM_POINT_SIZE, true}});
	const GLBlendSet glBlend({GL_ONE, GL_ONE});
	shaderProgram.setUnusedAttributesValues(
	    {{"color", std::vector<float>{1.0f, 1.0f, 1.0f}}});
	shaderProgram.setUniform("useDust", dustModelPath.isEmpty() ? 0.f : 1.f);
	if(getModel(dustModelPath) != nullptr)
	{
		GLHandler::useTextures({&getModel(dustModelPath)->getTexture()});
	}
	GLHandler::setUpRender(shaderProgram, model);
	shaderProgram.setUniform("pixelSolidAngle", camera.pixelSolidAngle());
	shaderProgram.setUniform("unitInKpc", unitInKpc);
	QMatrix4x4 dustTransform;
	if(getModel(dustModelPath) != nullptr)
	{
		dustTransform = getModel(dustModelPath)->getPosToTexCoord();
	}

	for(auto& gasTree : gasTrees)
	{
		if((gasTree->getFlags() & Octree::Flags::STORE_COLOR)
		   == Octree::Flags::NONE)
		{
			setShaderColor(gasColor);
		}
		gasTree->render(camera, model, campos, getAlpha(), dustTransform);
	}
	for(auto& starsTree : starsTrees)
	{
		if((starsTree->getFlags() & Octree::Flags::STORE_COLOR)
		   == Octree::Flags::NONE)
		{
			setShaderColor(starsColor);
		}
		starsTree->render(camera, model, campos, getAlpha(), dustTransform);
	}
	if(showdm())
	{
		for(auto& darkMatterTree : darkMatterTrees)
		{
			if((darkMatterTree->getFlags() & Octree::Flags::STORE_COLOR)
			   == Octree::Flags::NONE)
			{
				setShaderColor(darkMatterColor);
			}
			darkMatterTree->render(camera, model, campos, getAlpha(),
			                       dustTransform);
		}
	}
	if(hiiModel != nullptr)
	{
		hiiModel->render(camera, model, campos, getModel(dustModelPath));
	}
}

void TreeMethodLOD::dumpOctreesStates(QString const& dirPath,
                                      QString const& filePathPrefix)
{
	for(unsigned int i(0); i < gasTrees.size(); ++i)
	{
		gasTrees[i]->dumpState(dirPath + "/" + filePathPrefix + "_gas_"
		                       + QString::number(i) + ".obj");
	}
	for(unsigned int i(0); i < starsTrees.size(); ++i)
	{
		starsTrees[i]->dumpState(dirPath + "/" + filePathPrefix + "_stars_"
		                         + QString::number(i) + ".obj");
	}
	for(unsigned int i(0); i < darkMatterTrees.size(); ++i)
	{
		darkMatterTrees[i]->dumpState(dirPath + "/" + filePathPrefix
		                              + "_darkMatter_" + QString::number(i)
		                              + ".obj");
	}
}

void TreeMethodLOD::unload()
{
	for(auto& gasTree : gasTrees)
	{
		gasTree->unload();
	}
	for(auto& starsTree : starsTrees)
	{
		starsTree->unload();
	}
	for(auto& darkMatterTree : darkMatterTrees)
	{
		darkMatterTree->unload();
	}
}

void TreeMethodLOD::cleanUp()
{
	// dustModel.reset(); !!!!
	hiiModel.reset();
	for(auto& gasTree : gasTrees)
	{
		gasTree->waitOnAsyncLoader();
	}
	gasTrees.clear();
	for(auto& starsTree : starsTrees)
	{
		starsTree->waitOnAsyncLoader();
	}
	starsTrees.clear();
	for(auto& darkMatterTree : darkMatterTrees)
	{
		darkMatterTree->waitOnAsyncLoader();
	}
	darkMatterTrees.clear();
}

void TreeMethodLOD::loadOctreeFromFile(
    std::string const& path, std::vector<std::unique_ptr<OctreeLOD>>& container,
    std::string const& name, GLShaderProgram const& shaderProgram, bool silent)
{
	if(!silent)
	{
		qDebug() << "Loading " + QString(name.c_str()) + " octree...";
	}
	auto file = std::make_shared<std::ifstream>();
	file->open(path, std::fstream::in | std::fstream::binary);
	if(!file->is_open())
	{
		qCritical() << "Can't open octree from path:" << path.c_str();
		QCoreApplication::quit();
		return;
	}

	container.emplace_back(std::make_unique<OctreeLOD>(shaderProgram));
	auto& octree = container[container.size() - 1];

	// Init tree with progress bar
	const int64_t cursor(file->tellg());
	int64_t size = -1;
	brw::read(*file, size);
	file->seekg(cursor);
	size *= -1;

	std::unique_ptr<QProgressDialog> progress;
	if(!silent)
	{
		progress = std::make_unique<QProgressDialog>(
		    tr("Loading %1 tree structure").arg(name.c_str()), QString(), 0,
		    size);
		progress->setMinimumDuration(0);
		progress->setValue(0);
	}

	auto future
	    = std::async(std::launch::async, &initOctree, octree.get(), file);

	float p(0.f);
	if(!silent)
	{
		Octree::showProgress(p);
	}
	while(future.wait_for(std::chrono::duration<int, std::milli>(100))
	      != std::future_status::ready)
	{
		QCoreApplication::processEvents();
		p = static_cast<float>(file->tellg()) / size;
		if(0.f <= p && p <= 1.f)
		{
			if(progress != nullptr)
			{
				progress->setValue(file->tellg());
			}
			if(!silent)
			{
				Octree::showProgress(p);
			}
		}
	}
	if(!silent)
	{
		Octree::showProgress(1.f);
	}

	octree->setFile(file);
	// update bbox
	if(progress != nullptr)
	{
		progress->setLabelText(
		    tr("Loading %1 tree bounding boxes...").arg(name.c_str()));
	}
	QCoreApplication::processEvents();
	octree->readBBoxes(*file);
	// (*octree)->readData(*file);
	if(!silent)
	{
		qDebug() << QString(name.c_str()) + " loaded...";
	}
}

void TreeMethodLOD::initOctree(OctreeLOD* octree,
                               std::shared_ptr<std::istream> const& in)
{
	octree->init(*in);
}

void TreeMethodLOD::setShaderColor(QColor const& color)
{
	std::vector<float> colorVec;
	colorVec.reserve(3);
	colorVec.push_back(color.redF());
	colorVec.push_back(color.greenF());
	colorVec.push_back(color.blueF());
	shaderProgram.setUnusedAttributesValues({{"color", colorVec}});
}

TreeMethodLOD::~TreeMethodLOD()
{
	cleanUp();
}
