/*
  ==============================================================================

    TrombaString.cpp
    Created: 21 Oct 2019 4:47:58pm
    Author:  Silvin Willemsen

  ==============================================================================
*/

#include "../JuceLibraryCode/JuceHeader.h"
#include "TrombaString.h"

//==============================================================================
TrombaString::TrombaString (NamedValueSet& parameters, double k) :  k (k),
                                                                    rho (*parameters.getVarPointer("rhoS")),
                                                                    A (*parameters.getVarPointer("A")),
                                                                    T (*parameters.getVarPointer("T")),
                                                                    E (*parameters.getVarPointer("ES")),
                                                                    Iner (*parameters.getVarPointer("Iner")),
                                                                    s0 (*parameters.getVarPointer("s0S")),
                                                                    s1 (*parameters.getVarPointer("s1S"))
{
    
    // calculate wave speed and stiffness coefficient
    cSq = T / (rho * A);
    kappaSq = E * Iner / (rho * A);
    
    // calculate grid spacing and number of points
    h = sqrt ((cSq * k * k + 4.0 * s1 * k + sqrt(pow(cSq * k * k + 4.0 * s1 * k, 2) + 16.0 * kappaSq * k * k)) / 2.0);
    
//    // Scale damping by rho * A
//    s0 = s0 * rho * A;
//    s1 = s1 * rho * A;
    
    N = floor (1.0 / h);
    h = 1.0 / N;
    
    // initialise state vectors
    uVecs.reserve (3);
    
    for (int i = 0; i < 3; ++i)
        uVecs.push_back (std::vector<double> (N, 0));
    
    u.resize (3);
    
    for (int i = 0; i < u.size(); ++i)
        u[i] = &uVecs[i][0];
    
    // courant numbers
    lambdaSq = cSq * k * k / (h * h);
    muSq = k * k * kappaSq / (h * h * h * h);
    
    // Newton Variables bow model
    cOhSq = cSq / (h * h);
    kOhhSq = kappaSq / (h * h * h * h);
    tol = 1e-4;
    
    a = 100; // Free parameter
    BM = sqrt(2.0 * a) * exp (0.5);
    
    // set coefficients for update equation
    B1 = s0 * k;
    B2 = (2 * s1 * k) / (h * h);
    
    D = 1.0 / (1.0 + s0 * k);
    
    b1 = 2.0 / (k * k);
    b2 = (2.0 * s1) / (k * h * h);
    
    A1 = 2.0 - 2.0 * lambdaSq - 6.0 * muSq - 2.0 * B2;
    A2 = lambdaSq + 4.0 * muSq + B2;
    A3 = muSq;
    A4 = B1 - 1.0 + 2.0 * B2;
    A5 = B2;
    
    A1 *= D;
    A2 *= D;
    A3 *= D;
    A4 *= D;
    A5 *= D;
    
    E1 = k * k * (1.0 / h) * BM;
    
    qPrev = -_Vb.load();
    
    if (Global::debug)
        setBowingParameters (0.0, 0.0);

}

TrombaString::~TrombaString()
{
}

void TrombaString::paint (Graphics& g)
{
    /* This demo code just fills the component's background and
       draws some placeholder text to get you started.

       You should replace everything in this method with your own
       drawing code..
    */

    g.fillAll (getLookAndFeel().findColour (ResizableWindow::backgroundColourId));   // clear the background
    g.setColour (Colours::cyan);
    Path stringPath = visualiseState();
    g.strokePath (stringPath, PathStrokeType(2.0f));
}

void TrombaString::resized()
{

}

Path TrombaString::visualiseState()
{
    auto stringBounds = getHeight() / 2.0;
    Path stringPath;
    stringPath.startNewSubPath (0, stringBounds);
    int stateWidth = getWidth();
    auto spacing = stateWidth / static_cast<double>(N);
    auto x = spacing;
    
    for (int y = 1; y < N; y++)
    {
        int visualScaling = Global::outputScaling * 100;
        float newY = -u[1][y] * visualScaling + stringBounds;
        
        if (isnan(x) || isinf(abs(x) || isnan(newY) || isinf(abs(newY))))
        {
            std::cout << "Wait" << std::endl;
        };
        
        if (isnan(newY))
            newY = 0;
        stringPath.lineTo(x, newY);
        x += spacing;
    }
    stringPath.lineTo(stateWidth, stringBounds);
    return stringPath;
}

void TrombaString::calculateUpdateEq()
{
    for (int l = 2; l < N - 2; ++l)
    {
        u[0][l] = A1 * u[1][l] + A2 * (u[1][l + 1] + u[1][l - 1]) - A3 * (u[1][l + 2] + u[1][l - 2]) + A4 * u[2][l] - A5 * (u[2][l + 1] + u[2][l - 1]);
    }
    
    if (!bowing)
    {
        return;
    }
    else if (bowFlag)
    {
        // for using the same 'dynamic variables' during one loop
        Vb = _Vb.load();
        Fb = _Fb.load();
        bp = floor (_bowPos.load());
        alpha = _bowPos.load() - bp;
        NRbow();
        excitation = E1 * Fb * q * Global::exp1 (-a * q * q);
        Global::extrapolation(u[0], bp, alpha, -excitation);
    }
    
}

