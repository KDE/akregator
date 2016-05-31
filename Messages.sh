#! /bin/sh
$EXTRACT_GRANTLEE_TEMPLATE_STRINGS `find -name \*.html` >> src/html.cpp
$EXTRACTRC `find . -name "*.rc" -o -name "*.ui" -o -name "*.kcfg"` >> rc.cpp || exit 11
$XGETTEXT $(find . -name "*.cpp" -o -name "*.h") -o $podir/akregator.pot
rm -f rc.cpp src/html.cpp ./grantlee-extractor-pot-scripts/grantlee_strings_extractor.pyc
