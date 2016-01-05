set(articleviewerngngtest_common_SRCS)
ecm_qt_declare_logging_category(articleviewerngngtest_common_SRCS HEADER akregator_debug.h IDENTIFIER AKREGATOR_LOG CATEGORY_NAME log_akregator)


set(articleviewerwidgetngtest_SRCS  articleviewerwidgetngtest.cpp ../articleviewerwidgetng.cpp ${articleviewerngngtest_common_SRCS} ../articleviewerng.cpp)

add_executable( articleviewerwidgetngtest ${articleviewerwidgetngtest_SRCS} )
add_test( articleviewerwidgetngtest articleviewerwidgetngtest)
ecm_mark_as_test(articleviewerwidgetngtest)

target_link_libraries(
  articleviewerwidgetngtest
  KF5::MessageViewer
  Qt5::Test
  KF5::XmlGui
  KF5::WebKit
  KF5::GrantleeTheme
  KF5::PimTextEdit
)

