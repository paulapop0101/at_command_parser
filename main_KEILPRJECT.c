#include <stdint.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include "LPC177x_8x.h"
#include "system_LPC177x_8x.h"
#include <retarget.h>

#include <DRV\drv_sdram.h>
#include <DRV\drv_lcd.h>
#include <DRV\drv_uart.h>
#include <DRV\drv_touchscreen.h>
#include <DRV\drv_led.h>
#include <utils\timer_software.h>
#include <utils\timer_software_init.h>

#include "at_command_parser.h"

void timer_callback_1(timer_software_handler_t h)
{
	//DRV_LED_Toggle(LED_1);
	//printf("led 4");
}

void testLCD()
{
	uint32_t i,j;
	LCD_PIXEL foreground = {0, 255, 255, 0};
	LCD_PIXEL background = {0, 0, 0, 0};
	
	
	for (i = 0; i < LCD_HEIGHT; i++)
	{
		for (j = 0; j < LCD_WIDTH / 3; j++)
		{
			DRV_LCD_PutPixel(i, j, 255, 0, 0);
		}
		for (j = LCD_WIDTH / 3; j < 2 * (LCD_WIDTH / 3); j++)
		{
			DRV_LCD_PutPixel(i, j, 230, 220, 0);
		}
		for (j = 2 * (LCD_WIDTH / 3); j < LCD_WIDTH; j++)
		{
			DRV_LCD_PutPixel(i, j, 0, 0, 255);
		}
	}

	DRV_LCD_Puts("yugtu", 20, 30, foreground, background, TRUE);
	DRV_LCD_Puts("Hello", 20, 60, foreground, background, FALSE);	
}

const char* at_command_simple="AT\r\n";
const char* at_command_csq = "AT+CSQ\r\n";
const char* at_command_creg = "AT+CREG?\r\n";
const char* at_command_cops = "AT+COPS?\r\n";
const char* at_command_gsn = "AT+GSN\r\n";
const char* at_command_gmm = "AT+GMM\r\n";
const char* at_command_gmi = "AT+GMI\r\n";
const char* at_command_gmr = "AT+GMR\r\n";

int selected_info;
	float rssVal = 0;
	char response_csq [32];
	char state_reg [32];
	char op_name [32];
	char imei [32];
	char manufacturer [32];
	char modem_version [32];

void display_command_output(char * command, char * response)
{
	LCD_PIXEL foreground = {0, 255, 255, 0};
	LCD_PIXEL background = {0, 0, 0, 0};
	
	DRV_LCD_Puts(command, 220, 225, foreground, background, FALSE);
	DRV_LCD_Puts(response, 220, 245, foreground, background, FALSE);
}

void TouchScreenCallBack(TouchResult* touchData)
{
	//printf("touched X=%3d Y=%3d\n", touchData->X, touchData->Y);
	char emptyString [32];
		int buttonPressed = 0;
	
	if(touchData->Y >= 220 && touchData->Y< 260)
	{
		if(touchData->X >= 20 && touchData->X < 100)
		{
			printf("Touched previous!\n");
			selected_info = (selected_info + 5)%6;
			TIMER_SOFTWARE_Wait(100); 
			buttonPressed = 1;
		}
		else if (touchData->X >= 120 && touchData->X < 200)
		{
			printf("Touched next!\n");
			selected_info = (selected_info + 1)%6;
			TIMER_SOFTWARE_Wait(100); 
			buttonPressed = 1;
		}
	}
	
	if(buttonPressed)
	{
		for (buttonPressed = 0; buttonPressed < 32; buttonPressed++)
			emptyString[buttonPressed] = ' ';
		display_command_output(emptyString, emptyString);
		switch (selected_info)
			 {
				 case 0:
					  display_command_output((char *)at_command_csq, response_csq);
						break;
				 case 1:
						display_command_output((char *)at_command_creg, state_reg);
						break;
				 case 2:
						display_command_output((char *)at_command_cops, op_name);
						break;
				 case 3:
						display_command_output((char *)at_command_gsn, imei);
						break;
				 case 4:
						display_command_output((char *)at_command_gmm, manufacturer);
						break;
				 case 5:
						display_command_output((char *)at_command_gmr, modem_version);
						break;
			 }
	}
}




timer_software_handler_t my_handler;

void BoardInit()
{
	timer_software_handler_t handler;
	
	TIMER_SOFTWARE_init_system();
	
	
	DRV_SDRAM_Init();
	
	initRetargetDebugSystem();
	DRV_LCD_Init();
	DRV_LCD_ClrScr();
	DRV_LCD_PowerOn();	
	
	DRV_TOUCHSCREEN_Init();
	DRV_TOUCHSCREEN_SetTouchCallback(TouchScreenCallBack);
	DRV_LED_Init();
	printf ("Hello\n");	
	
	handler = TIMER_SOFTWARE_request_timer();
	TIMER_SOFTWARE_configure_timer(handler, MODE_1, 1000, 1);
	TIMER_SOFTWARE_set_callback(handler, timer_callback_1);
	TIMER_SOFTWARE_start_timer(handler);
	
	my_handler = TIMER_SOFTWARE_request_timer();
	TIMER_SOFTWARE_configure_timer(my_handler, MODE_1, 20000, 1);
	//TIMER_SOFTWARE_start_timer(my_handler);
	selected_info = 0;
}


