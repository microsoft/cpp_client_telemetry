#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <zlib.h>

using namespace std;

int main(int argc, char** argv)
{
    const char* buff = "Test string";
    auto crc = crc32(0, (const Bytef*)buff, strlen(buff));
    printf("crc32=%lu\n", crc);
    return 0;
}
