###############################################################
#   @bref : 检查编译环境，确保 g++ / boost / libevent / mysql / openssl 可用
###############################################################

macro(CHECK_ENV)
    message(STATUS "========== 开始检查编译环境 ==========")

    # 1. 检查 g++ 编译器
    message(STATUS "[1/5] 检查 C++ 编译器...")
    if(NOT CMAKE_CXX_COMPILER)
        set(CMAKE_CXX_COMPILER "g++")
    endif()
    enable_language(CXX)
    if(NOT CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        message(FATAL_ERROR "未检测到 g++ 编译器，当前编译器: ${CMAKE_CXX_COMPILER} (${CMAKE_CXX_COMPILER_ID})，请安装 g++ 后重试")
    endif()
    message(STATUS "  g++ 编译器: ${CMAKE_CXX_COMPILER} (版本 ${CMAKE_CXX_COMPILER_VERSION})")

    # 2. 检查 Boost
    message(STATUS "[2/5] 检查 Boost 库...")
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
    message(STATUS "[3/5] 检查 libevent 库...")
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

    # 4. 检查 MySQL
    message(STATUS "[4/5] 检查 MySQL 库...")
    find_path(MYSQL_INCLUDE_DIR
        NAMES mysql/mysql.h
        PATHS /usr/include /usr/local/include /opt/local/include
    )
    find_library(MYSQL_LIBRARY
        NAMES mysqlclient mysqlclient_r
        PATHS /usr/lib64/mysql /usr/lib64 /usr/lib /usr/local/lib /opt/local/lib
    )
    if(NOT MYSQL_INCLUDE_DIR OR NOT MYSQL_LIBRARY)
        message(FATAL_ERROR "未检测到 MySQL 库，请安装 mysql-devel 后重试")
    endif()
    message(STATUS "  MySQL 头文件: ${MYSQL_INCLUDE_DIR}")
    message(STATUS "  MySQL 库文件: ${MYSQL_LIBRARY}")
    include_directories(${MYSQL_INCLUDE_DIR})

    # 5. 检查 OpenSSL
    message(STATUS "[5/5] 检查 OpenSSL 库...")
    find_package(OpenSSL REQUIRED)
    if(NOT OPENSSL_FOUND)
        message(FATAL_ERROR "未检测到 OpenSSL 库，请安装 openssl-devel 后重试")
    endif()
    message(STATUS "  OpenSSL 头文件: ${OPENSSL_INCLUDE_DIR}")
    message(STATUS "  OpenSSL 库文件: ${OPENSSL_LIBRARIES}")

    message(STATUS "========== 编译环境检查通过 ==========")
endmacro(CHECK_ENV)
