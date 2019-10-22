/*
  ==============================================================================

    TrombaString.h
    Created: 21 Oct 2019 4:47:58pm
    Author:  Silvin Willemsen

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "Global.h"

//==============================================================================
/*
*/
class TrombaString    : public Component
{
public:
    TrombaString (NamedValueSet parameters, double k);
    ~TrombaString();

    void paint (Graphics&) override;
    void resized() override;
    
    Path visualiseState();
    
    void calculateUpdateEq();
    void updateStates();

    void excite();
    
    void mouseDown (const MouseEvent& e) override;
    
    double getOutput (double ratio) { int idx = floor(ratio * N); return u[1][idx]; };
    
    bool isExcited() { return exciteFlag; };
    
private:
    double k, h;
    double rho, A, T, E, Iner, s0, s1, cSq, kappaSq, lambdaSq, muSq;
    
    double A1, A2, A3, A4, A5, B1, B2, C1, C2, D;
    
    int N;
    
    // pointers to states
    std::vector<double*> u;
    
    // states
    std::vector<std::vector<double>> uVecs;
    
    // excitation variables
    int xPos, yPos;
    bool exciteFlag = false;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TrombaString)
};
