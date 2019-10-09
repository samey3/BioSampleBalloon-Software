#ifndef HAB_Structs_h
#define HAB_Structs_h

struct actuatorReadings {
	uint16_t position; //signed int
    float temperature;
    char actuatorStatusPtr[10] = "AUTO"; //OVR_CLOSE
    char heaterStatusPtr[13] = "AUTO"; //OVR_DISABLE
};
typedef struct actuatorReadings ActuatorReadings;

struct bmeReadings {
	float temperature; //signed int
    float pressure;
    float humidity;
};
typedef struct bmeReadings BMEReadings;

struct gpsReadings {  
    uint8_t second; //Seconds
    uint8_t minute; //Seconds
    uint8_t hour; //Seconds
	char dateTime[50];
    float speed; //Meters per second
    float altitude; //Meters
	float latitude; //Degrees
	float longitude; //Degrees
};
typedef struct gpsReadings GPSReadings;

#endif