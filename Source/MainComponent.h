/*
  ==============================================================================

    This file was auto-generated!

  ==============================================================================
*/

#pragma once
#include "../JuceLibraryCode/JuceHeader.h"
#include "../SenselWrapper/SenselWrapper.h"
#include "Tromba.h"
#include <iostream>
#include <fstream>


//==============================================================================
/*
    This component lives inside our window, and this is where you should put all
    your controls and content.
*/
class MainComponent   : public AudioAppComponent, public Timer, public Button::Listener, public Slider::Listener, public HighResolutionTimer
{
public:
    //==============================================================================
    MainComponent();
    ~MainComponent();

    //==============================================================================
    void prepareToPlay (int samplesPerBlockExpected, double sampleRate) override;
    void getNextAudioBlock (const AudioSourceChannelInfo& bufferToFill) override;
    void releaseResources() override;

    //==============================================================================
    void paint (Graphics& g) override;
    void resized() override;
    
    void timerCallback() override;
    void hiResTimerCallback() override;
    
    void buttonClicked (Button* button) override;
    void sliderValueChanged (Slider* slider) override;

private:
    //==============================================================================
    double fs;
    std::unique_ptr<Tromba> tromba;
    
    std::shared_ptr<TrombaString> trombaString;
    std::shared_ptr<Bridge> bridge;
    std::shared_ptr<Body> body;

    // Debug things
    std::unique_ptr<TextButton> continueButton;
    std::unique_ptr<Label> stateLabel;
    std::unique_ptr<Label> currentSampleLabel;
    std::atomic<bool> continueFlag;
    
    double offset;
    unsigned long curSample = 0; // first sample is index 1 (like Matlab)
    
    // Create c code
    std::unique_ptr<TextButton> createCButton;
    
    double bridgeLocRatio;
    double outputStringRatio;

    // Sensel Stuff
    OwnedArray<Sensel> sensels;
    int amountOfSensels = 1;
    
    std::unique_ptr<TextButton> resetButton;
    std::unique_ptr<ToggleButton> graphicsButton;
    OwnedArray<Slider> mixSliders;
    std::vector<float> mixVals;
    std::vector<float> prevMixVals;
    float aG = 0.99;
    
    // Parameter Sliders
    OwnedArray<Slider> parameterSliders;
    std::vector<float> initParams;
    
    // Sensel-interaction-to-parameter-maximums
    double maxVb = 0.5;
    double maxFb = 0.1;
    double maxFn = 0.7;
    double maxNoise = 0.5;
    
    bool quantisePitch = false;
    bool easyControl = false;
    bool graphicsToggle = false;
    
    std::ofstream outputSound;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};
