
//--------------------------------------------------------------------------------\
//GPS-----------------------------------------------------------------------------|
	
	#define GPS_BAUD 9600
	#define GPS_MAX_AGE 110
	#define GPS_RX_PIN 53
	#define GPS_TX_PIN 52


//--------------------------------------------------------------------------------\
//SD card-------------------------------------------------------------------------|
	
	#define SD_CHIPSELECT 4


//--------------------------------------------------------------------------------\
//Camera--------------------------------------------------------------------------|

	//Camera 1
	#define CAM1_RX_PIN A14
	#define CAM1_TX_PIN A14
	
	
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
//Thermistors---------------------------------------------------------------------|	

	#define SERIESRESISTOR 10000  
	#define THERMISTORNOMINAL 10000   
	#define TEMPERATURENOMINAL 25 
	#define BCOEFFICIENT 3950


//--------------------------------------------------------------------------------\
//Actuator, heaters, and thermistor pins------------------------------------------|

	//Pod 1
	#define HEAT1_EN 22
	#define ACT1_EN 23
	#define ACT1_PUSH 24
	#define ACT1_PULL 25
	#define THERMISTOR1 A8
	#define ACT1_POS A9
	
	//Pod 2
	#define HEAT2_EN 28
	#define ACT2_EN 29
	#define ACT2_PUSH 30
	#define ACT2_PULL 31
	#define THERMISTOR2 A10
	#define ACT2_POS A11
	
	//Pod 3
	#define HEAT3_EN 34
	#define ACT3_EN 35
	#define ACT3_PUSH 36
	#define ACT3_PULL 37
	#define THERMISTOR3 A12
	#define ACT3_POS A13
	
	//Pod 4
	#define HEAT4_EN 40
	#define ACT4_EN 41
	#define ACT4_PUSH 42
	#define ACT4_PULL 43
	#define THERMISTOR4 A14
	#define ACT4_POS A15