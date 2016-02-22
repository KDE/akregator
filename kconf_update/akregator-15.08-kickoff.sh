#!/bin/sh

kickoffrcname=`qtpaths --locate-file  GenericConfigLocation kickoffrc`
if [ -f "$kickoffrcname" ]; then
   sed -i "s/\/akregator.desktop/\/org.kde.akregator.desktop/" $kickoffrcname
fi
