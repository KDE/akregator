set(articleviewerngtest_common_SRCS)
ecm_qt_declare_logging_category(articleviewerngtest_common_SRCS HEADER akregator_debug.h IDENTIFIER AKREGATOR_LOG CATEGORY_NAME log_akregator)

add_executable(articleviewerngtest articleviewerngtest.cpp main.cpp ../articleviewerwidgetng.cpp ../articleviewerng.cpp ${articleviewerngtest_common_SRCS})

target_link_libraries(articleviewerngtest
    Qt5::Widgets
    KF5::CoreAddons
    KF5::I18n
    Qt5::Core
    KF5::XmlGui
    KF5::WebKit
    KF5::GrantleeTheme
    KF5::MessageViewer
)
