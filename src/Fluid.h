#pragma once
#include "ofMain.h"
#include "ofxFlowTools.h"
#include "irMarker.h"
#include "Contour.h"

class Fluid{
    public:
        Fluid();

        void setup(int width, int height, float scaleFactor = 4.0, bool doFasterInternalFormat = false);
        void update(float dt, vector<irMarker> &markers, Contour &contour, float mouseX, float mouseY);
        void updateDrawForces(float dt);
        void draw();
    
        ofVec2f getFluidOffset(ofPoint p);
    
        void reset();
        void resetDrawForces();
    
        //--------------------------------------------------------------
        bool isActive;          // Fluid active
        bool particlesActive;   // Particle flow active
        //--------------------------------------------------------------
        bool activeStarted;     // Active has started?
        bool isFadingIn;        // Opacity fading in?
        bool isFadingOut;       // Opacity fading out?
        bool startFadeIn;       // Fade in has started?
        bool startFadeOut;      // Fade out has started?
        float elapsedFadeTime;  // Elapsed time of fade
        float fadeTime;         // Transition time of fade
        //--------------------------------------------------------------
        int width;              // Fluid drawing boundaries
        int height;
        //--------------------------------------------------------------
        int flowWidth;          // Fluid computing boundaries
        int flowHeight;
        //--------------------------------------------------------------
        float red, green, blue;
        //--------------------------------------------------------------
        float opacity;
        float maxOpacity;
        //--------------------------------------------------------------
        float scaleFactor;          // scaling factor of the optical flow image
        //--------------------------------------------------------------
        bool markersInput;          // Fluid input are the IR markers?
        bool contourInput;          // Fluid input is the depth contour?
        bool markersInputParticles; // Particles flow input is the IR markers?
        bool contourInputParticles; // Particles flow input is the depth contour?
        //--------------------------------------------------------------
        bool drawVelocity;
        bool drawVelocityScalar;
        bool drawTemperature;
        //--------------------------------------------------------------
        int	numMarkerForces;
        ftDrawForce* markerForces;
        //--------------------------------------------------------------
        // fluid parameters
        float speed;
        float cellSize;
        int numJacobiIterations;
        float viscosity;
        float vorticity;
        float dissipation;
        float dissipationVelocityOffset;
        float dissipationDensityOffset;
        float dissipationTemperatureOffset;
        float smokeSigma;
        float smokeWeight;
        float ambientTemperature;
        ofPoint gravity;
        float clampForce;
        float maxVelocity;
        float maxDensity;
        float maxTemperature;
        float densityFromVorticity;
        float densityFromPressure;
        //--------------------------------------------------------------
        // particle flow parameters
        float particlesVelocity;
        float particlesCellSize;
        float particlesBirthChance;
        float particlesBirthVelocityChance;
        float particlesLifetime;
        float particlesLifetimeRnd;
        float particlesMass;
        float particlesMassRnd;
        float particlesSize;
        float particlesSizeRnd;
        //--------------------------------------------------------------
        // marker forces parameters
        float markerRed, markerGreen, markerBlue;
        float markerOpacity;
        ofFloatColor markerColor;
        bool markerRandomColor;
        vector<ofVec4f> markerForceForces;
        vector<float> markerForceStrengths;
        vector<float> markerForceRadiuses;
        vector<float> markerForceEdges;

    protected:
        ftFluidSimulation fluid;
        ftParticleFlow particleFlow;
        //--------------------------------------------------------------
        ftDisplayScalar displayScalar;
        ftVelocityField	velocityField;
        //--------------------------------------------------------------
        ofTexture fluidTexture;
        ofFloatPixels fluidPixels;
        //--------------------------------------------------------------
        ofRectangle rescaledRect;
        //--------------------------------------------------------------
        void fadeIn(float dt);
        void fadeOut(float dt);
};
