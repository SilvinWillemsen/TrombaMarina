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
    
    BowModel initBowModel = exponential;
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
        
        resetButton = std::make_unique<TextButton>("reset");
        resetButton->setButtonText ("Reset");
        addAndMakeVisible (resetButton.get());
        resetButton->addListener (this);
        
        graphicsButton = std::make_unique<ToggleButton>("graphics");
        graphicsButton->setButtonText ("Graphics");
        addAndMakeVisible (graphicsButton.get());
        graphicsButton->addListener (this);
    }
    mixVals.resize(3);
    prevMixVals.resize(3);
    for (int i = 0; i < 3; ++i)
    {
        mixSliders.add (new Slider(Slider::LinearBarVertical, Slider::NoTextBox));
        Slider* newSlider = mixSliders[mixSliders.size() - 1];
        newSlider->setRange (0.0, 1.0, 0.001);
        bool volumeDebug = false;
        switch (i)
        {
            case 0:
                newSlider->setValue (volumeDebug ? 1.0 : 0.25);
                break;
            case 1:
                newSlider->setValue (volumeDebug ? 1.0 : 0.0);
                break;
            case 2:
                newSlider->setValue (volumeDebug ? 1.0 : 0.5);
                break;
        }
        mixVals[i] = newSlider->getValue();
        newSlider->addListener (this);
        addAndMakeVisible (newSlider);
        prevMixVals[i] = mixSliders[i]->getValue();
    }
    
    continueFlag.store (true);
    fs = sampleRate;
    
    double k = 1.0 / fs;
    
    NamedValueSet parameters;
    
    offset = 5e-6;
    
    // string
    double r = 0.0005;
    double f0 = 32.0;
    double rhoS = 7850.0;
    double A = r * r * double_Pi;
    double L = 1.90;
    double T = (f0 * f0 * L * L * 4.0) * rhoS * A;
    
    bridgeLocRatio = 1.65 / 1.90;
    outputStringRatio = (1.0 - bridgeLocRatio);
    parameters.set ("L", L);
    parameters.set ("rhoS", rhoS);
    parameters.set ("r", r);
    parameters.set ("A", r * r * double_Pi);
    parameters.set ("T", T);
    parameters.set ("ES", 2e11);
    parameters.set ("Iner", r * r * r * r * double_Pi * 0.25);
    parameters.set ("s0S", 0.1);
    parameters.set ("s1S", 0.05);
    
    // bridge
    parameters.set ("M", 0.001);
    parameters.set ("R", 0.05);
    parameters.set ("w1", 2.0 * double_Pi * 500);
    parameters.set ("offset", offset);
    
    // body
    parameters.set ("rhoP", 50.0);
    parameters.set ("H", 0.01);
    parameters.set ("EP", 2e5);
    parameters.set ("Lx", 1.35);
    parameters.set ("Ly", 0.18);
    parameters.set ("s0P", 2);
    parameters.set ("s1P", 0.05);
    
    // connection
    parameters.set ("K1", 5.0e6);
    parameters.set ("alpha1", 1.3);
    parameters.set ("connRatio", bridgeLocRatio);
    
    // plate collision
    parameters.set ("K2", 5.0e8);
    parameters.set ("alpha2", 1.3);
    parameters.set ("colRatioX", 0.8);
    parameters.set ("colRatioY", 0.75);
    
    int numDynamicParameters = 3;
    initParams.resize (numDynamicParameters);
    for (int i = 0; i < numDynamicParameters; ++i)
    {
        parameterSliders.add (new Slider(Slider::RotaryVerticalDrag, Slider::TextBoxBelow));
        Slider* newSlider = parameterSliders[parameterSliders.size() - 1];
        newSlider->setRange (0.0, 1.0);
        newSlider->setValue (1.0);
        newSlider->addListener (this);
        addAndMakeVisible (newSlider);
    }
