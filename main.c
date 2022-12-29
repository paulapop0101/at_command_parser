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
			int flags[]={0,0,0,1,1,0,2,0,0,0,0,0,1,0,1,0,2,0,1,0};
			int line_flag=0;
			while(fgets(buf, LEN, f))
			{
				int i = 0;
				while(buf[i] != '\0')
				{
					result=at_command_parse(buf[i], flags[line_flag]);
					//printf("parsed %02x: %d\n", buf[i], result);
					if(result == 1)
					{
						printf("ready ok\n");
						print_transaction(&transfer,flags[line_flag]);
						printf("\n");
						line_flag++;
					}
					if(result == 2)
					{
						printf("ready with error %c\n", buf[i]);
						print_transaction(&transfer,flags[line_flag]);
						return 0;
					}
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