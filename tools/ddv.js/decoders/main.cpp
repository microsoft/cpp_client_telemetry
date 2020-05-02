#include <stdio.h>
#include <stdlib.h>

using namespace std;

void sayHello()
{
	printf("Hello!\n");
}

int main(int argc, char *argv[])
{
	sayHello();
	fflush(stdout);
}
