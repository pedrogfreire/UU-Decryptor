#include <stdio.h>
#include <unistd.h>			//Used for UART
#include <fcntl.h>			//Used for UART
#include <termios.h>		//Used for UART
#include "time_delay.c"
#include <string.h>
#include <stdlib.h>

int uart0_filestream = -1;

/************************************************
*				Module Functions				*
*************************************************/

unsigned char setup_uart();
unsigned char send_string_uart(unsigned char * str);
int read_uart(unsigned char * buffer);
int get_sensor_ID(unsigned char * buffer);
double string_to_double(unsigned char * integer, unsigned char * decimal);
double get_sensor_value(unsigned char * buffer);
int is_standard_format(unsigned char * buffer);


#define MAX_SENSOR_ID_SIZE  10
#define MAX_SENSOR_VALUE_SIZE  10


typedef struct{
	int Sensor_ID;
	double Sensor_Value;
}Message_Struct;

/***********************************************************************************
* setup_uart()														   			   *
* Sets up the UART at 9600, 8bits, no parity, 1 bit stop						   *
* 																				   *
* Arguments: void                                                                  *
* Return: unsigned char 0 if not able to open UART and 1 if successfull setup	   *
*		  																		   *
************************************************************************************/	
unsigned char setup_uart(){
	//-------------------------
	//----- SETUP USART 0 -----
	//-------------------------
	
	
	//OPEN THE UART
	//The flags (defined in fcntl.h):
	//	Access modes (use 1 of these):
	//		O_RDONLY - Open for reading only.
	//		O_RDWR - Open for reading and writing.
	//		O_WRONLY - Open for writing only.
	//
	//	O_NDELAY / O_NONBLOCK (same function) - Enables nonblocking mode. When set read requests on the file can return immediately with a failure status
	//											if there is no input immediately available (instead of blocking). Likewise, write requests can also return
	//											immediately with a failure status if the output can't be written immediately.
	//
	//	O_NOCTTY - When set and path identifies a terminal device, open() shall not cause the terminal device to become the controlling terminal for the process.
	uart0_filestream = open("/dev/serial0", O_RDWR | O_NOCTTY | O_NDELAY);		//Open in non blocking read/write mode
	if (uart0_filestream == -1)
	{
		//ERROR - CAN'T OPEN SERIAL PORT
		return 0;
	}
	
	//CONFIGURE THE UART
	//The flags (defined in /usr/include/termios.h - see http://pubs.opengroup.org/onlinepubs/007908799/xsh/termios.h.html):
	//	Baud rate:- B1200, B2400, B4800, B9600, B19200, B38400, B57600, B115200, B230400, B460800, B500000, B576000, B921600, B1000000, B1152000, B1500000, B2000000, B2500000, B3000000, B3500000, B4000000
	//	CSIZE:- CS5, CS6, CS7, CS8
	//	CLOCAL - Ignore modem status lines
	//	CREAD - Enable receiver
	//	IGNPAR = Ignore characters with parity errors
	//	ICRNL - Map CR to NL on input (Use for ASCII comms where you want to auto correct end of line characters - don't use for bianry comms!)
	//	PARENB - Parity enable
	//	PARODD - Odd parity (else even)
	struct termios options;
	tcgetattr(uart0_filestream, &options);
	options.c_cflag = B115200 | CS8 | CLOCAL | CREAD;		//<Set baud rate
	options.c_iflag = IGNPAR;
	options.c_oflag = 0;
	options.c_lflag = 0;
	tcflush(uart0_filestream, TCIFLUSH);
	tcsetattr(uart0_filestream, TCSANOW, &options);
	return 1;
}

/***********************************************************************************
* send_string_uart()									     			   			*
* Sends through UART a string													   *
* 																				   *
* Arguments: void                                                                  *
* Return: unsigned char 0 if not able to open send and 1 if successfull setup	   *
*		  																		   *
************************************************************************************/
unsigned char send_string_uart(unsigned char * str){
	unsigned char return_code=0;
	if (uart0_filestream != -1){
		int count = write(uart0_filestream, str, strlen(str));
		if(count<0){
			//ERROR
			return_code=0;
		}
		else{
			return_code=1;
		}
	}
	else{
		return_code = 0;
	}
	return return_code;
}
/***********************************************************************************
* read_uart()									     			   				   *
* Reads information from UART													   *
* 																				   *
* Arguments: unsigned char * buffer (Pointer to the output buffer to store the	   *
*											received message)					   *
* Return: int rx_lenght															   *
*					if (<0) ERROR reading file									   *
*					if (==0) No INFO in the entry buffer						   *
*					if (>0) return is the buffer lenght							   *
*		  																		   *
************************************************************************************/
int read_uart(unsigned char * buffer){
	int return_code = -1;
	if (uart0_filestream != -1){		
		int rx_length = read(uart0_filestream, (void*)buffer, 255);		//Filestream, buffer to store in, number of bytes to read (max)
		return_code = rx_length; //if (<0) ERROR reading file
								 //if (==0) No INFO in the entry buffer	
								 //if (>0) return is the buffer lenght
	}
	else{
		return_code = -1;
	}
	return return_code;
}

/***********************************************************************************
* get_sensor_ID()									     			     		   *
* Retrives the sensor id from the standards message format <ID;VALUE>			   *
* 																				   *
* Arguments: unsigned char * buffer (message already in the correct format)        *
* Return: int sensor ID or -1 if error occured									   *
*		  																		   *
************************************************************************************/

