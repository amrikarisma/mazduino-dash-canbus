#ifndef TEXT_UTILS_H
#define TEXT_UTILS_H

void clearBuffer(char *buf)
{
  for (uint8_t i = 0; i < strlen(buf); i++)
  {
    buf[i] = '\0';
  }
}

uint8_t formatValue(char *buf, int32_t value, uint8_t decimal)
{
  // static char temp[STRING_LENGTH];
  // clearBuffer(temp);

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
      // if negative, skip negative sign
      {
        buf[len - i + 1] = buf[len - i];
        buf[len - i] = '0';
      }
      buf[len + 1] = '\0';
      numLen++;
      len++;
    }
    // insert
    for (uint8_t i = 0; i < decimal + 1; i++)
    {
      buf[len - i + 1] = buf[len - i];
      buf[len - i] = '.';
    }
    // clearBuffer(buf);
    // snprintf(buf, STRING_LENGTH, "%d", target);
  }
  return strlen(buf);
}

#endif