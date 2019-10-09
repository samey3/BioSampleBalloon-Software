/*
*	Author	:	Stephen Amey
*	Date	:	June 25, 2019
*	Purpose	: 	This library is used to interface a GPS receiver.
*				It is specifically tailored to the Western University HAB project.
*/


#ifndef HAB_GPS_h
#define HAB_GPS_h


//--------------------------------------------------------------------------\
//								    Imports					   				|
//--------------------------------------------------------------------------/


	#include "Arduino.h"
	#include <SoftwareSerial.h>
	#include <SPI.h>
	#include <TinyGPS++.h>
	#include <HAB_Logging.h>
	

class HAB_GPS {
	
	
	//--------------------------------------------------------------------------\
	//								  Definitions					   			|
	//--------------------------------------------------------------------------/
	
	
		#ifndef GPS_MAX_AGE
			#define GPS_MAX_AGE 10000
		#endif
		#ifndef GPS_BAUD
			#define GPS_BAUD 9600
		#endif
	

	//--------------------------------------------------------------------------\
	//								   Variables					   			|
	//--------------------------------------------------------------------------/
		private:
			
		//Holds gps data
		TinyGPSPlus gpsData;
		
		//Holds a reference to the logging stringPtr
		char* stringPtr;
		
		//Set to true if the proper GPS mode is set
		bool modeSet = false;
     
	
	//--------------------------------------------------------------------------\
	//								  Constructor					   			|
	//--------------------------------------------------------------------------/
		public:
	
		HAB_GPS();
		

	//--------------------------------------------------------------------------\
	//								   Functions					   			|
	//--------------------------------------------------------------------------/
	
	
		//--------------------------------------------------------------------------------\
		//Getters-------------------------------------------------------------------------|
			char* getInfo(char* stringPtr);	
			char* getDate(char* stringPtr);
			char* getTime(char* stringPtr);
			bool getLockStatus();		
			TinyGPSPlus* getReadings();
			bool isAscending(); //This isn't used
			bool isModeSet();
		
		//--------------------------------------------------------------------------------\
		//Setters-------------------------------------------------------------------------|
		
		
		//--------------------------------------------------------------------------------\
		//Miscellaneous-------------------------------------------------------------------|
			void feedReceiver();
			void printInfo();
			void setGPS_DynamicModel6();
			void sendUBX(uint8_t *MSG, uint8_t len);
			bool getUBX_ACK(uint8_t *MSG);
};

#endif
