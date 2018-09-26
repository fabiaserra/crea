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

#include "Particle.h"

Particle::Particle()
    : m_id(0)
    , m_age(0.f)
    , m_mass(1.f * 0.005f)
    , m_lifetime(10.f)
    , m_initialRadius(1.f)
    , m_radius(1.f)
    , m_initialHue(255)
    , m_opacity(255.f)
    , m_pos(ofPoint(0.f, 0.f, 0.f))
    , m_prevPos(ofPoint(0.f, 0.f, 0.f))
    , m_initialPos(ofPoint(0.f, 0.f, 0.f))
    , m_vel(ofPoint(0.f, 0.f, 0.f))
    , m_acc(0.f)
    , m_color(ofColor(255.f, 255.f, 255.f))

    // Aging properties
    , m_changeSizeWithAge(false)
    , m_changeOpacityWithAge(false)
    , m_changeColorWithAge(false)
    , m_flickerWithAge(false)

    // Graphic properties
    , m_isEmpty(false)
    , m_drawLine(false)
    , m_drawContour(false)
    , m_contourWidth(1.f)

    // Physical properties
    , m_immortal(false)
    , m_isAlive(true)
    , m_isTouched(false)
    , m_limitSpeed(false)
    , m_friction(0.f)
    , m_bounceDamping(true)
    , m_damping(0.f)
    , m_bounces(true)
    , m_bounceTop(true)
    , m_steers(false)
    , m_infiniteWalls(false)

    // Flocking
    , m_flockingRadiusSqrd(60.f*60.f)
    , m_lowThresh(0.1333) // separate
    , m_highThresh(0.6867) // align
    , m_separationStrength(0.1f)
    , m_attractionStrength(0.04f)
    , m_alignmentStrength(0.1f)
    , m_maxSpeed(120.f)

    , m_width(ofGetWidth())
    , m_height(ofGetHeight())
{
}

Particle::Particle(int id, float radius, float lifetime, ofColor color, ofPoint pos,
        ofPoint vel, bool changeSizeWithAge, bool changeOpacityWithAge, bool changeColorWithAge,
        bool flickerWithAge, bool isEmpty, bool drawLine, bool drawContour, bool contourWidth,
        bool immortal, bool limitSpeed, float friction, float damping, int width, int height)
    : m_id(id)
    , m_age(0.f)
    , m_mass(m_radius * m_radius * 0.005f)
    , m_lifetime(lifetime)
    , m_initialRadius(radius)
    , m_radius(radius)
    , m_initialHue(color.getHue())
    , m_opacity(255.f)

    , m_pos(pos)
    , m_prevPos(pos)
    , m_initialPos(pos)
    , m_vel(vel)
    , m_acc(0.f)
    , m_color(color)

    // Aging properties
    , m_changeSizeWithAge(changeSizeWithAge)
    , m_changeOpacityWithAge(changeOpacityWithAge)
    , m_changeColorWithAge(changeColorWithAge)
    , m_flickerWithAge(flickerWithAge)

    // Graphic properties
    , m_isEmpty(isEmpty)
    , m_drawLine(drawLine)
    , m_drawContour(drawContour)
    , m_contourWidth(contourWidth)

    // Physical properties
    , m_immortal(immortal)
    , m_isAlive(true)
    , m_isTouched(false)
    , m_limitSpeed(limitSpeed)
    , m_friction(friction)
    , m_bounceDamping(true)
    , m_damping(damping)
    , m_bounces(false)
    , m_bounceTop(true)
    , m_steers(false)
    , m_infiniteWalls(false)

    // Flocking
    , m_flockingRadiusSqrd(60.f*60.f)
    , m_lowThresh(0.1333) // separate
    , m_highThresh(0.6867) // align
    , m_separationStrength(0.1f)
    , m_attractionStrength(0.04f)
    , m_alignmentStrength(0.1f)
    , m_maxSpeed(120.f)

    , m_width(width)
    , m_height(height)
{
}

