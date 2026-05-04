cmake_minimum_required(VERSION 2.8)
project(dune)

# 引入编译环境检查脚本
include(${PROJECT_SOURCE_DIR}/../cmake/checkEnv.cmake)
CHECK_ENV()

# 项目源文件
set(SRC
    main.cpp
    config/config.cpp
    log/logger.cpp
    net/event_loop.cpp
    net/http_server.cpp
    router/router.cpp
    middleware/middleware.cpp
    middleware/builtin_mw.cpp
    dao/mysql_pool.cpp
    dao/database.cpp
    auth/jwt.cpp
    auth/password.cpp
    auth/auth_mw.cpp
)

# 添加头文件搜索目录
include_directories(${PROJECT_SOURCE_DIR})

add_executable(dune ${SRC})

# 链接依赖库
target_link_libraries(dune
    ${Boost_LIBRARIES}
    ${LIBEVENT_LIBRARY}
    ${MYSQL_LIBRARY}
    ${OPENSSL_LIBRARIES}
    pthread
)
