/*
  ==============================================================================

    Body.cpp
    Created: 21 Oct 2019 4:49:29pm
    Author:  Silvin Willemsen

  ==============================================================================
*/

#include "../JuceLibraryCode/JuceHeader.h"
#include "Body.h"

//==============================================================================
Body::Body()
{
    // In your constructor, you should add any child components, and
    // initialise any special settings that your component needs.

}

Body::~Body()
{
}

void Body::paint (Graphics& g)
{
    /* This demo code just fills the component's background and
       draws some placeholder text to get you started.

       You should replace everything in this method with your own
       drawing code..
    */

    g.fillAll (getLookAndFeel().findColour (ResizableWindow::backgroundColourId));   // clear the background

    g.setColour (Colours::grey);
    g.drawRect (getLocalBounds(), 1);   // draw an outline around the component

    g.setColour (Colours::white);
    g.setFont (14.0f);
    g.drawText ("Body", getLocalBounds(),
                Justification::centred, true);   // draw some placeholder text
}

void Body::resized()
{
    // This method is where you should set the bounds of any child
    // components that your component contains..

}