Particle::Particle(Particle& particle)
{
    m_id = particle.m_id;
    m_age = particle.m_age;
    m_mass = particle.m_mass;
    m_lifetime = particle.m_lifetime;
    m_initialRadius = particle.m_initialRadius;
    m_radius = particle.m_radius;
    m_initialHue = particle.m_initialHue;
    m_opacity = particle.m_opacity;

    m_pos = particle.m_pos;
    m_prevPos = particle.m_prevPos;
    m_initialPos = particle.m_initialPos;
    m_vel = particle.m_vel;
    m_acc = particle.m_acc;
    m_color = particle.m_color;

    // Aging properties
    m_changeSizeWithAge = particle.m_changeSizeWithAge;
    m_changeOpacityWithAge = particle.m_changeOpacityWithAge;
    m_changeColorWithAge = particle.m_changeColorWithAge;
    m_flickerWithAge = particle.m_flickerWithAge;

    // Graphic properties
    m_isEmpty = particle.m_isEmpty;
    m_drawLine = particle.m_drawLine;
    m_drawContour = particle.m_drawContour;
    m_contourWidth = particle.m_contourWidth;

    // Physical properties
    m_immortal = particle.m_immortal;
    m_isAlive = particle.m_isAlive;
    m_isTouched = particle.m_isTouched;
    m_limitSpeed = particle.m_limitSpeed;
    m_friction = particle.m_friction;
    m_bounceDamping = particle.m_bounceDamping;
    m_damping = particle.m_damping;
    m_bounces = particle.m_bounces;
    m_bounceTop = particle.m_bounceTop;
    m_steers = particle.m_steers;
    m_infiniteWalls = particle.m_infiniteWalls;

    // Flocking
    m_flockingRadiusSqrd = particle.m_flockingRadiusSqrd;
    m_lowThresh = particle.m_lowThresh;
    m_highThresh = particle.m_highThresh;
    m_separationStrength = particle.m_separationStrength;
    m_attractionStrength = particle.m_attractionStrength;
    m_alignmentStrength = particle.m_alignmentStrength;
    m_maxSpeed = particle.m_maxSpeed;

    m_width = particle.m_width;
    m_height = particle.m_height;
}

Particle::~Particle()
{
}

void Particle::update(float dt)
{
    if (m_isAlive)
    {
        // Update position
        m_acc = m_force / m_mass; // Newton's second law F = m*a
        m_vel += m_acc * dt; // Euler's method
        m_vel *= m_friction; // Decay velocity
        if (m_limitSpeed)
        {
            limitVelocity();
        }
        m_pos += m_vel * dt;
        m_force.set(0.f, 0.f); // Restart force

        // Update age and check if particle has to die
        m_age += dt;
        if (!m_immortal && m_age >= m_lifetime)
        {
            m_isAlive = false;
        }
        else if (m_immortal)
        {
            m_age = fmodf(m_age, m_lifetime);
        }
        
        float normalizedAgeInv = 1.0f - m_age / m_lifetime;
        
        // Decrease particle radius with age
        if (m_changeSizeWithAge)
        {
            m_radius = m_initialRadius * normalizedAgeInv;
        }

        // Decrease particle opacity with age
        if (m_changeOpacityWithAge)
        {
            m_opacity *= normalizedAgeInv;
        }
        
        // Make particle opacity flicker when about to die
        if (m_flickerWithAge)
        {
            // If it's about to die, we half its opacity with increasing probability
            if ((m_age / m_lifetime) > 0.75 && ofRandomuf() < normalizedAgeInv)
            {
                m_opacity *= 0.5f;
            }
        }

        // Change particle color with age
        if (m_changeColorWithAge)
        {
            m_color.setBrightness(ofMap(m_age, 0.f, m_lifetime, 255.f, 180.f));
            m_color.setHue(ofMap(m_age, 0.f, m_lifetime, m_initialHue, m_initialHue - 100.f));
        }

        // hackish way to make particles glitter when they slow down a lot
//        if(vel.lengthSquared() < 5.0)
//        {
//            vel.x = ofRandom(-10, 10);
//            vel.y = ofRandom(-10, 10);
//        }

        // Bounce particle with the window margins
        if (m_bounces)
        {
            marginsBounce();
        }
        else if (m_steers)
        {
            marginsSteer();
        }
        else if (m_infiniteWalls)
        {
            marginsWrap();
        }
    }
}

