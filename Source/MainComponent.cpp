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
        setAudioChannels (0, 2);
    }
}

MainComponent::~MainComponent()
{
    // This shuts down the audio device and clears the audio source.
    Timer::stopTimer();
    HighResolutionTimer::stopTimer();
    
    for (auto sensel : sensels)
    {
        if (sensel->senselDetected)
        {
            sensel->shutDown();
        }
    }
    
    shutdownAudio();
}

//==============================================================================
void MainComponent::prepareToPlay (int samplesPerBlockExpected, double sampleRate)
{
    for (int i = 0; i < amountOfSensels; ++i)
    {
        sensels.add (new Sensel (i)); // chooses the device in the sensel device list
        std::cout << "Sensel added" << std::endl;
    }
    
    if (Global::debug)
    {
        continueButton = std::make_unique<TextButton>("ContinueButton");
        continueFlag.store (false);
        continueButton->setButtonText ("Continue");
        addAndMakeVisible (continueButton.get());
        continueButton->addListener (this);
        
        stateLabel = std::make_unique<Label>("StateLabel");
        stateLabel->setColour (Label::textColourId, Colours::white);
        addAndMakeVisible (stateLabel.get());
        
        currentSampleLabel = std::make_unique<Label>("CurrentSampleLabel");
        currentSampleLabel->setColour (Label::textColourId, Colours::white);
        addAndMakeVisible (currentSampleLabel.get());
        
    }

    continueFlag.store (true);
    fs = sampleRate;
    
    double k = 1.0 / fs;
    
    NamedValueSet parameters;
    
    offset = 1e-5;
    
    // string
    double r = 0.0005;
    double f0 = 60.0;
    double rhoS = 7850.0;
    double A = r * r * double_Pi;
    double T = (f0 * f0 * 4.0) * rhoS * A;
    bridgeLocRatio = 18.0 / 20.0;
    parameters.set ("rhoS", rhoS);
    parameters.set ("r", r);
    parameters.set ("A", r * r * double_Pi);
    parameters.set ("T", T);
    parameters.set ("ES", 2e11);
    parameters.set ("Iner", r * r * r * r * double_Pi * 0.25);
    parameters.set ("s0S", 0.1);
    parameters.set ("s1S", 1);
    
    // bridge
    parameters.set ("M", 0.001);
    parameters.set ("R", 0.1);
    parameters.set ("w1", 2.0 * double_Pi * 500);
    parameters.set ("offset", offset);
    
    // body
    parameters.set ("rhoP", 7850.0);
    parameters.set ("H", 0.001);
    parameters.set ("EP", 2e11);
    parameters.set ("Lx", 1.5);
    parameters.set ("Ly", 0.4);
    parameters.set ("s0P", 5);
    parameters.set ("s1P", 0.01);
    
    // connection
    parameters.set ("K1", 5.0e6);
    parameters.set ("alpha1", 1.0);
    parameters.set ("connRatio", bridgeLocRatio);
    
    // plate collision
    parameters.set ("K2", 5.0e10);
    parameters.set ("alpha2", 1.0);
    parameters.set ("colRatioX", 0.8);
    parameters.set ("colRatioY", 0.5);
    
    
    tromba = std::make_unique<Tromba> (parameters, k);
    addAndMakeVisible (tromba.get());
    
    trombaString = tromba->getString();
    bridge = tromba->getBridge();
    body = tromba->getBody();
    
    setSize (800, 600);
    Timer::startTimerHz (40);
    
    trombaString->setFingerPos (0.0);
    
    // start the hi-res timer
    if (sensels.size() != 0)
        if (sensels[0]->senselDetected)
            HighResolutionTimer::startTimer (1000.0 / 150.0); // 150 Hz
    
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
        if (body->isExcited())
            body->excite();
        
        if (Global::debug)
        {
            if (continueFlag.load() == true)
            {
//                if (curSample % 1 == 0)
//                    continueFlag.store (false);
                tromba->setCurSample (curSample);
                tromba->calculateUpdateEqs();
                tromba->calculateCollisions();
//                tromba->calculateConnection();
                tromba->solveSystem();
                tromba->updateStates();
                ++curSample;
            }
        } else {
            if (continueFlag.load() == true)
            {
                tromba->calculateUpdateEqs();
                trombaString->dampingFinger();
                tromba->calculateCollisions();
                tromba->solveSystem();
                tromba->updateStates();
            }
        }
        output = tromba->getOutput() * (Global::debug ? 1.0 : Global::outputScaling);
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
        Rectangle<int> totalArea = getLocalBounds();
        if (Global::debug)
        {
            int margin = 5;
            Rectangle<int> debugArea = totalArea.removeFromBottom (Global::debugButtonsHeight);
            debugArea.reduce (margin, margin);
            tromba->setBounds (totalArea);
            continueButton->setBounds (debugArea.removeFromRight (100));
            debugArea.removeFromRight (margin);
            stateLabel->setBounds (debugArea.removeFromLeft (100));
            debugArea.removeFromRight (margin);
            currentSampleLabel->setBounds (debugArea.removeFromLeft (100));
        } else {
            tromba->setBounds (totalArea.removeFromTop(getHeight()));
        }
    }
}

void MainComponent::timerCallback()
{
    repaint();
    if (Global::debug)
    {
        stateLabel->setText (String (trombaString->getStateAt (1, floor(bridgeLocRatio * trombaString->getNumPoints()))), dontSendNotification);
//        stateLabel->setText (String (trombaString->getStateAt (1, floor (trombaString->getNumPoints() / 2.0))), dontSendNotification);
        currentSampleLabel->setText (String (curSample), dontSendNotification);
    }
}

void MainComponent::hiResTimerCallback()
{
    double maxVb = 1;
    double maxFb = 0.1;
    for (auto sensel : sensels)
    {
        double finger0X = 0;
        double finger0Y = 0;
        if (sensel->senselDetected)
        {
            sensel->check();
            unsigned int fingerCount = sensel->contactAmount;
            int index = sensel->senselIndex;
            for (int f = 0; f < fingerCount; f++)
            {
                bool state = sensel->fingers[f].state;
                float x = sensel->fingers[f].x;
                float y = sensel->fingers[f].y;
                float Vb = -sensel->fingers[f].delta_y * 0.5;
                float Fb = sensel->fingers[f].force;
                int fingerID = sensel->fingers[f].fingerID;
                trombaString->setFingerPos (0);
                if (f == 0 && state) //fingerID == 0)
                {
                    finger0X = x;
                    finger0Y = y;
                    Vb = Global::clamp (Vb, -maxVb, maxVb);
                    Fb = Global::clamp (Fb, 0, maxFb);
                    trombaString->setBowingParameters (x, y, Fb, Vb, false);
                }
                else if (fingerID > 0)
                {
                    //                    float dist = sqrt ((finger0X - x) * (finger0X - x) + (finger0Y - y) * (finger0Y - y));
                    float verDist = std::abs(finger0Y - y);
                    float horDist = std::abs(finger0X - x);
                    //                    std::cout << horDist << std::endl;
                    if (!(verDist <= 0.3 && horDist < 0.05))
                    {
                        trombaString->setFingerPos (x);
                    }
                }
            }
            
            if (fingerCount == 0)
            {
                trombaString->disableBowing();
            }
        }
    }
}

void MainComponent::buttonClicked (Button* button)
{
    if (button == continueButton.get())
    {
        continueFlag.store (true);
    }
}
