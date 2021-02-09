// C function showing how to do time delay 
#include <stdio.h> 
// To use time library of C 
#include <time.h> 

unsigned char time_str[30];

void delay(int number_of_seconds) 
{ 
    // Converting time into milli_seconds 
    int milli_seconds = 1000 * number_of_seconds; 
  
    // Storing start time 
    clock_t start_time = clock(); 
  
    // looping till required time is not achieved 
    while (clock() < start_time + milli_seconds) 
        ; 
}
 
unsigned char * get_current_time (){	
	time_t t = time(NULL);
	struct tm tm = *localtime(&t);
	sprintf(time_str,"%d-%02d-%02dT%02d:%02d:%02d", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
	return time_str;
}