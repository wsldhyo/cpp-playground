# 纯头文件，使用接口库导出符号
add_library(unique_ptr_intf INTERFACE)

# 设置该接口库的公共包含路径
target_include_directories(unique_ptr_intf INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}
)


# 可选：添加编译警告选项（针对使用该库的目标）
# target_compile_options(unique_ptr INTERFACE -Wall -Wextra)