/**********************************
*        Bomberman3D Engine       *
*  Created by: Cmaranec (Kennny)  *
**********************************/
// Type defines

#ifndef BOMB_DEFINES_H
#define BOMB_DEFINES_H

#include <math.h>

#define PI 3.14159265f

typedef long  int64;
typedef int   int32;
typedef short int16;
typedef char  int8;
typedef unsigned long  uint64;
typedef unsigned int   uint32;
typedef unsigned short uint16;
typedef unsigned char  uint8;

typedef GLuint GLDisplayList;

#define MAKE_PAIR32(a,b) uint32(((uint32(a & 0xFFFF    ) << 16) | (uint32(b & 0xFFFF    ))))
#define MAKE_PAIR64(a,b) uint64(((uint64(a & 0xFFFFFFFF) << 32) | (uint64(b & 0xFFFFFFFF))))
#define HIPART32(a) uint16(((a & 0xFFFF0000) >> 16))
#define LOPART32(a) uint16((a & 0xFFFF))
#define HIPART64(a) uint32(((a & 0xFFFFFFFF00000000) >> 32))
#define LOPART64(a) uint32((a & 0xFFFFFFFF))

struct CVector3
{
    float x, y, z;

    float mySize()
    {
        return sqrt(pow(x,2)+pow(y,2)+pow(z,2));
    }
    void multiply(float a)
    {
        x *= a;
        y *= a;
        z *= a;
    }
    void unitMultiply(float a)
    {
        multiply(a/mySize());
    }
    CVector3 vectorMultiply(CVector3 second)
    {
        CVector3 result;
        result.x = y*second.z - z*second.y;
        result.y = z*second.x - x*second.z;
        result.z = x*second.y - y*second.x;

        return result;
    }
};

struct CVector2
{
    float x, y;

    float mySize()
    {
        return sqrt(pow(x,2)+pow(y,2));
    }
    void multiply(float a)
    {
        x *= a;
        y *= a;
    }
    void unitMultiply(float a)
    {
        multiply(a/mySize());
    }
};

#endif
