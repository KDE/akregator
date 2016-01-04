add_executable(articleviewerngtest articleviewerngtest.cpp main.cpp)

target_link_libraries(articleviewerngtest
    Qt5::Widgets
    KF5::CoreAddons
    KF5::I18n
    Qt5::Core
)
