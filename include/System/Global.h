/**********************************
*        Bomberman3D Engine       *
*  Created by: Cmaranec (Kennny)  *
**********************************/
// Global includes

#ifndef BOMB_GLOBAL_H
#define BOMB_GLOBAL_H

#include <windows.h>
#include <wingdi.h>
#include <iostream>
#include <gl\gl.h>
#include <gl\glu.h>
#include <gl\glaux.h>
#include <math.h>
#include <time.h>
#include <cstdarg>
#include <assert.h>
#include <ctype.h>
#include <locale>
#include <boost/thread.hpp>

#include <string> // for std::string
#include <vector> // for std::vector
#include <list>   // for std::list
#include <map>    // for std::map

using namespace std;

#include <Defines.h>
#include <Application.h>

#ifdef _DEBUG
 #define DATA_PATH "../data"
 #define SETTINGS_PATH "../data/settings"
#else
 #define DATA_PATH "./data"
 #define SETTINGS_PATH "./settings"
#endif

#pragma warning (disable:4996) //"fopen","fscanf",... may be unsafe
#pragma warning (disable:4018) //signed/unsigned mismatch
#pragma warning (disable:4244) //conversion from X to Y, possible loss of data
#pragma warning (disable:4068) //unknown pragma

#endif
