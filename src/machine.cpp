
#include "machine.h"

Machine::Machine(uint8_t toolNumber):
    toolNum(toolNumber)
{
    posE = (int32_t*)calloc(toolNumber, sizeof(int32_t));
    offsetE = (int32_t*)calloc(toolNumber, sizeof(int32_t));
}

Machine::~Machine()
{
    if(posE) free(posE);
    if(offsetE) free(offsetE);
}

bool Machine::move(int32_t x, int32_t y, int32_t z, uint8_t mask)
{
    if(absCoords)
    {
        if(mask & 1) pos.x = x + offset.x;
        if(mask & 2) pos.y = y + offset.x;
        if(mask & 4) pos.z = z + offset.x;
    }
    else
    {
        if(mask & 1) pos.x += x;
        if(mask & 2) pos.y += y;
        if(mask & 4) pos.z += z;
    }
    
    return true;
}
bool Machine::extrude(uint8_t tool, int32_t ammount)
{
    if(tool >= toolNum) return false;
    if(absCoordsE)
    {
        posE[tool] = ammount;
    }
    else
    {
        posE[tool] += ammount;
    }
    return true;
}

void Machine::setRelativeCoords(bool isRelative)
{
    absCoords = !isRelative;
}

void Machine::setRelativeExtrusion(bool isRelative)
{
    absCoordsE = !isRelative;
}
