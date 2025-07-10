#ifndef TEXT_UTILS_H
#define TEXT_UTILS_H

#include <stdint.h>

void clearBuffer(char *buf);
uint8_t formatValue(char *buf, int32_t value, uint8_t decimal);

#endif // TEXT_UTILS_H