//    initParams[0] = *parameters.getVarPointer ("s0P");
//    initParams[1] = *parameters.getVarPointer ("s1P");
//    initParams[2] = *parameters.getVarPointer ("w1");
    
    tromba = std::make_unique<Tromba> (parameters, k, initBowModel);
    addAndMakeVisible (tromba.get());
    
    trombaString = tromba->getString();
    bridge = tromba->getBridge();
    body = tromba->getBody();
    
    trombaString->setFingerPos (bridgeLocRatio * 0.5);
    trombaString->setFingerForce (0.1);
    
    setSize (800, 600);
    Timer::startTimerHz (40);
    
    // start the hi-res timer
    if (sensels.size() != 0)
        if (sensels[0]->senselDetected)
            HighResolutionTimer::startTimer (1000.0 / 150.0); // 150 Hz
    
//    outputSound.open ("outputSound.csv");
    if (Global::debug)
        trombaString->setFingerForce (0.0);
    stringState.open ("stringState.csv");
    bridgeState.open ("bridgeState.csv");
    bodyState.open ("bodyState.csv");
    sampleNumber.open ("sampleNumber.csv");

}

void MainComponent::getNextAudioBlock (const AudioSourceChannelInfo& bufferToFill)
{

    bufferToFill.clearActiveBufferRegion();
    
    float* const channelData1 = bufferToFill.buffer->getWritePointer (0, bufferToFill.startSample);
    float* const channelData2 = bufferToFill.buffer->getWritePointer (1, bufferToFill.startSample);
    
    float output = 0.0;
    float output2 = 0.0;
    
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
                if (curSample > 0 && curSample % 10 == 0)
                {
                    continueFlag.store (false);
                    for (int l = 0; l < trombaString->getNumPoints(); ++l)
                    {
                        stringState << trombaString->getStateAt (1, l) << ", ";
                    }
                    stringState << ";\n ";
                    
                    bridgeState << bridge->getState (1) << ";\n";
                    
                    for (int m = 0; m < body->getNumVertPoints(); ++m)
                    {
                        for (int l = 0; l < body->getNumHorPoints(); ++l)
                        {
                            bodyState << body->getStateAt (1, l, m);
                            if (l != body->getNumHorPoints() - 1)
                                bodyState << ", ";
                        }
                        bodyState << ";\n ";
                    }
                    sampleNumber << curSample << ";\n";
                }
                tromba->setCurSample (curSample);
                tromba->calculateUpdateEqs();
                trombaString->dampingFinger();
                tromba->calculateCollisions();
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
        
        output = tromba->getOutput(outputStringRatio) * (Global::debug ? 1.0 : 3.0 * Global::outputScaling) * prevMixVals[0]
        + tromba->getOutput() * (Global::debug ? 1.0 : 30.0 * Global::outputScaling) * prevMixVals[1]
        + tromba->getOutput(0.8, 0.75) * (Global::debug ? 1.0 : 50.0 * Global::outputScaling) * prevMixVals[2];
//        output = (tromba->getOutput(outputStringRatio) + offset) * Global::outputScaling;
//        output2 = tromba->getOutput(0.8, 0.75) * Global::outputScaling;
        channelData1[i] = Global::outputClamp (0.5 * output);
        channelData2[i] = Global::outputClamp (0.5 * output);
//        outputSound << output << ";\n";
    }
    
    body->checkTinyValues();
}

