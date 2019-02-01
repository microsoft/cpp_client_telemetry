#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "zlib.h"

char *buff = "0123456789";

int main(int argc, char *argv[])
{
    auto crc = crc32(0L, Z_NULL, 0);
    crc = crc32(crc, (const Bytef *)buff, strlen(buff));
    printf("%u\n", crc);
    return 0;
}
