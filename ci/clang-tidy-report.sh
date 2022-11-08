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

echo "-=-=-=- LAUNCHING CLANG-TIDY -=-=-=-"
# run all clang-tidies in parallel
max_parallel=$(nproc)
j=0
for f in ${files[@]}
do
	# limit to max_parallel
	if [[ $(($j % $max_parallel)) == "0" ]]; then
		wait
	fi
	j=$(($j + 1))
	out=/tmp/clang_tidy_$(basename $f).out

	echo $f
	$1 $f ${@:${i}} 2> /dev/null >> $out &
done

# wait for them to finish
wait

echo "-=-=-=- RESULTS -=-=-=-"
# print report and return
error=0
for f in ${files[@]}
do
	out=/tmp/clang_tidy_$(basename $f).out
	result=$(cat $out)
	rm $out

	if [[ "$result" != "" ]]; then
		echo $result
		error=1
	fi
done
exit $error
