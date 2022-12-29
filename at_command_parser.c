#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include "at_command_parser.h"

AT_COMMAND_DATA transfer;

uint32_t min(uint32_t a, uint32_t b)
{
	if (a < b)
		return a;
	return b;
}

void print_normal_transaction(AT_COMMAND_DATA * t){
    int i;
    for (i = 0; i < min(AT_COMMAND_MAX_LINES, t->line_count); ++i)
		if (t->data[i][0])
			printf("%s\n", t->data[i]);
}

void print_sms_transaction(AT_COMMAND_DATA * t){
    int i,nr, sms_count = 1;
    char phone[100];
    for (i = 0; i < min(AT_COMMAND_MAX_LINES, t->line_count); ++i){
        if(strstr((const char*)t->data[i],"CMGL")){
            printf("\nSMS message %d - ",sms_count++);
            if(strstr((char *)t->data[i],"REC READ"))
                sscanf((char *)t->data[i], "CMGL: %d, \"REC READ \", \"%s",&nr,phone);
            else
                sscanf((char *)t->data[i], "CMGL: %d, \"REC UNREAD \", \"%s",&nr,phone);
            printf("%s - ",phone);
        }
        else{
            printf("%s",t->data[i]);
        }
    }
}

void print_transaction(AT_COMMAND_DATA * t, uint8_t flag)
{
	printf("Transaction status: %d\n", t->ok);
	printf("Length (lines): %d\n", t->line_count);
	printf("Data:\n");

    if(flag == 2)
        print_sms_transaction(t);
    else print_normal_transaction(t);
}


