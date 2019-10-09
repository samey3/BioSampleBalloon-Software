/*
*	Author	:	Stephen Amey
*	Date	:	Aug 20, 2019
*	Purpose	: 	Flight software for the 2019 HAB launch. 
*/

//---------------------------------------------------------------------------------------------\
//                                           Imports                                           |
//---------------------------------------------------------------------------------------------/


    #ifndef HAB_Actuator_h
        #include <HAB_Actuator.h>
    #endif
    #include <HAB_Camera.h>
    #include <HAB_GPS.h>
    #ifndef HAB_Logging_h
        #include <HAB_Logging.h>
    #endif
        
    //Ethernet Shield Library
    #include <Ethernet.h>
    #include <EthernetUdp.h>
    
    //SPI and Sensor Libraries (BME uses I2C)
    #include <Wire.h>
    #include <SPI.h>
    #include <Adafruit_Sensor.h>
    #include <Adafruit_BME280.h>

    //Mission Specific Library. Struct definitions and Constants are in this library
    #ifndef HAB_Structs_h
        #include <HAB_Structs.h>
    #endif

    //This one MUST be included LAST in order to override any redeclared definitions
    #include <HAB_Definitions.h>
    

//---------------------------------------------------------------------------------------------\
//                                       Function prototypes                                   |
//---------------------------------------------------------------------------------------------/


    //Don't really need prototypes section otherwise...
    void sendGSmessage(const char* msg, bool ignoreConn = false);


//---------------------------------------------------------------------------------------------\
//                                           Variables                                         |
//---------------------------------------------------------------------------------------------/


    //Holds a reference to the logging stringPtr
    char* genStringPtr;

    //Used for formatting our messages
    char msgPtr[100]; 
    
    //Is the HAB descending? Control via command
    bool isDescending = false;

    //----------------------------------------------------------\
    //Sensors and camera----------------------------------------|
        //The interface of the BME sensor should be I2C
        Adafruit_BME280 _bme;
        BMEReadings _BMEreadings;
        bool BMPstatus = false;
        
        //Declare GPS object
        HAB_GPS* _gps; //Can just straight up query this but I set the results to the pointer below, which will be updated in the background
        GPSReadings _HABGPSreadings;
        GPSReadings _CSAGPSreadings;
        bool HAB_GPS_enabled = true;
        bool CSA_GPS_enabled = true;

        //Camera object, image name
        HAB_Camera* _cam;
        char imgNamePtr[30];
        
        //Heating temperatures
        float minTemp = MIN_ACTUATOR_TEMP;
        float maxTemp = MAX_ACTUATOR_TEMP;
        
        //Readings timing
        unsigned long lastReadingsTime = 0;  
    
    //----------------------------------------------------------\
    //Actuators-------------------------------------------------|
        //Length of actuator array and the active actuator index
        uint8_t act_arr_len;
        uint8_t activeIndex = 0;

        //If the actuator was force-switched
        bool switchForced = false;

        //Additional push time after the actuator has reached the limit, to ensure its extended/retracted as far as it can
        unsigned long limitReachedTime = 0;
        bool isHalting = false;
        
        HAB_Actuator _actArray[] = { //Order matters! We will give each atleast a 1.5Km buffer
            HAB_Actuator("POD_1", ACT1_EN, ACT1_PUSH, ACT1_PULL, ACT1_POS, HEAT1_EN, THERMISTOR1, 2000, 10000),
            HAB_Actuator("POD_2", ACT2_EN, ACT2_PUSH, ACT2_PULL, ACT2_POS, HEAT2_EN, THERMISTOR2, 12000, 20000),
            HAB_Actuator("POD_3", ACT3_EN, ACT3_PUSH, ACT3_PULL, ACT3_POS, HEAT3_EN, THERMISTOR3, 22000, 30000),
            HAB_Actuator("POD_4", ACT4_EN, ACT4_PUSH, ACT4_PULL, ACT4_POS, HEAT4_EN, THERMISTOR4, 32000, 999999)
        };
        actuatorReadings _actReadingsArray[] = {
            actuatorReadings(),
            actuatorReadings(),
            actuatorReadings(),
            actuatorReadings()
        };  

    //----------------------------------------------------------\
    //Ethernet, UDP, telemetry----------------------------------|
        //Connection status
        bool noConnection = true; //Initially set to true
        bool noGPS01Connection = true;

        //Last heartbeat and initialization attempt times
        unsigned long lastHeartbeat = 0;
        unsigned long lastInit = 0;
        unsigned long lastGPS01 = 0;

        //Buffer to hold incoming/outgoing packets
        char rcvBuffer[UDP_TX_PACKET_MAX_SIZE];
        char sendBuffer[UDP_TX_PACKET_MAX_SIZE];

        //Command arguments
        char firstArg[ARGUMENT_MAX_LENGTH];
        char secondArg[ARGUMENT_MAX_LENGTH];
        char thirdArg[ARGUMENT_MAX_LENGTH];
        
        //Creates the UDP connection object, IP address
        EthernetUDP _conn;
            //Balloon address
            IPAddress _localIP(LOCAL_IP_O1, LOCAL_IP_O2, LOCAL_IP_O3, LOCAL_IP_O4);
            byte _localMAC[] = MAC; 
    
            //Groundstation 1 address
            IPAddress _GSIP1(GS1_IP_O1, GS1_IP_O2, GS1_IP_O3, GS1_IP_O4);

            //Groundstation 2 address
            IPAddress _GSIP2(GS2_IP_O1, GS2_IP_O2, GS2_IP_O3, GS2_IP_O4);
            
            //PRISM address
            IPAddress _PRISMIP(PRISM_IP_O1, PRISM_IP_O2, PRISM_IP_O3, PRISM_IP_O4);

            byte dns[] = {172,20,4,254};
            byte gate[] = {172,20,4,254};
            byte sub[] = {255,255,255,0};
        

