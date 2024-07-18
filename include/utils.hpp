#ifndef UTILS_H
#define UTILS_H

#include <QDirIterator>
#include <QFile>
#include <QString>
#include <vector>

/** @ingroup pywrap
 */
enum class Side
{
	NONE,
	LEFT,
	RIGHT
};

// To use Side with <PythonQt>
class PySide : public QObject
{
	Q_OBJECT
  public:
	PySide(QObject* parent = nullptr)
	    : QObject(parent){};
	enum Side
	{
		NONE,
		LEFT,
		RIGHT
	};
	Q_ENUM(Side);
};
Q_DECLARE_METATYPE(int)
// </PythonQt>

template <typename T>
inline void append(std::vector<T>& v1, std::vector<T> const& v2);

template <typename T>
void append(std::vector<T>& v1, std::vector<T> const& v2)
{
	v1.insert(v1.end(), v2.begin(), v2.end());
}
namespace utils
{

/* if data/projectdir/relativeDataPath exists,
 *   returns data/projectdir/relativeDataPath
 * else if data/anythirdparty/relativeDataPath exists,
 *   returns data/anythirdparty/relativeDataPath
 * else if data/core/relativeDataPath exists,
 *   returns data/core/relativeDataPath
 * else,
 *   returns relativeDataPath
 */
QString getAbsoluteDataPath(QString const& relativeDataPath);

/** Transforms a position using a transformation matrix
 */
QVector3D transformPosition(QMatrix4x4 const& transform,
                            QVector3D const& position);

/** Transforms a direction using a transformation matrix (neglects translation)
 */
QVector3D transformDirection(QMatrix4x4 const& transform,
                             QVector3D const& direction);

} // namespace utils

#endif // UTILS_H
