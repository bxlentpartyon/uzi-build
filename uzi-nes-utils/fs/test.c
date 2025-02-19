#include <stdio.h>

#include <unix.h>

int main(int argc, char **argv)
{
	printf("dinode size: %lu\n", sizeof(dinode));
	printf("time_t size: %lu\n", sizeof(uzi_time_t));
	printf("off_t size: %lu\n", sizeof(uzi_off_t));
	printf("uint16 size: %lu\n", sizeof(uint16));
	printf("ushort size: %lu\n", sizeof(unsigned short));
}
