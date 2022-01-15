##################### SOURCE_DIRS ##############################
##
#   @bref : 添加可变参数列表的源文件
###############################################################
function(SOURCE_DIRS)
    if(${ARGC} LESS 1)
        return()
    else()
        foreach(each_source_dir ${ARGN})
            if(${each_source_dir} STREQUAL ".")
                #已添加当前路径
            elseif(IS_ABSOLUTE ${each_source_dir})#绝对路径
                list(APPEND _source_dirs ${each_source_dir})
            else()#相对路径
                list(APPEND _source_dirs ${CMAKE_CURRENT_SOURCE_DIR}/${each_source_dir})
            endif()
        endforeach()
        set(SOURCE_DIRS ${SOURCE_DIRS} ${_source_dirs} PARENT_SCOPE)
    endif()
endfunction(SOURCE_DIRS)
##############################################################
#   ARGC:函数或者宏传递的参数个数
#   ARGV:ARGC代表所有传递的参数，使用list表示，可以用ARGV0、ARGV1...来取得
#   ARGN:包含传入参数的list，与ARGV不同的是并不是代表所有参数，而是指宏或者函数声明的参数之后的所有参数
#   
#   STREQUAL：用于if括号内的字符串判等
#
##############################################################