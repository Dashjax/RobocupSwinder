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

uint32_t Solenoid::getLength() {
    return _length;
}

uint32_t Solenoid::getRadius() {
    return _radius;
}

uint32_t Solenoid::getInductance() {
    return _inductance;
}

void Solenoid::setPreset(Preset preset) {
    switch (preset) {
        case Preset::A:
            this->setLength((uint32_t) 500); // 5cm
            this->setRadius((uint32_t) 50); // 0.5cm
            this->setInductance((uint32_t) 4000); // 40mH
        break;
        case Preset::B:
            this->setLength((uint32_t) 500); // 5cm
            this->setRadius((uint32_t) 50); // 0.5cm
            this->setInductance((uint32_t) 8000); // 80mH
        break;
        case Preset::C:
            this->setLength((uint32_t) 300); // 3cm
            this->setRadius((uint32_t) 50); // 0.5cm
            this->setInductance((uint32_t) 4000); // 40mH
        break;
        case Preset::D:
            this->setLength((uint32_t) 100); // 1cm
            this->setRadius((uint32_t) 100); // 1cm
            this->setInductance((uint32_t) 10); // 0.1mH
        break;
        case Preset::None:
            this->setLength((uint32_t) 0); // 0cm
            this->setRadius((uint32_t) 0); // 0cm
            this->setInductance((uint32_t) 0); // 0mH
        break;
        default: // debug case
            this->setLength((uint32_t) 1234); // 0.1234m
            this->setRadius((uint32_t) 123); // 0.0123m
            this->setInductance((uint32_t) 1234567); // 12.34567H
    }
}

uint32_t Solenoid::turnsPerPass() {
    if (_length == 0) {
        return 0;
    }
    // (_length / 10000) / (_gauge / 1000000)
    return (_length * 100) / _gauge;
}

String lengthString() {
    
}

// PRIVATE

void Solenoid::updateTurns() {
    this->_numTurns = sqrt((_inductance * _length) / (_radius * _radius * K)) * CORRECTION_VALUE;
}