void GetCommandResponse(uint8_t flag)
{
	 uint8_t ch;
	 BOOLEAN ready = FALSE;
	STATE_MACHINE_RETURN_VALUE ret;
	 TIMER_SOFTWARE_reset_timer(my_handler);
	 TIMER_SOFTWARE_start_timer(my_handler);
	 while ((!TIMER_SOFTWARE_interrupt_pending(my_handler)) && (ready == FALSE))
	 {
		  //printf("first while\n");
		 while (DRV_UART_BytesAvailable(UART_3) > 0)
		 {
			  //printf("second while\n");
			 DRV_UART_ReadByte(UART_3, &ch);
			 ret=at_command_parse(ch, flag);
			 if (ret != STATE_MACHINE_NOT_READY)
			 {
				ready = TRUE;
			 }
		 }
	 }
	 TIMER_SOFTWARE_clear_interrupt(my_handler);
	 printf("Get response ready = %d, returned = %d\n", ready,ret);
} 

void SendCommand(const char *command)
{
 DRV_UART_FlushRX(UART_3);
 DRV_UART_FlushTX(UART_3);
 DRV_UART_Write(UART_3, (uint8_t *)command, strlen(command));
} 

void ExecuteCommand(const char *command, uint8_t flag)
{
 SendCommand(command);
 GetCommandResponse(flag);
}

int CommandResponseValid()
{	
	return transfer.ok == 1 ? 1 : 0;
}
float ExtractData(AT_COMMAND_DATA * t)
{
	float asu, temp;
	int p1, p2;
	p1 = 0;
	p2 = 0;
	printf("Debug %s\n", t->data[0]);
	sscanf(t->data[0], "CSQ: %d,%d", &p1,&p2);
	asu = p1;
	temp = p2;
	while(temp>1){
		temp/=10.;
	}
	asu+=temp;
	return asu;
}	

float ConvertAsuToDbmw(float rssi_value_asu)
{
	return 2*rssi_value_asu -113;
}	
void drawRect(int x1, int y1, int x2, int y2){
	int i;
	for(i = x1; i < x2; i++){
		DRV_LCD_PutPixel(i, y1, 0, 255, 255);
		DRV_LCD_PutPixel(i, y2, 0, 255, 255);
	}
	
	for(i = y1; i < y2; i++){
		DRV_LCD_PutPixel(x1, i, 0, 255, 255);
		DRV_LCD_PutPixel(x2, i, 0, 255, 255);
	}
}
void display_buttons()
{
	LCD_PIXEL foreground = {0, 255, 255, 0};
	LCD_PIXEL background = {0, 0, 0, 0};
	
	DRV_LCD_Puts("SMS Menu", 90, 80, foreground, background, FALSE);
	
	DRV_LCD_Puts("Previous", 35, 115, foreground, background, FALSE);
	drawRect(100,20,140,100);
	DRV_LCD_Puts("Send", 35, 165, foreground, background, FALSE);
	drawRect(150,20,190,100);
	DRV_LCD_Puts("Next", 135, 115, foreground, background, FALSE);
	drawRect(100,120,140,200);
	DRV_LCD_Puts("Delete", 135, 165, foreground, background, FALSE);
	drawRect(150,120,190,200);
	
	DRV_LCD_Puts("Info", 95, 205, foreground, background, FALSE);
	DRV_LCD_Puts("Previous", 35, 235, foreground, background, FALSE);
	drawRect(220,20,260,100);
	DRV_LCD_Puts("Next", 135, 235, foreground, background, FALSE);
	drawRect(220,120,260,200);
}

void display_status(float rssVal, char *state_reg, char *op_name){

	char buf [64];
	LCD_PIXEL foreground = {0, 255, 255, 0};
	LCD_PIXEL background = {0, 0, 0, 0};

	sprintf(buf, "Signal strength: %f dbmw", rssVal);
	DRV_LCD_Puts(buf, 20, 20, foreground, background, FALSE);
	DRV_LCD_Puts(state_reg, 20, 40, foreground, background, FALSE);
	DRV_LCD_Puts(op_name, 20, 60, foreground, background, FALSE);
}

