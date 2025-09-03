cmake_minimum_required(VERSION 3.20)

project(OpenIGTLinkMobile VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Core Quick Network Sensors)

qt6_standard_project_setup()

# OpenIGTLink dependency
find_package(OpenIGTLink QUIET)
if(NOT OpenIGTLink_FOUND)
    message(STATUS "OpenIGTLink not found, will use local copy")
    add_subdirectory(third_party/openigtlink)
endif()

# Source files
set(SOURCES
    src/main.cpp
    src/applicationcontroller.cpp
    src/orientationsensor.cpp
    src/igtlclient.cpp
    src/networkmanager.cpp
)

set(HEADERS
    src/applicationcontroller.h
    src/orientationsensor.h
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

qt6_add_executable(OpenIGTLinkMobile ${SOURCES} ${HEADERS})

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

target_include_directories(OpenIGTLinkMobile PRIVATE
    src/
    ${OpenIGTLink_INCLUDE_DIRS}
)

# Android specific configuration
if(ANDROID)
    set_target_properties(OpenIGTLinkMobile PROPERTIES
        QT_ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android"
    )
endif()

# iOS specific configuration
if(IOS)
    set_target_properties(OpenIGTLinkMobile PROPERTIES
        MACOSX_BUNDLE_GUI_IDENTIFIER com.openigtlink.mobile
        MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
        MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
        QT_IOS_LAUNCH_SCREEN "${CMAKE_CURRENT_SOURCE_DIR}/ios/LaunchScreen.storyboard"
    )
endif()