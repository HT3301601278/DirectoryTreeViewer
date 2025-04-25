cmake_minimum_required(VERSION 3.31)
project(DirectoryTreeViewer)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt5 COMPONENTS Widgets PrintSupport REQUIRED)

add_executable(DirectoryTreeViewer 
    main.cpp 
    MainWindow.cpp 
    MainWindow.h 
    DirectoryTree.cpp 
    DirectoryTree.h 
    OptionsDialog.cpp 
    OptionsDialog.h
    TreeNode.cpp
    TreeNode.h
    TreeModel.cpp
    TreeModel.h
    DirectoryScanner.cpp
    DirectoryScanner.h
    ExportDialog.cpp
    ExportDialog.h
)

target_link_libraries(DirectoryTreeViewer PRIVATE Qt5::Widgets Qt5::PrintSupport)
