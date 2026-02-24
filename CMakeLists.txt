cmake_minimum_required(VERSION 3.16)
project(FastAV VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Qt6 packages
find_package(Qt6 REQUIRED COMPONENTS Core Widgets Sql Network Concurrent)

# SQLite3
find_package(SQLite3 REQUIRED)

# Source files
set(SOURCES
    src/main.cpp
    src/core/Scanner.cpp
    src/core/Database.cpp
    src/core/Updater.cpp
    src/core/ThreatReport.cpp
    src/gui/MainWindow.cpp
    src/gui/ScanDialog.cpp
    src/gui/ScanProgress.cpp
    src/gui/ThreatViewer.cpp
    src/gui/HistoryViewer.cpp
    src/utils/MaterialTheme.cpp
    src/utils/FileScanner.cpp
)

set(HEADERS
    src/core/Scanner.h
    src/core/Database.h
    src/core/Updater.h
    src/core/ThreatReport.h
    src/gui/MainWindow.h
    src/gui/ScanDialog.h
    src/gui/ScanProgress.h
    src/gui/ThreatViewer.h
    src/gui/HistoryViewer.h
    src/utils/MaterialTheme.h
    src/utils/FileScanner.h
)

add_executable(fastav ${SOURCES} ${HEADERS})

target_link_libraries(fastav
    Qt6::Core
    Qt6::Widgets
    Qt6::Sql
    Qt6::Network
    Qt6::Concurrent
    SQLite::SQLite3
    pthread
)

# Compiler optimizations
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    target_compile_options(fastav PRIVATE -O3 -march=native -flto)
    target_link_options(fastav PRIVATE -flto)
endif()

# Install rules
install(TARGETS fastav DESTINATION bin)
