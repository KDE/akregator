# SPDX-License-Identifier: CC0-1.0
# SPDX-FileCopyrightText: none
macro(akregator_job_unittest _source)
    get_filename_component(_name ${_source} NAME_WE)
    ecm_add_test(${_source} ${_name}.h
        TEST_NAME ${_name}
        NAME_PREFIX "akregator-job"
        LINK_LIBRARIES Qt::Test akregatorprivate
    )
endmacro()

akregator_job_unittest(downloadfeediconjobtest.cpp)
