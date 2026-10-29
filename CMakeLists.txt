cmake_minimum_required(VERSION 3.24)

project(MD_AI_style VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

include(CTest)
include(FetchContent)
include(GNUInstallDirs)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Gui Widgets WebEngineCore WebEngineWidgets PrintSupport)

set(BUILD_SHARED_LIBS OFF CACHE BOOL "Build dependencies statically" FORCE)
set(BUILD_MD2HTML_EXECUTABLE OFF CACHE BOOL "Do not build md4c command line tool" FORCE)
FetchContent_Declare(
    md4c
    GIT_REPOSITORY https://github.com/mity/md4c.git
    GIT_TAG 729e6b8b320caa96328968ab27d7db2235e4fb47
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(md4c)

set(APP_SOURCES
    src/main.cpp
    src/MainWindow.cpp
    src/MainWindow.h
    src/MarkdownRenderer.cpp
    src/MarkdownRenderer.h
    src/OfflineRequestInterceptor.cpp
    src/OfflineRequestInterceptor.h
    src/SafeWebPage.cpp
    src/SafeWebPage.h
    resources/resources.qrc
)

if(WIN32)
    list(APPEND APP_SOURCES packaging/windows.rc)
endif()

qt_add_executable(MD_AI-style WIN32 MACOSX_BUNDLE ${APP_SOURCES})

target_link_libraries(MD_AI-style PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::WebEngineCore
    Qt6::WebEngineWidgets
    Qt6::PrintSupport
    md4c-html
)

target_compile_definitions(MD_AI-style PRIVATE
    MD_AI_STYLE_VERSION="${PROJECT_VERSION}"
    QT_NO_CAST_FROM_ASCII
    QT_NO_CAST_TO_ASCII
)

if(MSVC)
    target_compile_options(MD_AI-style PRIVATE /W4 /permissive- /utf-8)
else()
    target_compile_options(MD_AI-style PRIVATE -Wall -Wextra -Wpedantic)
endif()

set_target_properties(MD_AI-style PROPERTIES
    OUTPUT_NAME "MD_AI-style"
    MACOSX_BUNDLE_BUNDLE_NAME "MD_AI-style"
    MACOSX_BUNDLE_GUI_IDENTIFIER "io.github.dana4kin.md-ai-style"
    MACOSX_BUNDLE_BUNDLE_VERSION "${PROJECT_VERSION}"
    MACOSX_BUNDLE_SHORT_VERSION_STRING "${PROJECT_VERSION}"
    MACOSX_BUNDLE_ICON_FILE "MD_AI-style.icns"
)

if(APPLE)
    set(APP_ICON_MACOS "${CMAKE_CURRENT_SOURCE_DIR}/resources/MD_AI-style.icns")
    set_source_files_properties(${APP_ICON_MACOS} PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")
    target_sources(MD_AI-style PRIVATE ${APP_ICON_MACOS})
endif()

install(TARGETS MD_AI-style
    BUNDLE DESTINATION .
    RUNTIME DESTINATION bin
)
install(FILES LICENSE THIRD_PARTY_NOTICES.md DESTINATION .)

qt_generate_deploy_app_script(
    TARGET MD_AI-style
    OUTPUT_SCRIPT deployScript
    NO_UNSUPPORTED_PLATFORM_ERROR
)
install(SCRIPT ${deployScript})

if(BUILD_TESTING)
    find_package(Qt6 6.5 REQUIRED COMPONENTS Test)
    qt_add_executable(MarkdownRendererTests
        tests/MarkdownRendererTests.cpp
        src/MarkdownRenderer.cpp
        src/MarkdownRenderer.h
        resources/resources.qrc
    )
    target_link_libraries(MarkdownRendererTests PRIVATE Qt6::Core Qt6::Test md4c-html)
    target_compile_definitions(MarkdownRendererTests PRIVATE QT_NO_CAST_FROM_ASCII QT_NO_CAST_TO_ASCII)
    add_test(NAME MarkdownRendererTests COMMAND MarkdownRendererTests)
endif()
