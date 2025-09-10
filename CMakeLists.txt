cmake_minimum_required(VERSION 3.10)
project(GymManagement)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include directories
include_directories(include)
include_directories(include/core)

# Find SQLite3 (required for both executables)
if(WIN32)
    # Windows: You might need to specify SQLite3 path manually
    find_package(PkgConfig QUIET)
    if(PkgConfig_FOUND)
        pkg_check_modules(SQLITE3 sqlite3)
    endif()
    if(NOT SQLITE3_FOUND)
        # Fallback for Windows - assuming sqlite3 is in system path
        set(SQLITE3_LIBRARIES sqlite3)
        message(STATUS "Using fallback SQLite3 configuration for Windows")
    endif()
else()
    # Linux/macOS
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(SQLITE3 REQUIRED sqlite3)
    message(STATUS "SQLite3 found: ${SQLITE3_VERSION}")
endif()

# Core backend source files (no GUI dependencies)
set(CORE_SOURCES
    src/DatabaseManager.cpp
    src/Customer.cpp
    src/Transaction.cpp
    src/Reports.cpp
    src/Inventory.cpp
)

# GUI-specific source files
set(GUI_CORE_SOURCES
    src/core/GymApp.cpp
    src/core/MainFrame.cpp
    src/core/DashboardPanel.cpp
    src/core/CustomerPanel.cpp
    src/core/TransactionPanel.cpp
    src/core/QuickCheckInDialog.cpp
)

# GUI application sources
set(GUI_SOURCES
    ${CORE_SOURCES}
    ${GUI_CORE_SOURCES}
    src/gui_main.cpp
)

# Console application sources
set(CONSOLE_SOURCES
    ${CORE_SOURCES}
    src/main.cpp
)

# =============================================================================
# CONSOLE EXECUTABLE
# =============================================================================
add_executable(GymManagement_console ${CONSOLE_SOURCES})

# Link SQLite3 to console version
target_link_libraries(GymManagement_console ${SQLITE3_LIBRARIES})

# Add SQLite3 configuration to console version
if(SQLITE3_INCLUDE_DIRS)
    target_include_directories(GymManagement_console PRIVATE ${SQLITE3_INCLUDE_DIRS})
endif()

if(SQLITE3_CFLAGS_OTHER)
    target_compile_options(GymManagement_console PRIVATE ${SQLITE3_CFLAGS_OTHER})
endif()

# Console-specific compile options
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    target_compile_options(GymManagement_console PRIVATE -Wall -Wextra -Wno-unused-parameter)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    target_compile_options(GymManagement_console PRIVATE /W4)
    # Ensure console window for console version on Windows
    set_property(TARGET GymManagement_console PROPERTY WIN32_EXECUTABLE FALSE)
endif()

# =============================================================================
# GUI EXECUTABLE (Optional - only if wxWidgets is found)
# =============================================================================
find_package(wxWidgets REQUIRED COMPONENTS core base adv html xml net)
if(wxWidgets_FOUND)
    include(${wxWidgets_USE_FILE})
    message(STATUS "wxWidgets found: ${wxWidgets_VERSION}")
    message(STATUS "wxWidgets libraries: ${wxWidgets_LIBRARIES}")
    message(STATUS "wxWidgets include dirs: ${wxWidgets_INCLUDE_DIRS}")
    message(STATUS "wxWidgets definitions: ${wxWidgets_DEFINITIONS}")
    
# Create GUI executable
add_executable(GymManagement_gui ${GUI_SOURCES})

# macOS-specific frameworks
if(APPLE)
    find_library(COCOA_LIBRARY Cocoa)
    find_library(WEBKIT_LIBRARY WebKit)
    target_link_libraries(GymManagement_gui ${COCOA_LIBRARY} ${WEBKIT_LIBRARY})
endif()

# Link wxWidgets libraries to GUI version
    target_link_libraries(GymManagement_gui ${wxWidgets_LIBRARIES})
    target_include_directories(GymManagement_gui PRIVATE ${wxWidgets_INCLUDE_DIRS})
    target_compile_definitions(GymManagement_gui PRIVATE ${wxWidgets_DEFINITIONS})
    target_compile_options(GymManagement_gui PRIVATE ${wxWidgets_CXX_FLAGS})
    
    # Link SQLite3 to GUI version
    target_link_libraries(GymManagement_gui ${SQLITE3_LIBRARIES})
    
    # Add SQLite3 configuration to GUI version
    if(SQLITE3_INCLUDE_DIRS)
        target_include_directories(GymManagement_gui PRIVATE ${SQLITE3_INCLUDE_DIRS})
    endif()
    
    if(SQLITE3_CFLAGS_OTHER)
        target_compile_options(GymManagement_gui PRIVATE ${SQLITE3_CFLAGS_OTHER})
    endif()
    
    # GUI-specific compile options
    if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
        target_compile_options(GymManagement_gui PRIVATE -Wall -Wextra -Wno-unused-parameter)
    elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
        target_compile_options(GymManagement_gui PRIVATE /W4)
        # Disable console window for GUI application on Windows
        set_property(TARGET GymManagement_gui PROPERTY WIN32_EXECUTABLE TRUE)
    endif()
    
