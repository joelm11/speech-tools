
add_library(common INTERFACE)
target_include_directories(common INTERFACE ${CMAKE_CURRENT_LIST_DIR})

if(STANDALONE_BUILD AND DEBUG) 
    unit_test(test_common "test/spsc_queue_test.cc" "common")
endif()