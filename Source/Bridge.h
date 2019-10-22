/*
  ==============================================================================

    Bridge.h
    Created: 21 Oct 2019 4:48:59pm
    Author:  Silvin Willemsen

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

//==============================================================================
/*
*/
class Bridge    : public Component
{
public:
    Bridge();
    ~Bridge();

    void paint (Graphics&) override;
    void resized() override;

    double getOutput() { // return state
        
    };
    
    void updateStates() {};
private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Bridge)
};
