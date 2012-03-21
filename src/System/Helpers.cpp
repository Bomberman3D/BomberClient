#include <Global.h>
#include <Helpers.h>
#include <vector>
using namespace std;

CVector3 Vector(CVector3 vPoint1, CVector3 vPoint2)
{
    CVector3 vVector;

    vVector.x = vPoint1.x - vPoint2.x;
    vVector.y = vPoint1.y - vPoint2.y;
    vVector.z = vPoint1.z - vPoint2.z;

    return vVector;
}

CVector3 DivideVectorByScaler(CVector3 vVector1, float Scaler)
{
    CVector3 vResult;

    vResult.x = vVector1.x / Scaler;
    vResult.y = vVector1.y / Scaler;
    vResult.z = vVector1.z / Scaler;

    return vResult;
}

CVector3 Cross(CVector3 vVector1, CVector3 vVector2)
{
    CVector3 vCross;
    vCross.x = ((vVector1.y * vVector2.z) - (vVector1.z * vVector2.y));
    vCross.y = ((vVector1.z * vVector2.x) - (vVector1.x * vVector2.z));
    vCross.z = ((vVector1.x * vVector2.y) - (vVector1.y * vVector2.x));

    return vCross;
}

float RoundFloat(float number)
{
    if(number > 0 && number <  0.001f) number = 0;
    if(number < 0 && number > -0.001f) number = 0;

    return number;
}

vector<string> explode(const string& str, const char& ch)
{
    string next = "";
    vector<string> result;

    for (string::const_iterator it = str.begin(); it != str.end(); it++)
    {
        if (*it == ch)
        {
            if (next.length() > 0)
            {
                result.push_back(next);
                next = "";
            }
        }
        else
            next += *it;
    }

    if (next.length() > 0)
        result.push_back(next);

    return result;
}
