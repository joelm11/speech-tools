# Function to add a test executable for CTest.
# test_name:    Test executable name.
# test_source:  Test source file name.
# test_libs:    Test link libraries as ';' separated list.
function(unit_test test_name test_source test_libs)
    add_executable(${test_name} ${test_source})
    target_link_libraries(${test_name} 
        PRIVATE
            ${test_libs}
        PUBLIC
            GTest::gtest_main)
    gtest_discover_tests(${test_name} 
    DISCOVERY_MODE PRE_TEST)
endfunction()