void Particle::draw()
{
    if (m_isAlive)
    {
        ofPushStyle();
        ofSetColor(m_color, m_opacity);

        if (m_isEmpty)
        {
            ofNoFill();
            ofSetLineWidth(2);
        }
        else
        {
            ofFill();
        }

        if (!m_drawLine)
        {
            // Set the resolution of the circle based on the radius to
            // make it more efficient.
            // The larger the radius the bigger the resolution needs to be
            // so we can't notice the segments
            int resolution = ofMap(fabs(m_radius), 0, 10, 6, 22, true);
            ofSetCircleResolution(resolution);
            ofDrawCircle(m_pos, m_radius);
            if (m_drawContour)
            {
                ofPushStyle();
                ofNoFill();
                ofSetLineWidth(m_contourWidth);
                ofSetColor(0.f, m_opacity);
                ofDrawCircle(m_pos, m_radius);
                ofPopStyle();
            }
        }
        else
        {
//            if (m_pos.squareDistance(m_prevPos) >= 25.f)
//            {
//                ofDrawLine(m_pos, m_pos - m_vel.getNormalized() * 5.f);
//            }
//            else
//            {
//                ofDrawLine(m_pos, m_prevPos);
//            }
//            m_prevPos = m_pos;
            
            ofSetLineWidth(ofMap(m_radius, 0.f, 15.f, 1.f, 5.f, true));
            ofDrawLine(m_pos, m_pos - m_vel.getNormalized() * m_radius);
        }

        ofPopStyle();
    }
}

void Particle::addForce(ofPoint force)
{
    m_force += force;
}

void Particle::addNoise(float turbulence)
{
    // Perlin noise
    float angle = ofSignedNoise(m_id * 0.001f, m_pos.x * 0.005f, m_pos.y * 0.005f, ofGetElapsedTimef() * 0.1f) * 20.0f;
    
    ofPoint noiseVector(cos(angle), sin(angle));
    if (m_immortal)
    {
        m_force += noiseVector * turbulence;
    }
    else
    {
        m_force += noiseVector * turbulence * m_age; // if immortal this doesn't affect, age == 0
    }
}

void Particle::addRepulsionForce(ofPoint posOfForce, float radiusSqrd, float scale)
{
    // (1) calculate the direction to force source and distance
    ofPoint dir = m_pos - posOfForce;
    float distSqrd = m_pos.squareDistance(posOfForce); // faster than length or distance (no square root)

    // (2) if close enough update force
    if (distSqrd < radiusSqrd)
    {
        float pct = 1 - (distSqrd / radiusSqrd);  // stronger on the inside
        dir.normalize();
        m_force += dir * scale * pct;
    }
}

void Particle::addRepulsionForce(Particle &p, float radiusSqrd, float scale)
{
    // (1) make a vector of where this particle p is:
    ofPoint posOfForce(p.position().x, p.position().y);
    
    // (2) calculate the direction to particle and distance
    ofPoint dir = m_pos - posOfForce;
    float distSqrd = m_pos.squareDistance(posOfForce); // faster than length or distance (no square root)
    
    // (3) if close enough update both forces
    if (distSqrd < radiusSqrd)
    {
        float pct = 1 - (distSqrd / radiusSqrd);  // stronger on the inside
        dir.normalize();
        m_force += dir * scale * pct;
        p.m_force -= dir * scale * pct;
    }
}

void Particle::addRepulsionForce(Particle &p, float scale)
{
    // (1) make radius of repulsion equal to particle's radius sum
    float radius = m_radius + p.m_radius;
    float radiusSqrd = radius*radius;
    // (2) call addRepulsion force with the computed radius
    addRepulsionForce(p, radiusSqrd, scale);
}

