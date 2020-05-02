#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>

#include <emscripten.h>

using namespace std;

extern "C" void console_log(const char *s, uint16_t len);

EMSCRIPTEN_KEEPALIVE
extern "C" void sayHello()
{
  printf("Hello!\n");
  fflush(stdout);
}

#if 0
EM_JS(void, sayHelloJS, (const char* str), {
  console.log('console.log: hello ' + UTF8ToString(str));
});
#endif

EMSCRIPTEN_KEEPALIVE
extern "C" void getHello()
{
  std::string hi = "Hello!\n";
  // sayHelloJS(hi.c_str());
  console_log(hi.c_str(), strlen(hi.c_str()));
}

int main(int argc, char *argv[])
{
  sayHello();
  fflush(stdout);
}
