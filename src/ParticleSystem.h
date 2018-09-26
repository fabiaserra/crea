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

//#include "ofMain.h"
#include "Particle.h"
//#include "irMarker.h"
//#include "Contour.h"
//#include "Fluid.h"

enum ParticleMode {EMITTER, BOIDS, GRID, RANDOM, ANIMATIONS};
enum InputSource {MARKERS, CONTOUR};
enum Animation {SNOW, RAIN, EXPLOSION};

class ParticleSystem
{
public:
    ParticleSystem();
    ~ParticleSystem();

    void setup(ParticleMode particleMode, int width, int height);

    // void update(float dt, vector<irMarker>& markers, Contour& contour, Fluid& fluid);
    void update(float dt);

    void draw();

    void addParticle(ofPoint pos, ofPoint vel, ofColor color, float radius, float lifetime);
    void addParticles(int numParticles);
    // void addParticles(int numParticles, const irMarker& marker);
    // void addParticles(int numParticles, const ofPolyline& contour, Contour& flow);

    void createParticleGrid();

    // void removeParticles(int numParticles);
    // void killParticles();
    // void resetParticles();
    // void resetTouchedParticles();

    // void setAnimation(Animation animation);

private:
    bool m_isActive; // Particle system active

    bool m_activeStarted; // Active has started?
    bool m_isFadingIn; // Opacity fading in?
    bool m_isFadingOut; // Opacity fading out?
    bool m_startFadeIn; // Fade in has started?
    bool m_startFadeOut; // Fade out has started?
    float m_elapsedFadeTime; // Elapsed time of fade
    float m_fadeTime; // Transition time of fade

    // Particle system boundaries
    int m_width;
    int m_height;

    float m_opacity;
    float m_maxOpacity;

    vector<std::shared_ptr<Particle>> m_particles;

//    int m_totalParticlesCreated;

    ParticleMode m_particleMode;

    // Animation m_animation;

    // General properties
    bool m_immortal; // Can the particles die?
    float m_velocity; // Initial velocity magnitude of newborn particles
    float m_radius; // Radius of the particles
    float m_lifetime; // Lifetime of particles
    float m_red, m_green, m_blue; // Color of the particles

    // Specific properties
    int m_numParticles; // Total number of particles
    float m_bornRate; // Number of particles born per frame

    // Emitter
    float m_emitterSize; // Size of the emitter area
    float m_velocityRnd; // Magnitude randomness % of the initial velocity
    float m_velocityMotion; // Marker motion contribution to the initial velocity
    float m_lifetimeRnd; // Randomness of lifetime
    float m_radiusRnd; // Randomness of radius

    // Grid
    int m_gridRes; // Resolution of the grid

     // Flocking
    float m_lowThresh; // If dist. ratio lower than lowThresh separate
    float m_highThresh; // Else if dist. ratio higher than highThresh attract. Else just align
    float m_separationStrength; // Separation force
    float m_attractionStrength; // Attraction force
    float m_alignmentStrength; // Alignment force
    float m_maxSpeed; // Max speed particles
    float m_flockingRadius; // Radius of flocking

    // Graphic output
    bool m_changeSizeWithAge; // Particles change size with age?
    bool m_changeOpacityWithAge; // Particles change opacity with age?
    bool m_changeColorWithAge; // Particles change color with age?
    bool m_flickerWithAge; // Particles flicker opacity when about to die?
    bool m_isEmpty; // Particles are empty inside, only draw the contour?
    bool m_drawLine; // Draw a line instead of a circle for the particle?
    bool m_drawContour; // Draw stroke line around particle?
    float m_contourWidth; // Stroke line width
    bool m_drawConnections; // Draw a connecting line between close particles?
    float m_connectDist; // Maximum distance to connect particles with line
    float m_connectDistSqrd; // Maximum distance (squared) to connect particles with line
    float m_connectWidth; // Connected line width

    // Physics
    float m_friction; // Friction to velocity 0~100
    bool m_limitSpeed; // Limit the speed of the particle?
    ofPoint m_gravity; // Makes particles react to gravity
    float m_turbulence; // Turbulence perlin noise
    bool m_bounce; // Particles bounce with the window margins?
    bool m_steer; // Particles steer direction with the window margins?
    bool m_infiniteWalls; // Infinite walls?
    bool m_bounceDamping; // Decrease velocity when particle bounces walls?
    float m_damping; // Damping when particle bounces walls
    bool m_repulse; // Repulse particles between each other?
    float m_repulseDist; // Repulse particle-particle distance
    float m_returnToOriginForce;  // How fast particle returns to its position

    // Behavior
    bool m_interact; // Can we interact with the particles?
    bool m_emit; // Born new particles in each frame?
    bool m_flock; // Particles have flocking behavior?
    bool m_flowInteraction; // Interact with particles using optical flow?
    bool m_fluidInteraction; // Interact with the fluid velocities?
    bool m_repulseInteraction; // Repulse particles from input?
    bool m_attractInteraction; // Attract particles to input?
    bool m_seekInteraction; // Make particles seek target (markers)?
    bool m_gravityInteraction; // Apply gravity force to particles touched with input?
    bool m_bounceInteraction; // Bounce particles with depth contour?
    bool m_returnToOrigin; // Make particles return to their born position?

    // Input
    bool m_markersInput; // Input are the IR markers?
    bool m_contourInput; // Input is the depth contour?
    float m_interactionForce; // Force of interaction
    float m_interactionRadius; // Radius of interaction
    bool m_emitInMovement; // Emit particles only in regions that there has been some movement?
    bool m_emitAllTimeInside; // Emit particles every frame inside all the defined area?
    bool m_emitAllTimeContour; // Emit particles every frame only on the contour of the defined area?
    bool m_useFlow; // Use optical flow to get the motion velocity?
    bool m_useFlowRegion; // Use optical flow region to get the motion velocity?
    bool m_useContourArea; // Use contour area to interact with particles?
    bool m_useContourVel; // Use contour velocities to interact with particles?

protected:
    // Helper functions
    ofPoint randomVector();
    float randomRange(float percentage, float value);
    
    //    irMarker* getClosestMarker(const Particle& particle, vector<irMarker>& markers, float interactionRadiusSqrd);
    //    irMarker* getClosestMarker(const Particle &particle, vector<irMarker>& markers);
    //    ofPoint getClosestPointInContour(const Particle& particle, const Contour& contour, bool onlyInside = true, unsigned int* contourIdx = NULL);

    void fadeIn(float dt);
    void fadeOut(float dt);

    void repulseParticles();
    void flockParticles();
};
