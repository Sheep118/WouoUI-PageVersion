# WouoUI 库的 CMakeLists.txt
# 这个文件在 WouoUI/WouoUI-PageVersion 目录下
# 作用：将 Csource 下的 C 源文件编译成一个静态库，供外部项目使用

# 库的名称
set(WOUOUI_LIB_NAME wououi_lib)

# Csource 目录路径
set(WOUOUI_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Csource")

# 包含所有 C 源文件（显式列出比使用通配符更好）
set(WOUOUI_SOURCES
    "${WOUOUI_SRC_DIR}/WouoUI.c"
    "${WOUOUI_SRC_DIR}/WouoUI_anim.c"
    "${WOUOUI_SRC_DIR}/WouoUI_font.c"
    "${WOUOUI_SRC_DIR}/WouoUI_graph.c"
    "${WOUOUI_SRC_DIR}/WouoUI_msg.c"
    "${WOUOUI_SRC_DIR}/WouoUI_page.c"
    "${WOUOUI_SRC_DIR}/WouoUI_user.c"
    "${WOUOUI_SRC_DIR}/WouoUI_win.c"
)

# 创建静态库
# add_library(库名 STATIC 源文件列表)
# STATIC 表示创建静态库 .a/.lib；也可以用 SHARED 创建动态库 .dll/.so
add_library(${WOUOUI_LIB_NAME} STATIC ${WOUOUI_SOURCES})

# 设置库的头文件包含路径
# 这样外部项目在 target_include_directories 时就能引入这里的路径
target_include_directories(${WOUOUI_LIB_NAME}
    PUBLIC
        "${WOUOUI_SRC_DIR}"  # 公开 include 路径，外部项目能用
)

# 可选：给库编译添加 C 语言标准（比如 C99）
set_target_properties(${WOUOUI_LIB_NAME} PROPERTIES
    C_STANDARD 99
    C_STANDARD_REQUIRED ON
)

# 可选：如果库有其他依赖（如 SDL、数学库等），可在这里链接
# target_link_libraries(${WOUOUI_LIB_NAME} PRIVATE other_lib)

# 导出库名供父项目使用（可选但推荐）
# 这会让 WOUOUI_LIB_NAME 变量在父项目中也可用
set(WOUOUI_LIB_NAME ${WOUOUI_LIB_NAME} PARENT_SCOPE)
