#include "error.h"

String errorCode(err_t err){
    //get number from err_t enum and convert to string
    String errorCode = String(err);
    return errorCode;
}

void sendError(err_t err)
{
    COMMUNICATION_SERIAL_PORT.println("ERR " + errorCode(err));
}

void throwError(err_t err)
{
    sendError(err);
}
