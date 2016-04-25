#include <stdio.h>


void busy()
{
	int i;
	for ( i= 0; i < 1e8; i++);
}


int main (int argc, char *argv[])
{
	
	printf("Hello\n");
	busy();
	sleep(1);
	busy();
	sleep(1);
	printf("Bye\n");

	return 0;
}
