cmake_minimum_required(VERSION 3.31)
project(DirectoryTreeViewer)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt5 COMPONENTS Widgets REQUIRED)

add_executable(DirectoryTreeViewer WIN32
    main.cpp 
    MainWindow.cpp 
    MainWindow.h 
    DirectoryTree.cpp 
    DirectoryTree.h 
    OptionsDialog.cpp 
    OptionsDialog.h
    resources.qrc
)

target_link_libraries(DirectoryTreeViewer PRIVATE Qt5::Widgets)

configure_file(favicon.ico ${CMAKE_BINARY_DIR}/favicon.ico COPYONLY)
