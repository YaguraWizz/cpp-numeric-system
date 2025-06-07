# Собираем заголовочные и исходные файлы относительно текущего модуля
file(GLOB_RECURSE HEADER_FILES CONFIGURE_DEPENDS "include/*.h")
file(GLOB_RECURSE SOURCE_FILES CONFIGURE_DEPENDS "source/*.cpp")

# Создаём статическую библиотеку
add_library(${PROJECT_NAME} STATIC ${HEADER_FILES} ${SOURCE_FILES})

# Указываем include директорию относительно текущего модуля
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

# Для Visual Studio группируем файлы по папкам
if (CMAKE_VS_PLATFORM_NAME)
    source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/source PREFIX "Source Files" FILES ${SOURCE_FILES})
    source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/include PREFIX "Header Files" FILES ${HEADER_FILES})
endif()
