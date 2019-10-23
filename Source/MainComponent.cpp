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
    
    // string
    double r = 0.0005;
    double f0 = 100.0;
    double rhoS = 7850.0;
    double A = r * r * double_Pi;
    double T = (f0 * f0 * 4.0) * rhoS * A;
    
    parameters.set ("rhoS", rhoS);
    parameters.set ("r", r);
    parameters.set ("A", r * r * double_Pi);
    parameters.set ("T", T);
    parameters.set ("ES", 2e11);
    parameters.set ("Iner", r * r * r * r * double_Pi * 0.25);
    parameters.set ("s0S", 0.1);
    parameters.set ("s1S", 0.005);
    
    // bridge
    parameters.set ("M", 0.001);
    parameters.set ("R", 0.1);
    parameters.set ("w1", 2.0 * double_Pi * 1000.0);

    // collision
    parameters.set ("K", 5.5e10);
    parameters.set ("alpha", 1.3);
    
    // connection
    parameters.set ("K1", 1e6);
    parameters.set ("K3", 100);
    parameters.set ("sx", 1);
    parameters.set ("cRatio", 0.5);
    
    tromba = std::make_unique<Tromba> (parameters, k);
    addAndMakeVisible (tromba.get());
    
    trombaString = tromba->getString();
    bridge = tromba->getBridge();
    body = tromba->getBody();
    
    setSize (800, 600);
    Timer::startTimerHz (60);
}

void MainComponent::getNextAudioBlock (const AudioSourceChannelInfo& bufferToFill)
{

    bufferToFill.clearActiveBufferRegion();
    
    float* const channelData1 = bufferToFill.buffer->getWritePointer (0, bufferToFill.startSample);
    float* const channelData2 = bufferToFill.buffer->getWritePointer (1, bufferToFill.startSample);
    
    float output = 0.0;
    for (int i = 0; i < bufferToFill.numSamples; ++i)
    {
        
        if (trombaString->isExcited() && !trombaString->shouldBow())
            trombaString->excite();
        
        tromba->calculateUpdateEqs();
        tromba->calculateConnection();
        tromba->updateStates();
        output = tromba->getOutput(0.2) * (Global::debug ? 1.0 : Global::outputScaling);
        if (Global::debug)
        {
//             std::cout << output << std::endl;
        } else {
            channelData1[i] = Global::clamp(output, -1, 1);
            channelData2[i] = Global::clamp(output, -1, 1);
        }
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
