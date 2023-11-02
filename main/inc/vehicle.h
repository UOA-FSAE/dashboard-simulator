//
// Created by tazukiswift on 29/06/23.
//

#ifndef DASHBOARD_VEHICLE_H
#define DASHBOARD_VEHICLE_H

#include <stdint.h>
#include <stdbool.h>

// TODO: This class should contain something to check if the value changed

//TS Data
typedef struct {

    uint8_t soc;            // 0 to 100%

    float voltage;            // 0.0 to 600.0V
    float current;            // -15.0A to 100.0A SIGNED

    float minVoltage;        // 0.0000 to 4.2000 V
    float avgVoltage;
    float maxVoltage;
    int numCells;

    uint8_t maxTemp;        // 0 to (hopefully)59 degC
    uint8_t avgTemp;
    uint8_t minTemp;

} TS_DATA;

//GLV Data
typedef struct {
    uint8_t soc;            // 0 to 100%
    float voltage;            // 0.0 to 30.0 V
    float current;            // 0.0A to 15.0A SIGNED
} GLV_DATA;

typedef struct {
    bool GLV;
    bool shutdown;
    bool precharging;
    bool precharged;
    bool RTDState;
} VEHICLE_STATE;

//Driver Data
typedef struct {
    int8_t steeringAngle;            // -100 to 100 degrees from center SIGNED
    uint8_t brakeBias;                // 0 to 100% forward bias
    uint8_t frontBrakePressure;    // 0 to 255 kPa
    uint8_t rearBrakePressure;    // 0 to 255 kPa
    uint8_t throttle;                // 0 to 100%
    uint8_t torque;                    // 0 to 210%
    uint16_t rpm;                    // 0 to 65,535 rpms
} DRIVER_DATA;

//Motor/Inverter/Drive System Data type
typedef struct {
    uint8_t gearboxTemp;            // 0 to 100 degC
    uint8_t motorTemp;                // 0 to 100 degC
    uint8_t inverterTemp;            // 0 to 100 degC
    bool derating;                    // Drive is derating for some reason
    bool driveActive;                // Drive is acrive
    uint16_t errorCode;                // AMK error code
} DRIVE_DATA;

//Fault and Error information, specifically including the bad ones which stop the car and are required by rules
typedef struct {
    //These interrupt the shutdown circuit
    bool IMD;
    bool AMS;
    bool PDOC;
    bool BSPD;

    //These set motor torque to, but do not interrupt the shutdown circuit
    bool throttlePlausability;
    bool brakeThrottleImplausability;
} FAULT_DATA;

//Race Data
typedef struct {
    uint32_t currentLapTime;    // 0 to 300,000 ms
    uint32_t bestLapTime;        // 0 to 300,000 ms
    uint32_t previousLapTime;   // 0 to 300,000 ms
    int32_t deltaLapTime;        // -300,000 to 300,000 ms
    uint8_t currentSpeed;        // 0 to 255 kph
    uint8_t lapNumber;            // o to 255 laps
    VEHICLE_STATE vehicleState;
} RACE_DATA;

typedef struct {
    GLV_DATA glv;            // We have one GLV system
    TS_DATA ts;                // We have one tractive system
    RACE_DATA race;            // We have one set of race data
    DRIVER_DATA driver;        // We have one driver
    DRIVE_DATA drive[4];        // We have 4 motors and 4 motor controllers
    FAULT_DATA errors;        // We have a lot of errors
} Vehicle_Data;

void resetDataStructure(Vehicle_Data *input_data);

#endif //DASHBOARD_VEHICLE_H