//---------------------------------------------------------------------------------------------\
//                                            Setup                                            |
//---------------------------------------------------------------------------------------------/


    void setup() {
        //Serial setup
        Serial.begin(9600);

        Ethernet.init(10);

        //----------------------------------------------------------\
        //Setup objects---------------------------------------------|
            //Set up logging
            HAB_Logging::setChip(4);
            genStringPtr = HAB_Logging::getStringPtr();
    
            //Start message
            printHeader(); //Print the header
    
            //Gets the length of the actuator array
            act_arr_len = sizeof(_actArray) / sizeof(_actArray[0]);
    
            //Set up the Excel file
            HAB_Logging::initExcelFile(act_arr_len);
       
            //Sets up the GPS
            _gps = new HAB_GPS();
    
            //Sets up the camera
            _cam = new HAB_Camera(SD_CHIPSELECT, CAM1_RX_PIN, CAM1_TX_PIN);
                _cam->emptyImageBuffer(); //Ensures the buffer is empty beforehand

        //----------------------------------------------------------\
        //Begin and check startup conditions------------------------|  
            checkStartupConditions();

        //----------------------------------------------------------\
        //Startup checks passed, begin program----------------------|
            HAB_Logging::printLog("\r\n                     *!GPS lock obtained!*\r\n", "");
            printInfo();
    }


//---------------------------------------------------------------------------------------------\
//                                            Loop                                             |
//---------------------------------------------------------------------------------------------/


    void loop() {
               
        //----------------------------------------------------------\
        //Actuators-------------------------------------------------|
            //This will open/close pods based on the altitudes specified in the actuator constructor calls
            if(activeIndex < act_arr_len){ //Makes sure the actuator exists
                handleActuator(_actArray + activeIndex);
            }

        //----------------------------------------------------------\
        //Telecommands-----------------------------------------------|
             recievePacketsUDP();

        //----------------------------------------------------------\
        //GPS readings----------------------------------------------|

            if(!noGPS01Connection){
                //We have a connection to the CSA's GPS, so do nothing
                //Serial.println("Used CSA GPS!");
            }
            //No connection to CSA GPS, but connection to ours
            else if(_gps->getLockStatus()){
                _HABGPSreadings.second = _gps->getReadings()->time.second();
                _HABGPSreadings.minute = _gps->getReadings()->time.minute();
                _HABGPSreadings.hour = _gps->getReadings()->time.hour();
                _HABGPSreadings.speed = _gps->getReadings()->speed.mps();
                _HABGPSreadings.altitude = _gps->getReadings()->altitude.meters();
                _HABGPSreadings.latitude = _gps->getReadings()->location.lat();
                _HABGPSreadings.longitude = _gps->getReadings()->location.lng();               
            }    

        //----------------------------------------------------------\
        //Log and transmit readings---------------------------------|
            if((millis() - lastReadingsTime) > READINGS_TIME_STEP){
                //Sets the new last readings time
                lastReadingsTime = millis();

                //BME readings----------------------------------------------|
                    if(BMPstatus){
                        _BMEreadings.temperature = _bme.readTemperature();
                        _BMEreadings.pressure = _bme.readPressure();
                        _BMEreadings.humidity = _bme.readHumidity();
                    }
                //Actuator readings-----------------------------------------|
                    for(int i = 0; i != act_arr_len; i++){
                        _actReadingsArray[i].position = _actArray[i].getPosition();
                        _actReadingsArray[i].temperature = _actArray[i].getTemperature();
                        strcpy(_actReadingsArray[i].actuatorStatusPtr, (_actArray[i].isActuatorOverridden() ? (_actArray[i].isActuatorOverrideOpen() ? "OVR_OPEN" : "OVR_CLOSE") : "AUTO"));
                        strcpy(_actReadingsArray[i].heaterStatusPtr, (_actArray[i].isHeaterOverridden() ? (_actArray[i].isHeaterOverrideEnabled() ? "OVR_ENABLED" : "OVR_DISABLED") : "AUTO"));
                    }
                //Logging and telemetry-------------------------------------|
                    //Section for handling logging
                     HAB_Logging::writeToExcel(_BMEreadings, _HABGPSreadings, _actReadingsArray, act_arr_len);   
                     sendTelemetry();
            }

        //----------------------------------------------------------\
        //GPS feed and camera writing-------------------------------|
            //Feeds input to the GPS receiver to get new data
            _gps->feedReceiver();

            //If there is data in the camera buffer, writes it to the SD card
            _cam->writeImage();
        //----------------------------------------------------------\
        //Check if the balloon has descended------------------------|
            //THIS IS CURRENTLY NEVER TRIGGERED
            if(_HABGPSreadings.altitude < STOP_ALTITUDE && isDescending){
                HAB_Logging::printLogln("Flight ended!");
                sendGSmessage("Flight ended!");
                exit(0);
            }
    }


