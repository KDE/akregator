# SPDX-License-Identifier: CC0-1.0
# SPDX-FileCopyrightText: none
macro(akregator_widgets_unittest _source)
    get_filename_component(_name ${_source} NAME_WE)
    ecm_add_test(${_source} ${_name}.h
        TEST_NAME ${_name}
        NAME_PREFIX "akregator-widgets"
        LINK_LIBRARIES Qt::Test akregatorprivate Qt::Widgets
)
endmacro()
akregator_widgets_unittest(statussearchbuttonstest.cpp)
