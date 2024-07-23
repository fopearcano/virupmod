#!/bin/bash
#
#    Copyright (C) 2018 Florian Cabot <florian.cabot@hotmail.fr>
#
#    This program is free software; you can redistribute it and/or modify
#    it under the terms of the GNU General Public License as published by
#    the Free Software Foundation; either version 3 of the License, or
#    (at your option) any later version.
#
#    This program is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
#    GNU General Public License for more details.
#
#    You should have received a copy of the GNU General Public License along
#    with this program; if not, write to the Free Software Foundation, Inc.,
#    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

# parse all files names which are in arguments 2 to first argument starting with -
files=($2)
i=3
while [[ ${i} -lt $# ]]
do
	arg=${!i}
	if [[ ${arg:0:1} == "-" ]]
	then
		break
	fi
	files+=($arg)
	i=$((i + 1))
done

checksums_file="passed_tidy.txt"
touch $checksums_file

echo "-=-=-=- LAUNCHING CLANG-TIDY -=-=-=-"
# run all clang-tidies in parallel
max_parallel=$(nproc)
j=0
for f in ${files[@]}
do
	launched=$(ps aux | grep $1 | wc -l)
	launched=$(($launched - 2)) # why 2 ?
	# limit to max_parallel
	while [[ "$launched" -ge "$max_parallel" ]]; do
		sleep 1
		launched=$(ps aux | grep $1 | wc -l)
		launched=$(($launched - 2)) # why 2 ?
	done
	file_name=$(basename $f)
	if [[ "$file_name" == "main.cpp" ]]; then
		file_name="$(basename $(dirname $f))_$(basename $f)"
	fi
	j=$(($j + 1))
	out=/tmp/clang_tidy_${file_name}.out

	# Calculate the checksum of the current file
	checksum=$(sha256sum "$f" | cut -d' ' -f1)

	# If the checksum is not in the checksums_file, execute the long-running command
	if ! grep -q "$checksum" "$checksums_file"; then
		# Run the long command
		echo $f
		$1 --use-color $f ${@:${i}} 2> /dev/null > $out &
	else
		touch $out
	fi
done

# wait for them to finish
wait

echo "-=-=-=- RESULTS -=-=-=-"
# print report and return
error=0
for f in ${files[@]}
do
	file_name=$(basename $f)
	if [[ "$file_name" == "main.cpp" ]]; then
		file_name="$(basename $(dirname $f))_$(basename $f)"
	fi
	out=/tmp/clang_tidy_${file_name}.out

	result=$(cat $out)

	if [[ "$result" != "" ]]; then
		cat $out
		error=1
	else
		# Calculate the checksum of the current file
		checksum=$(sha256sum "$f" | cut -d' ' -f1)

		# If the command succeeded, store the checksum
		echo "$checksum" >> "$checksums_file"
	fi
	rm $out
done
exit $error