void Particle::addAttractionForce(ofPoint posOfForce, float radiusSqrd, float scale)
{
    // (1) calculate the direction to force source and distance
    ofPoint dir = m_pos - posOfForce;
    float distSqrd = m_pos.squareDistance(posOfForce); // faster than length or distance (no square root)

    // (2) if close enough update force
    if (distSqrd < radiusSqrd)
    {
        float pct = 1 - (distSqrd / radiusSqrd);  // stronger on the inside
        dir.normalize();
        m_force -= dir * scale * pct;
    }
}

void Particle::addAttractionForce(Particle &p, float radiusSqrd, float scale)
{
    // (1) make a vector of where this particle p is:
    ofPoint posOfForce;
    posOfForce.set(p.position().x, p.position().y);
    
    // (2) calculate the direction to particle and distance
    ofPoint dir = m_pos - posOfForce;
    float distSqrd = m_pos.squareDistance(posOfForce); // faster than length or distance (no square root)
    
    // (3) if close enough update both forces
    if (distSqrd < radiusSqrd)
    {
        float pct = 1 - (distSqrd / radiusSqrd);  // stronger on the inside
        dir.normalize();
        m_force -= dir * scale * pct;
        p.m_force += dir * scale * pct;
    }
}

void Particle::returnToOrigin(float radiusSqrd, float scale)
{
    // (1) calculate the direction to origin position and distance
    ofPoint dirToOrigin = m_initialPos - m_pos;
    float distSqrd = m_pos.squareDistance(m_initialPos);
    
    // (2) set force depending on the distance
    float pct = 1;
    if (distSqrd < radiusSqrd)
    {
        pct = distSqrd / radiusSqrd; // decrease force when closer to origin
    }
    
    // (3) update force
    dirToOrigin.normalize();
    m_force += dirToOrigin * scale * pct;
}

void Particle::addFlockingForces(Particle &p)
{
    ofPoint dir = m_pos - p.position();
    float distSqrd = m_pos.squareDistance(p.position());
    
    // if neighbor particle within zone radius...
    if (0.01f < distSqrd && distSqrd < m_flockingRadiusSqrd)
    {
        float percent = distSqrd / m_flockingRadiusSqrd;

        // ... and is within the lower threshold limits, separate
        if (percent < m_lowThresh)
        {
            float F = (m_lowThresh / percent - 1.0f) * m_separationStrength;
            dir = dir.getNormalized() * F;
            m_force += dir;
            p.m_force -= dir;
        }
        // ... else if it is within the higher threshold limits, align
        else if (percent < m_highThresh)
        {
            float threshDelta = m_highThresh - m_lowThresh;
            float adjustedPercent = (percent - m_lowThresh) / threshDelta;
            float F = (0.5f - cos(adjustedPercent * M_PI * 2.0f) * 0.5f + 0.5f) * m_alignmentStrength;
            m_force += p.m_vel.getNormalized() * F;
            p.m_force += m_vel.getNormalized() * F;
        }
        // ... else, attract
        else
        {
            float threshDelta = 1.0f - m_highThresh;
            float adjustedPercent = (percent - m_highThresh) / threshDelta;
            float F = (0.5f - cos(adjustedPercent * M_PI * 2.0f) * 0.5f + 0.5f) * m_attractionStrength;
            dir = dir.getNormalized() * F;
            m_force -= dir;
            p.m_force += dir;
        }
    }
}

void Particle::pullToCenter()
{
    ofPoint center(m_width/2, m_height/2);
    ofPoint dirToCenter = m_pos - center;
    float distToCenterSqrd = dirToCenter.lengthSquared();
    float distThresh = 900.0f;

    if (distToCenterSqrd > distThresh)
    {
        dirToCenter.normalize();
        const float pullStrength = 0.000015f;
        m_force -= dirToCenter * ( ( distToCenterSqrd - distThresh ) * pullStrength );
    }
}