else()
    message(WARNING "wxWidgets not found! GUI version will not be built.")
    message(STATUS "Only console version will be available.")
    message(STATUS "To install wxWidgets:")
    message(STATUS "  Ubuntu/Debian: sudo apt-get install libwxgtk3.0-gtk3-dev")
    message(STATUS "  macOS: brew install wxwidgets")
    message(STATUS "  CentOS/RHEL: sudo yum install wxGTK3-devel")
endif()

# =============================================================================
# CONVENIENCE TARGETS
# =============================================================================

# Create a convenience target for console version
add_custom_target(console
    COMMAND ${CMAKE_COMMAND} -E echo "Running console version..."
    COMMAND $<TARGET_FILE:GymManagement_console>
    DEPENDS GymManagement_console
    COMMENT "Launch console version"
)

# Create a convenience target for GUI version
if(wxWidgets_FOUND)
    add_custom_target(gui
        COMMAND ${CMAKE_COMMAND} -E echo "Running GUI version..."
        COMMAND $<TARGET_FILE:GymManagement_gui>
        DEPENDS GymManagement_gui
        COMMENT "Launch GUI version"
    )
    
    # Default target builds both
    add_custom_target(both ALL
        DEPENDS GymManagement_console GymManagement_gui
        COMMENT "Build both console and GUI versions"
    )
endif()

# =============================================================================
# INSTALLATION
# =============================================================================

# Install console version
#install(TARGETS GymManagement_console 
        #DESTINATION bin
        #RENAME gymmanagement-console)

# Install GUI version if built
if(wxWidgets_FOUND)
    install(TARGETS GymManagement_gui 
            DESTINATION bin
            RENAME gymmanagement-gui)
endif()

# Install data directory
install(DIRECTORY ${CMAKE_SOURCE_DIR}/data/ 
        DESTINATION share/gymmanagement/data 
        FILES_MATCHING PATTERN "*" PATTERN ".git" EXCLUDE)

# Create shell scripts for easy execution (Unix-like systems)
if(UNIX)
    # Console launcher script
    configure_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/scripts/gymmanagement-console.sh.in"
        "${CMAKE_CURRENT_BINARY_DIR}/gymmanagement-console"
        @ONLY
    )
    
    if(wxWidgets_FOUND)
        # GUI launcher script
        configure_file(
            "${CMAKE_CURRENT_SOURCE_DIR}/scripts/gymmanagement-gui.sh.in"
            "${CMAKE_CURRENT_BINARY_DIR}/gymmanagement-gui"
            @ONLY
        )
        
        install(PROGRAMS "${CMAKE_CURRENT_BINARY_DIR}/gymmanagement-gui"
                DESTINATION bin)
    endif()
    
    install(PROGRAMS "${CMAKE_CURRENT_BINARY_DIR}/gymmanagement-console"
            DESTINATION bin)
endif()

# =============================================================================
# CREATE DATA DIRECTORY
# =============================================================================
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/data)

# =============================================================================
# CONFIGURATION SUMMARY
# =============================================================================
message(STATUS "")
message(STATUS "=== GYM MANAGEMENT SYSTEM BUILD CONFIGURATION ===")
message(STATUS "Project: ${PROJECT_NAME}")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "")
message(STATUS "Available executables:")
message(STATUS "  Console version: GymManagement_console")
if(wxWidgets_FOUND)
    message(STATUS "  GUI version: GymManagement_gui")
    message(STATUS "  wxWidgets version: ${wxWidgets_VERSION}")
else()
    message(STATUS "  GUI version: Not available (wxWidgets not found)")
endif()
message(STATUS "")
message(STATUS "SQLite3 libraries: ${SQLITE3_LIBRARIES}")
message(STATUS "")
message(STATUS "Build commands:")
message(STATUS "  All: make")
message(STATUS "  Console only: make GymManagement_console")
if(wxWidgets_FOUND)
    message(STATUS "  GUI only: make GymManagement_gui")
endif()
message(STATUS "")
message(STATUS "Run commands:")
message(STATUS "  Console: ./GymManagement_console")
if(wxWidgets_FOUND)
    message(STATUS "  GUI: ./GymManagement_gui")
    message(STATUS "  Or use: make console (for console) / make gui (for GUI)")
endif()
message(STATUS "===============================================")
message(STATUS "")