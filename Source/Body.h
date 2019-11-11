/*
  ==============================================================================

    Body.h
    Created: 21 Oct 2019 4:49:29pm
    Author:  Silvin Willemsen

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "Global.h"
//#include "../Source/code.c"
#include <stdio.h>
#include <dlfcn.h>
#include <sstream> //for std::stringstream
#include <string>  //for std::string


//==============================================================================
/*
*/
class Body    : public Component
{
public:
    Body (NamedValueSet& parameters, double k);
    ~Body();

    void paint (Graphics&) override;
    void resized() override;

    void calculateUpdateEq();
    void updateStates();
    
    void excite();
    
    
    double getOutput (double ratioX, double ratioY) {
#ifdef CREATECCODE
        int idx = floor (Nx * ratioX) + floor (ratioY * Ny * Nx);
        return u[1][idx];
#else
        int idX = floor(ratioX * Nx);
        int idY = floor(ratioY * Ny);
        return u[1][idX][idY];
#endif
        
    };
    double getStateAt (int time, int idX, int idY) {
#ifdef CREATECCODE
        return u[time][idX + Nx * idY];
#else
        return u[time][idX][idY];
#endif
    };

    void addToStateAt (int idX, int idY, double val) {
#ifdef CREATECCODE
        u[0][idX + Nx * idY] += val;
#else
        u[0][idX][idY] += val;
#endif  
    }; // always uNext

    int getNumHorPoints() { return Nx; };
    int getNumVertPoints() { return Ny; };
    
    void mouseDrag (const MouseEvent& e) override;
    
    bool isExcited() { return exciteFlag; };
    double getGridSpacing() { return h; };
    
#ifdef CREATECCODE
    void updateEqGenerator1();
    void updateEqGenerator2();
    void updateEqGenerator3();
    const char* toConstChar (String string) { return static_cast<const char*> (string.toUTF8()); }
    bool isDoneCreatingCCode() { return doneCreatingCCode; };
    void increaseCont() { ++cont; };
    int getCont() { return cont; };
#endif
private:
    double k, h;
    int N, Nx, Ny;
    
    double rho, H, E, s0, s1, Lx, Ly, kappaSq;
    
#ifdef CREATECCODE
    // pointers to the different states
    std::vector<double*> u;
    // states
    std::vector<std::vector<double>> uVecs;
    
    std::vector<double> coefficients;
    void (*updateEq) (double* uNext, double* u, double* uPrev, double* parameters, int Nx);
    unsigned long curName = 0;
    
#else
    // pointers to the different states
    std::vector<std::vector<double>*> u;
    // states
    std::vector<std::vector<std::vector<double>>> uVecs;
#endif

    
    // update equation constants
    double A1, A2, A3, A4, A5, A6, B1, B2, D;
    
    int excitationWidth;
//    bool exciteFlag = Global::initialiseWithExcitation ? (Global::exciteBody ? true : false) : false;
    bool exciteFlag = false;
    int idX, idY;
    
#ifdef CREATECCODE
    bool doneCreatingCCode = false;
    std::unique_ptr<Label> contLabel;
    int cont = 0;
    void *handle;
    char *error;
    std::hash<int64> hasher;
    long curTime;
    unsigned long newName;
    String forloop;
    const char* eq;
    FILE *fd;
    String systemInstr;
#endif
   
   
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Body)
};
