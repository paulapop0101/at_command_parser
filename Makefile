
output: at_command_parser.o main.o
	gcc -Wall -o output main.o -L. -lat_command_parser

at_command_parser.o: at_command_parser.c at_command_parser.h
	gcc -Wall -c -o libat_command_parser.o at_command_parser.c
	ar rcs libat_command_parser.a libat_command_parser.o

main.o: main.c
	gcc -Wall -c -o main.o main.c

clean:
	rm *.o output
