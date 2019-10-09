/*
*	Author	:	Stephen Amey
*	Date	:	June 25, 2019
*	Purpose	: 	This library is used to interface with one or more GPS receivers.
*				It is specifically tailored to the Western University HAB project.
*/

//--------------------------------------------------------------------------\
//								    Imports					   				|
//--------------------------------------------------------------------------/


	#include "HAB_GPS.h"
	
	
//--------------------------------------------------------------------------\
//								  Constructor					   			|
//--------------------------------------------------------------------------/


	//Software serial(rx, tx) on arduino side. Thus GPS Tx->Arduino Rx
	//HAB_GPS::HAB_GPS(uint8_t rxPin, uint8_t txPin) : Serial1 (txPin, rxPin){
	HAB_GPS::HAB_GPS(){
		Serial1.begin(GPS_BAUD);
		setGPS_DynamicModel6();
		
		//Gets a reference to the logging stringPtr
		stringPtr = HAB_Logging::getStringPtr();
	}
	
	
//--------------------------------------------------------------------------\
//								   Functions					   			|
//--------------------------------------------------------------------------/


	//--------------------------------------------------------------------------------\
	//Getters-------------------------------------------------------------------------|
		
		/*-------------------------------------------------------------------------------------*\
		| 	Name: 		getInfo																	|
		|	Purpose: 	Returns the gps info													|
		|	Arguments:	void																	|
		|	Returns:	char*																	|
		\*-------------------------------------------------------------------------------------*/
			char* HAB_GPS::getInfo(char* stringPtr){
				
				/*
				Requires implementation
				*/
				
				return stringPtr;
			}

		/*-------------------------------------------------------------------------------------*\
		| 	Name: 		getDate																	|
		|	Purpose: 	Returns the current date.												|
		|	Arguments:	void																	|
		|	Returns:	char*																|
		\*-------------------------------------------------------------------------------------*/
			char* HAB_GPS::getDate(char* stringPtr){
				//Sets to the stringPtr pointer
				sprintf(stringPtr, "%d/%d/%d (UTC) ", gpsData.date.day(), gpsData.date.month(), gpsData.date.year());	
				return stringPtr;
			}
			
		/*-------------------------------------------------------------------------------------*\
		| 	Name: 		getTime																	|
		|	Purpose: 	Returns the current time.												|
		|	Arguments:	void																	|
		|	Returns:	char*																|
		\*-------------------------------------------------------------------------------------*/
			char* HAB_GPS::getTime(char* stringPtr){
				//Sets to the stringPtr pointer
				sprintf(stringPtr, "%d:%d:%d (UTC) ", gpsData.time.hour(), gpsData.time.minute(), gpsData.time.second());	
				return stringPtr;
			}
			
		/*-------------------------------------------------------------------------------------*\
		| 	Name: 		getReadings																|
		|	Purpose: 	Returns the most recent readings.										|
		|	Arguments:	void																	|
		|	Returns:	Readings																|
		\*-------------------------------------------------------------------------------------*/
			TinyGPSPlus* HAB_GPS::getReadings(){
				return &gpsData;
			}
			
		/*-------------------------------------------------------------------------------------*\
		| 	Name: 		getLockStatus															|
		|	Purpose: 	Checks to see if the receiver has a complete lock.						|
		|	Arguments:	void																	|
		|	Returns:	bool																	|
		\*-------------------------------------------------------------------------------------*/
			bool HAB_GPS::getLockStatus(){	
				feedReceiver();
				
				//Returns once we have enough valid data
				return (
					gpsData.location.age() < GPS_MAX_AGE
					&& gpsData.location.isValid()
					&& gpsData.date.isValid()
					&& gpsData.time.isValid()
					&& gpsData.altitude.isValid()
					&& gpsData.speed.isValid()
					&& gpsData.course.isValid()
					&& gpsData.satellites.isValid()
				);
			}
			
		/*-------------------------------------------------------------------------------------*\
		| 	Name: 		isAscending																|
		|	Purpose: 	Checks to see if the GPS is ascending.									|
		|	Arguments:	void																	|
		|	Returns:	bool																	|
		\*-------------------------------------------------------------------------------------*/
			bool HAB_GPS::isAscending(){	
				
				/*
				Requires implementation
				*/
				
				return true;
			}
		
		/*-------------------------------------------------------------------------------------*\
		| 	Name: 		isModeSet																|
		|	Purpose: 	Returns true if the 'airborne <1G' mode is set.							|
		|	Arguments:	void																	|
		|	Returns:	bool																	|
		\*-------------------------------------------------------------------------------------*/	
			bool HAB_GPS::isModeSet(){
				return modeSet;
			}
            
	//--------------------------------------------------------------------------------\
	//Setters-------------------------------------------------------------------------|
	

	//--------------------------------------------------------------------------------\
	//Miscellaneous-------------------------------------------------------------------|	
			
		/*-------------------------------------------------------------------------------------*\
		| 	Name: 		feedReceiver															|
		|	Purpose: 	Feed the GPS object with data from the receiver.						|
		|				Update readings when valid.												|
		|	Arguments:	void																	|
		|	Returns:	void																	|
		\*-------------------------------------------------------------------------------------*/
			void HAB_GPS::feedReceiver(){	
				while(Serial1.available()){
					gpsData.encode(Serial1.read());
				}
			}
				
		/*-------------------------------------------------------------------------------------*\
		| 	Name: 		printInfo																|
		|	Purpose: 	Prints the gps info														|
		|	Arguments:	void																	|
		|	Returns:	void																	|
		\*-------------------------------------------------------------------------------------*/
			void HAB_GPS::printInfo(){
				HAB_Logging::printLogln("*** GPS data dump ***", "");
				HAB_Logging::printLogln("", "");
			
			
				HAB_Logging::printLog("\tDate(UTC)  : ", "");
					HAB_Logging::printLog(itoa(gpsData.date.day(), stringPtr, 10), "");
					HAB_Logging::printLog(itoa(gpsData.date.month(), stringPtr, 10), "/");
					HAB_Logging::printLogln(itoa(gpsData.date.year(), stringPtr, 10), "/");
				HAB_Logging::printLog("\tTime(UTC)  : ", "");
					HAB_Logging::printLog(itoa(gpsData.time.hour(), stringPtr, 10), "");
					HAB_Logging::printLog(itoa(gpsData.time.minute(), stringPtr, 10), ":");
					HAB_Logging::printLogln(itoa(gpsData.time.second(), stringPtr, 10), ":");
				HAB_Logging::printLogln("\tLocation   : ", "");
					HAB_Logging::printLogln(dtostrf(gpsData.location.lat(), 10, 6, stringPtr), "\t\tLatitude(deg)  : ");
					HAB_Logging::printLogln(dtostrf(gpsData.location.lng(), 10, 6, stringPtr), "\t\tLongitude(deg) : ");
					HAB_Logging::printLogln(dtostrf(gpsData.altitude.meters(), 10, 6, stringPtr), "\t\tAltitude(m)    : ");
				HAB_Logging::printLogln("\tVelocity   : ", "");
					HAB_Logging::printLogln(dtostrf(gpsData.speed.mps(), 10, 6, stringPtr), "\t\tSpeed(m/s)     : ");
					HAB_Logging::printLogln(dtostrf(gpsData.course.deg(), 10, 6, stringPtr), "\t\tCourse(deg)    : ");
				HAB_Logging::printLog("\tSatellites : ", "");
					HAB_Logging::printLogln(itoa(gpsData.satellites.value(), stringPtr, 10), "");
				HAB_Logging::printLog("\tH-Dim.     : ", "");
					HAB_Logging::printLogln(itoa(gpsData.hdop.value(), stringPtr, 10), "");
			}
			
		/*-------------------------------------------------------------------------------------*\
		| 	Name: 		setDynamicModel															|
		|	Purpose: 	The following functions are used to set the dynamic model to the		|
		|				'airborne <1G' mode. This allows operation up to 50Km.					|
		|	Arguments:	void																	|
		|	Returns:	void																	|
		\*-------------------------------------------------------------------------------------*/
			void HAB_GPS::setGPS_DynamicModel6(){
				int tries = 0;
				uint8_t setdm6[] = {
					0xB5, 0x62, 0x06, 0x24, 0x24, 0x00, 0xFF, 0xFF, 0x06,
					0x03, 0x00, 0x00, 0x00, 0x00, 0x10, 0x27, 0x00, 0x00,
					0x05, 0x00, 0xFA, 0x00, 0xFA, 0x00, 0x64, 0x00, 0x2C,
					0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
					0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x16, 0xDC
				};
				
				while(!modeSet && tries++ < 3){
					HAB_GPS::sendUBX(setdm6, sizeof(setdm6)/sizeof(uint8_t));
					modeSet = HAB_GPS::getUBX_ACK(setdm6);
				}
			}
		
			void HAB_GPS::sendUBX(uint8_t *MSG, uint8_t len){
				Serial1.flush();
				Serial1.write(0xFF);
				delay(500);
				for(int i=0; i<len; i++){
					Serial1.write(MSG[i]);
				}
			}
		
			bool HAB_GPS::getUBX_ACK(uint8_t *MSG){
				uint8_t b;
				uint8_t ackByteID = 0;
				uint8_t ackPacket[10];
				unsigned long startTime = millis();
			 
				//Construct the expected ACK packet
				ackPacket[0] = 0xB5; // header
				ackPacket[1] = 0x62; // header
				ackPacket[2] = 0x05; // class
				ackPacket[3] = 0x01; // id
				ackPacket[4] = 0x02; // length
				ackPacket[5] = 0x00;
				ackPacket[6] = MSG[2]; // ACK class
				ackPacket[7] = MSG[3]; // ACK id
				ackPacket[8] = 0; // CK_A
				ackPacket[9] = 0; // CK_B
			 
				//Calculate the checksums
				for(uint8_t ubxi=2; ubxi<8; ubxi++){
					ackPacket[8] = ackPacket[8] + ackPacket[ubxi];
					ackPacket[9] = ackPacket[9] + ackPacket[8];
				}
			 
				while(1){		 
					//Test for success
					if(ackByteID > 9){
						// All packets in order!
						return true;
					}
			 
					//Timeout if no valid response in 3 seconds
					if(millis() - startTime > 3000){
						return false;
					}
			 
					//Make sure data is available to read
					if(Serial1.available()){
						b = Serial1.read();
			 
						//Check that bytes arrive in sequence as per expected ACK packet
						if(b == ackPacket[ackByteID]){
							ackByteID++;
						}
						else{
							ackByteID = 0; // Reset and look again, invalid order
						}
					}
				}
			}