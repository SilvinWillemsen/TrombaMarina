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
                                                        rhoP (*parameters.getVarPointer ("rhoP")),
                                                        H (*parameters.getVarPointer ("H")),
                                                        s0P (*parameters.getVarPointer ("s0P")),
                                                        K (*parameters.getVarPointer ("K")),
                                                        alph (*parameters.getVarPointer ("alpha")),
                                                        K1 (*parameters.getVarPointer ("K1")),
                                                        K3 (*parameters.getVarPointer ("K3")),
                                                        sx (*parameters.getVarPointer ("sx")),
                                                        connRatio (*parameters.getVarPointer ("connRatio")),
                                                        colRatioX (*parameters.getVarPointer ("colRatioX")),
                                                        colRatioY (*parameters.getVarPointer ("colRatioY"))

{
    trombaString = std::make_shared<TrombaString> (parameters, k);
    bridge = std::make_shared<Bridge> (parameters, k);
    body = std::make_shared<Body> (parameters, k);
    
    addAndMakeVisible (trombaString.get());
    addAndMakeVisible (bridge.get());
    addAndMakeVisible (body.get());
    
    if (Global::initialiseWithExcitation)
    {
        if (Global::exciteString)
            trombaString->excite();
        if (Global::exciteBody)
            body->excite();
    }
    etaSpring = trombaString->getStateAt (1, floor(connRatio * trombaString->getNumPoints())) - bridge->getState(1);
    etaSpringPrev = etaSpring;
    
    hS = 1.0 / trombaString->getNumPoints();
    cP = floor (trombaString->getNumPoints() * connRatio); // connection point
    cPX = floor (body->getNumHorPoints() * colRatioX);
    cPY = floor (body->getNumVertPoints() * colRatioY);
    
    //String
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
    
    // Bridge
    
    B1B = M / (k * k);
    B2B = R / (2.0 * k);
    
    A1B = 2.0 * B1B - M * w1 * w1;
    A2B = -B1B + B2B;
    
    DB = 1.0 / (B1B + B2B);
    
    A1B *= DB;
    A2B *= DB;
    
    // Body
    hP = body->getGridSpacing();
    B1P = rhoP * H / (k * k);
    B2P = s0P / k;
    DP = 1.0 / (B1P + B2P);
    
    // Variables for linear system solve
    a11 = B1S + B4S;
    a12 = 0.0;
    a21 = 0.0;
    a31 = 0.0;
    
    // the rest changes every loop
    oOhS = 1.0 / hS;
    oOFourhPSq = 1.0 / (4.0 * hP * hP);
    
    s0S *= rhoS * A;
    s1S *= rhoS * A;
    s0P *= rhoP * H;
    s1P *= rhoP * H;
    
    etaCol = body->getStateAt(1, cPX, cPY) - bridge->getState(1);
    etaColPrev = etaCol;
    etaColNext = etaCol; // probably not necessary
    
    psiPrev = 0;
    
    
    
}

Tromba::~Tromba()
{
}

void Tromba::paint (Graphics& g)
{
    g.setColour (Colours::yellow);
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
    {
        FalphaTick = (phiPlus * hS * (rhoS * A + s0S * k) * (4.0 * M + 2.0 * R * k + g * g * k * k)
                      * ((4.0 * M + 2.0 * R * k + g * g * k * k) * (2.0 * k * K1 * etaSpring + phiMinus * etaSpringPrev + phiPlus * trombaString->getStateAt (0, cP))
                      - phiPlus * ((4.0 * M + 2.0 * R * k) * bridge->getState (0) - g * g * k * k * etaColPrev + 4.0 * k * k * psiPrev * g)))
                      / (varPsi * phiPlus * (4.0 * M + 2.0 * R * k + g * g * k * k));
    }
    
//    trombaString->addToStateAt (cP, -FalphaTick / hS * DS);
//    bridge->setState (A1B * bridge->getState (1) + A2B * bridge->getState (2) + FalphaTick * DB);
    

}

void Tromba::calculateCollision()
{
    g = 0;
    if (alph == 1)
    {
        if (etaCol > 0)
            g = sqrt(K * (alph + 1.0) / 2.0);
    } else {
        if (etaCol > 0) // subplus
            g = sqrt(K * (alph + 1.0) / 2.0) * pow(etaCol, (alph - 1) * 0.5);
    }
}

