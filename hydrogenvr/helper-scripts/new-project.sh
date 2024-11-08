#!/bin/bash

# Function to prompt for a string with a default value
prompt_string() {
    read -p "$1 [$2]: " input
    echo "${input:-$2}"
}

# Function to prompt for a Y/N boolean with a default value
prompt_boolean() {
	printed=$(if [[ $2 == true ]]; then echo y; else echo n; fi)
    read -p "$1 (y/n) [$printed]: " input
    case "${input,,}" in
        y|yes|Y) echo "true" ;;
        n|no|N) echo "false" ;;
        *) echo "$2" ;;
    esac
}
#
# Function to convert git remote URL to front-page URL
convert_git_url_to_front_page() {
    local git_url=$1
    local front_page_url=""

    if [[ $git_url == git@* ]]; then
        # Convert SSH URL to HTTPS URL
        front_page_url=$(echo "$git_url" | sed -e 's/:/\//g' -e 's/^git@/https:\/\//')
    elif [[ $git_url == https://* ]]; then
        front_page_url=$git_url
    fi

    # Remove the .git suffix if it exists
    front_page_url=${front_page_url%.git}

    echo "$front_page_url"
}

# Get the default project URL
DEFAULT_PROJECT_URL=$(git remote get-url origin 2>/dev/null)
DEFAULT_PROJECT_URL=$(convert_git_url_to_front_page "$DEFAULT_PROJECT_URL")

# Prompt for project details
PROJECT_NAME=$(prompt_string "Enter project name" "" | tr ' ' '-')
PROJECT_DIRECTORY=$(prompt_string "Enter project directory path" "$(echo $PROJECT_NAME | tr '[:upper:]' '[:lower:]')")
PROJECT_ICON=$(prompt_string "Enter project icon path" "./hydrogenvr/icon.png")
PROJECT_URL=$(prompt_string "Enter project URL" "$DEFAULT_PROJECT_URL")
PROJECT_DESCRIPTION=$(prompt_string "Enter project description" "")
PROJECT_MAINTAINER_NAME=$(prompt_string "Enter maintainer name" "")
PROJECT_MAINTAINER_EMAIL=$(prompt_string "Enter maintainer email" "")
PROJECT_MAINTAINER="$PROJECT_MAINTAINER_NAME <$PROJECT_MAINTAINER_EMAIL>"

# Prompt for boolean values
if [[ -f LICENSE ]]
then
	PROJECT_USE_GPL3="false"
else
	PROJECT_USE_GPL3=$(prompt_boolean "Use GPL3? (Don't forget to set a license yourself if 'no'.)" "true")
fi
PROJECT_CHECK_FORMAT=$(prompt_boolean "Check format?" "true")
PROJECT_CHECK_TIDY=$(prompt_boolean "Check tidy?" "true")
PROJECT_CHECK_FORMAT_THIRDPARTY=$(prompt_boolean "Check format for third party?" "true")
PROJECT_CHECK_TIDY_THIRDPARTY=$(prompt_boolean "Check tidy for third party?" "true")
PROJECT_DOC=$(prompt_boolean "Generate documentation?" "true")
PROJECT_DOC_ENGINE=$(prompt_boolean "Generate documentation for engine?" "false")
PROJECT_DOC_THIRDPARTY=$(prompt_boolean "Generate documentation for third party?" "true")
PROJECT_TRANSLATE=$(prompt_boolean "Enable translation?" "true")
PROJECT_TRANSLATE_ENGINE=$(prompt_boolean "Enable translation for engine?" "false")
PROJECT_TRANSLATE_THIRDPARTY=$(prompt_boolean "Enable translation for third party?" "true")

# Prompt for integer values
OPENGL_MAJOR_VERSION=$(prompt_string "OpenGL major version" "4")
OPENGL_MINOR_VERSION=$(prompt_string "OpenGL minor version" "5")

# Prompt for OpenGL profile
OPENGL_PROFILE=$(prompt_string "OpenGL profile (Core/Compatibility)" "Core")

# Output the variables

echo "-=-=- SUMMARY -=-=-"
echo ""
echo "PROJECT_NAME=\"$PROJECT_NAME\""
echo "PROJECT_DIRECTORY=\"$PROJECT_DIRECTORY\""
echo "PROJECT_ICON=\"$PROJECT_ICON\""
echo "PROJECT_URL=\"$PROJECT_URL\""
echo "PROJECT_DESCRIPTION=\"$PROJECT_DESCRIPTION\""
echo "PROJECT_MAINTAINER=\"$PROJECT_MAINTAINER\""
echo "PROJECT_USE_GPL3=\"$PROJECT_USE_GPL3\""
echo "PROJECT_CHECK_FORMAT=$PROJECT_CHECK_FORMAT"
echo "PROJECT_CHECK_TIDY=$PROJECT_CHECK_TIDY"
echo "PROJECT_CHECK_FORMAT_THIRDPARTY=$PROJECT_CHECK_FORMAT_THIRDPARTY"
echo "PROJECT_CHECK_TIDY_THIRDPARTY=$PROJECT_CHECK_TIDY_THIRDPARTY"
echo "PROJECT_DOC=$PROJECT_DOC"
echo "PROJECT_DOC_ENGINE=$PROJECT_DOC_ENGINE"
echo "PROJECT_DOC_THIRDPARTY=$PROJECT_DOC_THIRDPARTY"
echo "PROJECT_TRANSLATE=$PROJECT_TRANSLATE"
echo "PROJECT_TRANSLATE_ENGINE=$PROJECT_TRANSLATE_ENGINE"
echo "PROJECT_TRANSLATE_THIRDPARTY=$PROJECT_TRANSLATE_THIRDPARTY"
echo "OPENGL_MAJOR_VERSION=$OPENGL_MAJOR_VERSION"
echo "OPENGL_MINOR_VERSION=$OPENGL_MINOR_VERSION"
echo "OPENGL_PROFILE=\"$OPENGL_PROFILE\""
echo ""
CONT=$(prompt_boolean "Continue?" "false")

if [[ $CONT == false ]]; then
	exit 0
fi

echo "Generating new project..."
echo "PROJECT_DIRECTORY=\"$PROJECT_DIRECTORY\"" > project_directory.conf
echo "HVR_DIRECTORY=\"hydrogenvr\"" >> project_directory.conf
mkdir -p $PROJECT_DIRECTORY
cp -r hydrogenvr/blank/* $PROJECT_DIRECTORY/
BUILD_CONF=$(echo $PROJECT_DIRECTORY/build.conf)

SETTINGS_CLASS_NAME=$(echo $PROJECT_NAME | tr -cd '[:alnum:]')
SETTINGS_GUARD_NAME=$(echo $SETTINGS_CLASS_NAME | tr '[:lower:]' '[:upper:]')

mv $PROJECT_DIRECTORY/include/BlankSettings.hpp $PROJECT_DIRECTORY/include/${SETTINGS_CLASS_NAME}Settings.hpp
sed -i "s/Blank/${SETTINGS_CLASS_NAME}/g" $PROJECT_DIRECTORY/include/${SETTINGS_CLASS_NAME}Settings.hpp
sed -i "s/BLANK/${SETTINGS_GUARD_NAME}/g" $PROJECT_DIRECTORY/include/${SETTINGS_CLASS_NAME}Settings.hpp
sed -i "s/Blank/${SETTINGS_CLASS_NAME}/g" $PROJECT_DIRECTORY/include/Launcher.hpp
sed -i "s/Blank/${SETTINGS_CLASS_NAME}/g" $PROJECT_DIRECTORY/src/Launcher.cpp

echo "PROJECT_NAME=\"$PROJECT_NAME\"" > $BUILD_CONF
echo "PROJECT_ICON=\"$PROJECT_ICON\"" >> $BUILD_CONF
echo "PROJECT_URL=\"$PROJECT_URL\"" >> $BUILD_CONF
echo "PROJECT_DESCRIPTION=\"$PROJECT_DESCRIPTION\"" >> $BUILD_CONF
echo "PROJECT_MAINTAINER=\"$PROJECT_MAINTAINER\"" >> $BUILD_CONF
echo "PROJECT_CHECK_FORMAT=$PROJECT_CHECK_FORMAT" >> $BUILD_CONF
echo "PROJECT_CHECK_TIDY=$PROJECT_CHECK_TIDY" >> $BUILD_CONF
echo "PROJECT_CHECK_FORMAT_THIRDPARTY=$PROJECT_CHECK_FORMAT_THIRDPARTY" >> $BUILD_CONF
echo "PROJECT_CHECK_TIDY_THIRDPARTY=$PROJECT_CHECK_TIDY_THIRDPARTY" >> $BUILD_CONF
echo "PROJECT_DOC=$PROJECT_DOC" >> $BUILD_CONF
echo "PROJECT_DOC_ENGINE=$PROJECT_DOC_ENGINE" >> $BUILD_CONF
echo "PROJECT_DOC_THIRDPARTY=$PROJECT_DOC_THIRDPARTY" >> $BUILD_CONF
echo "PROJECT_TRANSLATE=$PROJECT_TRANSLATE" >> $BUILD_CONF
echo "PROJECT_TRANSLATE_ENGINE=$PROJECT_TRANSLATE_ENGINE" >> $BUILD_CONF
echo "PROJECT_TRANSLATE_THIRDPARTY=$PROJECT_TRANSLATE_THIRDPARTY" >> $BUILD_CONF
echo "OPENGL_MAJOR_VERSION=$OPENGL_MAJOR_VERSION" >> $BUILD_CONF
echo "OPENGL_MINOR_VERSION=$OPENGL_MINOR_VERSION" >> $BUILD_CONF
echo "OPENGL_PROFILE=\"$OPENGL_PROFILE\"" >> $BUILD_CONF

# copy additional files

if [[ ! -f CMakeLists.txt ]]
then
	echo 'cmake_minimum_required(VERSION 3.10.0)
include("hydrogenvr/CMakeLists.txt")' > CMakeLists.txt
fi
if [[ ! -f .gitlab-ci.yml ]]
then
	echo "include: 'hydrogenvr/.gitlab-ci.yml'" > .gitlab-ci.yml
fi
if [[ ! -f .gitignore ]]
	cp hydrogenvr/.gitignore .gitignore
fi
if [[ "$PROJECT_USE_GPL3" == "true" ]]
then
	cp hydrogenvr/LICENSE LICENSE
fi
if [[ "$PROJECT_CHECK_FORMAT" == "true" || "$PROJECT_CHECK_FORMAT_THIRDPARTY" == "true" ]]
then
	cp hydrogenvr/.clang-format $PROJECT_DIRECTORY/.clang-format
fi
if [[ "$PROJECT_CHECK_TIDY" == "true" || "$PROJECT_CHECK_TIDY_THIRDPARTY" == "true" ]]
then
	cp hydrogenvr/.clang-tidy $PROJECT_DIRECTORY/.clang-tidy
fi


echo "Done..."
echo "If you want to setup CI/CD, please refer to: https://gitlab.com/Dexter9313/hydrogenvr/-/wikis/tutorials/1-general-topics/9-continuous-integration"

rm -rf build
