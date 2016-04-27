#!/bin/bash
set -e

for i in {0..7} ; do
	TopAsteroidsLinux.x86 &
	sleep 5
	window[$i]=`xdotool getwindowfocus`

	# Bot being optimized
	xdotool mousemove --window $window[$i] --sync 600 320 click 1
	sleep .1
	xdotool mousemove --window $window[$i] --sync 530 310 click 1
	sleep .1

	# Other bots
	xdotool mousemove --window $window[$i] --sync 600 350 click 1
	sleep .1
	xdotool mousemove --window $window[$i] --sync 530 310 click --repeat 3 1
	sleep .1

	xdotool mousemove --window $window[$i] --sync 600 240 click 1
	sleep .1
	xdotool mousemove --window $window[$i] --sync 530 310 click --repeat 4 1
	sleep .1

	# Number of matches
	xdotool mousemove --window $window[$i] --sync 480 550 click 1
	sleep .1
	xdotool key 1

	# Start
	xdotool mousemove --window $window[$i] --sync 480 590 click 1
done

sleep $((60 * 2))

for i in {0..7} ; do
	xdotool windowfocus --sync $window[$i]
	sleep 2
	scrot -zu "Bot $i - "'%F-%R.png'
done

killall TopAsteroidsLinux.x86 shibata