int get_sensor_ID(unsigned char * buffer){
	int i;
	unsigned char buffer_ID[MAX_SENSOR_ID_SIZE];
	buffer++;
	for(i=0; i<=MAX_SENSOR_ID_SIZE && *buffer!=';'; i++){
		buffer_ID[i] = *buffer++;	
	}
	if(i==MAX_SENSOR_ID_SIZE && *buffer!=';'){
		return(-1);//return error
	}
	else{
		buffer_ID[i+1]=0;
		return atoi(buffer_ID);
	}	
}

/***********************************************************************************
* string_to_double()									     			     	   *
* Takes 2 strings that contains number, the integer and decimal part, and returns  *
* the corresponding number in double format										   *
* 																				   *
* Arguments: unsigned char * integer, unsigned char * decimal                      *
* Return: double number_in_double_format										   *
*		  																		   *
************************************************************************************/

double string_to_double(unsigned char * integer, unsigned char * decimal){
	double dec;
	dec = (double) atoi(decimal);
	for(;dec>1; dec/=10);
	dec = dec + (double) atoi(integer);
	return dec;
}

/***********************************************************************************
* get_sensor_value()									     			     	   *
* Retrives the sensor sampled value from the standards message format <ID;VALUE>   *
* 																				   *
* Arguments: unsigned char * buffer (message already in the correct format)        *
* Return: double sample_value_in_double_format										   *
*		  																		   *
************************************************************************************/
double get_sensor_value(unsigned char * buffer){
	int i=0;
	double result;
	buffer++;
	for(;*buffer!=';';buffer++){
		i++;
		if(i>MAX_SENSOR_ID_SIZE)break;
	}
	buffer++;
	unsigned char buffer_INT[MAX_SENSOR_VALUE_SIZE];
	unsigned char buffer_DEC[MAX_SENSOR_VALUE_SIZE];
	memset(buffer_INT,0,MAX_SENSOR_VALUE_SIZE);
	memset(buffer_DEC,0,MAX_SENSOR_VALUE_SIZE);
	for(i=0; i<MAX_SENSOR_VALUE_SIZE && *buffer!='>' && *buffer!='.'; i++){
		buffer_INT[i] = *buffer++;			
	}	
	buffer_INT[i]=0;
	if(*buffer=='.'){	
		buffer++;
		for(i=0; i<MAX_SENSOR_VALUE_SIZE && *buffer!='>'; i++){
			buffer_DEC[i] = *buffer++;				
		}
		buffer_DEC[i]=0;
	}
	result = string_to_double(buffer_INT, buffer_DEC);		
	return result;
}

/***********************************************************************************
* is_standard_format()									     			     	   *
* Checks if the buffer is in the standard message format <ID;VALUE>   			   *
* 																				   *
* Arguments: unsigned char * buffer 									           *
* Return: int    1 if in the correct format  									   *
*		  		 0 if in the wrong format  										   *
************************************************************************************/
int is_standard_format(unsigned char * buffer){
	int is_standard=1;
	int i;
	if(*buffer=='<'){
		buffer++;
		for(i=0;i<MAX_SENSOR_ID_SIZE && *buffer!=';';i++){
			if(*buffer<'0' || *buffer>'9') is_standard=0;
			buffer++;
		}
		buffer--;
		if(*buffer=='<')is_standard=0;
		buffer++;
		if(*buffer!=';' || i>(MAX_SENSOR_ID_SIZE-1)){
			is_standard=0;
		}
		else{			
			buffer++;
			if(*buffer=='-')buffer++;
			for(i=0;i<MAX_SENSOR_VALUE_SIZE && *buffer!='.' && *buffer!='>';i++){
				if(*buffer<'0' || *buffer>'9') is_standard=0;
				buffer++;
			}
			if(i>(MAX_SENSOR_VALUE_SIZE-1)){
				is_standard=0;
			}
			else if(*buffer!='.' && *buffer!='>'){
				is_standard=0;
			}
			else{
				buffer++;
				for(i=0;i<MAX_SENSOR_VALUE_SIZE && *buffer!='>';i++){
					if(*buffer<'0' || *buffer>'9') is_standard=0;
					buffer++;
				}
				if(i>(MAX_SENSOR_VALUE_SIZE-1)){
					is_standard=0;
				}
			}
		}		
	}
	else{
		is_standard=0;
	}
	return is_standard;
}

/***********************************************************************************
* check_for_new_message 								     			     	   *
* Run indefinetly checking the UART port for new valid messages and extracting the *
* corresponding values															   *
*																				   *
* Arguments: void 									           					   *
* Return: Message_Struct strunct with the correct values of the Sensor			   *
*		  		 																   *
************************************************************************************/
Message_Struct check_for_new_message(){
	Message_Struct new_message;
	new_message.Sensor_ID=0;
	new_message.Sensor_Value=0;
	unsigned char buffer[100];	
	if(setup_uart()){	
		while(new_message.Sensor_ID==0){
			int buf_lenght = read_uart(buffer);
			if(buf_lenght>0){
				if(is_standard_format(buffer)){
					new_message.Sensor_ID = get_sensor_ID(buffer);
					new_message.Sensor_Value = get_sensor_value(buffer);
					if(new_message.Sensor_ID<=0){
						new_message.Sensor_ID=0;
						new_message.Sensor_Value=0;
					}					
				}
			}
			delay(20);
		}
		if(!send_string_uart("OK"))printf("Error sending OK back\n");
	}
	else{
		printf("Failed to setup UART\n");
	}
	close(uart0_filestream);
	return new_message;
}

