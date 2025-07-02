add_library(noise_filter INTERFACE)
target_include_directories(noise_filter INTERFACE "${CMAKE_CURRENT_LIST_DIR}/src")

if(STANDALONE_BUILD AND DEBUG) 
    unit_test(test_noise_filter "test/noise_reduction_test.cc" "noise_filter")
endif()