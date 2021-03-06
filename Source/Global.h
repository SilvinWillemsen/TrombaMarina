/*
  ==============================================================================

    Global.h
    Created: 22 Oct 2019 9:55:53am
    Author:  Silvin Willemsen

  ==============================================================================
*/

#pragma once
#define CUBICINTERPOL

namespace Global
{
    static double outputClamp (double val)
    {
        if (val < -1.0)
        {
            val = -1.0;
            std::cout << "outputClamped!!" << std::endl;
            return val;
        }
        else if (val > 1.0)
        {
            val = 1.0;
            std::cout << "outputClamped!!" << std::endl;
            return val;
        }
        return val;
    }
    
    static double clamp (double val, double min, double max)
    {
        if (val < min)
        {
            val = min;
            return val;
        }
        else if (val > max)
        {
            val = max;
            return val;
        }
        return val;
    }
    static inline int sgn (double val) { return (0 < val) - (val < 0); };

#ifdef NOINTERPOL
    static double interpolation (double* uVec, int bp, double alpha)
    {
        return uVec[bp];
    }
    
    static void extrapolation (double* uVec, int bp, double alpha, double val)
    {
        uVec[bp] = uVec[bp] + val;
    }
#endif
    
#ifdef LINEARINTERPOL
    static double interpolation (double* uVec, int bp, double alpha)
    {
        return uVec[bp] * (1 - alpha) + uVec[bp + 1] * alpha;
    }
    
    static void extrapolation (double* uVec, int bp, double alpha, double val)
    {
        uVec[bp] = uVec[bp] + val * (1 - alpha);
        uVec[bp + 1] = uVec[bp + 1] + val * alpha;
    }
#endif
    
#ifdef CUBICINTERPOL
    static double interpolation (double* uVec, int bp, double alpha)
    {
        return uVec[bp - 1] * (alpha * (alpha - 1) * (alpha - 2)) / -6.0
        + uVec[bp] * ((alpha - 1) * (alpha + 1) * (alpha - 2)) / 2.0
        + uVec[bp + 1] * (alpha * (alpha + 1) * (alpha - 2)) / -2.0
        + uVec[bp + 2] * (alpha * (alpha + 1) * (alpha - 1)) / 6.0;
    }
    
    static void extrapolation (double* uVec, int bp, double alpha, double val)
    {
        uVec[bp - 1] = uVec[bp - 1] + val * (alpha * (alpha - 1) * (alpha - 2)) / -6.0;
        uVec[bp] = uVec[bp] + val * ((alpha - 1) * (alpha + 1) * (alpha - 2)) / 2.0;
        uVec[bp + 1] = uVec[bp + 1] + val * (alpha * (alpha + 1) * (alpha - 2)) / -2.0;
        uVec[bp + 2] = uVec[bp + 2] + val * (alpha * (alpha + 1) * (alpha - 1)) / 6.0;
        
    }
#endif
    
    static inline double exp1 (double x) {
        x = 1.0 + x / 1024.0;
        x *= x; x *= x; x *= x; x *= x;
        x *= x; x *= x; x *= x; x *= x;
        x *= x; x *= x;
        return x;
    }
    
    static bool debug = false;
    static bool bowDebug = false;
    static int debugButtonsHeight = 100;
    static bool initialiseWithExcitation = false;
    static bool exciteString = false;
    static bool exciteBody = false;
    static double outputScaling = 1000.0;
    
    static bool takeSnapShots = false;
}

enum BowModel
{
    exponential,
    elastoPlastic
};
