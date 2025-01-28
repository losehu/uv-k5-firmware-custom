#include <stdlib.h>
#include <memory.h>
#include <stdbool.h>

char substr(const char *str, int start, int len, char *buf) {


    // 复制子串到缓冲区
    memcpy(buf, str + start, len);
    buf[len] = '\0';  // 添加字符串结束符

}

int strl(const char *str, long *value) {
    char *end_ptr = NULL;
    long result = strtol(str, &end_ptr, 10);
    if (result == 0 && str == end_ptr) {
        return 0;
    }

    *value = result;
    return 1;
}


int strd(const char *str, double *value) {
    if (!str || !value) {
        return 0; // 输入无效
    }

    double result = 0.0;
    bool is_negative = false;
    bool is_fraction = false;
    double fraction_divisor = 1.0;

    // 跳过前导空格
    while (*str == ' ') {
        str++;
    }

    // 处理正负号
    if (*str == '-') {
        is_negative = true;
        str++;
    } else if (*str == '+') {
        str++;
    }

    // 检查第一个字符是否是数字或小数点
    if ((*str < '0' || *str > '9') && *str != '.') {
        return 0; // 非法输入
    }

    // 处理数字部分
    while ((*str >= '0' && *str <= '9') || *str == '.') {
        if (*str == '.') {
            if (is_fraction) {
                return 0; // 多个小数点，非法输入
            }
            is_fraction = true;
        } else {
            if (is_fraction) {
                fraction_divisor *= 10.0;
                result += (*str - '0') / fraction_divisor;
            } else {
                result = result * 10.0 + (*str - '0');
            }
        }
        str++;
    }

    // 处理负号
    if (is_negative) {
        result = -result;
    }

    // 检查是否还有多余字符
    if (*str != '\0') {
        return 0; // 非法输入
    }

    *value = result;
    return 1; // 成功解析
}
