include_directories( ${CMAKE_SOURCE_DIR}/akregator/kfeed )

########### next target ###############

set(testxmlserializer_SRCS testxmlserializer.cpp )

kde4_automoc(${testxmlserializer_SRCS})

kde4_add_unit_test(testxmlserializer TESTNAME kfeed-testxmlserializer  ${testxmlserializer_SRCS})

target_link_libraries(testxmlserializer kfeed ${KDE4_KDECORE_LIBS} ${QT_QTTEST_LIBRARY} ${QT_QTGUI_LIBRARY} )

########### next target ###############

set(testitem_SRCS testitem.cpp )

kde4_automoc(${testitem_SRCS})

kde4_add_unit_test(testitem TESTNAME kfeed-testitem  ${testitem_SRCS})

target_link_libraries(testitem kfeed ${KDE4_KDECORE_LIBS} ${QT_QTTEST_LIBRARY} ${QT_QTGUI_LIBRARY} )

########### next target ###############

set(testcategory_SRCS testcategory.cpp )

kde4_automoc(${testcategory_SRCS})

kde4_add_unit_test(testcategory TESTNAME kfeed-testcategory  ${testcategory_SRCS})

target_link_libraries(testcategory kfeed ${KDE4_KDECORE_LIBS} ${QT_QTTEST_LIBRARY} ${QT_QTGUI_LIBRARY} )

########### next target ###############

set(testenclosure_SRCS testenclosure.cpp )

kde4_automoc(${testenclosure_SRCS})

kde4_add_unit_test(testenclosure TESTNAME kfeed-testenclosure  ${testenclosure_SRCS})

target_link_libraries(testenclosure kfeed ${KDE4_KDECORE_LIBS} ${QT_QTTEST_LIBRARY} ${QT_QTGUI_LIBRARY} )

########### next target ###############

set(testperson_SRCS testperson.cpp )

kde4_automoc(${testperson_SRCS})

kde4_add_unit_test(testperson TESTNAME kfeed-testperson  ${testperson_SRCS})

target_link_libraries(testperson kfeed ${KDE4_KDECORE_LIBS} ${QT_QTTEST_LIBRARY} ${QT_QTGUI_LIBRARY} )