//---------------------------------------------------------------------------------------------\
//                                          Functions                                          |
//---------------------------------------------------------------------------------------------/


    /*-------------------------------------------------------------------------------------*\
    |   Name:       handleActuator                                                          |
    |   Purpose:    Used to open and close the pods, and maintain proper temperature        |
    |               of the actuators when they are required to move.                        |
    |   Arguments:  Actuator                                                                |
    |   Returns:    Void                                                                    |
    \*-------------------------------------------------------------------------------------*/
        void handleActuator(HAB_Actuator* actuator){

            //----------------------------------------------------------\
            //Actuator movement-----------------------------------------|
                //Handle opening-----------------------------------------------//
                    //If overridden open, not fully open, not opening: start opening
                    if(actuator->isActuatorOverridden() && actuator->isActuatorOverrideOpen() && !actuator->isFullyOpen() && (!actuator->isMoveEnabled() || !actuator->isOpening())){
                        //Starts opening the pod
                        actuator->retract();

                        //Send a message to the ground station
                        strcpy(msgPtr, "Retracting actuator of ");
                        strcat(msgPtr, actuator->getName());
                        sendGSmessage(msgPtr);
                                                   
                        //Optional picture
                    }
                    //Else if overridden open, fully opened, and is moving: halt movement
                    else if((actuator->isActuatorOverridden() && actuator->isActuatorOverrideOpen()) && actuator->isFullyOpen() && actuator->isMoveEnabled()){
                        //If this is the first time in here, records the time
                        if(!isHalting){
                            isHalting = true;
                            limitReachedTime = millis();
                        }
                        //Once the additional push time has elapsed, halt the actuator and capture an image
                        else if(isHalting && (millis() - limitReachedTime) >= ADDITIONAL_PUSH_TIME){
                            isHalting = false;
                            
                             //Halts the actuator
                            actuator->halt();

                            //Send a message to the ground station
                            strcpy(msgPtr, "Halting actuator of ");
                            strcat(msgPtr, actuator->getName());
                            HAB_Logging::printLogln(msgPtr);
                            sendGSmessage(msgPtr);
                            
                            //Creates the name of the image and attempts capture (DOS 8.3 format)
                            strcpy(imgNamePtr, "");
                            strcat(imgNamePtr, itoa(activeIndex, genStringPtr, 10)); strcat(imgNamePtr, "_O.jpg");                                             
                            _cam->captureImage(imgNamePtr, 0);
                        }
                    }

                //Handle closing-----------------------------------------------//
                    //If overridden close, open, not closing: start closing
                    if(actuator->isActuatorOverridden() && !actuator->isActuatorOverrideOpen() && !actuator->isClosed() && (!actuator->isMoveEnabled() || actuator->isOpening())){
                        //Starts closing the pod
                        actuator->extend();

                        //Send a message to the ground station
                        strcpy(msgPtr, "Extending actuator of ");
                        strcat(msgPtr, actuator->getName());
                        HAB_Logging::printLogln(msgPtr);
                        sendGSmessage(msgPtr);                      
                                                
                        //Optional picture
                    }
                    //Else if overridden close, and is closed
                    else if(actuator->isActuatorOverridden() && !actuator->isActuatorOverrideOpen() && actuator->isClosed()){

                        //If overridden close or not overridden, and move is enabled: halts (avoids disabling in case of overridden-open), take picture
                        if((!actuator->isActuatorOverridden() || (actuator->isActuatorOverridden() && !actuator->isActuatorOverrideOpen())) && actuator->isMoveEnabled()){
                            //If this is the first time in here, records the time
                            if(!isHalting){
                                isHalting = true;
                                limitReachedTime = millis();
                            }
                            //Once the additional push time has elapsed, halt the actuator and capture an image
                            else if(isHalting && (millis() - limitReachedTime) >= ADDITIONAL_PUSH_TIME){
                                isHalting = false;
                                
                                //Halts the actuator
                                actuator->halt();

                                //Send a message to the ground station
                                strcpy(msgPtr, "Halting actuator of ");
                                strcat(msgPtr, actuator->getName());
                                HAB_Logging::printLogln(msgPtr);
                                sendGSmessage(msgPtr);
                                
                                //Creates the name of the image and attempts capture (DOS 8.3 format)
                                strcpy(imgNamePtr, "");
                                strcat(imgNamePtr, itoa(activeIndex, genStringPtr, 10)); strcat(imgNamePtr, "_C.jpg"); 
                                _cam->captureImage(imgNamePtr, 0);
                            }                       
                        }       
                    }       

            //----------------------------------------------------------\
            //Heating---------------------------------------------------|
                //If its not overridden
                if(!actuator->isHeaterOverridden()){
                    //If we're below the temperature limit and not currently heating: start heating
                    if(actuator->getTemperature() <= minTemp && !actuator->isHeatEnabled()){
                         actuator->startHeating();            
                    }
                    //Else if we're above the temperature limit and currently heating: stop heating
                    else if(actuator->getTemperature() > maxTemp && actuator->isHeatEnabled()){
                        actuator->stopHeating();
                    }
                }
                else{
                    //If heater is not already doing what we want (check its status first), then set it
                    if(actuator->isHeaterOverrideEnabled() && !actuator->isHeatEnabled()){
                        actuator->startHeating();
                    }
                    else if(!actuator->isHeaterOverrideEnabled() && actuator->isHeatEnabled()){
                        actuator->stopHeating();
                    }
                }
        }

    /*-------------------------------------------------------------------------------------*\
    |   Name:       getPodIndex                                                             |
    |   Purpose:    Gets the index of a pod by name. -1 if no such pod.                     |
    |   Arguments:  char*                                                                   |
    |   Returns:    int8_t                                                                  |
    \*-------------------------------------------------------------------------------------*/
        int8_t getPodIndex(char* podName){
            //If "NONE" given, returns a not used index
            if(strcmp(podName, "NONE") == 0){ return act_arr_len; }

            //Else searches for the pod name
            for(int i = 0; i != act_arr_len; i++){ 
                //If it finds the actuator return its index
                if(strcmp(podName, _actArray[i].getName()) == 0){
                    return i;
                }
            }
            
            //Else return -1 (not found)
            return -1;
        }

    /*-------------------------------------------------------------------------------------*\
    |   Name:       recievePacketsUDP                                                       |
    |   Purpose:    Reads in UDP packets and deals with them accordingly.                   |
    |   Arguments:  void                                                                    |
    |   Returns:    void                                                                    |
    \*-------------------------------------------------------------------------------------*/
        void recievePacketsUDP(){
            //Gets the size of the packet (0 if no packet)
            int pktSize = _conn.parsePacket();

            //If there was a packet
            if(pktSize){
                //Read the packet from the buffer
                _conn.read(rcvBuffer, UDP_TX_PACKET_MAX_SIZE);
                //Serial.print("Received message : ");
                //Serial.println(rcvBuffer);

                //Attempts to find the sender, if it was listed in the packet
                char* msgPtr = strtok(rcvBuffer, FIELD_DELIMITER);

                //If PRISM, GPS or GROUNDSTATION packets, interpret them
                if(strcmp(msgPtr, PRISM_NAME) == 0){
                    msgPtr = strtok(NULL, FIELD_DELIMITER);
                    //Check if its a GPS packet and parse it

                    //Parse extra fields...
                    msgPtr = strtok(NULL, FIELD_DELIMITER);
                    msgPtr = strtok(NULL, FIELD_DELIMITER);

                    if(strcmp(msgPtr, GPS_NAME) == 0 && CSA_GPS_enabled){
                        //Latitude
                        msgPtr = strtok(NULL, FIELD_DELIMITER);             
                        _CSAGPSreadings.latitude = atof(msgPtr);

                        //Longitude
                        msgPtr = strtok(NULL, FIELD_DELIMITER);
                        _CSAGPSreadings.longitude = atof(msgPtr);

                        //Altitude
                        msgPtr = strtok(NULL, FIELD_DELIMITER);
                        _CSAGPSreadings.altitude = atof(msgPtr);
                    }
                }
                else if(strcmp(msgPtr, GROUNDSTATION_NAME) == 0 && HAB_GPS_enabled){
                    msgPtr = strtok(NULL, FIELD_DELIMITER);

                    //Converts all ASCII characters in the packet to upper case and ends the string if non-ascii characters found
                    for(uint16_t i = 0; sizeof(msgPtr); i++){
                        //If its a non-ascii character, replace it with the null terminator and break
                        if((uint32_t)msgPtr[i] > 255 || (uint32_t)msgPtr[i] == 0){
                            msgPtr[i] = '\0';
                            break;
                        }
    
                        //Converts the character to upper case
                        msgPtr[i] = toupper(msgPtr[i]);
                     }
    
                    //If it was a heartbeat packet, record the last time
                    if(strcmp(msgPtr, "HBT") == 0){
                        lastHeartbeat = millis();
                        if(noConnection){
                            HAB_Logging::printLogln("Connection obtained!");
                            noConnection = false;
                        }                   
                    }
                    //If not a heartbeat, attempt to interpret it as a command
                    else{
                        handleCommand(msgPtr);
                    }
                }

                //Wipes the buffer
                for(int i=0;i<UDP_TX_PACKET_MAX_SIZE;i++)rcvBuffer[i] = 0;
            }
            
            //Check the last heartbeat time
            if((millis() - lastHeartbeat) > HEARTBEAT_TIMEOUT && !noConnection){               
                noConnection = true;
                HAB_Logging::printLogln("Connection lost!");

                //Releases any actuator overrides+
                if(_actArray[activeIndex].isActuatorOverridden()){
                    _actArray[activeIndex].overrideActuatorRelease();
                }
                //Releases heater overrides if its set to 'OFF' (does not if overridden to 'ON')
                if(_actArray[activeIndex].isHeaterOverridden() && !_actArray[activeIndex].isHeaterOverrideEnabled()){
                    _actArray[activeIndex].overrideHeaterRelease(); 
                }                  
            }

            //CSA GPS01 timeout
            if((millis() - lastGPS01) > CSA_GPS_TIMEOUT && !noGPS01Connection){
                noGPS01Connection = true;
                HAB_Logging::printLogln("GPS01 connection lost!");
                sendGSmessage("GPS01 connection lost!");
            }

            //If no connection, attempts to reinitialize it every second
            if((millis() - lastInit) > RECONNECT_DELAY && noConnection){
                lastInit = millis();
                sendGSmessage("INTLZ", true);
            }
        }

    /*-------------------------------------------------------------------------------------*\
    |   Name:       handleCommands                                                          |
    |   Purpose:    Interprets the given string and executes it if it is a command.         |
    |   Arguments:  char*                                                                   |
    |   Returns:    void                                                                    |
    \*-------------------------------------------------------------------------------------*/        
        void handleCommand(char* command){

            //----------------------------------------------------------\
            //Parse the command-----------------------------------------|
                //Used for replying back the result
                bool validCommand = true;

                //Outputs the recieved command
                HAB_Logging::printLog("GROUNDSTATION : ");
                HAB_Logging::printLogln(command, "");
                sendGSmessage(command);
                
                //Gets the first argument
                char* argPtr = strtok(command, COMMAND_DELIMITER);
                strncpy(firstArg, argPtr, ARGUMENT_MAX_LENGTH);
    
                //Gets the second argument
                argPtr = strtok(NULL, COMMAND_DELIMITER);
                strncpy(secondArg, argPtr, ARGUMENT_MAX_LENGTH);
    
                //Gets the third argument
                argPtr = strtok(NULL, COMMAND_DELIMITER);
                strncpy(thirdArg, argPtr, ARGUMENT_MAX_LENGTH);

            //----------------------------------------------------------\
            //Execute the command---------------------------------------|
    
                //Actuators-------------------------------------------------|
                    if(!strcmp(firstArg, "SET_ACTIVE")){
                        if(strcmp(secondArg, "") != 0 && getPodIndex(secondArg) != -1){
                            //Disable the last actuator
                            if(activeIndex < act_arr_len){
                                _actArray[activeIndex].deactivateAll();
                            }
                            activeIndex = getPodIndex(secondArg); switchForced = true; }
                        else{
                            validCommand = false; }                           
                    }
                    //else if(!strcmp(firstArg, "RELEASE_ACTIVE")) { switchForced = true; } 
                    //else if(!strcmp(firstArg, "OVR_ACT_RELEASE")){ if(activeIndex < act_arr_len) _actArray[activeIndex].overrideActuatorRelease(); }
                    else if(!strcmp(firstArg, "OVR_ACT_HALT"))   { if(activeIndex < act_arr_len) _actArray[activeIndex].overrideActuatorHalt();    }
                    else if(!strcmp(firstArg, "OVR_ACT_OPEN"))   {
                        if(activeIndex < act_arr_len){
                            if(!_actArray[activeIndex].isLocked()){
                                _actArray[activeIndex].overrideActuatorOpen();
                            }
                            else{
                                validCommand = false;
                                HAB_Logging::printLogln("Actuator is locked!");
                                sendGSmessage("Actuator is locked!");
                            }
                        }   
                    }
                    else if(!strcmp(firstArg, "OVR_ACT_CLOSE"))  {
                        if(activeIndex < act_arr_len){
                            _actArray[activeIndex].overrideActuatorClose();
                            _actArray[activeIndex].setLock(true);
                            HAB_Logging::printLogln("Locking actuator!");
                            sendGSmessage("Locking actuator!");
                        }
                    }                  
                    //Locks and unlocks the actuators
                    else if(!strcmp(firstArg, "ACT_ENABLE_LOCK")) { if(activeIndex < act_arr_len) _actArray[activeIndex].setLock(true); }
                    else if(!strcmp(firstArg, "ACT_DISABLE_LOCK")){ if(activeIndex < act_arr_len) _actArray[activeIndex].setLock(false); }
                    
                //Heaters---------------------------------------------------|
                    else if(!strcmp(firstArg, "SET_MIN_TEMP")){
                        if(strcmp(secondArg, "") != 0 && atof(secondArg) >= -20 && atof(secondArg) <= 30){ //We allow a 50 degree range. This sets the minimum for ALL heaters.
                            minTemp = atof(secondArg); } 
                        else{
                            validCommand = false; }
                    }
                    else if(!strcmp(firstArg, "SET_MAX_TEMP")){
                        if(strcmp(secondArg, "") != 0 && atof(secondArg) >= -20 && atof(secondArg) <= 30){ //We will allow a 50 degree range. This sets the maximum for ALL heaters.
                            maxTemp = atof(secondArg); } 
                        else{
                            validCommand = false; }
                    }
                    else if(!strcmp(firstArg, "OVR_HEAT_ENABLE")) { if(activeIndex < act_arr_len) _actArray[activeIndex].overrideHeaterEnable();  }
                    else if(!strcmp(firstArg, "OVR_HEAT_DISABLE")){ if(activeIndex < act_arr_len) _actArray[activeIndex].overrideHeaterDisable(); }
                    else if(!strcmp(firstArg, "OVR_HEAT_RELEASE")){ if(activeIndex < act_arr_len) _actArray[activeIndex].overrideHeaterRelease(); }

                    //Enable or disable use of the HAB and CSA GPS units
                    //else if(!strcmp(firstArg, "HAB_GPS_ENABLE")) { HAB_GPS_enabled = true;  }
                    //else if(!strcmp(firstArg, "HAB_GPS_DISABLE")){ HAB_GPS_enabled = false; }
                    //else if(!strcmp(firstArg, "CSA_GPS_ENABLE")) { CSA_GPS_enabled = true;  }
                    //else if(!strcmp(firstArg, "CSA_GPS_DISABLE")){ CSA_GPS_enabled = false; }

                    //End flight
                    else if(!strcmp(firstArg, "SET_DESCENDING")){ isDescending = true; }
                    else if(!strcmp(firstArg, "HAB_END_FLIGHT")){ sendGSmessage("Ending flight!"); exit(0); }

                //Else if not any of those, it is invalid
                else{ validCommand = false; }

            //----------------------------------------------------------\
            //Send result message---------------------------------------|
                HAB_Logging::printLogln(validCommand ? "Command executed!" : "Invalid command!");
                sendGSmessage(validCommand ? "Command executed!" : "Invalid command!");      
        }

    /*-------------------------------------------------------------------------------------*\
    |   Name:       sendGSmessage                                                           |
    |   Purpose:    Sends a message to the ground station.                                  |
    |   Arguments:  char*                                                                   |
    |   Returns:    void                                                                    |
    \*-------------------------------------------------------------------------------------*/
        void sendGSmessage(const char* msg, bool ignoreConn = false){
            if(!noConnection || ignoreConn){
                strcpy(genStringPtr, "[EVENT]");
                strcat(genStringPtr, HAB_Logging::getTimestamp());
                strcat(genStringPtr, msg);
                _conn.beginPacket(_GSIP1, GS1_PORT);
                _conn.write(genStringPtr);
                _conn.endPacket();

                _conn.beginPacket(_GSIP2, GS2_PORT);
                _conn.write(genStringPtr);
                _conn.endPacket();
            }
        }
        
    /*-------------------------------------------------------------------------------------*\
    |   Name:       sendTelemetry                                                           |
    |   Purpose:    Sends telemetry station to our groundstation and CSA's PRISM.           |
    |   Arguments:  void                                                                    |
    |   Returns:    void                                                                    |
    \*-------------------------------------------------------------------------------------*/
        void sendTelemetry(){
            if(!noConnection){
                //Formats the packet to PRISM's standards
                strcpy(sendBuffer, ",,");
                strcat(sendBuffer, _gps->getDate(genStringPtr));
                strcat(sendBuffer, " ");
                strcat(sendBuffer, HAB_Logging::getTimeFormatted());
                strcat(sendBuffer, ",HAB,");
                strcat(sendBuffer, dtostrf(_HABGPSreadings.altitude,    6, 3, genStringPtr)); strcat(sendBuffer, ",");
                strcat(sendBuffer, dtostrf(_HABGPSreadings.speed,       6, 3, genStringPtr)); strcat(sendBuffer, ",");
                strcat(sendBuffer, dtostrf(_HABGPSreadings.longitude,   6, 3, genStringPtr)); strcat(sendBuffer, ",");
                strcat(sendBuffer, dtostrf(_HABGPSreadings.latitude,    6, 3, genStringPtr)); strcat(sendBuffer, ",");
                strcat(sendBuffer, dtostrf(_CSAGPSreadings.altitude,    6, 3, genStringPtr)); strcat(sendBuffer, ",");
                strcat(sendBuffer, dtostrf(_CSAGPSreadings.longitude,   6, 3, genStringPtr)); strcat(sendBuffer, ",");
                strcat(sendBuffer, dtostrf(_CSAGPSreadings.latitude,    6, 3, genStringPtr)); strcat(sendBuffer, ",");
                strcat(sendBuffer, dtostrf(_BMEreadings.temperature, 6, 3, genStringPtr)); strcat(sendBuffer, ",");
                strcat(sendBuffer, dtostrf(_BMEreadings.pressure,    6, 3, genStringPtr)); strcat(sendBuffer, ",");
                strcat(sendBuffer, dtostrf(_BMEreadings.humidity,    6, 3, genStringPtr));
                
                
                //Statuses of each actuator
                for(int i = 0; i != act_arr_len; i++){
                    strcat(sendBuffer, ",");
                    strcat(sendBuffer, dtostrf(_actReadingsArray[i].position, 6, 3, genStringPtr));
                    strcat(sendBuffer, ",");
                    strcat(sendBuffer, dtostrf(_actReadingsArray[i].temperature, 6, 3, genStringPtr));
                    strcat(sendBuffer, ",");
                    //Status of actuator override: auto(none), open, close
                    strcat(sendBuffer, (_actArray[i].isActuatorOverridden() ? (_actArray[i].isActuatorOverrideOpen() ? "1" : "0") : "2")); //OVR_OPEN(1), OVR_CLOSE(0), AUTO(2)
                    strcat(sendBuffer, ",");
                    //Status of heater override: auto(none), enabled, disabled
                    strcat(sendBuffer, (_actArray[i].isHeaterOverridden() ? (_actArray[i].isHeaterOverrideEnabled() ? "1" : "0") : "2")); //OVR_ENABLE(1), OVR_DISABLE(0), AUTO(2)
                }
    
                //Appends the end of the packet
                strcat(sendBuffer, "\r\n");
                //strcat(sendBuffer, 0x0D); strcat(rcvBuffer, 0x0A);

                //Sends the packet to our first groundstation
                _conn.beginPacket(_GSIP1, GS1_PORT);
                _conn.write(sendBuffer);
                _conn.endPacket();

                //Sends the packet to our second groundstation
                _conn.beginPacket(_GSIP2, GS2_PORT);
                _conn.write(sendBuffer);
                _conn.endPacket();
                
                //Sends the packet to PRISM
                //_conn.beginPacket(_PRISMIP, PRISM_PORT);
                //_conn.write(sendBuffer);
                //_conn.endPacket();
            }       
        }

    /*-------------------------------------------------------------------------------------*\
    |   Name:       printHeader                                                             |
    |   Purpose:    Prints a nice header.                                                   |
    |   Arguments:  void                                                                    |
    |   Returns:    void                                                                    |
    \*-------------------------------------------------------------------------------------*/
        void printHeader(){
            HAB_Logging::printLogln(" ############################################################", "");
            HAB_Logging::printLogln("##                                                          ##", "");
            HAB_Logging::printLogln("#               Western University HAB Project               #", "");
            HAB_Logging::printLogln("#                                                            #", "");
            HAB_Logging::printLogln("#                    Flight Software Logs                    #", "");
            HAB_Logging::printLogln("##                                                          ##", "");
            HAB_Logging::printLogln(" ############################################################", "");
        }

    /*-------------------------------------------------------------------------------------*\
    |   Name:       printInfo                                                               |
    |   Purpose:    Prints some useful info.                                                |
    |   Arguments:  void                                                                    |
    |   Returns:    void                                                                    |
    \*-------------------------------------------------------------------------------------*/
        void printInfo(){
            //Print author and team info
            HAB_Logging::printLogln("\r\nAuthor:", ""); HAB_Logging::printLogln("---------------", ""); HAB_Logging::printLogln("Stephen Amey", "");
            //Add rest of info here
        
            //Print the date from the GPS
            HAB_Logging::printLogln("\r\n\r\nFlight date:", ""); HAB_Logging::printLogln("---------------", ""); HAB_Logging::printLogln(_gps->getDate(genStringPtr), "");
        
            //Print out the camera info
            HAB_Logging::printLogln("\r\n\r\nCamera info:", ""); HAB_Logging::printLogln("---------------", ""); HAB_Logging::printLogln(_cam->getInfo(genStringPtr), "");
        
            //Print out GPS info
            HAB_Logging::printLogln("", "");
            _gps->printInfo();
               
            //Print out final header
            HAB_Logging::printLogln("\r\n\r\n##############################################################", "");
            HAB_Logging::printLogln(        "#                       Logging begins                       #", "");
            HAB_Logging::printLogln(        "##############################################################\r\n\r\n", "");
        }


    /*-------------------------------------------------------------------------------------*\
    |   Name:       checkStartupConditions                                                  |
    |   Purpose:    Holds the program until all startup conditions passed.                  |
    |   Arguments:  void                                                                    |
    |   Returns:    void                                                                    |
    \*-------------------------------------------------------------------------------------*/
        void checkStartupConditions(){

            //----------------------------------------------------------\
            //Telemetry check-------------------------------------------|
                //Sets up the ethernet
                Ethernet.begin(_localMAC, _localIP, dns, gate, sub);
                Ethernet.setRetransmissionCount(0);
                
                while(!_conn.begin(LOCAL_PORT));
                //_conn.flush(); //?
                HAB_Logging::printLogln("UDP OKAY");

                //Obtain a connection to the ground station               
                while(noConnection){
                    sendGSmessage("INTLZ", true);
                    recievePacketsUDP();
                    delay(500);
                }
                HAB_Logging::printLogln("CONNECTION OKAY");
                sendGSmessage("CONNECTION OKAY");

                //Sets status LED
                //digitalWrite(TELEMETRY_LED, HIGH);

            //----------------------------------------------------------\
            //Pod check-------------------------------------------------|
                for(int i = 0; i != act_arr_len; i++){
                    if(!_actArray[i].isClosed()){
                        strcpy(msgPtr, _actArray[i].getName());
                        strcat(msgPtr, " is not closed.");
                        HAB_Logging::printLogln(msgPtr);              
                        sendGSmessage(msgPtr);
                        //while(!podBypass){ recievePacketsUDP(); }
                    }
                }
                HAB_Logging::printLogln("PODS OKAY");              
                sendGSmessage("PODS OKAY");
                
                //Sets status LED
                //digitalWrite(POD_LED, HIGH);
            
            //----------------------------------------------------------\
            //Logging check---------------------------------------------|
                //while(!HAB_Logging::checkReady() && !loggingBypass){ recievePacketsUDP(); }
                    HAB_Logging::printLogln("LOGGING OKAY");              
                    sendGSmessage("LOGGING OKAY");
                    
                //Sets status LED
                //digitalWrite(LOGGING_LED, HIGH);
    
            //----------------------------------------------------------\
            //Camera check----------------------------------------------|
                if(!_cam->getReadyStatus()){
                    HAB_Logging::printLogln("CAMERA OKAY");              
                    sendGSmessage("CAMERA OKAY");

                    //Sets status LED
                    //digitalWrite(CAMERA_LED, HIGH);
                }
                else{
                    HAB_Logging::printLogln("CAMERA FAILED");              
                    sendGSmessage("CAMERA FAILED");
                }     
    
            //----------------------------------------------------------\
            //BME check-------------------------------------------------|
                if(BMPstatus = _bme.begin()){
                    HAB_Logging::printLogln("BME OKAY");              
                    sendGSmessage("BME OKAY");
                    
                    //Sets status LED
                    //digitalWrite(BME_LED, HIGH);
                }
                else{
                    HAB_Logging::printLogln("BME FAILED");              
                    sendGSmessage("BME FAILED");
                }      

            //----------------------------------------------------------\
            //GPS check-------------------------------------------------|
                if(!_gps->isModeSet()){
                    HAB_Logging::printLogln("GPS MODE OKAY");              
                    sendGSmessage("GPS MODE OKAY"); 
                }
                else{
                    HAB_Logging::printLogln("GPS MODE FAILED");              
                    sendGSmessage("GPS MODE FAILED"); 
                }
                    
     
                if(!_gps->getLockStatus()){           
                    HAB_Logging::printLogln("GPS LOCK OKAY");              
                    sendGSmessage("GPS LOCK OKAY");

                    //Sets status LED
                    //digitalWrite(GPS_LED, HIGH);
                }
                else{
                    HAB_Logging::printLogln("GPS LOCK FAILED");              
                    sendGSmessage("GPS LOCK FAILED");
                }     

           /*delay(3000);
           digitalWrite(TELEMETRY_LED, LOW);
           digitalWrite(POD_LED, LOW);
           digitalWrite(LOGGING_LED, LOW);
           digitalWrite(CAMERA_LED, LOW);
           digitalWrite(BME_LED, LOW);
           digitalWrite(BME_LED, LOW);
           digitalWrite(GPS_LED, LOW);*/
        }
