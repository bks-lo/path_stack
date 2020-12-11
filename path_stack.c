/**
 * @file path_stack.c
 * @author your name (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2020-09-24
 * 
 * @copyright Copyright (c) 2020
 * 
 * 
 * 合法的目录格式
 *  cd /home/////test
 *  cd ///////home/../../../../             多个/////被看做是一个
 *  当前在根目录  cd ../../../../../        还是在根目录
 *  cd .aaaaa
 *  cd ..aaaa
 *  cd ......aaa
 *  cd aaa.....aa
 *  cd ...      linux可以建立 全是...的目录
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "path_stack.h"

struct path_stack_struct
{
    char full_path[PATH_LENGTH_MAX];
    int top;
    char path_stack[PATH_DEPTH_MAX][PATH_ONE_MAX];
};

/* 是否为绝对路径 */
static int is_absolute_path(char *dir)
{
    if (dir[0] == '/')
        return 1;
    return 0;
}

/* 创建路径栈结构 */
path_stack_st *path_stack_create()
{
    return malloc(sizeof(path_stack_st));
}

/* 销毁目录栈 */
void path_stack_destroy(path_stack_st **ppstack)
{
    if (*ppstack != NULL) {
        free(*ppstack);
    }
    *ppstack = NULL;
}

/* 清空目录栈 */
void path_stack_clear(path_stack_st *pstack)
{
    memset(pstack , 0 , sizeof(path_stack_st));
}

/* 拷贝目录栈 */
void path_stack_copy(path_stack_st *dst, path_stack_st *src)
{
    memcpy(dst, src, sizeof(path_stack_st));
}

/* 当前目录深度 */
int path_stack_get_depth(path_stack_st *pstack)
{
    return pstack->top;
}

/* 将一层目录压入栈中 */
static int path_stack_push(path_stack_st *pstack, char *part)
{
    if (part == NULL || part[0] == 0) {
        return -1;
    }

    if (strcmp(part, ".") == 0 || strcmp(part, "..") == 0) {
        return 0;
    }

    if (pstack->top >= PATH_DEPTH_MAX) {
        return -1;
    }

    strncpy(pstack->path_stack[pstack->top], part, PATH_ONE_MAX - 1);
    pstack->top += 1;
    return 0;
}

/* 从目录栈中弹出一层目录 */
static int path_stack_pop(path_stack_st *pstack)
{
    /* 
    TODO 这里的需求比较丰富，每种客户端支持的不同 
     例如 cd //////   cd ../../../home  
    */

    /* 已经到栈底不再继续pop，也不报错 */
    if (pstack->top <= 1) {
        return 0;
    }

    pstack->top -= 1;
    return 0;
}

/* 将目录栈中的字符串，格式化成完整的路径 */
static int path_stack_to_string(path_stack_st *pstack)
{
    /* 目录栈的栈底不为"/",目录栈格式不合法 */
    if (strcmp(pstack->path_stack[0], "/") != 0) {
        return -1;
    }

    int i;
    int len = 1;
    snprintf(pstack->full_path, PATH_LENGTH_MAX, "/");
    
    for (i = 1; i < pstack->top; ++i) {
        snprintf(pstack->full_path + len,  PATH_LENGTH_MAX - len, "%s/",
                 pstack->path_stack[i]);
        len = strlen(pstack->full_path);
    }

    if (len > 1 && pstack->full_path[len - 1] == '/')
        pstack->full_path[len - 1] = 0;

    return 0;
}

typedef enum parser_dir_em
{
    PDIR_STAT_NULL = 0,
    PDIR_STAT_PATH,         /*路径*/
    PDIR_STAT_DOT,          /* . */
    PDIR_STAT_DOUBLE_DOT,   /* .. */
    PDIR_STAT_SLASH         /* / */
} parser_dir_em;


#define set_tmp_path(arr, idx, c) do {       \
    if (idx + 1 >= PATH_ONE_MAX) {           \
        /* path length over array */         \
        return -1;                           \
    } else {                                 \
        arr[idx] = c;                        \
        ++idx;                               \
    }                                        \
} while (0)

