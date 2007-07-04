include_directories( ${CMAKE_SOURCE_DIR}/akregator/kfeed )

########### next target ###############

set(testxmlserializer_SRCS testxmlserializer.cpp )

kde4_automoc(${testxmlserializer_SRCS})

kde4_add_unit_test(testxmlserializer TESTNAME kfeed-testxmlserializer  ${testxmlserializer_SRCS})

target_link_libraries(testxmlserializer kfeed ${KDE4_KDECORE_LIBS} ${QT_QTTEST_LIBRARY} )
