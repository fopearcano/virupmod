#include <QApplication>
#include <QLibraryInfo>
#include <QSettings>
#include <QTranslator>
#include <sstream>
#ifdef Q_OS_UNIX
#include <QDir>
#include <unistd.h>
#endif

#include "Launcher.hpp"
#include "Logger.hpp"
#include "MainWin.hpp"

#define STRINGIFY2(X) #X
#define STRINGIFY(X) STRINGIFY2(X)

int main(int argc, char* argv[])
{
	std::ostringstream versionOss;
	versionOss << PROJECT_NAME << " version " << PROJECT_VERSION << std::endl;
	versionOss << "\tCompiled against Qt " << QT_VERSION_STR
	           << " (Runtime version: " << qVersion() << ")" << std::endl;
	versionOss << "\tWill request OpenGL version " << OPENGL_MAJOR_VERSION
	           << "." << OPENGL_MINOR_VERSION << ' '
	           << STRINGIFY(OPENGL_PROFILE) << std::endl;
	versionOss << "\tGamepad support : ";
#ifdef QT_GAMEPAD
	versionOss << "ON" << std::endl;
#else
	versionOss << "OFF" << std::endl;
#endif
	versionOss << "\tPythonQt support : ";
#ifdef PYTHONQT
	versionOss << "ON (Python version " << PYTHON_VERSION << ")" << std::endl;
#else
	versionOss << "OFF" << std::endl;
#endif
	versionOss << "\tPythonQt_QtAll support : ";
#ifdef PYTHONQT_QTALL
	versionOss << "ON (Python version " << PYTHON_VERSION << ")" << std::endl;
#else
	versionOss << "OFF" << std::endl;
#endif
	versionOss << "\tLeapMotion support : ";
#ifdef LEAP_MOTION
	versionOss << "ON" << std::endl;
#else
	versionOss << "OFF" << std::endl;
#endif
	versionOss << "\tlibktx support : ";
#ifdef LIBKTX
	versionOss << "ON" << std::endl;
#else
	versionOss << "OFF" << std::endl;
#endif
	versionOss << "\tlibzstd support : ";
#ifdef LIBZSTD
	versionOss << "ON" << std::endl;
#else
	versionOss << "OFF" << std::endl;
#endif
	if(argc == 2 && std::string(argv[1]) == "--version")
	{
		std::cout << versionOss.str();
		return EXIT_SUCCESS;
	}

	// Set config file names for QSettings
	QCoreApplication::setOrganizationName(PROJECT_NAME);
	QCoreApplication::setApplicationName("config");

	// setup logging
	Logger::init();

	qDebug() << versionOss.str().c_str();

	const QApplication a(argc, argv);

	// Set arguments
	QCommandLineParser parser;
	parser.addHelpOption();
	const QCommandLineOption noLauncher(
	    "no-launcher", QCoreApplication::translate(
	                       "main", "By-pass launcher and launch application."));
	parser.addOption(noLauncher);
	const QCommandLineOption config(
	    "config",
	    QCoreApplication::translate("main", "Read .ini config from <file>."),
	    "file");
	parser.addOption(config);
	const QCommandLineOption version(
	    "version",
	    QCoreApplication::translate("main", "Display version information."));
	parser.addOption(version);
	parser.process(a);

	// set settings
	QSettings::setDefaultFormat(QSettings::IniFormat);
	if(parser.isSet(config))
	{
		const QString configPath(parser.value(config));
		const QFileInfo file(configPath);
		QDir dir(file.absoluteDir());
		QCoreApplication::setOrganizationName(dir.dirName());
		dir.cdUp();
		QCoreApplication::setApplicationName(file.baseName());
		QSettings::setPath(QSettings::IniFormat, QSettings::UserScope,
		                   dir.absolutePath());
	}

	// set translation
	const QString localeName(
	    QSettings()
	        .value("window/language", QLocale::system().name().left(2))
	        .toString());
	QLocale::setDefault(QLocale{localeName});
	QTranslator qtTranslator;
	if(qtTranslator.load("qt_" + localeName,
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
	                     QLibraryInfo::path
#else
	                     QLibraryInfo::location
#endif
	                     (QLibraryInfo::TranslationsPath)))
	{
		QCoreApplication::installTranslator(&qtTranslator);
	}

	QTranslator hvrTranslator;
	if(hvrTranslator.load("HydrogenVR_" + localeName, "data/translations/"))
	{
		QCoreApplication::installTranslator(&hvrTranslator);
	}

	QTranslator programTranslator;
	if(programTranslator.load(QString(PROJECT_NAME) + "_" + localeName,
	                          "data/translations/"))
	{
		QCoreApplication::installTranslator(&programTranslator);
	}

#ifdef Q_OS_UNIX
// set current dir as application dir path to avoid reading coincidental
// data/core that doesn't belong to it
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-result"
	chdir(QCoreApplication::applicationDirPath().toLocal8Bit().data());
#pragma GCC diagnostic pop
	if(!QDir("./data/core").exists())
	{
		// if no data/core in application dir, search in
		// ../share/PROJECT_NAME
		QString path(QString("../share/") + PROJECT_NAME);
		if(!QDir(path).exists())
		{
			// else let it be /usr/share/PROJECT_NAME
			path = QString("/usr/share/") + PROJECT_NAME;
		}
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-result"
		chdir((path.toLocal8Bit().data()));
#pragma GCC diagnostic pop
	}
#endif

#ifdef PYTHONQT
	// set PYTHONPATH
	const QDir pathDir(QDir::currentPath() + "/python");
	if(pathDir.exists())
	{
		if(QString::fromLocal8Bit(qgetenv("PYTHONPATH")) == "")
		{
			qputenv("PYTHONPATH",
			        (QDir::currentPath() + "/python").toLocal8Bit());
		}
	}
#endif

	if(!parser.isSet(noLauncher))
	{
		Launcher launcher;
		launcher.init();
		if(launcher.exec() == QDialog::Rejected)
		{
			return EXIT_SUCCESS;
		}
	}

	MainWin w;
	w.setTitle(PROJECT_NAME + QString(" - Loading..."));
	w.setFullscreen(w.isFullscreen());
	// start event loop
	QCoreApplication::postEvent(&w, qt_owned<QEvent>(QEvent::UpdateRequest));
	return QApplication::exec();

	// close log file
	Logger::close();
}
