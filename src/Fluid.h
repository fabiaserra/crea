/*
 * Copyright (C) 2015 Fabia Serra Arrizabalaga
 *
 * This file is part of Crea
 *
 * Crea is free software: you can redistribute it and/or modify it under
 * the terms of the GNU Affero General Public License as published by the Free
 * Software Foundation (FSF), either version 3 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the Affero GNU General Public License
 * version 3 along with this program.  If not, see http://www.gnu.org/licenses/
 */

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
        //--------------------------------------------------------------
        bool drawVelocity;
        bool drawVelocityScalar;
        bool drawPressure;
        bool drawVorticity;
        bool drawTemperature;
        bool drawMarkerFluid;
        bool drawContourFluid;
        //--------------------------------------------------------------
        int numMarkerForces;
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
        // fluid particles parameters
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
        ftParticleFlow fluidParticles;
        //--------------------------------------------------------------
        ftDisplayScalar displayScalar;
        ftDisplayScalar densityDisplayScalar;
        ftVelocityField velocityField;
        //--------------------------------------------------------------
        ofTexture fluidFlow;
        ofFbo fluidFbo;
        ofFloatPixels fluidVelocities;
        //--------------------------------------------------------------
        ofRectangle rescaledRect;
        //--------------------------------------------------------------
        void fadeIn(float dt);
        void fadeOut(float dt);
};
