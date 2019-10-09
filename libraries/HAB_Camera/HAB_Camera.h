/*
*	Author	:	Stephen Amey
*	Date	:	June 17, 2019
*	Purpose	: 	This library is used to interface with a camera to capture and store images. 
*				It is specifically tailored to the Western University HAB project.
*/


#ifndef HAB_Camera_h
#define HAB_Camera_h


//--------------------------------------------------------------------------\
//								    Imports					   				|
//--------------------------------------------------------------------------/


	#include "Arduino.h"
	#include <SoftwareSerial.h>
	#include <SPI.h>
	#include <SD.h>
	#include <Adafruit_VC0706.h>
	#ifndef HAB_Logging_h
        #include <HAB_Logging.h>
    #endif


class HAB_Camera {
	
	//--------------------------------------------------------------------------\
	//								  Definitions					   			|
	//--------------------------------------------------------------------------/
		private:
	
		#ifndef WRITES_PER_LOOP
			#define WRITES_PER_LOOP 3
		#endif
	
	
	//--------------------------------------------------------------------------\
	//								   Variables					   			|
	//--------------------------------------------------------------------------/
		
		
		//SD card
		uint8_t chipSelect;
		bool sdFound;
		
		//Camera pins
		uint8_t rxPin, txPin;

		//Camera
        SoftwareSerial tempConn = SoftwareSerial(0, 0);
		Adafruit_VC0706 cam = Adafruit_VC0706(&tempConn);
		bool cameraFound;
		
		//Image
		char fileName[50] = "";
		uint16_t bytesLeft;
		uint8_t *buffer;
		uint8_t bytesToRead;
		uint16_t imgCount = 0;
		
		//Holds a reference to the logging stringPtr
		char* stringPtr;
     
	
	//--------------------------------------------------------------------------\
	//								  Constructor					   			|
	//--------------------------------------------------------------------------/
		public:
	
		HAB_Camera(uint8_t chipSelect, uint8_t rxPin, uint8_t txPin);
		HAB_Camera(void);
		

	//--------------------------------------------------------------------------\
	//								   Functions					   			|
	//--------------------------------------------------------------------------/
	
	
		//--------------------------------------------------------------------------------\
		//Getters-------------------------------------------------------------------------|
			char* getInfo(char* stringPtr);	
			bool getReadyStatus();
			bool getBufferStatus();
		
		
		//--------------------------------------------------------------------------------\
		//Setters-------------------------------------------------------------------------|
		
		
		//--------------------------------------------------------------------------------\
		//Miscellaneous-------------------------------------------------------------------|
			void captureImage(const char* fileName, uint8_t size);
			void writeImage();
			void emptyImageBuffer();
};

#endif
