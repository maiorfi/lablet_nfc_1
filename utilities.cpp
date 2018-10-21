#include "mbed.h"

#include <string>

std::string getByteArrayHexString(uint8_t *content, size_t length)
{
    std::string retval = "";

    char buffer[4];
    buffer[3] = '\0';

    for (size_t i = 0; i < length; i++)
    {
        sprintf(buffer, "%02X-", content[i]);
        retval.append(buffer);
    }

    retval.pop_back();

    return retval;
}

std::string getByteArrayString(uint8_t *content, size_t length)
{
    char *buffer = new char[length + 1];

    memcpy(buffer, content, length);
    buffer[length] = '\0';

    std::string retval(buffer);

    delete[] buffer;

    return retval;
}
