#include "text_utils.h"
#include <cstring>
#include <cstdio>

void clearBuffer(char *buf)
{
  for (uint8_t i = 0; i < strlen(buf); i++)
  {
    buf[i] = '\0';
  }
}

uint8_t formatValue(char *buf, int32_t value, uint8_t decimal)
{
  clearBuffer(buf);
  snprintf(buf, 22, "%d", value);
  uint8_t len = strlen(buf);

  if (decimal != 0)
  {
    uint8_t target = decimal + 1;
    uint8_t numLen = len - ((value < 0) ? 1 : 0);
    while (numLen < target)
    {
      for (uint8_t i = 0; i < numLen + 1; i++)
      {
        buf[len - i + 1] = buf[len - i];
      }
      buf[((value < 0) ? 1 : 0)] = '0';
      len++;
      numLen++;
    }
    for (uint8_t i = 0; i < decimal; i++)
    {
      buf[len - i + 1] = buf[len - i];
    }
    buf[len - decimal + 1] = '.';
    buf[len + 2] = '\0';
    len++;
  }
  return len;
}
