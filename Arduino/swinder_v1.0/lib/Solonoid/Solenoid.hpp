#ifndef SOLENOID_HPP
#define SOLENOID_HPP

#include <Arduino.h>

#define MAX_LENGTH 2000 // 0.2m stored with 0.01cm precision. Divide by 10000
#define MAX_INDUCTANCE 4000000 // 40H stored with 0.01mH precision. Divide by 100000
#define MAX_RADIUS 500 // 0.005m stored with 0.01cm precision. Divide by 10000

#define CORRECTION_VALUE 10000000 // TODO
#define K 394784

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

enum WireGauge {
    AWG20,
    AWG22,
    AWG24,
    AWG26,
    AWG28,
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
     * @brief Updates all values of solenoid to predefined presets
     * 
     * @param preset selected preset
     */
    void setPreset(Preset preset);

private:
    /**
     * @brief Calculates the number of turns required for solenoid
     * 
     * Equation: sqrt((inductance * length) / (R^2 * K))
     * K = 4 * pi^2 * 10^-7 = ~394784 * 10^-11 for ~1.76 * 10^12 error
     */
    void updateTurns();

    uint32_t _length = 0;
    uint32_t _radius = 0;
    uint32_t _inductance = 0;
    WireGauge _gauge = WireGauge::AWG20;
    uint32_t _numTurns = 0;
};

#endif