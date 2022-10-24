#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <ctype.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "at_command_parser.h"
#define LEN 4096

int main(int argc, char **argv){

	if(argc<2)
    {
  	    printf("Usage : test_program_executable <test_file>");
        exit(1);
    }

	struct stat stt;
	int res, result;
	if((res=stat(argv[1],&stt))<0)
	{
		printf("%d\n", res);
		perror("Stat failed\n");
	}
	else
	{
		if(S_ISREG(stt.st_mode))
		{
			char buf[LEN];
			FILE *f = fopen(argv[1], "rb");
			while(fgets(buf, LEN, f))
			{
				int i = 0;
				while(buf[i] != '\0')
				{
					result=at_command_parse(buf[i]);
					if(result == 1)
						printf("ready ok\n");
					if(result == 2)
						printf("ready with error %c\n",buf[i]);
					//if(result == 0)
						//printf("not ready %c\n",buf[i]);
					i++;
				}
				
			}
			fclose(f);
		}
	}
	return 0;
}
