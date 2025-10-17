#include "Solenoid.hpp"

Solenoid::Solenoid() {}


// PUBLIC

void Solenoid::begin(Preset preset) {
    this->setPreset(preset);
}

SolenoidError Solenoid::setLength(uint32_t length) {
    if (length > MAX_LENGTH || length < 0) {
        return SolenoidError::VALUE_ERROR;
    }
    this->_length = length;
    return SolenoidError::NO_ERROR;
}

SolenoidError Solenoid::setRadius(uint32_t radius) {
    if (radius > MAX_LENGTH || radius < 0) {
        return SolenoidError::VALUE_ERROR;
    }
    this->_radius = radius;
    return SolenoidError::NO_ERROR;
}

SolenoidError Solenoid::setInductance(uint32_t inductance) {
    if (inductance > MAX_INDUCTANCE || inductance < 0) {
        return SolenoidError::VALUE_ERROR;
    }
    this->_inductance = inductance;
    return SolenoidError::NO_ERROR;
}

SolenoidError Solenoid::setGauge(WireGauge gauge) {
    this->_gauge = gauge;
    return SolenoidError::NO_ERROR;
}

uint32_t Solenoid::getLength() {
    return _length;
}

uint32_t Solenoid::getRadius() {
    return _radius;
}

uint32_t Solenoid::getInductance() {
    return _inductance;
}

WireGauge Solenoid::getGauge() {
    return _gauge;
}

void Solenoid::setPreset(Preset preset) {
    switch (preset) {
        case Preset::A:
            this->setLength((uint32_t) 500); // 5cm
            this->setRadius((uint32_t) 50); // 0.5cm
            this->setInductance((uint32_t) 4000); // 40mH
            this->setGauge(WireGauge::AWG24);
        break;
        case Preset::B:
            this->setLength((uint32_t) 500); // 5cm
            this->setRadius((uint32_t) 50); // 0.5cm
            this->setInductance((uint32_t) 8000); // 80mH
            this->setGauge(WireGauge::AWG24);
        break;
        case Preset::C:
            this->setLength((uint32_t) 300); // 3cm
            this->setRadius((uint32_t) 50); // 0.5cm
            this->setInductance((uint32_t) 4000); // 40mH
            this->setGauge(WireGauge::AWG24);
        break;
        case Preset::D:
            this->setLength((uint32_t) 100); // 1cm
            this->setRadius((uint32_t) 100); // 1cm
            this->setInductance((uint32_t) 10); // 0.1mH
            this->setGauge(WireGauge::AWG24);
        break;
        case Preset::None:
            this->setLength((uint32_t) 0); // 0cm
            this->setRadius((uint32_t) 0); // 0cm
            this->setInductance((uint32_t) 0); // 0mH
            this->setGauge(WireGauge::AWG24);
        break;
        default: // debug case
            this->setLength((uint32_t) 1234); // 0.1234m
            this->setRadius((uint32_t) 123); // 0.0123m
            this->setInductance((uint32_t) 1234567); // 12.34567H
            this->setGauge(WireGauge::AWG18);
    }
}

uint32_t Solenoid::turnsPerPass() {
    if (_length == 0) {
        return 0;
    }
    // (_length / 10000) / (_gauge / 1000000)
    return (_length / _gauge) * 100;
}

String Solenoid::lengthString() {
    return formatVal(_length, MAX_LENGTH);
}

String Solenoid::radiusString() {
    return formatVal(_radius, MAX_RADIUS);
}

String Solenoid::inductanceString() {
    return formatVal(_inductance, MAX_INDUCTANCE);
}

String Solenoid::gaugeString() {
    switch(_gauge) {
        case WireGauge::AWG18: return "AWG18";
        case WireGauge::AWG19: return "AWG19";
        case WireGauge::AWG20: return "AWG20";
        case WireGauge::AWG21: return "AWG21";
        case WireGauge::AWG22: return "AWG22";
        case WireGauge::AWG23: return "AWG23";
        case WireGauge::AWG24: return "AWG24";
        case WireGauge::AWG25: return "AWG25";
        case WireGauge::AWG26: return "AWG26";
        case WireGauge::AWG27: return "AWG27";
        case WireGauge::AWG28: return "AWG28";
        case WireGauge::AWG29: return "AWG29";
        case WireGauge::AWG30: return "AWG30";
    }
}

// PRIVATE

String formatVal(uint32_t num, uint32_t max) {
  uint8_t maxLength = String(max).length() + 1; // Cannot be greater than 10
  String returnString = "";
  String numberString = String(num);

  // Add leading zeros to match length
  for (size_t i = 0; i < maxLength - (numberString.length() + 1); i++) {
      returnString += "0";
  }
  
  // Add decimal point with 2 positions of precision
  returnString += numberString.substring(0, numberString.length() - 3);
  returnString +=  ".";
  returnString += numberString.substring(numberString.length() - 2);

  return returnString;
}

void Solenoid::updateTurns() {
    this->_numTurns = round(sqrt((_inductance * _length) / (_radius * _radius * K))) * UT_SCALING_FACTOR;
}