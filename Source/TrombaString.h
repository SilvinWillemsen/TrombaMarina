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
    TrombaString (NamedValueSet& parameters, double k);
    ~TrombaString();

    void paint (Graphics&) override;
    void resized() override;
    
    Path visualiseState();
    
    void calculateUpdateEq();
    void dampingFinger();
    void updateStates();

    void excite();
    void NRbow();
    void disableBowing() { bowFlag = false; };
    
    void mouseDown (const MouseEvent& e) override;
    void mouseDrag (const MouseEvent& e) override;
    void mouseUp (const MouseEvent& e) override;
    
    double getOutput (double ratio) { int idx = floor(ratio * N); return u[1][idx]; };
    double getStateAt (int time, int idx) { return u[time][idx]; };
    
    void setStateAt (int idx, double val) { u[0][idx] = val; }; // always uNext
    void addToStateAt (int idx, double val) { u[0][idx] += val; }; // always uNext
    
    bool isExcited() { return exciteFlag; };
    bool shouldBow() { return bowing; };
    
    int getNumPoints() { return N; };
    
    void setBowingParameters (float x, float y, double Fb, double Vb, bool mouseInteraction);
    
    void setFingerPos (double val) { _dampingFingerPos.store(val); };
    
    // debug getters
    int getBowPos() { return bp; };
    double getq() { return q; };
    double getb() { return b; };
    double getNRiterator() { return NRiterator; };
private:
    double k, h;
    int N;
    
    // Physical parameters
    double rho, A, T, E, Iner, s0, s1, cSq, kappaSq, lambdaSq, muSq;
    
    // update equation constants
    double A1, A2, A3, A4, A5, B1, B2, C1, C2, D, E1;
    
    // NR bow constants
    double b1, b2;
    
    // pointers to states
    std::vector<double*> u;
    
    // states
    std::vector<std::vector<double>> uVecs;
    
    // excitation variables
    int xPos, yPos;
//    bool exciteFlag = Global::initialiseWithExcitation ? (Global::exciteString ? true : false) : false;
    bool exciteFlag = false;
    bool bowFlag = Global::debug ? true : false;
    bool bowing = true;
    
    std::atomic<double> _Fb {1.0};
    std::atomic<double> _Vb {-0.2};
    std::atomic<double> _bowPos {96};
    double Fb, Vb, alpha;
    int bp;
    
    // Newton variables
    double a, b, uI, uIPrev, uI1, uI2, uIM1, uIM2, uIPrev1, uIPrevM1, cOhSq, kOhhSq, BM, eps, tol, NRiterator, q, qPrev, excitation;
    
    double offset;
    
    std::atomic<double> _dampingFingerPos;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TrombaString)
};
