/*
  ==============================================================================

    This file was auto-generated!

  ==============================================================================
*/

#include "MainComponent.h"

//==============================================================================
MainComponent::MainComponent()
{
    // Make sure you set the size of the component after
    // you add any child components.

    // Some platforms require permissions to open input channels so request that here
    if (RuntimePermissions::isRequired (RuntimePermissions::recordAudio)
        && ! RuntimePermissions::isGranted (RuntimePermissions::recordAudio))
    {
        RuntimePermissions::request (RuntimePermissions::recordAudio,
                                     [&] (bool granted) { if (granted)  setAudioChannels (2, 2); });
    }
    else
    {
        // Specify the number of input and output channels that we want to open
        setAudioChannels (2, 2);
    }
}

MainComponent::~MainComponent()
{
    // This shuts down the audio device and clears the audio source.
    shutdownAudio();
}

//==============================================================================
void MainComponent::prepareToPlay (int samplesPerBlockExpected, double sampleRate)
{
    fs = sampleRate;
    
    double k = 1.0 / fs;
    
    NamedValueSet parameters;
    parameters.set ("K", 5.5e10);
    parameters.set ("alpha", 1.3);
    parameters.set ("K1", 100);
    parameters.set ("K3", 1e6);
    parameters.set ("rho", 7850);
    parameters.set ("A", 1e-7);
    parameters.set ("T", 100);
    parameters.set ("E", 2e11);
    parameters.set ("Iner", 1e-14);
    parameters.set ("s0", 1);
    parameters.set ("s1", 0.005);

    tromba = std::make_unique<Tromba> (parameters, k);
    addAndMakeVisible (tromba.get());
    
    trombaString = tromba->getString();
    bridge = tromba->getBridge();
    body = tromba->getBody();
    
    setSize (800, 600);
    Timer::startTimerHz (15);
}

void MainComponent::getNextAudioBlock (const AudioSourceChannelInfo& bufferToFill)
{

    bufferToFill.clearActiveBufferRegion();
    
    float* const channelData1 = bufferToFill.buffer->getWritePointer (0, bufferToFill.startSample);
    float* const channelData2 = bufferToFill.buffer->getWritePointer (1, bufferToFill.startSample);
    
    float output = 0.0;
    for (int i = 0; i < bufferToFill.numSamples; ++i)
    {
        
        if (trombaString->isExcited())
            trombaString->excite();
        tromba->calculateUpdateEqs();
        tromba->updateStates();
        output = tromba->getOutput (0.3);
        
        channelData1[i] = Global::clamp(output, -1, 1);
        channelData2[i] = Global::clamp(output, -1, 1);
    }
}

void MainComponent::releaseResources()
{
    // This will be called when the audio device stops, or when it is being
    // restarted due to a setting change.

    // For more details, see the help for AudioProcessor::releaseResources()
}

//==============================================================================
void MainComponent::paint (Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (ResizableWindow::backgroundColourId));

    // You can add your drawing code here!
}

void MainComponent::resized()
{
    if (tromba.get() != nullptr)
    {
        tromba->setBounds(getLocalBounds());
    }
}

void MainComponent::timerCallback()
{
    repaint();
}
