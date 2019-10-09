//So apparently redefinitions don't work if its declared last...
//Import this lib first, then use ifndef

//--------------------------------------------------------------------------------\
//General-------------------------------------------------------------------------|

	#define READINGS_TIME_STEP 1000	
	#define STOP_ALTITUDE 5000
	
	#define GROUNDSTATION_NAME "GROUNDSTATION"
	#define PRISM_NAME "PRISM"
	#define GPS_NAME "POS0"


//--------------------------------------------------------------------------------\
//Ethernet & UDP------------------------------------------------------------------|

	#define UDP_TX_PACKET_MAX_SIZE 300 //Is this a safe size?
	#define HEARTBEAT_TIMEOUT 10000
	#define GPS_TIMEOUT 10000 //Our Timeout
	#define CSA_GPS_TIMEOUT 30000 //CSA timeout
	#define RECONNECT_DELAY 1000
	#define ARGUMENT_MAX_LENGTH 20
	#define COMMAND_DELIMITER " "
	#define FIELD_DELIMITER ","
	#define MAX_TRANSMIT_ATTEMPTS 0 //Each additional attempt adds 200ms, which can delay the program a significant amount

	//Local MAC, IP, port
	#define MAC {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED}
	
	//Arduino
	#define LOCAL_IP_O1 172
	#define LOCAL_IP_O2 20
	#define LOCAL_IP_O3 4
	#define LOCAL_IP_O4 240
	#define LOCAL_PORT 10027
	
	//Groundstation 1
	#define GS1_IP_O1 172
	#define GS1_IP_O2 20
	#define GS1_IP_O3 3
	#define GS1_IP_O4 240
	#define GS1_PORT 54444
	
	//Groundstation 2
	#define GS2_IP_O1 172
	#define GS2_IP_O2 20
	#define GS2_IP_O3 3
	#define GS2_IP_O4 241
	#define GS2_PORT 54444
	
	//Remote IP, port (PRISM)
	#define PRISM_IP_O1 172
	#define PRISM_IP_O2 20
	#define PRISM_IP_O3 4
	#define PRISM_IP_O4 123 //200
	#define PRISM_PORT 10001
	

//--------------------------------------------------------------------------------\
//GPS-----------------------------------------------------------------------------|

	#define GPS_BAUD 9600
	#define GPS_MAX_AGE 110
	#define GPS_RX_PIN 38 //Any digital
	#define GPS_TX_PIN 10 //Recieve pin


//--------------------------------------------------------------------------------\
//SD card-------------------------------------------------------------------------|
	
	#define SD_CHIPSELECT 4


//--------------------------------------------------------------------------------\
//Camera--------------------------------------------------------------------------|

	#define WRITES_PER_LOOP 3

	//Camera 1
	#define CAM1_RX_PIN 39 //Any digital
	#define CAM1_TX_PIN 11 //Recieve pin
	
	
//--------------------------------------------------------------------------------\
//BME280--------------------------------------------------------------------------|	

	//SPI (Interferes with SD card)
	//#define BME_SCK 13
    //#define BME_MISO 12
    //#define BME_MOSI 11
    //#define BME_CS 10
	
	//I2C (Set up wiring for this, no software config required)
	//SDI to 20
	//SCK to 21
	
//--------------------------------------------------------------------------------\
//Fan-----------------------------------------------------------------------------|	
	
	//What other pins for this?
	#define FAN_SPEED_PIN 12
	

//--------------------------------------------------------------------------------\
//Thermistors---------------------------------------------------------------------|	

	#define SERIESRESISTOR 10000  
	#define THERMISTORNOMINAL 10000   
	#define TEMPERATURENOMINAL 25 
	#define BCOEFFICIENT 3950


//--------------------------------------------------------------------------------\
//Actuator, heaters, and thermistor pins------------------------------------------|

	#define MIN_ACTUATOR_TEMP -10
	#define MAX_ACTUATOR_TEMP 0
	
	//#define POD_OPEN 0 //10 //THESE DON'T WORK HERE?? 
	//#define POD_CLOSED 1 //1015 //Modify them in actuator.h
	#define ADDITIONAL_PUSH_TIME 5000

	//Pod 1
	#define HEAT1_EN 22
	#define ACT1_EN 23
	#define ACT1_PUSH 24
	#define ACT1_PULL 25
	#define THERMISTOR1 A8
	#define ACT1_POS A9
	
	//Pod 2
	#define HEAT2_EN 26
	#define ACT2_EN 27
	#define ACT2_PUSH 28
	#define ACT2_PULL 29
	#define THERMISTOR2 A10
	#define ACT2_POS A11
	
	//Pod 3
	#define HEAT3_EN 30
	#define ACT3_EN 31
	#define ACT3_PUSH 32
	#define ACT3_PULL 33
	#define THERMISTOR3 A12
	#define ACT3_POS A13
	
	//Pod 4
	#define HEAT4_EN 34
	#define ACT4_EN 35
	#define ACT4_PUSH 36
	#define ACT4_PULL 37
	#define THERMISTOR4 A14
	#define ACT4_POS A15
	
//--------------------------------------------------------------------------------\
//Status LEDs---------------------------------------------------------------------|

	#define TELEMETRY_LED 40
	#define POD_LED 41
	#define LOGGING_LED 42
	#define CAMERA_LED 43
	#define BME_LED 44
	#define GPS_LED 45

