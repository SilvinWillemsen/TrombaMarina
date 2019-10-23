/*
  ==============================================================================

    Tromba.cpp
    Created: 21 Oct 2019 4:50:13pm
    Author:  Silvin Willemsen

  ==============================================================================
*/

#include "../JuceLibraryCode/JuceHeader.h"
#include "Tromba.h"

//==============================================================================
Tromba::Tromba (NamedValueSet& parameters, double k)  : k (k),
                                                        rhoS (*parameters.getVarPointer ("rhoS")),
                                                        A (*parameters.getVarPointer ("A")),
                                                        T (*parameters.getVarPointer ("T")),
                                                        ES (*parameters.getVarPointer ("ES")),
                                                        Iner (*parameters.getVarPointer ("Iner")),
                                                        s0S (*parameters.getVarPointer ("s0S")),
                                                        M (*parameters.getVarPointer ("M")),
                                                        R (*parameters.getVarPointer ("R")),
                                                        w1 (*parameters.getVarPointer ("w1")),
                                                        K (*parameters.getVarPointer ("K")),
                                                        alpha (*parameters.getVarPointer ("alpha")),
                                                        K1 (*parameters.getVarPointer ("K1")),
                                                        K3 (*parameters.getVarPointer ("K3")),
                                                        sx (*parameters.getVarPointer ("sx")),
                                                        cRatio (*parameters.getVarPointer ("cRatio"))
{
    trombaString = std::make_shared<TrombaString> (parameters, k);
    bridge = std::make_shared<Bridge> (parameters, k);
    body = std::make_shared<Body> (parameters, k);
    
    addAndMakeVisible (trombaString.get());
    addAndMakeVisible (bridge.get());
    addAndMakeVisible (body.get());
    
    if (Global::initialiseWithExcitation)
    {
        trombaString->excite();
    }
    etaSpring = trombaString->getStateAt (1, floor(cRatio * trombaString->getNumPoints())) - bridge->getState(1);
    etaSpringPrev = etaSpring;
    
    hS = 1.0 / trombaString->getNumPoints();
    cP = floor (trombaString->getNumPoints() * cRatio);
    
    
    B1S = rhoS * A / (k * k);
    B2S = T / (hS * hS);
    B3S = ES * Iner / (hS * hS * hS * hS);
    B4S = s0S / k;
    B5S = s1S / (k * hS * hS);
    
    DS = 1.0 / (B1S + B4S);
    
    A1S = 2.0 * B1S - 2.0 * B2S - 6.0 * B3S - 4.0 * B5S;
    A2S = B2S + 4.0 * B3S + 2.0 * B5S;
    A3S = -B3S;
    A4S = -B1S + s0S / k + 4.0 * B5S;
    A5S = -2.0 * B5S;

    A1S *= DS;
    A2S *= DS;
    A3S *= DS;
    A4S *= DS;
    A5S *= DS;
    
    
    B1B = M / (k * k);
    B2B = R / (2.0 * k);
    
    A1B = 2.0 * B1B - M * w1 * w1;
    A2B = -B1B + B2B;
    
    DB = 1.0 / (B1B + B2B);
    
    A1B *= DB;
    A2B *= DB;
}

Tromba::~Tromba()
{
}

void Tromba::paint (Graphics& g)
{
}

void Tromba::resized()
{
    Rectangle<int> totArea = getLocalBounds();
    bridge->setBounds(totArea.removeFromRight (getWidth() / 8.0));
    trombaString->setBounds(totArea.removeFromTop (getHeight() / 2.0));
    body->setBounds(totArea);
}

void Tromba::calculateConnection()
{
    phiMinus = k * (K1 + 2 * K3 * etaSpring * etaSpring) - 4.0 * sx;
    phiPlus = k * (K1 + 2 * K3 * etaSpring * etaSpring) + 4.0 * sx;
    
    varPsi = phiPlus * k * k * (4.0 * M + 2.0 * R * k + g * g * k * k + 4 * hS * (rhoS * A + s0S * k)) + 4.0 * k * (hS * (rhoS * A + s0S * k)) * (4.0 * M + 2.0 * R * k + g * g * k * k);
    
    if (phiPlus == 0)
        FalphaTick = 0;
    else // one division (SEE WHETHER SOME STUFF CAN BE PRECALCULATED HERE)
        FalphaTick = (phiPlus * hS * (rhoS * A + s0S * k) * (4.0 * M + 2.0 * R * k + g * g * k * k)
                      * ((4.0 * M + 2.0 * R * k + g * g * k * k) * (2.0 * k * K1 * etaSpring + phiMinus * etaSpringPrev + phiPlus * trombaString->getStateAt (0, cP))
                      - phiPlus * ((4.0 * M + 2.0 * R * k) * bridge->getState (0) - g * g * k * k * etaColPrev + 4.0 * k * k * psiPrev * g)))
                      / (varPsi * phiPlus * (4.0 * M + 2.0 * R * k + g * g * k * k));
    
    
//    trombaString->setStateAt (cP, (trombaString->getStateAt(0, cP) * (B1S + B4S) + FalphaTick / (hS)) / (B1S + B4S));
//    if (trombaString->getStateAt (0, cP) != 0)
//        std::cout << "wait" << std::endl;
    trombaString->addToStateAt (cP, -FalphaTick / hS * DS);
//    double bridgeState = (M / (k * k) * (2.0 * bridge->getState(1) - bridge->getState(2)) - M * w1 * w1 * (bridge->getState(1)) + R / (2.0 * k) * bridge->getState(2) + FalphaTick) / (M / (k * k) + R / (2.0*k));
    bridge->setState (A1B * bridge->getState (1) + A2B * bridge->getState (2) + FalphaTick * DB);
//    bridge->setState (bridgeState);
}

void Tromba::calculateCollision()
{
    
}

void Tromba::calculateUpdateEqs()
{
    trombaString->calculateUpdateEq();
    bridge->calculateUpdateEq();
}

void Tromba::updateStates()
{
    etaSpringPrev = etaSpring;
    etaSpring = trombaString->getStateAt (0, cP) - bridge->getState (0);

    trombaString->updateStates();
    bridge->updateStates();
    body->updateStates();
//    etaPrev = eta;
//    eta = etaNext;
}
