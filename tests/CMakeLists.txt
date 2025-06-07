# GoogleTest без GMock
set(BUILD_GMOCK OFF CACHE BOOL "" FORCE)
set(INSTALL_GMOCK OFF CACHE BOOL "" FORCE)

add_subdirectory(${EXTERNAL_DIR}/googletest googletest_build EXCLUDE_FROM_ALL)
set_property(TARGET gtest PROPERTY FOLDER "external/googletest")
set_property(TARGET gtest_main PROPERTY FOLDER "external/googletest")


file(GLOB_RECURSE GTEST_SOURCES CONFIGURE_DEPENDS "*.cpp")
set(NUMSYS_GTEST_NAME gtest_cpp_numeric_system)

# Тесты напрямую добавляются к таргету
add_executable(${NUMSYS_GTEST_NAME} ${GTEST_SOURCES})
target_link_libraries(${NUMSYS_GTEST_NAME}
    PRIVATE
        ${PROJECT_NAME}
        GTest::gtest_main
)
set_property(TARGET ${NUMSYS_GTEST_NAME} PROPERTY FOLDER "external/googletest")

include(GoogleTest)
gtest_discover_tests(${NUMSYS_GTEST_NAME}
    PROPERTIES
        LABELS "unit"
        DISCOVERY_TIMEOUT 30
        ENVIRONMENT "GTEST_OUTPUT=xml:${CMAKE_BINARY_DIR}/rtest.xml"
)
