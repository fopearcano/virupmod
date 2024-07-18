#include "utils.hpp"

#include <QMatrix4x4>
#include <QVector3D>
#include <QVector4D>

namespace utils
{

QString getAbsoluteDataPath(QString const& relativeDataPath)
{
	if(QFile(QString("data/") + PROJECT_DIRECTORY + '/' + relativeDataPath)
	       .exists())
	{
		return QString("data/") + PROJECT_DIRECTORY + '/' + relativeDataPath;
	}

	QStringList dirs;
	QDirIterator it("data/", QStringList() << "*", QDir::AllDirs);
	while(it.hasNext())
	{
		dirs << it.next();
	}

	dirs.removeAll("data/.");
	dirs.removeAll("data/..");
	dirs.removeAll("data/core");
	dirs.removeAll(QString("data/") + PROJECT_DIRECTORY);

	for(auto& dir : dirs)
	{
		if(QFile(dir + "/" + relativeDataPath).exists())
		{
			return dir + "/" + relativeDataPath;
		}
	}

	if(QFile(QString("data/core/") + relativeDataPath).exists())
	{
		return QString("data/core/") + relativeDataPath;
	}
	return relativeDataPath;
}

QVector3D transformPosition(QMatrix4x4 const& transform,
                            QVector3D const& position)
{
	return (transform * QVector4D(position, 1.f)).toVector3D();
}

QVector3D transformDirection(QMatrix4x4 const& transform,
                             QVector3D const& direction)
{
	return (transform * QVector4D(direction, 0.f)).toVector3D();
}

} // namespace utils
