/*
  ==============================================================================

    Body.h
    Created: 21 Oct 2019 4:49:29pm
    Author:  Silvin Willemsen

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

//==============================================================================
/*
*/
class Body    : public Component
{
public:
    Body();
    ~Body();

    void paint (Graphics&) override;
    void resized() override;

    double getOutput (double ratioX, double ratioY) { //return u[1][floor(ratio * N)];
        
    };
    
    void updateStates() {};
private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Body)
};
