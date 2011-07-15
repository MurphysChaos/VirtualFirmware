#include "opt.h"

int main(int argc, char **argv) {
	ReadOptions(NULL);
	printf("mcastip=%s\n", OPT.mcastip);
	printf("mcastport=%s\n", OPT.mcastport);
	printf("tcpport=%s\n", OPT.tcpport);
	printf("magicnum=%d\n", OPT.magicnum);

	return 0;
}
