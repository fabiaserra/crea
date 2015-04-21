#pragma once
#include "ofMain.h"
#include "ofxFlowTools.h"
#include "irMarker.h"
#include "Contour.h"

class Fluid{
    public:
        Fluid();

        void setup(int flowWidth, int flowHeight, int drawWidth, int drawHeight, bool doFasterInternalFormat = false);
        void update(float dt, vector<irMarker> &markers, Contour &contour, float mouseX, float mouseY);
        void updateDrawForces(float dt);
        void draw();
    
        void reset();
        void resetDrawForces();
    
        //--------------------------------------------------------------
        bool isActive;          // Fluid active
        bool particlesActive;   // Particle flow active
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
        vector<ofVec4f> markerForceForces;
        vector<int> markerForceTypes;
        vector<float> markerForceStrengths;
        vector<float> markerForceRadiuses;
        vector<float> markerForceEdges;

    protected:
        ftFluidSimulation fluid;
        ftParticleFlow particleFlow;
    
        ftDisplayScalar displayScalar;
        ftVelocityField	velocityField;
};