STATE_MACHINE_RETURN_VALUE at_command_parse(uint8_t current_character, uint8_t flag)
{
    static uint32_t state = 0;
    static uint32_t line = 0;
	static uint32_t cursor = 0;
    switch (state){
			
        case 0:{
			transfer.ok = 2;
			transfer.line_count = 0;
			cursor = 0;
			
            if (current_character == 0x0D){
                state = 1;
            }
            else{
                printf("Error at line %d: State %d, input 0x%02x, expected 0x0D.\n", line, state, current_character);
                return STATE_MACHINE_READY_WITH_ERROR;
            }
            break;
        }

        case 1:{
            if (current_character == 0x0A){
                state = 2;
                line ++;
            }
            else{
                state = 0;
                printf("Error at line %d: State %d, input 0x%02x, expected 0x0A.\n", line, state, current_character);
                return STATE_MACHINE_READY_WITH_ERROR;
            }
            break;
        }

        case 2:{
            if (current_character == 'O'){
                state = 3;
            }
            else if (current_character == 'E'){
                state = 4;
            }
            else if (current_character == '+' && (flag == 0 || flag == 2)){
                state = 5;
            }
			else if(flag == 1 && current_character >= 32 && current_character <= 126){
				if(transfer.line_count < AT_COMMAND_MAX_LINES)
				{
					transfer.data[transfer.line_count][cursor] = current_character;
					transfer.data[transfer.line_count][cursor + 1] = 0;
					++cursor;
				}
                state = 20;
			}
            else{
                state = 0;
                printf("Error at line %d: State %d, input 0x%02x, expected 'O', 'E' or '+'.\n", line, state, current_character);
                return STATE_MACHINE_READY_WITH_ERROR;
            }
            break;
        }

        case 3:{
            if (current_character == 'K'){
                state = 6;
            }
            else{
                state = 0;
                printf("Error at line %d: State %d, input 0x%02x, expected 'K'.\n", line, state, current_character);
                return STATE_MACHINE_READY_WITH_ERROR;
            }
            break;
        }

        case 4:{
            if (current_character == 'R'){
                state = 9;
            }
            else{
                state = 0;
                printf("Error at line %d: State %d, input 0x%02x, expected 'R'.\n", line, state, current_character);
                return STATE_MACHINE_READY_WITH_ERROR;
            }
            break;
        }

        case 5:{
            if (current_character >= 32 && current_character <= 126){
				if(transfer.line_count < AT_COMMAND_MAX_LINES)
				{
					transfer.data[transfer.line_count][cursor] = current_character;
					transfer.data[transfer.line_count][cursor + 1] = 0;
					++cursor;
				}
                state = 15;
            }
            else{
                state = 0;
                printf("Error at line %d: State %d, input 0x%02x, expected [32-126]].\n", line, state, current_character);
                return STATE_MACHINE_READY_WITH_ERROR;
            }
            break;
        }

        case 6:{
            if (current_character == 0x0D){
                state = 7;
            }
            else{
                state = 0;
                printf("Error at line %d: State %d, input 0x%02x, expected 0x0D.\n", line, state, current_character);
                return STATE_MACHINE_READY_WITH_ERROR;
            }
            break;
        }

        case 7:{
            if (current_character == 0x0A){
                state = 0;
                line = 0;
				cursor = 0;
				transfer.ok = 1;
				
				//print_transaction(&transfer);

                return STATE_MACHINE_READY_OK;
            }
            else{
                state = 0;
                printf("Error at line %d: State %d, input 0x%02x, expected 0x0A.\n", line, state, current_character);
                transfer.ok = 0;
                return STATE_MACHINE_READY_WITH_ERROR;
            }
            break;
        }
        case 8:{
            //ok final state
            break;
        }
        case 9:{
            if (current_character == 'R'){
                state = 10;
            }
            else{
                state = 0;
                printf("Error at line %d: State %d, input 0x%02x, expected 'R'.\n", line, state, current_character);
                return STATE_MACHINE_READY_WITH_ERROR;
            }
            break;
        }
        case 10:{
            if (current_character == 'O'){
                state = 11;
            }
            else{
                state = 0;
                printf("Error at line %d: State %d, input 0x%02x, expected 'O'.\n", line, state, current_character);
                return STATE_MACHINE_READY_WITH_ERROR;
            }
            break;
        }
        case 11:{
            if (current_character == 'R'){
                state = 12;
            }
            else{
                state = 0;
                printf("Error at line %d: State %d, input 0x%02x, expected 'R'.\n", line, state, current_character);
                return STATE_MACHINE_READY_WITH_ERROR;
            }
            break;
        }
        case 12:{
            if (current_character == 0x0D){
                state = 13;
            }
            else{
                state = 0;
                printf("Error at line %d: State %d, input 0x%02x, expected 0x0D.\n", line, state, current_character);
                return STATE_MACHINE_READY_WITH_ERROR;
            }
            break;
        }
        case 13:{
            if (current_character == 0x0A){
                state = 0;
                line = 0;
				cursor = 0;
                transfer.ok = 0;
                return STATE_MACHINE_READY_OK;
            }
            else{
                state = 0;
                printf("Error at line %d: State %d, input 0x%02x, expected 0x0A.\n", line, state, current_character);
                return STATE_MACHINE_READY_WITH_ERROR;
            }
            break;
        }
        case 14:{
            //error final state
            break;
        }
        case 15:{
            if (current_character >= 32 && current_character <= 126){
				if(transfer.line_count < AT_COMMAND_MAX_LINES)
				{
                     if(flag == 2 && current_character == 34){
                        transfer.data[transfer.line_count][cursor] = ' ';
                        cursor++;
                    }
					transfer.data[transfer.line_count][cursor] = current_character;
					transfer.data[transfer.line_count][cursor + 1] = 0;
					++cursor;
				}
                state = 15;
            }
            else if(current_character == 0x0D){
				transfer.line_count++;
				cursor = 0;
                state = 16;
            }
            else{
                state = 0;
                printf("Error at line %d: State %d, input 0x%02x, expected '0x0D' or [32-126].\n", line, state, current_character);
                return STATE_MACHINE_READY_WITH_ERROR;
            }
            break;
        }
        case 16:{
            if (current_character == 0x0A){
                state = 17;
                line++;
            }
            else{
                state = 0;
                printf("Error at line %d: State %d, input 0x%02x, expected 0x0A.\n", line, state, current_character);
                return STATE_MACHINE_READY_WITH_ERROR;
            }
            break;
        }
        case 17:{
            if (flag == 0 && current_character == '+'){
                state = 5;
            }
            else if (flag == 0 && current_character == 0x0D){
                state = 18;
            }
            else if(flag == 2 && current_character >= 32 && current_character <= 126){
                transfer.data[transfer.line_count][cursor] = current_character;
                transfer.data[transfer.line_count][cursor + 1] = 0;
                ++cursor;
                state = 25;
            }
            else{
                state = 0;
                printf("Error at line %d: State %d, input 0x%02x, expected 0x0D or '+'.\n", line, state, current_character);
                return STATE_MACHINE_READY_WITH_ERROR;
            }
            break;
        }
        case 18:{
            if (current_character == 0x0A){
                state = 19;
                line++;
            }
            else{
                state = 0;
                printf("Error at line %d: State %d, input 0x%02x, expected 0x0A.\n", line, state, current_character);
                return STATE_MACHINE_READY_WITH_ERROR;
            }
            break;
        }
        case 19:{
            if (current_character == 'O'){
                state = 3;
            }
            else if (current_character == 'E'){
                state = 4;
            }
            else{
                state = 0;
                printf("Error at line %d: State %d, input 0x%02x, expected 'O' or 'E'.\n", line, state, current_character);
                return STATE_MACHINE_READY_WITH_ERROR;
            }
            break;
        }
		case 20:{
			if (current_character >= 32 && current_character <= 126){
				if(transfer.line_count < AT_COMMAND_MAX_LINES)
				{
					transfer.data[transfer.line_count][cursor] = current_character;
					transfer.data[transfer.line_count][cursor + 1] = 0;
					++cursor;
				}
                state = 20;
            }
            else if(current_character == 0x0D){
				transfer.line_count++;
				cursor = 0;
                state = 21;
            }
            else{
                printf("Error at line %d: State %d, input 0x%02x, expected '0x0D' or [32-126].\n", line, state, current_character);
                state = 0;
				return STATE_MACHINE_READY_WITH_ERROR;
            }
            break;
		}
		case 21:{
			if (current_character == 0x0A){
                state = 22;
                line++;
            }
            else{
                printf("Error at line %d: State %d, input 0x%02x, expected 0x0A.\n", line, state, current_character);
                state = 0;
				return STATE_MACHINE_READY_WITH_ERROR;
            }
            break;
		}
		case 22: {
			if(current_character == 0x0D){
				cursor = 0;
                state = 23;
            }
            else{
                printf("Error at line %d: State %d, input 0x%02x, expected '0x0D' or [32-126].\n", line, state, current_character);
                state = 0;
				return STATE_MACHINE_READY_WITH_ERROR;
            }
			break;
		}
		case 23: {
			if (current_character == 0x0A){
                state = 24;
                line++;
            }
            else{
                printf("Error at line %d: State %d, input 0x%02x, expected 0x0A.\n", line, state, current_character);
                state = 0;
				return STATE_MACHINE_READY_WITH_ERROR;
            }
            break;
		}
		case 24:{
			if (current_character == 'O'){
                state = 3;
            }
            else if (current_character == 'E'){
                state = 4;
            }
            else{
                printf("Error at line %d: State %d, input 0x%02x, expected 'O' or 'E'.\n", line, state, current_character);
                state = 0;
				return STATE_MACHINE_READY_WITH_ERROR;
            }
            break;
		}
        case 25:{
            if(flag == 2 && current_character >= 32 && current_character <= 126){
                transfer.data[transfer.line_count][cursor] = current_character;
                transfer.data[transfer.line_count][cursor + 1] = 0;
                ++cursor;
                if(cursor == AT_COMMAND_MAX_LINE_SIZE){
                    transfer.line_count++;
                    cursor=0;
                }
            }
            else if(flag == 2 && current_character == 0x0D){
                cursor=0;
                transfer.line_count++;
                state = 26;
            }
            else{
                state = 0;
                printf("Error at line %d: State %d, input 0x%02x, expected 0x0D or '+'.\n", line, state, current_character);
                return STATE_MACHINE_READY_WITH_ERROR;
            }
            break;
        }
        case 26:{
            if(current_character == 0x0A){
                state = 27;
            }
            else{
                state = 0;
                printf("Error at line %d: State %d, input 0x%02x, expected 0x0A.\n", line, state, current_character);
                return STATE_MACHINE_READY_WITH_ERROR;
            }
            break;
        }
        case 27: {
            if(current_character == 0x0D){
                state = 28;
            }
            else{
                state = 0;
                printf("Error at line %d: State %d, input 0x%02x, expected 0x0A.\n", line, state, current_character);
                return STATE_MACHINE_READY_WITH_ERROR;
            }
            break;
        }
        case 28: {
            if(current_character == 0x0A){
                state = 29;
            }
            else{
                state = 0;
                printf("Error at line %d: State %d, input 0x%02x, expected 0x0A.\n", line, state, current_character);
                return STATE_MACHINE_READY_WITH_ERROR;
            }
            break;
        }
        case 29:{
            if (current_character == 'O'){
                state = 3;
            }
            else if (current_character == 'E'){
                state = 4;
            }
            else if(current_character == '+'){
                state = 5;
            }
            else{
                state = 0;
                printf("Error at line %d: State %d, input 0x%02x, expected 'O' or 'E'.\n", line, state, current_character);
                return STATE_MACHINE_READY_WITH_ERROR;
            }
            break;
        }
    }
    return STATE_MACHINE_NOT_READY;
} 