macro(add_testcase name)

    if(NOT CMAKE_CROSSCOMPILING)
        add_test(NAME ${name} COMMAND $<TARGET_FILE:${name}>)
    endif()

    if(BUILD_INSTALL_TESTS)
        install(TARGETS ${name} RUNTIME DESTINATION share/vulkan/tests)
    endif()

endmacro()
