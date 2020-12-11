/**
 * @file path_stack.h
 * @author your name (you@domain.com)
 * @brief  将路径格式化到栈中，用于切换目录时的判断
 * @version 0.1
 * @date 2020-09-23
 * 
 * @copyright Copyright (c) 2020
 * 
 */
#ifndef _PATH_STACK_H_
#define _PATH_STACK_H_

typedef struct path_stack_struct path_stack_st;

#define PATH_LENGTH_MAX     1024    /*完整路径最大长度*/
#define PATH_DEPTH_MAX      128     /*路径最多128层*/
#define PATH_ONE_MAX        256     /*单个目录的最大长度*/

/* 创建路径栈结构 */
path_stack_st *path_stack_create();

/* 获取目录栈中 栈顶的目录名称，不改变栈结构 */
char *path_stack_get_top(path_stack_st *pstack);

/* 获取完整路径 */
char *path_stack_get_full_path(path_stack_st *pstack);

/* 将dir设置到目录栈中 */
int path_stack_set_dir(path_stack_st *pstack, char *dir);

/* 当前目录深度 */
int path_stack_get_depth(path_stack_st *pstack);

/* 拷贝目录栈 */
void path_stack_copy(path_stack_st *dst, path_stack_st *src);

/* 清空目录栈 */
void path_stack_clear(path_stack_st *pstack);

/* 销毁目录栈 */
void path_stack_destroy(path_stack_st **ppstack);

#endif