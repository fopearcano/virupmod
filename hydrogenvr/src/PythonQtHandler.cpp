/*
    Copyright (C) 2019 Florian Cabot <florian.cabot@hotmail.fr>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#include "PythonQtHandler.hpp"

#ifdef PYTHONQT
std::unique_ptr<PythonQtObjectPtr>& PythonQtHandler::mainModule()
{
	static std::unique_ptr<PythonQtObjectPtr> mainModule;
	return mainModule;
}

std::unique_ptr<PythonQtScriptingConsole>& PythonQtHandler::console()
{
	static std::unique_ptr<PythonQtScriptingConsole> console;
	return console;
}
#endif

bool PythonQtHandler::isSupported()
{
#ifdef PYTHONQT
	return true;
#else
	return false;
#endif
}

bool PythonQtHandler::isInitialized()
{
#ifdef PYTHONQT
	return console() != nullptr;
#else
	return false;
#endif
}

void PythonQtHandler::init()
{
#ifdef PYTHONQT
	clean();
	// init PythonQt and Python
	PythonQt::init(PythonQt::RedirectStdOut);
#ifdef PYTHONQT_QTALL
	PythonQt_QtAll::init();
#endif

	// get the __main__ python module
	mainModule() = std::make_unique<PythonQtObjectPtr>(
	    PythonQt::self()->getMainModule());

	console()
	    = std::make_unique<PythonQtScriptingConsole>(nullptr, *mainModule());
#endif
}

void PythonQtHandler::addVariable(QString const& name, QVariant const& v)
{
#ifdef PYTHONQT
	mainModule()->addVariable(name, v);
#else
	(void) name;
	(void) v;
#endif
}

QVariant PythonQtHandler::getVariable(QString const& name)
{
#ifdef PYTHONQT
	return mainModule()->getVariable(name);
#else
	(void) name;
	return {};
#endif
}

void PythonQtHandler::removeVariable(QString const& name)
{
#ifdef PYTHONQT
	mainModule()->removeVariable(name);
#else
	(void) name;
#endif
}

void PythonQtHandler::addObject(QString const& name, QObject* object)
{
#ifdef PYTHONQT
	mainModule()->addObject(name, object);
#else
	(void) name;
	(void) object;
#endif
}

QVariant PythonQtHandler::evalScript(QString const& script)
{
#ifdef PYTHONQT
	return mainModule()->evalScript(script);
#else
	(void) script;
	return {};
#endif
}

void PythonQtHandler::evalFile(QString const& filename)
{
#ifdef PYTHONQT
	mainModule()->evalFile(filename);
#else
	(void) filename;
#endif
}

void PythonQtHandler::openConsole()
{
#ifdef PYTHONQT
	console()->show();
#endif
}

void PythonQtHandler::toggleConsole()
{
#ifdef PYTHONQT
	console()->setVisible(!console()->isVisible());
#endif
}

void PythonQtHandler::closeConsole()
{
#ifdef PYTHONQT
	console()->hide();
#endif
}

void PythonQtHandler::clean()
{
#ifdef PYTHONQT
	mainModule().reset();
	console().reset();
#endif
}

void PythonQtWrapper::overloadStaticBinary(const char* op) const
{
	PythonQtHandler::evalScript(
	    QString() + "def __" + op
	    + "__(x,y):\n"
	      "\treturn "
	    + wrappedClassPackage() + "." + wrappedClassName() + "." + op
	    + "(x,y)\n"
	      "setattr("
	    + wrappedClassPackage() + "." + wrappedClassName() + ", '__" + op
	    + "__', __" + op + "__)");
}

void PythonQtWrapper::overloadMember(const char* op) const
{
	PythonQtHandler::evalScript(
	    QString() + "def __" + op + "__(x):\n\treturn x." + op
	    + "()\n"
	      "setattr("
	    + wrappedClassPackage() + "." + wrappedClassName() + ", '__" + op
	    + "__', __" + op + "__)");
}

void PythonQtWrapper::overloadMemberUnary(const char* op) const
{
	PythonQtHandler::evalScript(
	    QString() + "def __" + op + "__(x,y):\n\treturn x." + op
	    + "(y)\n"
	      "setattr("
	    + wrappedClassPackage() + "." + wrappedClassName() + ", '__" + op
	    + "__', __" + op + "__)");
}

void PythonQtWrapper::overloadMemberBinary(const char* op) const
{
	PythonQtHandler::evalScript(
	    QString() + "def __" + op + "__(x,y,z):\n\treturn x." + op
	    + "(y,z)\n"
	      "setattr("
	    + wrappedClassPackage() + "." + wrappedClassName() + ", '__" + op
	    + "__', __" + op + "__)");
}

void PythonQtWrapper::overloadPythonOperators()
{
	PythonQtHandler::evalScript(QString("import PythonQt.")
	                            + wrappedClassPackage() + " as "
	                            + wrappedClassPackage());

	overloadStaticBinary("add");
	overloadStaticBinary("sub");
	overloadStaticBinary("mul");
	overloadStaticBinary("rmul");
	overloadStaticBinary("truediv");
	overloadStaticBinary("floordir");
	overloadStaticBinary("mod");
	overloadStaticBinary("pow");

	overloadMemberUnary("iadd");
	overloadMemberUnary("isub");
	overloadMemberUnary("imul");
	overloadMemberUnary("itruediv");
	overloadMemberUnary("ifloordiv");
	overloadMemberUnary("imod");
	overloadMemberUnary("ipow");

	overloadStaticBinary("lt");
	overloadStaticBinary("gt");
	overloadStaticBinary("le");
	overloadStaticBinary("ge");
	overloadStaticBinary("eq");
	overloadStaticBinary("ne");

	overloadMemberUnary("getitem");
	overloadMemberBinary("setitem");

	overloadMember("str");
}