int main(void)
{
	float rssi_value_asu;
	float rssi_value_dbmw;

	int change = 0;
	int old_info = 0;
	uint8_t ch;
	timer_software_handler_t my_timer_handler;
	BoardInit();
	//testLCD();
	display_status(0, "Searching", "");
	display_buttons();
	
	DRV_UART_Configure(UART_3, UART_CHARACTER_LENGTH_8, 115200, UART_PARITY_NO_PARITY, 1, TRUE, 3);
	DRV_UART_Configure(UART_2, UART_CHARACTER_LENGTH_8, 115200, UART_PARITY_NO_PARITY, 1, TRUE, 3);
	
	DRV_UART_Write(UART_3, (uint8_t*)at_command_simple, strlen(at_command_simple));
	TIMER_SOFTWARE_Wait(1000); 
	
	printf("hei\n");
	
	
	my_timer_handler = TIMER_SOFTWARE_request_timer();
	TIMER_SOFTWARE_configure_timer(my_timer_handler, MODE_1, 5000, 1);
	TIMER_SOFTWARE_start_timer(my_timer_handler); 
	
	
	
	while (1)
  {
		change = 0;
		 if (TIMER_SOFTWARE_interrupt_pending(my_timer_handler)!=0)
		 {
				printf("hei in if\n");
				ExecuteCommand(at_command_csq, 0);
			 if (CommandResponseValid())
			 {
				 rssi_value_asu = ExtractData(&transfer);
				 rssi_value_dbmw = ConvertAsuToDbmw(rssi_value_asu);
				 if(rssVal != rssi_value_dbmw){
					 rssVal = rssi_value_dbmw;
					 strncpy(response_csq, (char *)transfer.data[0], 30);
					 change =1;
				 }
				 printf("GSM Modem signal %f -> %f\n",rssi_value_asu, rssi_value_dbmw);
			 }
			  ExecuteCommand(at_command_creg, 0);
			 if (CommandResponseValid())
			 {
				 if(strcmp((char *)transfer.data[0],state_reg)){
					 strncpy(state_reg,(char *)transfer.data[0], 30);
					 change = 1;
				 }
				 printf("State of registration: %s\n", transfer.data[0]);
			 }
			 ExecuteCommand(at_command_cops, 0);
			 if (CommandResponseValid())
			 {
				 if(strcmp((char *)transfer.data[0],op_name)){
					 strncpy(op_name,(char *)transfer.data[0], 30);
					 change = 1;
				 }
				 printf("Operator name: %s\n", transfer.data[0]);
			 }
			 ExecuteCommand(at_command_gsn, 1);
			 if (CommandResponseValid())
			 {
				 strncpy(imei,(char *)transfer.data[0], 30);
				 printf("Modem IMEI: %s\n", transfer.data[0]);
			 }
			 ExecuteCommand(at_command_gmm, 1);
			 if (CommandResponseValid())
			 {
				 strncpy(manufacturer,(char *)transfer.data[0], 30);
				 printf("Modem manufacturer: %s\n", transfer.data[0]);
			 }
			 ExecuteCommand(at_command_gmr, 1);
			 if (CommandResponseValid())
			 {
				 strncpy(modem_version,(char *)transfer.data[0], 30);
				 printf("Modem software version: %s\n", transfer.data[0]);
			 }
			 TIMER_SOFTWARE_clear_interrupt(my_timer_handler);
			 
			 if(old_info != selected_info)
			 {
				 old_info = selected_info;
				 change = 1;
			 }
		 }
		 if(change){
			 display_status(rssVal, state_reg, op_name);
		 }
		 DRV_TOUCHSCREEN_Process();
	} 
 
	//as2
/**	while(1)
	{
		DRV_LED_Toggle(LED_1);
		DRV_LED_Toggle(LED_2);
		DRV_LED_Toggle(LED_3);
		DRV_LED_Toggle(LED_4);
		printf("led 2");
		TIMER_SOFTWARE_Wait(1000);
	}**/
	
	//as3
	/**handler = TIMER_SOFTWARE_request_timer();
	TIMER_SOFTWARE_configure_timer(handler, MODE_1, 1000, 1);
	TIMER_SOFTWARE_start_timer(handler); 

	while(1)
  {
    // user code
    if (TIMER_SOFTWARE_interrupt_pending(handler) != 0)
    {
      DRV_LED_Toggle(LED_1);
			printf("led 3");
      TIMER_SOFTWARE_clear_interrupt(handler);
    }
    // user code
   }**/
	 
	
	/*
	while(1)
	{
		DRV_UART_SendByte(UART_3, 'A');
	//	TIMER_SOFTWARE_Wait(1000);
	}
	*/
	/*
	while(1)
	{
		if (DRV_UART_ReadByte(UART_3, &ch) == OK)
		{
			DRV_UART_SendByte(UART_3, ch);
		}		
	}
*/
	/**while(1)
	{
		if (DRV_UART_ReadByte(UART_0, &ch) == OK)
		{
			DRV_UART_SendByte(UART_3, ch);
		}
		if (DRV_UART_ReadByte(UART_3, &ch) == OK)
		{
			DRV_UART_SendByte(UART_0, ch);
		}
		if (DRV_UART_ReadByte(UART_2, &ch) == OK)
		{
			DRV_UART_SendByte(UART_0, ch);
		}
	}
	
	while(1)
	{
		DRV_UART_Process();
		DRV_TOUCHSCREEN_Process();
	}**/
	
	return 0; 
}