void MainComponent::releaseResources()
{
    // This will be called when the audio device stops, or when it is being
    // restarted due to a setting change.

    // For more details, see the help for AudioProcessor::releaseResources()
//    outputSound.close();
    stringState.close();
    bridgeState.close();
    bodyState.close();
    sampleNumber.close();

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
            controlArea.reduce (10, 10);
            resetButton->setBounds (controlArea.removeFromBottom(30));
            controlArea.removeFromBottom (10);
//            graphicsButton->setBounds (controlArea.removeFromBottom(30));
//            controlArea.removeFromBottom (10);
//            stateLabel->setBounds (controlArea.removeFromTop (20));
//            controlArea.removeFromTop (10);
//            currentSampleLabel->setBounds (controlArea.removeFromTop (20))
            float oOMSsize = 1.0 / static_cast<float> (mixSliders.size());
            for (int i = 0; i < mixSliders.size(); ++i)
            {
                mixSliders[i]->setBounds(controlArea.removeFromLeft(80.0 * oOMSsize));
            }
            
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
    for (auto sensel : sensels)
    {
        double finger0X = 0;
        double finger0Y = 0;
        if (sensel->senselDetected)
        {
            sensel->check();
            unsigned int fingerCount = sensel->contactAmount;
            int index = sensel->senselIndex;
            if (!easyControl)
                trombaString->setFingerForce (0.0);
            for (int f = 0; f < fingerCount; f++)
            {
                bool state = sensel->fingers[f].state;
                float x = sensel->fingers[f].x;
                float y = sensel->fingers[f].y;
                float Vb = -sensel->fingers[f].delta_y * 0.5;
                float Fb = Global::clamp (sensel->fingers[f].force, 0, maxFb);
                float Fn = Global::clamp (sensel->fingers[f].force * 5.0, 0, maxFn);
                
                int fingerID = sensel->fingers[f].fingerID;
//                std::cout << "Finger " << f << ": " << fingerID << std::endl;
//                trombaString->setFingerPos (0);
                if (fingerID == 0 && state) //fingerID == 0)
                {
                    finger0X = x;
                    finger0Y = y;
                    Vb = Global::clamp (Vb, -maxVb, maxVb);
                    if (trombaString->getBowModel() == exponential)
                    {
                        Fb = Global::clamp (Fb, 0, maxFb);
                        trombaString->setBowingParameters (x, y, Fb, Vb, false);
                    }
                    else if (trombaString->getBowModel() == elastoPlastic)
                    {
                        trombaString->setNoise (abs(Vb) > 0.01 ? Fn * 0.1 : 0);
                        trombaString->setBowingParameters (x, y, Fn, Vb, false);
                    }
 
                }
                else if (fingerID > 0)
                {
                    //                    float dist = sqrt ((finger0X - x) * (finger0X - x) + (finger0Y - y) * (finger0Y - y));
                    float verDist = std::abs(finger0Y - y);
                    float horDist = std::abs(finger0X - x);
                    //                    std::cout << horDist << std::endl;
                    if (!(verDist <= 0.3 && horDist < 0.05))
                    {
                        if (quantisePitch){
                            std::cout << "before " << x << std::endl;
                            x = 1.0 / round (1.0 / x);
                            std::cout << "after " << x << std::endl;
                        }
                        
                        trombaString->setFingerPos (x * bridgeLocRatio);
//                        std::cout << "force = " << sensel->fingers[f].force << std::endl;
                        if (!easyControl)
                            trombaString->setFingerForce (Global::clamp(sensel->fingers[f].force * 10.0, 0.0, 1.0));
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
    else if (button == resetButton.get())
    {
        tromba->reset();
        mixSliders[0]->setValue (0.25);
        mixSliders[1]->setValue (0.0);
        mixSliders[2]->setValue (0.5);
//        if (trombaString->getBowModel() == elastoPlastic)
//        {
//            resetButton->setButtonText("Cur bow: exp");
//            std::cout << "bow model is exponential" << std::endl;
//            trombaString->setBowModel (exponential);
//        }
//        else if (trombaString->getBowModel() == exponential)
//        {
//            resetButton->setButtonText("Cur bow: elasto");
//            std::cout << "bow model is elastoplastic" << std::endl;
//            trombaString->setBowModel (elastoPlastic);
//
//        }
    }
    else if (button == graphicsButton.get())
    {
        if (graphicsToggle)
            Timer::stopTimer();
        else
            Timer::startTimerHz(40);
        graphicsToggle = !graphicsToggle;
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
//    if (slider == parameterSliders[0])
//    {
//        mixVals[0] = mixSliders[0]->getValue();
//        return;
//    }
}
