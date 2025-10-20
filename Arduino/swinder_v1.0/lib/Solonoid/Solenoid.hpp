#ifndef SOLENOID_HPP
#define SOLENOID_HPP

#include <Arduino.h>

#define MAX_LENGTH 2000 // 0.2m stored with 0.01cm precision. Divide by 10000
#define MAX_INDUCTANCE 4000000 // 40H stored with 0.01mH precision. Divide by 100000
#define MAX_RADIUS 500 // 0.005m stored with 0.01cm precision. Divide by 10000

#define UT_SCALING_FACTOR 100000 // 10^5
#define K 394784 // K = 4 * pi^2 * 10^-7 = ~394784 * 10^-11 for ~1.76 * 10^12 error

enum SolenoidError {
    NO_ERROR = 0,
    VALUE_ERROR = 1,
};

enum Preset {
    A,
    B,
    C,
    D,
    None,
    Debug,
};

enum WireGauge { // Diameter, divide by 1000000 to get m
    AWG18 = 1020, // 1.020mm
    AWG19 = 910, // 0.910mm
    AWG20 = 810, // 0.810mm
    AWG21 = 720, // 0.720mm
    AWG22 = 643, // 0.643mm
    AWG23 = 574, // 0.574mm
    AWG24 = 511, // 0.511mm
    AWG25 = 450, // 0.450mm
    AWG26 = 404, // 0.404mm
    AWG27 = 361, // 0.361mm
    AWG28 = 320, // 0.320mm
    AWG29 = 290, // 0.290mm
    AWG30 = 254, // 0.254mm
};

class Solenoid {
public:
    /**
     * @brief Create a new instance of the solenoid driver.
     */
    Solenoid();

    /**
     * @brief Initializes a solenoid from the given preset to garuntee error free.
     * 
     * @param preset the Preset to default the values to
     */
    void begin(Preset preset);

    /**
     * @brief Getter for solenoid length
     * 
     * @returns length of solenoid
     */
    uint32_t getLength();

    /**
     * @brief Getter for solenoid radius
     * 
     * @returns radius of solenoid
     */
    uint32_t getRadius();

    /**
     * @brief Getter for solenoid inductance
     * 
     * @returns inductance of solenoid
     */
    uint32_t getInductance();

    /**
     * @brief Getter for solenoid wire gauge
     * 
     * @returns gauge of solenoid
     */
    WireGauge getGauge();

    /**
     * @brief Getter for number of turns required for solenoid
     * 
     * Only updates turns count on call
     * 
     * @returns number of turns for solenoid
     */
    uint32_t getTurns();

    /**
     * @brief Setter for solenoid length
     * 
     * @returns VAL_ERROR if input value is outside of expected range
     */
    SolenoidError setLength(uint32_t length);

    /**
     * @brief Setter for solenoid radius
     * 
     * @returns VAL_ERROR if input value is outside of expected range
     */
    SolenoidError setRadius(uint32_t radius);

    /**
     * @brief Setter for solenoid inductance
     * 
     * @returns VAL_ERROR if input value is outside of expected range
     */
    SolenoidError setInductance(uint32_t inductance);

    /**
     * @brief Setter for solenoid wire gauge
     * 
     * @returns VAL_ERROR if input value is outside of expected range
     */
    SolenoidError setGauge(WireGauge gauge);

    /**
     * @brief Updates all values of solenoid to predefined presets
     * 
     * @param preset selected preset
     */
    void setPreset(Preset preset);

    /**
     * @brief Returns the number of turns that can fit in one pass across the solenoid
     * 
     * @returns number of turns per pass
     */
    uint32_t turnsPerPass();

    /**
     * @brief Returns the string formatted version of length in cm.
     * 
     * @returns length as string
     */
    String lengthString();

    /**
     * @brief Returns the string formatted version of radius in cm.
     * 
     * @returns radius as string
     */
    String radiusString();

    /**
     * @brief Returns the string formatted version of inductance in mH.
     * 
     * @returns inductance as string
     */
    String inductanceString();

    /**
     * @brief Returns the string formatted version of wire gauge.
     * 
     * @returns gauge as string
     */
    String gaugeString();

    /**
     * @brief Returns the string formatted version of turn count
     * 
     * @returns turn count string
     */
    String turnsString();

private:
    /**
     * @brief Calculates the number of turns required for solenoid
     * 
     * Equation: sqrt((inductance * length) / (R^2 * K))
     */
    void updateTurns();

    /**
     * @brief General format from converting int values to strings
     * 
     * @param num int to be formatted
     * @param max maximum value num can be
     * @returns string format of given value
     */
    String formatVal(uint32_t num, uint32_t max);

    uint32_t _length = 0;
    uint32_t _radius = 0;
    uint32_t _inductance = 0;
    WireGauge _gauge = WireGauge::AWG24;
    uint32_t _numTurns = 0;
};

#endif