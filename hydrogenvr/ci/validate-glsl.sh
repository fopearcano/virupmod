#!/bin/bash

# Checking if no file
if [ $# == 2 ]
then
	exit 0
fi

# Checking if at least three arguments are passed
if [ $# -lt 2 ]
then
	echo "Usage: $0 <command> <dir_paths> [file1] [file2] ..."
	exit 1
fi

# Extracting the command and directory paths
cmd=$1
IFS=':' read -ra dir_paths <<< "$2"

# Shifting the positional parameters to remove the first two arguments
shift 2

# Function to add defines for compute shaders
add_compute_defines() {
	local file="$1"
	local tmpfile="$2"
	local tmpfile2=$(mktemp)

	# Add the defines
	echo "#define LOCAL_SIZE_1D_X 1" > "$tmpfile2"
	echo "#define LOCAL_SIZE_2D_X 1" >> "$tmpfile2"
	echo "#define LOCAL_SIZE_2D_Y 1" >> "$tmpfile2"
	echo "#define LOCAL_SIZE_3D_X 1" >> "$tmpfile2"
	echo "#define LOCAL_SIZE_3D_Y 1" >> "$tmpfile2"
	echo "#define LOCAL_SIZE_3D_Z 1" >> "$tmpfile2"

	# Use awk to insert the defines after the line containing "#version"
	awk -v defines="$(cat "$tmpfile2")" '
		/#version/ { print; print defines; next }
		{ print }
	' "$file" > "$tmpfile"

	# Clean up temporary file
	rm "$tmpfile2"
}

# Function to process includes recursively
process_includes() {
	local file="$1"
	local tmpfile="$2"

	while IFS= read -r line
	do
		if [[ "$line" =~ \#include\ \<(.*)\> ]]
		then
			local relative_path="${BASH_REMATCH[1]}"
			local found=0

			for dir_path in "${dir_paths[@]}"
			do
				if [ -f "$dir_path/$relative_path" ]
				then
					process_includes "$dir_path/$relative_path" "$tmpfile"
					found=1
					break
				fi
			done

			if [ $found -eq 0 ]
			then
				echo "File $relative_path does not exist in the directories."
			fi
		else
			echo "$line" >> "$tmpfile"
		fi
	done < "$file"
}

# Process glsl lib files
set_executable() {
	local file="$1"
	local tmpfile="$2"
	local tmpfile2=$(mktemp)

	# Add the defines
	echo "#version 460" > "$tmpfile"
	cat $file >> "$tmpfile"

	# Use awk to insert the defines after the line containing "#version"
	#awk -v defines="$(cat "$tmpfile2")" '
	#	/#version/ { print; print defines; next }
	#	{ print }
	#' "$file" > "$tmpfile"

	# Clean up temporary file
	rm "$tmpfile2"
}

ret=0
conf="./.conf"
$cmd -c > $conf
sed -i 's/MaxAtomicCounterBindings 1/MaxAtomicCounterBindings 10/g' $conf
sed -i 's/MaxDualSourceDrawBuffersEXT 1//g' $conf

# Looping over all files and executing the command
for file in "$@"
do
	if [ -f "$file" ]
	then
		ext="${file##*.}"
		tmpfile=$(mktemp --suffix=".$ext")
		# If file is a compute shader, add defines
		if [ "$ext" = "comp" ]
		then
			add_compute_defines "$file" "$tmpfile"
		else
			cp "$file" "$tmpfile"
		fi
		tmpfile2=$(mktemp --suffix=".$ext")
		process_includes "$tmpfile" "$tmpfile2"
		if [ "$ext" = "glsl" ]
		then
			tmpfile3=$(mktemp --suffix=".frag")
			set_executable "$tmpfile2" "$tmpfile3"
		else
			tmpfile3=$(mktemp --suffix=".$ext")
			cp "$tmpfile2" "$tmpfile3"
		fi
		echo $file
		$cmd "$conf" "$tmpfile3"
		ret=$(($ret + $?))
		rm "$tmpfile"
		rm "$tmpfile2"
		rm "$tmpfile3"
	else
		echo "File $file does not exist."
	fi
done
rm $conf

exit $ret
