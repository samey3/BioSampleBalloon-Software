/*
*	Author	:	Stephen Amey
*	Date	:	June 10, 2019
*	Purpose	: 	This library is used to interface with a linear actuator with position feedback. 
*				It is specifically tailored to the Western University HAB project.
*/


#ifndef HAB_Actuator_h
#define HAB_Actuator_h


//--------------------------------------------------------------------------\
//								    Imports					   				|
//--------------------------------------------------------------------------/


	#include "Arduino.h"
	#include <Wire.h>
	#include <SPI.h>
	#ifndef HAB_Logging_h
        #include <HAB_Logging.h>
    #endif


class HAB_Actuator {

	//--------------------------------------------------------------------------\
	//								  Definitions					   			|
	//--------------------------------------------------------------------------/
		private:
		
		#ifndef POD_OPEN
			#define POD_OPEN 10 //10 //0 most open
		#endif
		#ifndef POD_CLOSED
			#define POD_CLOSED 1020 //1020 //1023 most closed, give some leeway here
		#endif
		
		#define SERIESRESISTOR 10000  
		#define THERMISTORNOMINAL 10000   
		#define TEMPERATURENOMINAL 25 
		#define BCOEFFICIENT 3950
	
	
	//--------------------------------------------------------------------------\
	//								   Variables					   			|
	//--------------------------------------------------------------------------/
		
		//Actuator name
		char namePtr[10] = "";
		
		//Actuator pins
		uint8_t act_en, act_push, act_pull, act_pos;
		uint8_t heat_en, thermistor;
		
		//Enables statuses
		bool moveEnabled, heatEnabled;
		
		//Opening and closing altitudes
		double openAlt, closeAlt;
		
		//If pod has opened already
		bool hasOpened;
		
		//If it reached its opening interval (used when closeAlt < openAlt)
		bool hasEnteredInterval;
		
		//Actuator and heater override
		bool actuatorOverride, actuatorOverrideOpen;
		bool heaterOverride, heaterOverrideEnabled;
		
		//Direction the actuator is moving (true for retracting, false for extending)
		bool isMovingOpen = false;
		
		//If use of the actuator is loked
		bool locked = false;
		
		
		
		//TESTING
		uint16_t pos = 1023;
		int8_t dir = 0;
	
	
	//--------------------------------------------------------------------------\
	//								  Constructor					   			|
	//--------------------------------------------------------------------------/
		public:
	
		HAB_Actuator(const char* namePtr, uint8_t act_en, uint8_t act_push, uint8_t act_pull,
				uint8_t act_pos, uint8_t heat_en, uint8_t thermistor, double openAlt, double closeAlt
		);
		
		
	//--------------------------------------------------------------------------\
	//								   Functions					   			|
	//--------------------------------------------------------------------------/
	
	
		//--------------------------------------------------------------------------------\
		//Getters-------------------------------------------------------------------------|
			char* getName();
			double getOpenAlt();
			double getCloseAlt();
			uint16_t getPosition();			
			bool isMoveEnabled();
			bool isHeatEnabled();				
			bool isClosed();
			bool isFullyOpen();
			float getTemperature();			
			bool getHasOpened();
			bool isInInterval(float altitude);
			bool isOpening();
			bool isLocked();
		
		
		//--------------------------------------------------------------------------------\
		//Setters-------------------------------------------------------------------------|
			//void setMoveEnabled(boolean moveEnabled); //Is this needed in the new setup?
			void setOpenAltitude(double openAlt);
			void setCloseAltitude(double closeAlt);
			void setHasOpened(bool hasOpened);
			void setLock(bool locked);
		
		
		//--------------------------------------------------------------------------------\
		//Miscellaneous-------------------------------------------------------------------|
			//Actuator
			void extend();
			void retract();
			void halt();
			void overrideActuatorHalt();
			void overrideActuatorOpen(); //The overrides don't actually modify the outputs, it just modifies the booleans for you to read and make decisions from. Perhaps change this later.
			void overrideActuatorClose();
			void overrideActuatorRelease();
			bool isActuatorOverridden();
			bool isActuatorOverrideOpen();
			
			//Heater
			void stopHeating();
			void startHeating();
			void overrideHeaterEnable();
			void overrideHeaterDisable();
			void overrideHeaterRelease();
			bool isHeaterOverridden();
			bool isHeaterOverrideEnabled();
			
			void deactivateAll();
};

#endif