/* 根据dir格式化目录栈结构 */
static int path_stack_spilt(path_stack_st *pstack, char *dir)
{
    char c;
    int i = 0;
    int j = 0;
    char tmp_path[PATH_ONE_MAX] = {0};
    parser_dir_em state = PDIR_STAT_SLASH;

    while ((c = dir[i]) != 0) {
        if (c == '/') {
            switch (state) {
            case PDIR_STAT_DOT:
            case PDIR_STAT_SLASH:   /* 用于合并目录中连续的 "////" 这种情况*/
                state = PDIR_STAT_SLASH;
                break;
            case PDIR_STAT_DOUBLE_DOT:
                path_stack_pop(pstack);
                state = PDIR_STAT_SLASH;
                break;
            case PDIR_STAT_PATH:
                path_stack_push(pstack, tmp_path);
                state = PDIR_STAT_SLASH;
                break;
            default:
                // invalid state
                return -1; 
                break;
            }
            
            memset(tmp_path, 0, sizeof(tmp_path));
            j = 0;
        } else if (c == '.') {
            switch (state) {
            case PDIR_STAT_SLASH:
                state = PDIR_STAT_DOT;
                break;
            case PDIR_STAT_DOT:
                state = PDIR_STAT_DOUBLE_DOT;
                break;
            case PDIR_STAT_DOUBLE_DOT:      /*目录名称为 ...*/
                state = PDIR_STAT_PATH;
                break;
            case PDIR_STAT_PATH:
                break;
            default:
                // invalid state
                return -1; 
                break;
            }
            set_tmp_path(tmp_path, j, c);
        } else {
            switch (state) {
            case PDIR_STAT_SLASH:
            case PDIR_STAT_DOT:
            case PDIR_STAT_DOUBLE_DOT:
                state = PDIR_STAT_PATH;
                break;
            case PDIR_STAT_PATH:
                break;
            default:
                // invalid state
                return -1; 
                break;
            }
            set_tmp_path(tmp_path, j, c);
        }
        
        ++i;
    }

    if (strlen(tmp_path) > 0) {
        path_stack_push(pstack, tmp_path);
    }

    path_stack_to_string(pstack);
    return 0;
}


/* 将路径字符串，格式化到目录栈中 */
static int path_stack_to_stack(path_stack_st *pstack, char *dir)
{
    
    /* 符合完整完成路径的规则，必须以 / 开头  */
    if (dir[0] != '/') {
        return -1;
    }

    /* cd ///////////../home 是合法的, 由多个/开头统一认为是一个/ */
    int i = 1;
    for (; dir[i] != 0; ++i) {
        if (dir[i] != '/')
            break;
    }

    /* 将起始的 "/" 填充到目录栈中，后面的目录使用spilt继续分析 */
    path_stack_clear(pstack);
    path_stack_push(pstack, "/");
    path_stack_spilt(pstack, &(dir[i]));
    return 0;
}

/* 将dir设置到目录栈中 */
int path_stack_set_dir(path_stack_st *pstack, char *dir)
{
    if (is_absolute_path(dir)) {
        path_stack_to_stack(pstack, dir);
    } else {
        path_stack_spilt(pstack, dir);
    }
    return 0;
}

/* 获取完整路径 */
char *path_stack_get_full_path(path_stack_st *pstack)
{
    return pstack->full_path;
}


/* 获取目录栈中 栈顶的目录名称，不改变栈结构 */
char *path_stack_get_top(path_stack_st *pstack) 
{
    return pstack->path_stack[pstack->top];
}


#ifdef PSTACK_UNIT_TEST

#define ASSERT_TRUE(pstack, orig_path, exepct_path) do {                \
    path_stack_set_dir(pstack, orig_path);                          \
    char *tmp = path_stack_get_full_path(pstack);                   \
    printf("%s:%d>>func out: %s\n", __FILE__, __LINE__, tmp);      \
    if (strcmp(tmp, exepct_path)) {                       \
        printf("%s:%d>>ERROR: %s != %s\n", __FILE__, __LINE__, orig_path, exepct_path);      \
        exit(0);                                \
    } else {                                    \
        printf("%s:%d>>SUCCESS: %s == %s\n", __FILE__, __LINE__, orig_path, exepct_path);    \
    }                                           \
    printf("\n");                               \
} while (0)

int main() 
{
    /*  cd /home/////test
    *  cd ///////home/../../../../             多个/////被看做是一个
    *  当前在根目录  cd ../../../../../        还是在根目录
    *  cd .aaaaa
    *  cd ..aaaa
    *  cd ......aaa
    *  cd aaa.....aa
    *  cd ...      linux可以建立 全是...的目录
    */
    path_stack_st a = {0};
    path_stack_st b = {0};
    
    ASSERT_TRUE(&a, "/home/////test", "/home/test");

    path_stack_clear(&a);
    ASSERT_TRUE(&a, "///////home/../../../../", "/");

    path_stack_clear(&a);
    ASSERT_TRUE(&a, "/../../../../../", "/");

    path_stack_clear(&a);
    ASSERT_TRUE(&a, "/.aaaaa", "/.aaaaa");

    path_stack_clear(&a);
    ASSERT_TRUE(&a, "/..aaaa", "/..aaaa");

    path_stack_clear(&a);
    ASSERT_TRUE(&a, "//......aaa", "/......aaa");

    path_stack_clear(&a);
    ASSERT_TRUE(&a, "//aaa.....aa", "/aaa.....aa");

    path_stack_clear(&a);
    ASSERT_TRUE(&a, "//...aaa.....aa", "/...aaa.....aa");

    path_stack_clear(&a);
    ASSERT_TRUE(&a, "/...", "/...");

    path_stack_clear(&a);
    path_stack_set_dir(&a, "/home/a/b/c/d");
    ASSERT_TRUE(&a, "../../../../../../", "/");

    path_stack_clear(&a);
    path_stack_set_dir(&a, "/home/a/b/c/d");
    ASSERT_TRUE(&a, "../../../e/f", "/home/a/e/f");

    path_stack_clear(&a);
    path_stack_set_dir(&a, "/home/a/b/c/d");
    ASSERT_TRUE(&a, "../../././../e/f", "/home/a/e/f");
}

#endif