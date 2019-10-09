/*
*	Author	:	Stephen Amey
*	Date	:	June 17, 2019
*	Purpose	: 	This library is used to write log messages to an SD card. 
*				It is specifically tailored to the Western University HAB project.
*/

//--------------------------------------------------------------------------\
//								    Imports					   				|
//--------------------------------------------------------------------------/


	#include "HAB_Logging.h"
 

//--------------------------------------------------------------------------\
//                                 Variables                                |
//--------------------------------------------------------------------------/


   uint8_t chipSelect = 0;
   bool status = false;
   char stringPtr[100] = "";
   char timestampPtr[15] ="";

	
//--------------------------------------------------------------------------\
//								   Functions					   			|
//--------------------------------------------------------------------------/

	/*-------------------------------------------------------------------------------------*\
	| 	Name: 		getChip																	|
	|	Purpose: 	Gets the value for chipSelect											|
	|	Arguments:	void																	|
	|	Returns:	integer																	|
	\*-------------------------------------------------------------------------------------*/
		uint8_t HAB_Logging::getChip(){
			return chipSelect;
		}
		
	/*-------------------------------------------------------------------------------------*\
	| 	Name: 		getStatus																|
	|	Purpose: 	Gets the status of the SD card.											|
	|	Arguments:	void																	|
	|	Returns:	boolean																	|
	\*-------------------------------------------------------------------------------------*/
		bool HAB_Logging::getStatus(){
			return status;
		}
		
	/*-------------------------------------------------------------------------------------*\
	| 	Name: 		setChip																	|
	|	Purpose: 	Sets the value for chipSelect											|
	|	Arguments:	integer																	|
	|	Returns:	void																	|
	\*-------------------------------------------------------------------------------------*/
		void HAB_Logging::setChip(uint8_t _chipSelect){
			chipSelect = _chipSelect;
          
            //Start with the new chipSelect
			status = SD.begin(chipSelect);
            if(status){
                Serial.println("Card found.");
            }
            else{
                Serial.println("Card failed, or not present.");
            }
		}
				
	/*-------------------------------------------------------------------------------------*\
	| 	Name: 		printLog																|
	|	Purpose: 	Logs the given string to the log file (no newline)						|
	|	Arguments:	char*																	|
	|	Returns:	void																	|
	\*-------------------------------------------------------------------------------------*/
		void HAB_Logging::printLog(const char* msg, const char* prepend){
            //If no SD card
            if(chipSelect == 0){
				Serial.println("No chip set!"); 
            }
			//If SD card, write to it
			else{
				//Prints to the SD card
				File dataFile = SD.open("log.txt", FILE_WRITE);
				dataFile.print(prepend);			
				dataFile.print(msg);	
				dataFile.close();
			}
			
			//Prints to serial
			Serial.print(prepend);
			Serial.print(msg);      
		}
		
	/*-------------------------------------------------------------------------------------*\
	| 	Name: 		printLogln																|
	|	Purpose: 	Logs the given string to the log file (with newline)					|
	|	Arguments:	char*																	|
	|	Returns:	void																	|
	\*-------------------------------------------------------------------------------------*/		
		void HAB_Logging::printLogln(const char* msg, const char* prepend){
            //If no SD card
            if(chipSelect == 0){
				Serial.println("No chip set!"); 
            }
			//If SD card, write to it
			else{
				//Prints to the SD card
				File dataFile = SD.open("log.txt", FILE_WRITE);
				dataFile.print(prepend);				
				dataFile.println(msg);	
				dataFile.close();
			}
			
			//Prints to serial
			Serial.print(prepend);
			Serial.println(msg);      
		}

		
	/*-------------------------------------------------------------------------------------*\
	| 	Name: 		getTimestamp															|
	|	Purpose: 	Returns a pointer to a character array containing the up-time.			|
	|	in a [xx:xx:xx] format.																|
	|	Arguments:	void																	|
	|	Returns:	char*																	|
	\*-------------------------------------------------------------------------------------*/
		char* HAB_Logging::getTimestamp(){
			unsigned long uptime = millis()/1000;
				
			uint16_t hours = uptime / 3600;
				uptime = uptime % 3600;
			uint8_t minutes = uptime / 60;
				uptime = uptime % 60;
			uint8_t seconds = uptime;
			
			//Sets to the timestamp pointer
			sprintf(timestampPtr, "[%02d:%02d:%02d] ", hours, minutes, seconds);
			
			//Returns the pointer to the timestamp character array
			return timestampPtr;
		}
		
	/*-------------------------------------------------------------------------------------*\
	| 	Name: 		getTimeFormatted														|
	|	Purpose: 	Returns a pointer to a character array containing the up-time.			|
	|	in a xx:xx:xx format.																|
	|	Arguments:	void																	|
	|	Returns:	char*																	|
	\*-------------------------------------------------------------------------------------*/
		char* HAB_Logging::getTimeFormatted(){
			unsigned long uptime = millis()/1000;
				
			uint16_t hours = uptime / 3600;
				uptime = uptime % 3600;
			uint8_t minutes = uptime / 60;
				uptime = uptime % 60;
			uint8_t seconds = uptime;
			
			//Sets to the time pointer
			sprintf(timestampPtr, "%02d:%02d:%02d", hours, minutes, seconds);
			
			//Returns the pointer to the time character array
			return timestampPtr;
		}
		
	/*-------------------------------------------------------------------------------------*\
	| 	Name: 		getStringPtr															|
	|	Purpose: 	Returns the pointer used for strings, for general usage.				|
	|	Arguments:	void																	|
	|	Returns:	char*																	|
	\*-------------------------------------------------------------------------------------*/
		char* HAB_Logging::getStringPtr(){
			return stringPtr;
		}
		
	/*-------------------------------------------------------------------------------------*\
	| 	Name: 		checkReady																|
	|	Purpose: 	Returns true if files on the SD card can be written to.					|
	|	Arguments:	void																	|
	|	Returns:	bool																	|
	\*-------------------------------------------------------------------------------------*/
		bool HAB_Logging::checkReady(){	
			//If the SD card has not been initialized, return false
			if(chipSelect == 0 || !status){ return false; }
			
			//Holds the number of bytes written, and file open status
			uint8_t bytesWritten;
			bool filesOpened = true;
					
			//Attempts to open and print to log.txt on the SD card
			File dataFile = SD.open("log.txt", FILE_WRITE);
			if(!dataFile){ filesOpened = false; }
			bytesWritten = dataFile.println("Logging check!");				
			dataFile.close();
			
			//Attempts to open datalog.txt on the SD card
			dataFile = SD.open("datalog.txt", FILE_WRITE);
			if(!dataFile){ filesOpened = false; }				
			dataFile.close();
			
			//If it was able to write, return true
			return (bytesWritten > 0 && filesOpened );
		}
			
		
	/*-------------------------------------------------------------------------------------*\
	| 	Name: 		initExcelFile															|
	|	Purpose: 	Initializes an Excel file for data logging.								|
	|	Arguments:	void																	|
	|	Returns:	void																	|
	\*-------------------------------------------------------------------------------------*/
		void HAB_Logging::initExcelFile(uint8_t _podCount) {
            File dataFile = SD.open("datalog.txt", FILE_WRITE);

            //If the file exists,
            if(dataFile){
                //Write the column headers
                dataFile.print("Time(s),Altitude(m),Speed(m/s),Longitude(deg),Latitude(deg),Temperature(C),Pressure(hPa),Humidity(%)");
                for(int i = 0; i != _podCount; i++){
                   //dataFile.print(",");
                   //dataFile.print(i);
                   //dataFile.print("_open");
				   
				   dataFile.print(",");
                   dataFile.print(i);
                   dataFile.print("_position");
				   
				   dataFile.print(",");
                   dataFile.print(i);
                   dataFile.print("_temperature");
				   
				   dataFile.print(",");
                   dataFile.print(i);
                   dataFile.print("_act_status");
				   
				   dataFile.print(",");
                   dataFile.print(i);
                   dataFile.print("_heat_status");
                }

                dataFile.println();

                //Close the file
                dataFile.close();
            }
            else{      
                Serial.println("error opening datalog.txt");
				//SD.begin(chipSelect);
				//delay(100);				
            }
        }
		
	/*-------------------------------------------------------------------------------------*\
	| 	Name: 		writeToExcel															|
	|	Purpose: 	Initializes an Excel file for data logging.								|
	|	Arguments:	BMEReadings, GPSReadings												|
	|	Returns:	void																	|
	\*-------------------------------------------------------------------------------------*/
		void HAB_Logging::writeToExcel(BMEReadings bmeReadings, GPSReadings gpsReadings, actuatorReadings* actReadingsArray, int arrLength) {
        
            File dataFile = SD.open("datalog.txt", FILE_WRITE);
        
            if(dataFile) {               
                //Time (H:M:S), Altitude, Speed, Longitude, Latitude, Temperature, Pressure, Humidity
                //dataFile.print(gpsReadings.hour);      		dataFile.print(":"); 
                //dataFile.print(gpsReadings.minute);    		dataFile.print(":"); 
                //dataFile.print(gpsReadings.second);    		dataFile.print(",");
                dataFile.print(HAB_Logging::getTimeFormatted());	dataFile.print(",");
                dataFile.print(gpsReadings.altitude);       		dataFile.print(",");
                dataFile.print(gpsReadings.speed);          		dataFile.print(",");
                dataFile.print(gpsReadings.longitude);      		dataFile.print(",");
                dataFile.print(gpsReadings.latitude);       		dataFile.print(",");
                dataFile.print(bmeReadings.temperature);    		dataFile.print(",");
                dataFile.print(bmeReadings.pressure);       		dataFile.print(",");
                dataFile.print(bmeReadings.humidity);

                //Actuator statuses
                for(int i = 0; i != arrLength; i++){
				   dataFile.print(",");
                   dataFile.print(actReadingsArray[i].position);
				   dataFile.print(",");
				   dataFile.print(actReadingsArray[i].temperature);
				   dataFile.print(",");
                   dataFile.print(actReadingsArray[i].actuatorStatusPtr);
				   dataFile.print(",");
                   dataFile.print(actReadingsArray[i].heaterStatusPtr);
				}

                //Print New Line
                dataFile.println();

                //close the file
                dataFile.close();    			
            }
            else{        
                Serial.println("error opening datalog.txt");     
				//SD.begin(chipSelect);
				//delay(100);	
            }
        }