
#ifndef MACHINE_H
#define MACHINE_H

#include <Arduino.h>
#include "Vector.h"

class CNC_machine
{
public:

    CNC_machine(uint8_t toolNumber);
    ~CNC_machine();
    bool move(int32_t x, int32_t y, int32_t z, uint8_t mask);
    bool extrude(uint8_t tool, int32_t ammount);
    void setRelativeCoords(bool isRelative);
    void setRelativeExtrusion(bool isRelative);

private:
    const uint8_t toolNum;

    Vec3 pos;
    int32_t* posE;

    Vec3 offset;
    int32_t* offsetE;

    bool absCoords;
    bool absCoordsE;

    // Need this?
    uint16_t feedrate_X;
    uint16_t feedrate_Y;
    uint16_t feedrate_Z;
    uint16_t feedrate_E;
};

#endif