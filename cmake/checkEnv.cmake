###############################################################
#   @bref : 检查编译环境，确保 g++ / boost / libevent 可用
###############################################################

macro(CHECK_ENV)
    message(STATUS "========== 开始检查编译环境 ==========")

    # 1. 检查 g++ 编译器
    message(STATUS "[1/3] 检查 C++ 编译器...")
    if(NOT CMAKE_CXX_COMPILER)
        set(CMAKE_CXX_COMPILER "g++")
    endif()
    enable_language(CXX)
    if(NOT CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        message(FATAL_ERROR "未检测到 g++ 编译器，当前编译器: ${CMAKE_CXX_COMPILER} (${CMAKE_CXX_COMPILER_ID})，请安装 g++ 后重试")
    endif()
    message(STATUS "  g++ 编译器: ${CMAKE_CXX_COMPILER} (版本 ${CMAKE_CXX_COMPILER_VERSION})")

    # 2. 检查 Boost
    message(STATUS "[2/3] 检查 Boost 库...")
    find_package(Boost REQUIRED)
    if(NOT Boost_FOUND)
        message(FATAL_ERROR "未检测到 Boost 库，请安装 libboost-all-dev 后重试")
    endif()
    message(STATUS "  Boost 版本: ${Boost_VERSION}")
    message(STATUS "  Boost 头文件: ${Boost_INCLUDE_DIRS}")
    message(STATUS "  Boost 库文件: ${Boost_LIBRARY_DIRS}")
    include_directories(${Boost_INCLUDE_DIRS})
    link_directories(${Boost_LIBRARY_DIRS})

    # 3. 检查 libevent
    message(STATUS "[3/3] 检查 libevent 库...")
    find_path(LIBEVENT_INCLUDE_DIR
        NAMES event2/event.h
        PATHS /usr/include /usr/local/include /opt/local/include
    )
    find_library(LIBEVENT_LIBRARY
        NAMES event libevent
        PATHS /usr/lib /usr/local/lib /opt/local/lib
    )
    if(NOT LIBEVENT_INCLUDE_DIR OR NOT LIBEVENT_LIBRARY)
        message(FATAL_ERROR "未检测到 libevent 库，请安装 libevent-dev 后重试")
    endif()
    message(STATUS "  libevent 头文件: ${LIBEVENT_INCLUDE_DIR}")
    message(STATUS "  libevent 库文件: ${LIBEVENT_LIBRARY}")
    include_directories(${LIBEVENT_INCLUDE_DIR})

    message(STATUS "========== 编译环境检查通过 ==========")
endmacro(CHECK_ENV)
