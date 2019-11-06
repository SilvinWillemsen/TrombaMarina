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
    void solveSystem();
    void calculateUpdateEqs();
    void updateStates();
    
    double getOutput() { return bridge->getOutput() * 10; };
    double getOutput (double ratio) { return trombaString->getOutput (ratio); };
    double getOutput (double ratioX, double ratioY) { return body->getOutput (ratioX, ratioY); };
    
    std::shared_ptr<TrombaString> getString() { return trombaString; };
    std::shared_ptr<Bridge> getBridge() { return bridge; };
    std::shared_ptr<Body> getBody() { return body; };
    
    double getEtaCol (int time)
    {
        switch (time)
        {
            case 0:
                return etaColNext;
            case 1:
                return etaCol;
            case 2:
                return etaColPrev;
        };
    };
    
    void setCurSample (int curSamp) { curSample = curSamp; };
    
private:
    
    double k; 
    // Instrument components (String, body and bridge)
    std::shared_ptr<TrombaString> trombaString;
    std::shared_ptr<Bridge> bridge;
    std::shared_ptr<Body> body;
    
    // String variables needed for calculating connections
    double rhoS, A, T, ES, Iner, s0S, s1S, hS;

    // Mass variables needed for calculating connections
    double M, w1, R;
    
    // Body variables needed for calculating collision
    double rhoP, H, E, hP, s0P, s1P;

    // Collision variables
    double K, alph, g, etaColNext, etaCol, etaColPrev, psi, psiPrev, plateTerm = 0;
    double colRatioX, colRatioY;
    int cPX, cPY;
    
    // Connection variables
    double K1, K3, sx, etaSpring, etaSpringPrev, connRatio, phiMinus, phiPlus, varPsi, FalphaTick, Falpha;
    int cP;
    
    // Connection calculation coefficients
    double A1S, A2S, A3S, A4S, A5S, B1S, B2S, B3S, B4S, B5S, DS;
    double A1B, A2B, B1B, B2B, DB;
    double B1P, B2P, DP;
    
    // Variables for doing the linear system solve
    double a11, a12, a13, a21, a22, a23, a31, a32, a33;
    double b11, b12, b13, b21, b22, b23, b31, b32, b33;
    double c1, c2, c3;
    double solut1, solut2, solut3;
    
    double oOdet;
    double oOhS, oOFourhPSq;
    
    int curSample;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Tromba)
};
