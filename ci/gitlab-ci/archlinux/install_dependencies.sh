#!/usr/bin/bash

pacman -Sy --noconfirm

pacman -S --noconfirm git base-devel cmake assimp openvr qt5-base qt5-gamepad

# AUR
mkdir deps
cd deps

# Remove makepkg noroot safety rule
file=$(which makepkg)
line_number=$(grep -n 'catastrophic' "$file" | cut -f1 -d:)
start=$((line_number - 2))
end=$((line_number + 2))
sed -i "${start},${end}c\echo \"Running as root...\"" "$file"
# Accelerate build
line_number=$(grep -n 'MAKEFLAGS=' /etc/makepkg.conf | cut -f1 -d:)
sed -i "${line_number}cMAKEFLAGS=\"-j\$\(nproc\)\"" /etc/makepkg.conf

# KTX
git clone https://aur.archlinux.org/ktx_software-git.git
cd ktx_software-git
# Fix : https://aur.archlinux.org/packages/ktx_software-git#comment-911429
sed -i "s/master/main?tag=v4.1.0/" PKGBUILD
makepkg -si --noconfirm
cd ..


# PythonQt
git clone https://aur.archlinux.org/pythonqt.git
cd pythonqt
# Fix Qt Webkit package name
sed -i "s/webkit/webengine/" PKGBUILD
makepkg -si --noconfirm
cd ..

cd ..

# install project additional deps
/project_install_dependencies.sh
