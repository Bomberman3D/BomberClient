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

static float numBounds(float value)
{
    if (fabs(value) < (1 / 1000000.0f))
        return 0;
    else
        return value;
}

/** \brief Struktura prostoroveho vektoru
 */
struct CVector3
{
    CVector3() {};
    CVector3(float nx, float ny, float nz): x(nx), y(ny), z(nz) {};

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
        if (mySize() == 0)
            return;

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
    void makeNull()
    {
        x = 0;
        y = 0;
        z = 0;
    }

    CVector3 operator+(const CVector3 &sec)
    {
        x = x + sec.x;
        y = y + sec.y;
        z = z + sec.z;
        return (*this);
    }
    CVector3 operator-(const CVector3 &sec)
    {
        x = x - sec.x;
        y = y - sec.y;
        z = z - sec.z;
        return (*this);
    }
    CVector3 operator*(float sec)
    {
        x = x * sec;
        y = y * sec;
        z = z * sec;
        return (*this);
    }
    float operator*(const CVector3 &sec)
    {
        return numBounds((x*sec.x)+(y*sec.y)+(z*sec.z));
    }

    CVector3 rotate(CVector3 &axis, float angle)
    {
        CVector3 v = *this;

        return ((v - axis * (axis * v)) * cos(angle)) + (axis.vectorMultiply(v) * sin(angle)) + (axis * (axis * v));
    }
};

/** \fn CVector3::CVector3
 *  \brief Implicitni konstruktor
 */

/** \fn CVector3::CVector3(float nx, float ny, float nz)
 *  \brief Konstruktor s implicitne zadanymi souradnicemi
 */

/** \var CVector3::x
 *  \brief Xova souradnice vektoru
 */

/** \var CVector3::y
 *  \brief Yova souradnice vektoru
 */

/** \var CVector3::z
 *  \brief Zova souradnice vektoru
 */

/** \fn CVector3::mySize
 *  \brief Vraci velikost vektoru
 */

/** \fn CVector3::multiply(float a)
 *  \brief Vynasobi souradnice vektoru danym cislem
 */

/** \fn CVector3::unitMultiply(float a)
 *  \brief Zkrati vektor na jednotkovy a pote vynasobi souradnice danym cislem
 */

/** \fn CVector3::vectorMultiply
 *  \brief Provede vektorovy soucin se zadanym druhym vektorem
 */

/** \fn CVector3::makeNull
 *  \brief Vynuluje souradnice vektoru
 */

/** \fn CVector3::operator+(const CVector3 &sec)
 *  \brief Secte souradnice vektoru se souradnicemi jineho vektoru
 */

/** \fn CVector3::operator-(const CVector3 &sec)
 *  \brief Odecte souradnice jineho vektoru od svych souradnic
 */

/** \fn CVector3::operator*(float sec)
 *  \brief Vynasobi souradnice vektoru danym cislem. V podstate stejna funkce jako CVector3::multiply
 */

/** \fn CVector3::operator*(const CVector3 &sec)
 *  \brief Provede skalarni soucin a vraci jeho vysledek
 */

/** \fn CVector3::rotate
 *  \brief Otoci vektor okolo osy \a axis o \a angle radianu
 */

/** \brief Struktura vekoru v rovine
 */
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
        if (mySize() == 0)
            return;

        multiply(a/mySize());
    }
    void makeNull()
    {
        x = 0;
        y = 0;
    }
};

/** \var CVector2::x
 *  \brief Xova souradnice vektoru
 */

/** \var CVector2::y
 *  \brief Yova souradnice vektoru
 */

/** \fn CVector2::mySize
 *  \brief Vraci velikost vektoru
 */

/** \fn CVector2::multiply
 *  \brief Vynasobi souradnice danym cislem
 */

/** \fn CVector2::unitMultiply
 *  \brief Zkrati vektor na jednotkovy a pote vynasobi souradnice danym cislem
 */

/** \fn CVector2::makeNull
 *  \brief Vynuluje souradnice vektoru
 */

#endif
