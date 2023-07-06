#!/bin/bash

threads=$(lscpu -p | grep -v "#" | wc -l)
echo Using $threads threads

mkdir -p ./output

for dir in ./*/
do
	if [[ $dir == "./output/" ]]
	then
		continue
	fi
	echo $dir
	cd $dir
	for resdir in ./*/
	do
		echo -e '\t'$resdir
		cd $resdir
		size=$(convert frame00000.png -print "%wx%h\n" /dev/null)
		aspect=${size/x/:}
		fps=$(echo $resdir | cut -d '_' -f2)
		fps=${fps:0:-4}
		output="../../output/${dir:2:-1}_${size}_${fps}fps.mp4"
		lastmodified=$(ls -tL | head -n 1)
		if [[ "$lastmodified" -nt "$output" ]]
		then
			ffmpeg -y -r $fps -f image2 -s $size -start_number 0 -i frame%05d.png -aspect $aspect -vcodec libx265 -crf 18 -preset veryslow -x265-params "pools=$threads" -pix_fmt yuv420p $output
		fi
		cd ..
	done
	cd ..
done
