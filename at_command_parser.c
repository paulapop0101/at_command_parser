#include <stdint.h>
#include <stdio.h>
#include "at_command_parser.h"
STATE_MACHINE_RETURN_VALUE at_command_parse(uint8_t current_character)
{
	static uint32_t state = 0;
	switch (state){
		 case 0:{
			 if (current_character == 0x0D){
			 	state = 1;
			 }
			 break;
		 }
		 case 1:{
			 if (current_character == 0x0A){
			 	state = 2;
			 }
			 else{
				state = 0;
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
			 else if (current_character == '+'){
			 	state = 5;
			 }
			 else{
				state = 0;
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
			 	return STATE_MACHINE_READY_WITH_ERROR;
			 }
			break;
		 }
		case 5:{
			if (current_character >= 32 && current_character <= 126){
			 	state = 15;
			 }
			 else{
				state = 0;
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
			 	return STATE_MACHINE_READY_WITH_ERROR;
			 }
			break;
		 }
		case 7:{
			 if (current_character == 0x0A){
				state = 0;
				return STATE_MACHINE_READY_OK;
			 }
			 else{
				state = 0;
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
			 	return STATE_MACHINE_READY_WITH_ERROR;
			 }
			break;
		 }
		case 13:{
			 if (current_character == 0x0A){
				state = 0;
			 	return STATE_MACHINE_READY_WITH_ERROR;
			 }
			 else{
				state = 0;
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
			 	state = 15;
			 }
			else if(current_character == 0x0D){
				state = 16;
			}
			 else{
				state = 0;
			 	return STATE_MACHINE_READY_WITH_ERROR;
			 }
			break;
		}
		case 16:{
			 if (current_character == 0x0A){
			 	state = 17;
			 }
			 else{
				state = 0;
			 	return STATE_MACHINE_READY_WITH_ERROR;
			 }
			break;
		 }
		case 17:{
			if (current_character == '+'){
			 	state = 5;
			 }
			 else if (current_character == 0x0D){
			 	state = 18;
			 }
			 else{
				state = 0;
			 	return STATE_MACHINE_READY_WITH_ERROR;
			 }
			break;
		 }
		case 18:{
			if (current_character == 0x0A){
			 	state = 19;
			 }
			 else{
				state = 0;
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
			 	return STATE_MACHINE_READY_WITH_ERROR;
			 }
			break;
		 }
	}
	return STATE_MACHINE_NOT_READY;
} 
