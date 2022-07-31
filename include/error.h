#ifndef ERROR_H
#define ERROR_H

#include <Arduino.h>
#include "config.h"

enum err_t {
  OK_CODE = 0,
  ERR_ANY_CODE = 1,
  ERR_CMD_NOT_FOUND_CODE = 2,
  ERR_CMD_INVALID_CODE = 3,
  ERR_CMD_ACTION_INVALID_CODE = 4,
  ERR_CMD_PARAM_INVALID_CODE = 5,
  ERR_CMD_PARAM_OUT_OF_RANGE_CODE = 6,
  ERR_ACTION_FAILED_CODE = 7,
};

void throwError(err_t err);
void sendAck();

#endif // ERROR_H