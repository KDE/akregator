#!/bin/sh

kickoffrcname=`kf5-config --path config --locate kickoffrc`
if [ -f "$kickoffrcname" ]; then
   sed -i "s/\/akregator.desktop/\/org.kde.akregator.desktop/" $kickoffrcname
fi
