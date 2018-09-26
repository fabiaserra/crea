/*
 * Copyright (C) 2018 Fabia Serra Arrizabalaga
 *
 * This file is part of CREA
 *
 * CREA is free software: you can redistribute it and/or modify it under
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
//#include "IrMarker.h"

class Particle
{
public:
    Particle();

    Particle(int id, float radius, float lifetime, ofColor color, ofPoint pos,
        ofPoint vel, bool changeSizeWithAge, bool changeOpacityWithAge, bool changeColorWithAge,
        bool flickerWithAge, bool isEmpty, bool drawLine, bool drawContour, bool contourWidth,
        bool immortal, bool limitSpeed, float friction, float damping, int width, int height);

    Particle(Particle& particle);

    ~Particle();

    void update(float dt);
    void draw();

    void addForce(ofPoint force);
    void addNoise(float turbulence);
    void addRepulsionForce(ofPoint posOfForce, float radiusSqrd, float scale);
    void addAttractionForce(ofPoint posOfForce, float radiusSqrd, float scale);
    void addRepulsionForce(Particle &p, float radiusSqrd, float scale);
    void addAttractionForce(Particle &p, float radiusSqrd, float scale);
    void addRepulsionForce(Particle &p, float scale);
    void returnToOrigin(float radiusSqrd, float scale);

    void addFlockingForces(Particle &p);
    void seek(ofPoint target, float radiusSqrd, float scale);
    void seek(ofPoint target, float scale);
    void pullToCenter();
    void limitVelocity();

    void marginsBounce();
    void marginsSteer();
    void marginsWrap();

    void contourBounce(ofPolyline contour);

    void kill();

    // Getters
    int id();
    float age();
    float mass();

    ofPoint position();

    
protected:
    int m_id; // Particle ID
    float m_age; // Time of living
    float m_mass; // Mass of the particle
    float m_lifetime; // Allowed lifetime
    float m_initialRadius; // Radius of the particle when borns
    float m_radius; // Radius of the particle
    float m_initialHue; // Initial hue color
    float m_opacity; // Particle opacity
    
    ofPoint m_pos; // Position
    ofPoint m_prevPos; // Previous position
    ofPoint m_initialPos; // Initial position
    ofPoint m_vel; // Velocity
    ofPoint m_acc; // Acceleration
    ofPoint m_force; // Force
    ofColor m_color; // Color

    ofPolyline m_contour; // Particle creation boundaries
    
    // Aging properties
    bool m_changeSizeWithAge; // Particle changes size with age?
    bool m_changeOpacityWithAge; // Particle changes opacity with age?
    bool m_changeColorWithAge; // Particle changes color with age?
    bool m_flickerWithAge; // Particle flickers opacity when about to die?
    
    // Graphic properties
    bool m_isEmpty; // Draw only contour of the particle?
    bool m_drawLine; // Draw particle as a line from prevPos to pos?
    bool m_drawContour; // Draw contour line around particle?
    float m_contourWidth; // Contour line width
    
    // Physical properties
    bool m_immortal; // Can the particle die?
    bool m_isAlive; // Is the particle alive?
    bool m_isTouched; // Particle has been activated through some event
    bool m_limitSpeed; // Limit the speed of the particle?
    float m_friction; // Decay of the velocity
    bool m_bounceDamping; // Decrease velocity when particle bounces?
    float m_damping; // Damping when particle bounces walls
    bool m_bounces; // Particle bounces with the window margins?
    bool m_bounceTop; // Particle bounces with top margin?
    bool m_steers; // Particle steers direction before touching the walls?
    bool m_infiniteWalls; // Particle goes back to the opposite wall?
    
    // Flocking
    float m_flockingRadiusSqrd;
    float m_lowThresh; // threshold of separation
    float m_highThresh; // threshold of alignment
    float m_separationStrength;
    float m_attractionStrength;
    float m_alignmentStrength;
    float m_maxSpeed; // Maximum speed
    
    // Particle boundaries
    int m_width;
    int m_height;
};

inline int Particle::id()
{
    return m_id;
}

inline float Particle::age()
{
    return m_age;
}

inline float Particle::mass()
{
    return m_mass;
}

inline ofPoint Particle::position()
{
    return m_pos;
}

