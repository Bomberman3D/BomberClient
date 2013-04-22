#ifndef BOMB_HELPERS_H
#define BOMB_HELPERS_H

#include <Defines.h>

extern CVector3 Vector(CVector3 vPoint1, CVector3 vPoint2);
extern CVector3 DivideVectorByScaler(CVector3 vVector1, float Scaler);
extern CVector3 Cross(CVector3 vVector1, CVector3 vVector2);
extern float RoundFloat(float number);

static float frand() { return rand() / (float) RAND_MAX; }
static float frand(float f) { return frand() * f; }
static float frand(float f, float g) { return frand(g - f) + f; }
static uint32 urand(uint32 a) { return rand()%a; }
static uint32 urand(uint32 a, uint32 b) { return urand(b - a) + a; }
static int32 irand(int32 a, int32 b) { return ((int32)urand(abs(b - a))) + a; }

static float pythagoras_c(float a, float b) { return sqrt(a*a+b*b); }

vector<string> explode(const string& str, const char& ch);

#endif
