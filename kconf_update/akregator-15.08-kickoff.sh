#!/bin/sh

sed -i "s/\/akregator.desktop/\/org.kde.akregator.desktop/" `kf5-config --path config --locate kickoffrc`
