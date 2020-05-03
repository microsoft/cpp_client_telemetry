#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>

#include <emscripten.h>

#include <thread>
#include <chrono>

using namespace std;

EMSCRIPTEN_KEEPALIVE
extern "C" void sayHello()
{
    printf("printf: up and running\n");
    fflush(stdout);
}

EM_JS(void, console_log, (const char* str), { console.log(UTF8ToString(str)); });

EMSCRIPTEN_KEEPALIVE
void console_log_string(const std::string& msg)
{
  // Call JS function using EM_ASM
  EM_ASM({console_log($0, $1)}, msg.c_str(), msg.length() );
}

EMSCRIPTEN_KEEPALIVE
void console_log_strdup(const std::string& msg)
{
  char *s = strdup(msg.c_str());
  // Call JS function using EM_ASM
  EM_ASM({console_log($0, $1)}, s, msg.length() );
  free(s);
}

EMSCRIPTEN_KEEPALIVE
extern "C" void sendBuffer(char* buffer, size_t size)
{
  for(size_t i=0; i<size; i++)
  {
    printf("%02x ", buffer[i]);
  }
  printf("\n");
}

int main(int argc, char *argv[])
{
  // Notify Javascript that we're ready
  EM_ASM({onEmInit()});

  sayHello();

  console_log("some [const char*]");

  // allocated on stack or heap?
  std::string s1 = "some [std::string]";
  console_log_string(s1); // log original

  std::string s2 = "some [std::string] copy";
  console_log_strdup(s2); // log HEAP copy

  fflush(stdout);

  int i = 0;
  while(true)
  {
     printf("I'm still alive %d\n", i);
     std::this_thread::sleep_for(std::chrono::seconds(1));
     i++;
  }

  EM_ASM({onEmDone()});
}
