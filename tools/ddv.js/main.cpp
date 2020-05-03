#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>

#include <emscripten.h>

#include <thread>
#include <chrono>

using namespace std;

extern "C" void console_log(const char *s, uint16_t len);
extern "C" void native_ready();

EMSCRIPTEN_KEEPALIVE
extern "C" void sayHello()
{
    printf("Hello printf!\n");
    fflush(stdout);
}

EM_JS(void, sayHelloJS, (const char* str), {
  console.log('Hello console.log! ' + UTF8ToString(str));
});

EMSCRIPTEN_KEEPALIVE
extern "C" void getHello()
{
  std::string hi = "Hello from C++ to JS!\n";
  console_log(hi.c_str(), strlen(hi.c_str()));
}

int main(int argc, char *argv[])
{
  // native_ready();
  sayHello();
  sayHelloJS("[some const char*]");
  // getHello();
  fflush(stdout);

  int i = 0;
  while(true)
  {
     printf("I'm still alive %d\n", i);
     std::this_thread::sleep_for(std::chrono::seconds(1));
     i++;
  }
}
