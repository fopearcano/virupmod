# Find Qt6Gamepad
#
# Sets Qt6Gamepad_FOUND, QT6GAMEPAD_INCLUDE_DIRS, QT6GAMEPAD_LIBRARIES
#

find_path(QT6GAMEPAD_INCLUDE_DIRS QtGamepad/qgamepad.h
	HINTS
	${QT6GAMEPAD_DIR}
	$ENV{QT6GAMEPAD_DIR}
	PATH_SUFFIXES headers/
	# TODO: Unsure on handling of the possible default install locations
	PATHS
	~/Library/Frameworks
	/Library/Frameworks
	/usr/local/include/
	/usr/include/
	/usr/include/x86_64-linux-gnu
	/usr/include/x86_64-linux-gnu/qt6
	/sw # Fink
	/opt/local # DarwinPorts
	/opt/csw # Blastwave
	/opt
)
find_library(QT6GAMEPAD_LIBRARIES NAMES Qt6Gamepad 
	PATHS
	/usr/lib
	/usr/lib/x86_64-linux-gnu
	/usr/local/lib
	"${QT6GAMEPAD_DIR}/lib"
	)

set(Qt6Gamepad_FOUND 0)
if(QT6GAMEPAD_INCLUDE_DIRS AND QT6GAMEPAD_LIBRARIES)
  set(Qt6Gamepad_FOUND 1)
endif()
