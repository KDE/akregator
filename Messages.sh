#! /bin/sh
./grantlee-extractor-pot-scripts/extract_strings_ki18n.py `find -name \*.html` >> src/html.cpp
$EXTRACTRC `find . -name "*.rc" -o -name "*.ui" -o -name "*.kcfg"` >> rc.cpp || exit 11
$XGETTEXT $(find . -name "*.cpp" -o -name "*.h") -o $podir/akregator.pot
rm -f rc.cpp src/html.cpp ./grantlee-extractor-pot-scripts/grantlee_strings_extractor.pyc
