/*
  ==============================================================================

    Global.h
    Created: 22 Oct 2019 9:55:53am
    Author:  Silvin Willemsen

  ==============================================================================
*/

#pragma once

namespace Global
{
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
}
