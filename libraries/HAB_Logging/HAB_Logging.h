/*
*	Author	:	Stephen Amey
*	Date	:	June 17, 2019
*	Purpose	: 	This library is used to write log messages to an SD card. 
*				It is specifically tailored to the Western University HAB project.
*/


#ifndef HAB_Logging_h
#define HAB_Logging_h


//--------------------------------------------------------------------------\
//								    Imports					   				|
//--------------------------------------------------------------------------/
	
	
	#include "Arduino.h"
	#ifndef HAB_Structs_h
        #include <HAB_Structs.h>
    #endif
	#ifndef HAB_Actuator_h
        #include <HAB_Actuator.h>
    #endif
	#ifndef SD_h
		#include <SD.h>
	#endif
	

class HAB_Logging {


	//--------------------------------------------------------------------------\
	//								   Functions					   			|
	//--------------------------------------------------------------------------/
		public:
			
		static uint8_t getChip();
		static bool getStatus();
		static void setChip(uint8_t chipSelect);
		static void printLog(const char* msg, const char* prepend = HAB_Logging::getTimestamp());
		static void printLogln(const char* msg, const char* prepend = HAB_Logging::getTimestamp());
		static char* getTimestamp(void);
		static char* getTimeFormatted(void);
		static char* getStringPtr(void);
		static bool checkReady(void);
		static void initExcelFile(uint8_t _podCount);
		static void writeToExcel(BMEReadings bmeReadings, GPSReadings gpsReadings, actuatorReadings* actArray, int arrLength);
};

#endif