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
    system("rm *.so");
    system("rm -rf *.so.dSYM");
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
//        stateLabel->setColour (Label::backgroundColourId, Colours::black);
        addAndMakeVisible (stateLabel.get());
        
        currentSampleLabel = std::make_unique<Label>("CurrentSampleLabel");
        currentSampleLabel->setColour (Label::textColourId, Colours::white);
//        currentSampleLabel->setColour (Label::backgroundColourId, Colours::black);
        addAndMakeVisible (currentSampleLabel.get());
        
    }
    
    fs = sampleRate;
    
    double k = 1.0 / fs;
    
    NamedValueSet parameters;
    
    // string
    double r = 0.0005;
    double f0 = 45.0;
    double rhoS = 7850.0;
    double A = r * r * double_Pi;
    double T = (f0 * f0 * 4.0) * rhoS * A;
    bridgeLocRatio = 0.9;
    parameters.set ("rhoS", rhoS);
    parameters.set ("r", r);
    parameters.set ("A", r * r * double_Pi);
    parameters.set ("T", T);
    parameters.set ("ES", 2e11);
    parameters.set ("Iner", r * r * r * r * double_Pi * 0.25);
    parameters.set ("s0S", 1);
    parameters.set ("s1S", 0.005);
    
    // bridge
    parameters.set ("M", 0.001);
    parameters.set ("R", 1.0);
    parameters.set ("w1", 2.0 * double_Pi * 500.0);
    parameters.set ("offset", 1e-6);
    
    // body
    parameters.set ("rhoP", 7850.0);
    parameters.set ("H", 0.001);
    parameters.set ("EP", 2e11);
    parameters.set ("Lx", 1.5);
    parameters.set ("Ly", 0.3);
    parameters.set ("s0P", 5);
    parameters.set ("s1P", 0.01);
    parameters.set ("colRatioX", bridgeLocRatio);
    parameters.set ("colRatioY", 0.5);
    
    // collision
    parameters.set ("K", 5.0e12);
    parameters.set ("alpha", 1.0);
    
    // connection
    parameters.set ("K1", 1e6);
    parameters.set ("K3", 100);
    parameters.set ("sx", 1.0);
    parameters.set ("connRatio", bridgeLocRatio);
    
    tromba = std::make_unique<Tromba> (parameters, k);
    addAndMakeVisible (tromba.get());
    
    trombaString = tromba->getString();
    bridge = tromba->getBridge();
    body = tromba->getBody();
    
    setSize (800, 600);
    Timer::startTimerHz (40);
    
    trombaString->setFingerPos (bridgeLocRatio * 0.5);
    
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
                if (curSample % 1000 == 0)
                    continueFlag.store (false);
                tromba->setCurSample (curSample);
                tromba->calculateUpdateEqs();
                tromba->calculateCollision();
                tromba->calculateConnection();
                tromba->solveSystem();
                tromba->updateStates();
                ++curSample;
            }
        } else {
//            if (continueFlag.load() == true)
//            {
////                if (curSample ==)
//                    continueFlag.store (false);
                tromba->calculateUpdateEqs();
                trombaString->dampingFinger();
                tromba->calculateCollision();
                tromba->calculateConnection();
                tromba->solveSystem();
                tromba->updateStates();
//            }
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
        if (Global::debug)
        {
            int margin = 5;
            Rectangle<int> totalArea = getLocalBounds();
            Rectangle<int> debugArea = totalArea.removeFromBottom (Global::debugButtonsHeight);
            debugArea.reduce (margin, margin);
            tromba->setBounds (totalArea);
            continueButton->setBounds (debugArea.removeFromRight (100));
            debugArea.removeFromRight (margin);
            stateLabel->setBounds (debugArea.removeFromLeft (100));
            debugArea.removeFromRight (margin);
            currentSampleLabel->setBounds (debugArea.removeFromLeft (100));
        } else {
            tromba->setBounds (getLocalBounds());
        }
    }
}

void MainComponent::timerCallback()
{
    repaint();
    if (Global::debug)
    {
        stateLabel->setText (String (body->getStateAt(1, 5, 5)), dontSendNotification);
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
