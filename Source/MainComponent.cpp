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
        
    } else {
        stateLabel = std::make_unique<Label>("StateLabel");
        stateLabel->setColour (Label::textColourId, Colours::white);
        addAndMakeVisible (stateLabel.get());
        
        currentSampleLabel = std::make_unique<Label>("CurrentSampleLabel");
        currentSampleLabel->setColour (Label::textColourId, Colours::white);
        addAndMakeVisible (currentSampleLabel.get());
        
        outputButton = std::make_unique<TextButton>("ContinueButton");
        outputButton->setButtonText ("change output");
        addAndMakeVisible (outputButton.get());
        outputButton->addListener (this);
        
        mixVals.resize(3);
        prevMixVals.resize(3);
        for (int i = 0; i < 3; ++i)
        {
            mixSliders.add (new Slider(Slider::LinearBarVertical, Slider::NoTextBox));
            Slider* newSlider = mixSliders[mixSliders.size() - 1];
            newSlider->setRange (0, 1);
            newSlider->setValue (0.5);
            mixVals[i] = newSlider->getValue();
            newSlider->addListener (this);
            addAndMakeVisible (newSlider);
            prevMixVals[i] = mixSliders[i]->getValue();
        }
        
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
    parameters.set ("rhoP", 10.0);
    parameters.set ("H", 0.01);
    parameters.set ("EP", 2e5);
    parameters.set ("Lx", 1.5);
    parameters.set ("Ly", 0.4);
    parameters.set ("s0P", 5);
    parameters.set ("s1P", 0.05);
    
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

        // lowpass mixcontrol
        for (int j = 0; j < 3; ++j)
            prevMixVals[j] = aG * prevMixVals[j] + (1-aG) * mixVals[j];
        
        output = tromba->getOutput(9.0 / 10.0) * (Global::debug ? 1.0 : 3.0 * Global::outputScaling) * prevMixVals[0]
                + tromba->getOutput() * (Global::debug ? 1.0 : 3.0 * Global::outputScaling) * prevMixVals[1]
                + tromba->getOutput(0.8, 0.5) * (Global::debug ? 1.0 : 50.0 * Global::outputScaling) * prevMixVals[2];
//        else
//            output = tromba->getOutput(0.8, 0.5) * (Global::debug ? 1.0 : 50.0 * Global::outputScaling);
        channelData1[i] = Global::outputClamp (output);
        channelData2[i] = Global::outputClamp (output);
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
            Rectangle<int> controlArea = totalArea.removeFromRight(100);
            tromba->setBounds (totalArea.removeFromTop(getHeight()));
            outputButton->setBounds(controlArea.removeFromBottom(100));
            stateLabel->setBounds (controlArea.removeFromTop (20));
            controlArea.removeFromTop (10);
            currentSampleLabel->setBounds (controlArea.removeFromTop (20));
            float oOMSsize = 1.0 / static_cast<float> (mixSliders.size());
            for (int i = 0; i < mixSliders.size(); ++i)
            {
                mixSliders[i]->setBounds(controlArea.removeFromLeft(100.0 * oOMSsize));
            }
//            mixSliders[0]->setBounds(controlArea)
        }
    }
    
}

void MainComponent::timerCallback()
{
    repaint();
//    if (Global::debug)
//    {
        stateLabel->setText(String(trombaString->getVb()) + " " + String(trombaString->getBowPos()), dontSendNotification);
//        stateLabel->setText (String (trombaString->getStateAt (1, floor(bridgeLocRatio * trombaString->getNumPoints()))), dontSendNotification);
//        stateLabel->setText (String (trombaString->getStateAt (1, floor (trombaString->getNumPoints() / 2.0))), dontSendNotification);
        currentSampleLabel->setText (String (curSample), dontSendNotification);
//    }
}

void MainComponent::hiResTimerCallback()
{
    double maxVb = 0.5;
#ifdef EXPONENTIALBOW
    double maxFb = 0.1;
#else
    double maxFn = 0.7;
#endif
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
#ifdef EXPONENTIALBOW
                float Fb = sensel->fingers[f].force;
#else
                float Fn = Global::clamp (sensel->fingers[f].force * 10.0, 0, maxFn);
                std::cout << Fn << std::endl;
#endif
                int fingerID = sensel->fingers[f].fingerID;
                trombaString->setFingerPos (0);
                if (f == 0 && state) //fingerID == 0)
                {
                    finger0X = x;
                    finger0Y = y;
                    Vb = Global::clamp (Vb, -maxVb, maxVb);
#ifdef EXPONENTIALBOW
                    Fb = Global::clamp (Fb, 0, maxFb);
                    trombaString->setBowingParameters (x, y, Fb, Vb, false);
#else
                    trombaString->setNoise (Fn * 0.2);
                    trombaString->setBowingParameters (x, y, Fn, Vb, false);
#endif
 
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
    if (button == outputButton.get())
    {
        if (outputMass)
        {
            outputButton->setButtonText("change output");
            std::cout << "plate" << std::endl;
        }
        else
        {
            outputButton->setButtonText("change output");
            std::cout << "mass" << std::endl;
        }
        outputMass = !outputMass;
    }
}

void MainComponent::sliderValueChanged(Slider* slider)
{
    for (int i = 0; i < mixSliders.size(); ++i)
    {
        if (slider == mixSliders[i])
        {
            mixVals[i] = mixSliders[i]->getValue();
            return;
        }
    }
        
}
