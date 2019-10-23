/*
  ==============================================================================

    Bridge.cpp
    Created: 21 Oct 2019 4:48:59pm
    Author:  Silvin Willemsen

  ==============================================================================
*/

#include "../JuceLibraryCode/JuceHeader.h"
#include "Bridge.h"
#include "Global.h"

//==============================================================================
Bridge::Bridge (NamedValueSet& parameters, double k) : k (k),
                                                      M (*parameters.getVarPointer("M")),
                                                      w1 (*parameters.getVarPointer("w1")),
                                                      R (*parameters.getVarPointer("R"))
{
    // initialise state vectors
    uVecs.resize(3, 0);
    
    u.resize (3);
    
    for (int i = 0; i < u.size(); ++i)
        u[i] = &uVecs[i];
    
    B1 = M / (k * k);
    B2 = M * w1 * w1;
    
    D = 1.0 / (M / (k * k) + R / (2.0 * k));
    
    A1 = 2.0 * B1 - B2;
    A2 = -B1 + R / (2 * k);
    A3 = offset * B2;
    
    A1 *= D;
    A2 *= D;
    A3 *= D;
}

Bridge::~Bridge()
{
}

void Bridge::paint (Graphics& g)
{
    /* This demo code just fills the component's background and
       draws some placeholder text to get you started.

       You should replace everything in this method with your own
       drawing code..
    */
    g.fillAll (getLookAndFeel().findColour (ResizableWindow::backgroundColourId));   // clear the background
    g.setColour(Colours::cyan);
    int scalingFactor = 100000000;
    g.drawEllipse (getWidth() * 0.5, u[1][0] * scalingFactor + getHeight() * 0.5, 5.0, 5.0, 5.0);
}

void Bridge::resized()
{
    // This method is where you should set the bounds of any child
    // components that your component contains..

}

void Bridge::calculateUpdateEq()
{
    u[0][0] = A1 * u[1][0] + A2 * u[2][0]; // + A3
}

void Bridge::updateStates()
{
    double* uTmp = u[2];
    u[2] = u[1];
    u[1] = u[0];
    u[0] = uTmp;
}

void Bridge::excite()
{
    u[1][0] += 1.0 / (Global::outputScaling * 1.0);
    u[2][0] += 1.0 / (Global::outputScaling * 1.0);
}

void Bridge::mouseDown (const MouseEvent& e)
{
    excite();
}
