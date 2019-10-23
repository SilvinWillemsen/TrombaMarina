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
    Tromba (NamedValueSet& parameters, double k);
    ~Tromba();

    void paint (Graphics&) override;
    void resized() override;
    
    void calculateConnection();
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
    
    // String variables needed for calculating connections
    double rhoS, A, T, ES, Iner, s0S, s1S;

    // Mass variables needed for calculating connections
    double M, w1, R;
    
    // Collision variables
    double K, alpha, g, etaCol, etaColPrev, psi, psiPrev = 0;
    
    // Connection variables
    double K1, K3, sx, etaSpring, etaSpringPrev, cRatio, phiMinus, phiPlus, hS, varPsi, FalphaTick;
    int cP;
    
    // Connection calculation coefficients
    double A1S, A2S, A3S, A4S, A5S, B1S, B2S, B3S, B4S, B5S, DS;
    double A1B, A2B, B1B, B2B, DB;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Tromba)
};
