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
TrombaString::TrombaString (NamedValueSet parameters, double k) : rho (*parameters.getVarPointer("rho")),
                                                                  A (*parameters.getVarPointer("A")),
                                                                  T (*parameters.getVarPointer("T")),
                                                                  E (*parameters.getVarPointer("E")),
                                                                  Iner (*parameters.getVarPointer("Iner")),
                                                                  s0 (*parameters.getVarPointer("s0")),
                                                                  s1 (*parameters.getVarPointer("s1"))
{
    
    // calculate wave speed and stiffness coefficient
    cSq = T / (rho * A);
    kappaSq = E * Iner / (rho * A);
    
    // calculate grid spacing and number of points
    h = sqrt ((cSq * k * k + 4.0 * s1 * k + sqrt(pow(cSq * k * k + 4.0 * s1 * k, 2) + 16.0 * kappaSq * k * k)) / 2.0);
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
    
    // set coefficients for update equation
    B1 = s0 * k;
    B2 = (2 * s1 * k) / (h * h);
    
//    b1 = 2.0 / (k * k);
//    b2 = (2 * s1) / (k * h * h);
    
    D = 1.0 / (1.0 + s0 * k);
    
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
        int visualScaling = 10;
        float newY = u[0][y] * visualScaling + stringBounds;
        
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
}

void TrombaString::updateStates()
{
    double* uTmp = u[2];
    u[2] = u[1];
    u[1] = u[0];
    u[0] = uTmp;
}

void TrombaString::excite()
{
    exciteFlag = false;
    int width = floor ((N * 2.0) / 5.0) / 2.0;
    int loc = floor (N * static_cast<float>(xPos) / static_cast<float>(getWidth()));
    int startIdx = Global::clamp (loc - width / 2.0, 2, N - 2 - width);
    for (int i = 0; i < width; ++i)
    {
        double val = (1 - cos (2 * double_Pi * i / width)) * 0.5;
        u[1][startIdx + i] = u[1][startIdx + i] + val;
        u[2][startIdx + i] = u[2][startIdx + i] + val;
    }
}

void TrombaString::mouseDown (const MouseEvent& e)
{
    xPos = e.x;
    yPos = e.y;
    exciteFlag = true;
}
