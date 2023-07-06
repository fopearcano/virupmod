#!/bin/bash

# Checking if at least three arguments are passed
if [ $# -lt 3 ]
then
    echo "Usage: $0 <command> <dir_paths> <file1> [file2] ..."
    exit 1
fi

# Extracting the command and directory paths
cmd=$1
IFS=':' read -ra dir_paths <<< "$2"

# Shifting the positional parameters to remove the first two arguments
shift 2

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

ret=0
# Looping over all files and executing the command
for file in "$@"
do
    if [ -f "$file" ]
    then
        ext="${file##*.}"
        tmpfile=$(mktemp --suffix=".$ext")
        process_includes "$file" "$tmpfile"
        echo "$file"
        $cmd "$tmpfile"
        ret=$(($ret + $?))
        rm "$tmpfile"
    else
        echo "File $file does not exist."
    fi
done

exit $ret