void TrombaString::updateStates()
{
    double* uTmp = u[2];
    u[2] = u[1];
    u[1] = u[0];
    u[0] = uTmp;
    
    qPrev = q;
}

void TrombaString::excite()
{
    exciteFlag = false;
//    int width = floor ((N * 2.0) / 5.0) / 2.0;
    int width = 10;
    int loc = floor (N * static_cast<float>(xPos) / static_cast<float>(getWidth()));
    if (Global::debug)
        loc = floor(N / 4.0);
    
    int startIdx = Global::clamp (loc - width / 2.0, 2, N - 2 - width);
    for (int i = 0; i < width; ++i)
    {
        double val = (1 - cos (2 * double_Pi * i / width)) * (Global::debug ? 0.5 : 0.5 / (Global::outputScaling));
        u[1][startIdx + i] = u[1][startIdx + i] + val;
        u[2][startIdx + i] = u[2][startIdx + i] + val;
    }
}

void TrombaString::NRbow()
{
    
    // Interpolation
    uI = Global::interpolation (u[1], bp, alpha);
    uIPrev = Global::interpolation (u[2], bp, alpha);
    uI1 = Global::interpolation (u[1], bp + 1, alpha);
    uI2 = Global::interpolation (u[1], bp + 2, alpha);
    uIM1 = Global::interpolation (u[1], bp - 1, alpha);
    uIM2 = Global::interpolation (u[1], bp - 2, alpha);
    uIPrev1 = Global::interpolation (u[2], bp + 1, alpha);
    uIPrevM1 = Global::interpolation (u[2], bp - 1, alpha);
    
    
    // Calculate precalculable part
    b = 2.0 / k * Vb - b1 * (uI - uIPrev) - cOhSq * (uI1 - 2.0 * uI + uIM1) + kOhhSq * (uI2 - 4.0 * uI1 + 6.0 * uI - 4.0 * uIM1 + uIM2) + 2.0 * s0 * Vb - b2 * ((uI1 - 2 * uI + uIM1) - (uIPrev1 - 2.0 * uIPrev + uIPrevM1));
    
    // error term
    eps = 1;
    NRiterator = 0;

    // NR loop
    while (eps > tol && NRiterator < 100)
    {
//        q = qPrev - (Fb * BM * qPrev * exp (-a * qPrev * qPrev) + 2.0 * qPrev / k + 2.0 * s0 * qPrev + b) / (Fb * BM * (1.0 - 2.0 * a * qPrev * qPrev) * exp (-a * qPrev * qPrev) + 2.0 / k + 2.0 * s0);
        
        // working
//        q = qPrev - (Fb * BM * qPrev * exp (-a * qPrev * qPrev) + 2.0 * qPrev / k + 2.0 * s0 * qPrev + b) /
//        (Fb * BM * (1.0 - 2.0 * a * qPrev * qPrev) * exp (-a * qPrev * qPrev) + 2.0 / k + 2.0 * s0);
        
        q = qPrev - (1.0 / (rho * A * h) * Fb * BM * qPrev * exp (-a * qPrev * qPrev) + 2.0 * qPrev / k + 2.0 * s0 * qPrev + b) /
                (1.0 / (rho * A * h) * Fb * BM * (1.0 - 2.0 * a * qPrev * qPrev) * exp (-a * qPrev * qPrev) + 2.0 / k + 2.0 * s0);
        eps = std::abs (q - qPrev);
        qPrev = q;
        ++NRiterator;
        if (NRiterator > 98)
        {
            std::cout << "Nope" << std::endl;
        }
    }
    
}

void TrombaString::mouseDown (const MouseEvent& e)
{
    if (bowing)
        return;
    xPos = e.x;
    yPos = e.y;
    exciteFlag = true;
}

void TrombaString::mouseDrag (const MouseEvent& e)
{
    if (!bowing)
        return;
    
    setBowingParameters (e.x, e.y);
}

void TrombaString::mouseUp (const MouseEvent& e)
{
    bowFlag = false;
}

void TrombaString::setBowingParameters (int x, int y)
{
    xPos = x;
    yPos = y;
    bowFlag = true;
    
    _Vb.store (Global::debug ? -0.2 : (yPos / static_cast<float> (getHeight()) - 0.5) * 2.0 * 0.2);
    _Fb.store (1.0);
    
    int loc = Global::debug ? floor(N * 0.5) + 5 : floor (N * static_cast<float> (xPos) / static_cast<float> (getWidth()));
    _bowPos.store (Global::clamp (loc, 3, N - 5)); // check whether these values are correct!!);
}