void Tromba::solveSystem()
{
    /*
        [   rhoS * A / (k * k) + s0S / k                    0                                      -Theta / hS                     ]
    A = |              0                   M / (k * k) + R / (2 * k) + g * g / 4                 Theta - g * g / 4                 |
        [              0                         -g * g / (4 * hP * hP)           rhoP * H / (k * k) + s0P / k + g * g / (4 * hP)  ]
     
    */

    // find determinant
    plateTerm = (phiPlus * hS * (rhoS * A + s0S * k) * (4.0 * M + 2.0 * R * k + g * g * k * k) * g * g * k * k) / (varPsi * (4.0 * M + 2.0 * R * k + g * g * k * k));
    a13 = -plateTerm * oOhS;
    a22 = M / (k * k) + R / (2.0 * k) + g * g / 4.0;
    a23 = plateTerm - g * g / 4.0;
    a32 =  -g * g / (4.0 * hP * hP);
    a33 = rhoP * H / (k * k) + s0P / k + g * g / (4.0 * hP * hP);
    oOdet = 1.0 / (a11 * (a22 * a33 - a23 * a32));
    
    b11 = (a22 * a33 - a32 * a23) * oOdet;
    b12 = -(-a32 * a13) * oOdet;
    b13 = -a22 * a13 * oOdet;
    b21 = 0; // doesn't have to be assigned every time
    b22 = a11 * a33 * oOdet;
    b23 = -(a11 * a23) * oOdet;
    b31 = 0; // doesn't have to be assigned every time
    b32 = -(a11 * a32) * oOdet;
    b33 = a11 * a22 * oOdet;
    
    c1 = trombaString->getStateAt (0, cP) * (B1S + B4S) - FalphaTick / hS;
    c2 = bridge->getState(0) * (B1B + B2B) - g * g / 4.0 * etaColPrev + psiPrev * g + FalphaTick;
    c3 = body->getStateAt (0, cPX, cPY) * (B1P + B2P) + (g * g / 4.0 * etaColPrev - psiPrev * g) / (hP * hP);

    solut1 = b11 * c1 + b12 * c2 + b13 * c3;
    solut2 = b21 * c1 + b22 * c2 + b23 * c3;
    solut3 = b31 * c1 + b32 * c2 + b33 * c3;
    bridge->setState (solut2);
    Falpha = FalphaTick - plateTerm * solut3;
    etaColNext = solut3 - solut2;
    trombaString->addToStateAt (cP, -Falpha / hS * DS);

    body->addToStateAt (cPX, cPY, -1.0 / (hP * hP) * (g * g / 4.0 * (etaColNext - etaColPrev) + psiPrev * g) * DP);
    bridge->setState (solut2);
    psi = psiPrev + 0.5 * g * (etaColNext - etaColPrev);
}

void Tromba::calculateUpdateEqs()
{
    trombaString->calculateUpdateEq();
    bridge->calculateUpdateEq();
    body->calculateUpdateEq();
}

void Tromba::updateStates()
{

//    if (Global::debug)
//        std::cout << "Sample " << curSample << ", String value (bP): " << trombaString->getStateAt(0, trombaString->getBowPos()) << ", g value: " << g << ", FalphaTick value " << FalphaTick << ", Falpha value: " << Falpha << std::endl;
//    std::cout << "Sample " << curSample << ", String value (bP): " << trombaString->getStateAt(0, trombaString->getBowPos()) << ", q: " << trombaString->getq()
//    << ", b:" << trombaString->getb() << ", NRiterator: " << trombaString->getNRiterator() << std::endl;
//    std::cout << "Sample " << curSample << ", String value (bP): " << trombaString->getStateAt(0, 90) << std::endl;
    psiPrev = psi;
    
    etaSpringPrev = etaSpring;
    etaSpring = trombaString->getStateAt (0, cP) - bridge->getState (0);
    
    etaColPrev = etaCol;
    etaCol = etaColNext;

//    if (psiPrev != 0)
//        std::cout << "wait" << std::endl;
    trombaString->updateStates();
    bridge->updateStates();
    body->updateStates();
}