void Particle::seek(ofPoint target, float radiusSqrd, float scale)
{
    // (1) calculate the direction to target & length
    ofPoint dirToTarget = target - m_pos;
    float distSqrd = m_pos.squareDistance(target);
    
    // (2) scale force depending on the distance
    float pct = 1;
    if (distSqrd < radiusSqrd)
    {
        pct = distSqrd / radiusSqrd; // decrease force when closer to target
    }
    
    // (3) update force
    dirToTarget.normalize();
    m_force += dirToTarget * scale * pct;
}

// seek target independent of distance
void Particle::seek(ofPoint target, float scale)
{
    // (1) calculate the direction to target & length
    ofPoint dirToTarget = target - m_pos;
    
    // (2) scale force randomly
    float pct = ofRandomuf();
    
    // (3) update force
    dirToTarget.normalize();
    m_vel += dirToTarget * scale * pct;
}


void Particle::marginsBounce()
{
    bool isBouncing = false;
    
    if (m_pos.x > m_width - m_radius)
    {
        m_pos.x = m_width - m_radius;
        m_vel.x *= -1.f;
    }
    else if (m_pos.x < m_radius)
    {
        m_pos.x = m_radius;
        m_vel.x *= -1.f;
    }
    if (m_pos.y > m_height - m_radius)
    {
        m_pos.y = m_height - m_radius;
        m_vel.y *= -1.f;
        isBouncing = true;
    }
    else if (m_bounceTop && m_pos.y < m_radius)
    {
        m_pos.y = m_radius;
        m_vel.y *= -1.f;
    }
    
    // Only apply damping when it bounces on the bottom
    if (isBouncing && m_bounceDamping)
    {
        m_vel *= m_damping;
    }
}

// Particle steers direction before touching the wall
void Particle::marginsSteer()
{
    float margin = m_radius * 10.f;

    if (m_pos.x > m_width - margin)
    {
        m_vel.x -= ofMap(m_pos.x, m_width - margin, m_width, m_maxSpeed / 1000.f, m_maxSpeed / 10.f);
    }
    else if (m_pos.x < margin)
    {
        m_vel.x += ofMap(m_pos.x, 0.f, margin, m_maxSpeed / 10.f, m_maxSpeed / 1000.f);
    }

    if (m_pos.y > m_height - margin){
        m_vel.y -= ofMap(m_pos.y, m_height - margin, m_height, m_maxSpeed / 1000.f, m_maxSpeed / 10.f);
    }
    else if(m_pos.y < margin){
        m_vel.y += ofMap(m_pos.y, 0.f, margin, m_maxSpeed / 10.f, m_maxSpeed / 1000.f);
    }
}

// Particle moves to the opposite wall to simulate there's no margins
void Particle::marginsWrap()
{
    if (m_pos.x - m_radius > (float)m_width){
        m_pos.x = -m_radius;
    }
    else if (m_pos.x + m_radius < 0.f)
    {
        m_pos.x = m_width;
    }

    if (m_pos.y - m_radius > (float)m_height)
    {
        m_pos.y = -m_radius;
    }
    else if (m_pos.y + m_radius < 0.f)
    {
        m_pos.y = m_height;
    }
}

// Bounce particle against a polyLine object
void Particle::contourBounce(ofPolyline contour)
{
    unsigned int index;
    ofPoint contactPoint = contour.getClosestPoint(m_pos, &index);
    ofVec2f normal = contour.getNormalAtIndex(index);
    m_vel = m_vel - 2.f * m_vel.dot(normal)*normal; //reflection vector
    m_vel *= 0.35f; // damping
    m_age += 0.5f;
}

void Particle::kill()
{
    m_isAlive = false;
}

void Particle::limitVelocity()
{
    if (m_vel.lengthSquared() > (m_maxSpeed * m_maxSpeed))
    {
        m_vel.normalize();
        m_vel *= m_maxSpeed;
    }
}

