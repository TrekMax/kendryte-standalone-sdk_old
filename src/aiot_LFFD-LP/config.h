#ifndef _CONFIG_H_
#define _CONFIG_H_

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#define CONFIG_LOG_COLORS 1

/* clang-format off */
#if CONFIG_LOG_COLORS
#define LOG_COLOR_BLACK   "30"
#define LOG_COLOR_RED     "31"
#define LOG_COLOR_GREEN   "32"
#define LOG_COLOR_BROWN   "33"
#define LOG_COLOR_BLUE    "34"
#define LOG_COLOR_PURPLE  "35"
#define LOG_COLOR_CYAN    "36"
#define LOG_COLOR(COLOR)  "\033[0;" COLOR "m"
#define LOG_BOLD(COLOR)   "\033[1;" COLOR "m"
#define LOG_RESET_COLOR   "\033[0m"
#define LOG_COLOR_E       LOG_COLOR(LOG_COLOR_RED)
#define LOG_COLOR_W       LOG_COLOR(LOG_COLOR_BROWN)
#define LOG_COLOR_I       LOG_COLOR(LOG_COLOR_GREEN)
#define LOG_COLOR_D       LOG_COLOR(LOG_RESET_COLOR)
#define LOG_COLOR_V
#else /* CONFIG_LOG_COLORS */
#define LOG_COLOR_E
#define LOG_COLOR_W
#define LOG_COLOR_I
#define LOG_COLOR_D
#define LOG_COLOR_V
#define LOG_RESET_COLOR
#endif /* CONFIG_LOG_COLORS */

#define KPU_DEBUG 0

#define PRINTF_KPU_OUTPUT(output, size)                          \
    {                                                            \
        printf("%s addr is %ld\r\n", #output, (uint64_t)(output)); \
        printf("[");                                             \
        for (size_t i=0; i < (size); i++) {                      \
            if (i%5 == 0) {                                      \
                printf("\r\n");                                    \
            }                                                    \
            printf("%f, ", *((output)+i));                       \
        }                                                        \
        printf("]\r\n");                                           \                                           
    }

/*********************************************************************************/
#define LOG_ERROR_FLAGE       (1 << 1)
#define LOG_WRAN_FLAGE        (1 << 2)
#define LOG_INFO_FLAGE        (1 << 3)
#define LOG_DEBUG_FLAGE       (1 << 4)

#define LOG_FLAGE             (LOG_ERROR_FLAGE|LOG_WRAN_FLAGE|LOG_INFO_FLAGE|LOG_DEBUG_FLAGE)
// #define LOG_FLAGE              0

#define LOG_PRINT(class, fmt, ...)\
    printf("%s" fmt "[line:%d]\r\n", class, ##__VA_ARGS__, __LINE__);
    // printf("%s" fmt "[line:%d] [%s]\r\n", class, ##__VA_ARGS__, __LINE__, __FILE__);


#if (LOG_ERROR_FLAGE & LOG_FLAGE)
#define APP_LOGE(fmt, ...) LOG_PRINT(LOG_COLOR_E "[ERROR]", fmt, ##__VA_ARGS__)
#else
#define APP_LOGE(fmt, ...) 
#endif // LOG_ERROR_FLAGE

#if (LOG_WRAN_FLAGE & LOG_FLAGE)
#define APP_LOGW(fmt, ...) LOG_PRINT(LOG_COLOR_W "[WRAR] ", fmt, ##__VA_ARGS__)
#else
#define APP_LOGW(fmt, ...)
#endif // LOG_WRAN_FLAGE

#if (LOG_INFO_FLAGE & LOG_FLAGE)
#define APP_LOGI(fmt, ...) LOG_PRINT(LOG_COLOR_I "[INFO] ", fmt, ##__VA_ARGS__)
#else
#define APP_LOGI(fmt, ...)
#endif // LOG_INFO_FLAGE

#if (LOG_DEBUG_FLAGE & LOG_FLAGE)
#define APP_LOGD(fmt, ...) LOG_PRINT(LOG_COLOR_D "[DEBUG]", fmt, ##__VA_ARGS__)
#else
#define APP_LOGD(fmt, ...)
#endif // LOG_DEBUG_FLAGE
/**********************************************************************************/

#endif //_CONFIG_H_