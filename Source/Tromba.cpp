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
Tromba::Tromba (NamedValueSet parameters, double k) : k (k),
                                                      K (*parameters.getVarPointer("K")),
                                                      alpha (*parameters.getVarPointer("alpha")),
                                                      K1 (*parameters.getVarPointer("K1")),
                                                      K3 (*parameters.getVarPointer("K3"))
{
    trombaString = std::make_shared<TrombaString> (parameters, k);
    bridge = std::make_shared<Bridge>();
    body = std::make_shared<Body>();
    
    addAndMakeVisible (trombaString.get());
    addAndMakeVisible (bridge.get());
    addAndMakeVisible (body.get());
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

void Tromba::calculateCollision()
{
    
}

void Tromba::calculateUpdateEqs()
{
    trombaString->calculateUpdateEq();
}

void Tromba::updateStates()
{
    trombaString->updateStates();
    bridge->updateStates();
    body->updateStates();
}
