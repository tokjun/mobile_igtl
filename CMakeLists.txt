cmake_minimum_required(VERSION 3.20)

project(OpenIGTLinkMobile VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Core Quick Network Sensors)

qt6_standard_project_setup()

# OpenIGTLink dependency - always use local copy
message(STATUS "Using local OpenIGTLink copy")
# Set minimum policy version for third-party code compatibility
set(CMAKE_POLICY_DEFAULT_CMP0000 NEW)
set(CMAKE_POLICY_VERSION_MINIMUM 3.5)
add_subdirectory(third_party/openigtlink)

# Set variables for local build
set(OpenIGTLink_LIBRARIES OpenIGTLink)
set(OpenIGTLink_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/third_party/openigtlink/Source)
set(USING_LOCAL_OPENIGTLINK TRUE)

# Source files
set(SOURCES
    src/main.cpp
    src/applicationcontroller.cpp
    src/orientationsensor.cpp
    src/rotationsensor.cpp
    src/igtlclient.cpp
    src/networkmanager.cpp
)

set(HEADERS
    src/applicationcontroller.h
    src/orientationsensor.h
    src/rotationsensor.h
    src/igtlclient.h
    src/networkmanager.h
)

# QML files
set(QML_FILES
    qml/main.qml
    qml/MainWindow.qml
    qml/ConnectionPanel.qml
    qml/OrientationView.qml
)

# Resources
set(RESOURCES
    resources/icon.svg
)

qt6_add_executable(OpenIGTLinkMobile ${SOURCES} ${HEADERS})

# Add resources
qt6_add_resources(OpenIGTLinkMobile "app_resources"
    PREFIX "/resources"
    FILES ${RESOURCES}
)

qt6_add_qml_module(OpenIGTLinkMobile
    URI OpenIGTLinkMobile
    VERSION 1.0
    QML_FILES ${QML_FILES}
)

target_link_libraries(OpenIGTLinkMobile PRIVATE
    Qt6::Core
    Qt6::Quick
    Qt6::Network
    Qt6::Sensors
    ${OpenIGTLink_LIBRARIES}
)

# Add OpenIGTLink compile definition if using local build
message(STATUS "USING_LOCAL_OPENIGTLINK = ${USING_LOCAL_OPENIGTLINK}")
if(USING_LOCAL_OPENIGTLINK)
    message(STATUS "Adding OPENIGTLINK_FOUND definition")
    target_compile_definitions(OpenIGTLinkMobile PRIVATE OPENIGTLINK_FOUND)
endif()

target_include_directories(OpenIGTLinkMobile PRIVATE
    src/
    ${OpenIGTLink_INCLUDE_DIRS}
)

# Android specific configuration
if(ANDROID)
    set_target_properties(OpenIGTLinkMobile PROPERTIES
        QT_ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android"
        QT_ANDROID_APPLICATION_ARGUMENTS "--icon"
    )
endif()

# iOS specific configuration
if(IOS)
    set_target_properties(OpenIGTLinkMobile PROPERTIES
        MACOSX_BUNDLE_GUI_IDENTIFIER com.openigtlink.openigtlinkmobile
        MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
        MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
        QT_IOS_LAUNCH_SCREEN "${CMAKE_CURRENT_SOURCE_DIR}/ios/LaunchScreen.storyboard"
        XCODE_ATTRIBUTE_ASSETCATALOG_COMPILER_APPICON_NAME "AppIcon"
        QT_IOS_ASSET_CATALOGS "${CMAKE_CURRENT_SOURCE_DIR}/ios/Assets.xcassets"
    )
endif()

# Desktop/macOS icon configuration
if(APPLE AND NOT IOS)
    set_target_properties(OpenIGTLinkMobile PROPERTIES
        MACOSX_BUNDLE_GUI_IDENTIFIER com.openigtlink.openigtlinkmobile
        MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
        MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
        MACOSX_BUNDLE_ICON_FILE "icon.svg"
    )
endif()

# Set window icon for desktop platforms
if(NOT ANDROID AND NOT IOS)
    set_target_properties(OpenIGTLinkMobile PROPERTIES
        QT_TARGET_RC_ICONS "${CMAKE_CURRENT_SOURCE_DIR}/resources/icon.svg"
    )
endif()
