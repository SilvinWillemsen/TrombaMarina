/*
  ==============================================================================

    Tromba.h
    Created: 21 Oct 2019 4:50:13pm
    Author:  Silvin Willemsen

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "Global.h"
#include "Body.h"
#include "Bridge.h"
#include "TrombaString.h"

//==============================================================================
/*
*/
class Tromba    : public Component
{
public:
    Tromba (NamedValueSet parameters, double k);
    ~Tromba();

    void paint (Graphics&) override;
    void resized() override;

    double getValue (NamedValueSet parameters, String name) { return static_cast<double> (*parameters.getVarPointer(name)); };
    
    void calculateCollision();
    
    void calculateUpdateEqs();
    void updateStates();
    
    double getOutput() { return bridge->getOutput(); };
    double getOutput (double ratio) { return trombaString->getOutput (ratio); };
    double getOutput (double ratioX, double ratioY) { return body->getOutput (ratioX, ratioY); };
    
    std::shared_ptr<TrombaString> getString() { return trombaString; };
    std::shared_ptr<Bridge> getBridge() { return bridge; };
    std::shared_ptr<Body> getBody() { return body; };
    
private:
    
    double k; 
    // Instrument components (String, body and bridge)
    std::shared_ptr<TrombaString> trombaString;
    std::shared_ptr<Bridge> bridge;
    std::shared_ptr<Body> body;
    
    // Collision variables
    double K, alpha, g;
    
    // Connection variables
    double K1, K3, sx;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Tromba)
};
