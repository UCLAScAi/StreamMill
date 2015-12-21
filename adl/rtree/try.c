#include <stdio.h>
#include <errno.h>

main()
{
    FILE *fp = fopen("try.out", "wb+");
    char buffer[10] = "hello";

    if(fp == NULL)
	exit(1);

    if(fseek(fp, 10240, SEEK_SET) != 0)
    {
	fprintf(stderr, "Error no=%d\n", errno);
	exit(1);
    }

    if(fwrite(buffer, 1, 10, fp) != 10)
    {
	fprintf(stderr, "Error happens\n");
	exit(1);
    }
}
