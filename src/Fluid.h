#pragma once
#include "ofMain.h"
#include "ofxFlowTools.h"
#include "irMarker.h"
#include "Contour.h"

class Fluid{
    public:
        Fluid();

        void setup(int flowWidth, int flowHeight, int drawWidth, int drawHeight, bool doFasterInternalFormat = false);
        void update(float dt, vector<irMarker> &markers, Contour &contour);
        void draw();
        
        //--------------------------------------------------------------
        bool isActive;          // Fluid active
        bool particlesActive;   // Particles active
        //--------------------------------------------------------------
        int width;              // Fluid boundaries
        int height;
        //--------------------------------------------------------------
        float red, green, blue;
        //--------------------------------------------------------------
        float opacity;
        //--------------------------------------------------------------
        bool markersInput;          // Input are the IR markers?
        bool contourInput;          // Input is the depth contour?
        float markerRadius;         // Radius of interaction of the markers
        //--------------------------------------------------------------
        bool drawVelocity;
        bool drawVelocityScalar;

    protected:
        ftFluidSimulation fluid;
        ftParticleFlow particleFlow;
    
        ftDisplayScalar displayScalar;
        ftVelocityField	velocityField;
};
