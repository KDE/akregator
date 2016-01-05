set(articleviewerwidgetngtest_SRCS  articleviewerwidgetngtest.cpp )

add_executable( articleviewerwidgetngtest ${articleviewerwidgetngtest_SRCS} )
add_test( articleviewerwidgetngtest articleviewerwidgetngtest)
ecm_mark_as_test(articleviewerwidgetngtest)

target_link_libraries(
  articleviewerwidgetngtest
  KF5::MessageViewer
  Qt5::Test
)

