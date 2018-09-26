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

#include "ParticleSystem.h"

static bool comparisonFunction(const std::shared_ptr<Particle>& a, const std::shared_ptr<Particle>& b)
{
    return a->position().x < b->position().x;
}

ParticleSystem::ParticleSystem()
    : m_isActive(true) // Particle system is active?
    , m_activeStarted(false) // Active has started?
    , m_isFadingIn(false) // Opacity fading in?
    , m_isFadingOut(false) // Opacity fading out?
    , m_startFadeIn(false) // Fade in has started?
    , m_startFadeOut(false) // Fade out has started?
    , m_elapsedFadeTime(0.f) // Elapsed time of fade
    , m_fadeTime(1.2f) // Transition time of fade

    // Opacity
    , m_opacity(255.f) // Actual opacity of the particles
    , m_maxOpacity(255.f) // Maximum opacity of particles

    // General properties
    , m_immortal(false) // Are particles immortal?
    , m_velocity(0.f) // Initial velocity magnitude of newborn particles
    , m_radius(3.f) // Radius of the particles
    , m_lifetime(5.f) // Lifetime of particles
    , m_red(255.f)
    , m_green(255.f)
    , m_blue(255.f)

    // Specific properties
    , m_numParticles(300) // Number of particles to start with
    , m_bornRate(5.f) // Number of particles born per frame

    // Emitter
    , m_emitterSize(3.f) // Size of the emitter area
    , m_velocityRnd(20.f) // Magnitude randomness % of the initial velocity
    , m_velocityMotion(50.f) // Marker motion contribution to the initial velocity
    , m_lifetimeRnd(20.f) // Randomness of lifetime
    , m_radiusRnd(50.f) // Randomness of radius

    // Grid
    , m_gridRes(10.f) // Resolution of the grid

    // Flocking
    , m_lowThresh(0.1333) // If dist. ratio lower than lowThresh separate
    , m_highThresh(0.6867) // Else if dist. ratio higher than highThresh attract. Else just align
    , m_separationStrength(0.1f) // Separation force
    , m_attractionStrength(0.04f) // Attraction force
    , m_alignmentStrength(0.1f) // Alignment force
    , m_maxSpeed(80.f) // Max speed particles
    , m_flockingRadius(60.f) // Radius of flocking

    // Graphic output
    , m_changeSizeWithAge(false) // Decrease size when particles get older?
    , m_changeOpacityWithAge(false) // Decrease opacity when particles get older?
    , m_changeColorWithAge(false) // Change color when particles get older?
    , m_flickerWithAge(false) // Particle flickers opacity when about to die?
    , m_isEmpty(false) // Draw only contours of the particles?
    , m_drawLine(false) // Draw a line instead of a circle for the particle?
    , m_drawContour(false) // Draw stroke line around particle?
    , m_contourWidth(1.2) // Stroke line width
    , m_drawConnections(true) // Draw a connecting line between close particles?
    , m_connectDist(10.f) // Maximum distance to connect particles with line
    , m_connectDistSqrd(10.f * 10.f) // Maximum distance (squared) to connect particles with line
    , m_connectWidth(2.f) // Connected line width

    // Physics
    , m_friction(5.f) // Friction to velocity 0~100
    , m_gravity(ofPoint(0, 9.8f)) // Makes particles react to gravity
    , m_turbulence(0.f) // Turbulence perlin noise
    , m_repulse(false) // Repulse particles between each other?
    , m_bounce(false) // Bounce particles with the walls of the window?
    , m_steer(false) // Steers direction before touching the walls of the window?
    , m_infiniteWalls(false) // Infinite walls?
    , m_bounceDamping(true) // Decrease velocity when particle bounces walls?
    , m_repulseDist(5.f * m_radius) // Repulse particle-particle distance

    , m_returnToOriginForce (10.f) // How fast particle returns to its position

    // Behavior
    , m_emit(false) // Born new particles in each frame?
    , m_interact(false) // Can we interact with the particles?
    , m_flock(false) // Particles have flocking behavior?
    , m_flowInteraction(false) // Interact with particles using optical flow?
    , m_fluidInteraction(false) // Interact with the fluid velocities?
    , m_repulseInteraction(false) // Repulse particles from input?
    , m_attractInteraction(false) // Attract particles to input?
    , m_seekInteraction(false) // Make particles seek target (markers)?
    , m_gravityInteraction(false) // Apply gravity force to particles touched with input?
    , m_bounceInteraction(false) // Bounce particles with depth contour?
    , m_returnToOrigin(false) // Make particles return to their born position?

    // Input
    , m_markersInput(false) // Input are the IR markers?
    , m_contourInput(false) // Input is the depth contour?
    , m_interactionForce(100.f) // Force of interaction
    , m_interactionRadius(80.f) // Radius of interaction

    , m_emitAllTimeInside(true) // Emit particles every frame inside all the defined area?
    , m_emitAllTimeContour(false) // Emit particles every frame only on the contour of the defined area?
    , m_emitInMovement(false) // Emit particles only in regions that there has been some movement?

    , m_useFlow(true) // Use optical flow to get the motion velocity?
    , m_useFlowRegion(false) // Use optical flow region to get the motion velocity?
    , m_useContourArea(false) // Use contour area to interact with particles?
    , m_useContourVel(false) // Use contour velocities to interact with particles?
{
}

ParticleSystem::~ParticleSystem()
{
}

void ParticleSystem::setup(ParticleMode particleMode, int width, int height)
{
    m_particleMode = particleMode;

    m_width = width;
    m_height = height;

//    if (m_particleMode == EMITTER)
//    {
//        m_emit = true;
//        m_changeSizeWithAge = true; // Decrease size when particles get older?
//        m_changeOpacityWithAge = true; // Decrease opacity when particles get older?
//        m_changeColorWithAge = true; // Change color when particles get older?
//        m_velocity = 20.f;
//    }
    // else if (m_particleMode == GRID)
    // {
         m_interact = true;
         m_radiusRnd = 0.f;
         m_returnToOrigin = true;
         m_immortal = true;
         m_velocity = 0.f;
         m_velocityRnd = 0.f;
         m_gravity = ofPoint(0.f, 0.f);
         m_friction = 100.f;
         createParticleGrid();
    // }
    // else if (m_particleMode == BOIDS)
    // {
    //     m_flock = true;
    //     m_bounceDamping = false;
    //     m_immortal = true;
//         addParticles(m_numParticles);
    // }
    // else if (m_particleMode == ANIMATIONS)
    // {
    //     m_immortal = false;
    //     m_friction = 0.f;
    //     m_turbulence = 0.f;
    //     m_bounce = false;
    //     m_opacityAge = true;
    //     m_sizeAge = false;
    //     m_flickersAge = false;
    
    //     if (animation == RAIN)
    //     {
    //         m_radius = 0.65;
    //         m_radiusRnd = 10.f;
    
    //         m_velocity = 80.f;
    //         m_velocityRnd = 20.f;
    
    //         m_lifetime = 1.3;
    //         m_lifetimeRnd = 50.f;
    
    //         m_gravity = ofPoint(0, 80);
    //         m_bounce = true;
    //     }
    //     else if (animation == SNOW)
    //     {
    //         m_radius = 2.f;
    //         m_radiusRnd = 30.f;
    
    //         m_velocity = 40.f;
    //         m_velocityRnd = 20.f;
    
    //         m_lifetime = 2.8;
    //         m_lifetimeRnd = 30.f;
    
    //         m_gravity = ofPoint(0, 15);
    //         m_bounce = true;
    //     }
    //     else if (animation == EXPLOSION)
    //     {
    //         m_radius = 7.f;
    //         m_radiusRnd = 80.f;
    
    //         m_velocity = 700.f;
    //         m_velocityRnd = 70.f;
    
    //         m_lifetime = 1.6;
    //         m_lifetimeRnd = 40.f;
    
    //         m_gravity = ofPoint(0, 100);
    //         m_friction = 90.f;
    //         m_turbulence = 15.f;
    //         m_flickersAge = true;
    //         m_sizeAge = true;
    //         m_opacityAge = false;
    //         addParticles(m_numParticles);
    //     }
    // }
}

// void ParticleSystem::update(float dt, vector<irMarker>& markers, Contour& contour, Fluid& fluid)
void ParticleSystem::update(float dt)
{   
    // if is active or we are fading out, update particles
    // if (m_isActive || m_isFadingOut)
    // {
    // // if it is the first frame where m_isActive is true and we are not fading out
    // // start fadeIn and change activeStarted to true so we dont enter anymore
    // if (!activeStarted && !m_isFadingOut)
    // {
    //     m_activeStarted = true;
    //     m_isFadingIn = true;
    //     m_isFadingOut = false;
    //     m_startFadeIn = true;
    //     m_startFadeOut = false;
    //     m_opacity = 0.f;
    //     createParticles();
    // }
    
    // if (m_isFadingIn)
    // {
    //     fadeIn(dt);
    // }
    // else if (m_isFadingOut && !m_isActive)
    // {
    //     fadeOut(dt); // if it is not active and it is fading out, fade out
    // }
    // else
    // {
    //     m_opacity = m_maxOpacity;
    // }
    
    // sort particles so it is more effective to do particle/particle interactions
//    std::sort(m_particles.begin(), m_particles.end(), comparisonFunction);

    // compute radius squareds so we just do it once
    // float interactionRadiusSqrd = m_interactionRadius * m_interactionRadius;
    // float flockingRadiusSqrd = m_flockingRadius * m_flockingRadius;

    // ---------- (1) Delete inactive particles
    // int i = 0;
    // while (i < m_particles.size())
    // {
    //     if (!m_particles[i]->isAlive)
    //     {
    //         delete m_particles.at(i);
    //         m_particles.erase(m_particles.begin() + i);
    //         --numParticles;
    //     }
    //     else
    //     {
    //         ++i;
    //     }
    // }

    // ---------- (2) Calculate specific particle system behavior
    for (int i = 0; i < m_particles.size(); ++i)
    {
        //     if (m_interact) // Interact particles with input
        //     {
        //         if (m_markersInput)
        //         {
        //             irMarker* closestMarker;
        //             if (particleMode == BOIDS) // get closest marker to particle
        //             {
        //                 closestMarker = getClosestMarker(*m_particles[i], markers);
        //             }
        //             else // get closest marker to particle only if particle is inside interaction area
        //             {
        //                 closestMarker = getClosestMarker(*m_particles[i], markers, interactionRadiusSqrd);
        //             }

        //             // Get direction vector to closest marker
        //             // dir = closestMarker->smoothPos - m_particles[i]->pos;
        //             // dir.normalize();

        //             if (closestMarker != NULL)
        //             {
        //                 if (m_flowInteraction)
        //                 {
        //                     float markerDistSqrd = m_particles[i]->pos.squareDistance(closestMarker->smoothPos);
        //                     float pct = 1 - (markerDistSqrd / interactionRadiusSqrd); // stronger on the inside
        //                     m_particles[i]->addForce(closestMarker->velocity*pct*interactionForce);
        //                 }
        //                 else if (m_repulseInteraction)
        //                 {
        //                     m_particles[i]->addRepulsionForce(closestMarker->smoothPos,
        //                         interactionRadiusSqrd, interactionForce);
        //                 }
        //                 else if (m_attractInteraction)
        //                 {
        //                     m_particles[i]->addAttractionForce(closestMarker->smoothPos,
        //                         interactionRadiusSqrd, interactionForce);
        //                 }
        //                 else if (m_seekInteraction)
        //                 {
        //                     m_particles[i]->seek(closestMarker->smoothPos, interactionRadiusSqrd,
        //                         interactionForce*10.f);
        //                 }
        //                 else if (m_gravityInteraction)
        //                 {
        //                     m_particles[i]->addForce(ofPoint(ofRandom(-100.f, 100.f), 500.f) * m_particles[i]->mass);
        //                     m_particles[i]->isTouched = true;
        //                 }
        //                 else if (m_bounceInteraction)
        //                 {
        //                     unsigned int contourIdx = -1;
        //                     ofPoint closestPointInContour = getClosestPointInContour(*m_particles[i],
        //                         contour, true, &contourIdx);
        //                     if (closestPointInContour != ofPoint(-1, -1))
        //                     {
        //                         if (contourIdx != -1)
        //                         {
        //                             m_particles[i]->contourBounce(contour.contours[contourIdx]);
        //                         }
        //                     }
        //                 }
        //             }
        //             else if (m_gravityInteraction && m_particles[i]->isTouched)
        //             {
        //                 m_particles[i]->addForce(ofPoint(0, 500.f) * m_particles[i]->mass);
        //             }
        //         }
        //         if (m_contourInput)
        //         {
        //             unsigned int contourIdx = -1;
        //             ofPoint closestPointInContour;
        //             if (particleMode == BOIDS && m_seekInteraction) // get closest point to particle
        //             {
        //                 closestPointInContour = getClosestPointInContour(*m_particles[i], contour, false, &contourIdx);
        //             }
        //             else // get closest point to particle only if particle is inside contour
        //             {
        //                 closestPointInContour = getClosestPointInContour(*m_particles[i], contour, true, &contourIdx);
        //             }
        
        //             if (m_flowInteraction)
        //             {
        //                 ofPoint frc = contour.getFlowOffset(m_particles[i]->pos);
        //                 m_particles[i]->addForce(frc*interactionForce);
        //             }

        //             if (closestPointInContour != ofPoint(-1, -1))
        //             {
        //                 if (m_repulseInteraction)
        //                 { // it is an attractForce but result is more logical saying repulse
        //                     m_particles[i]->addAttractionForce(closestPointInContour, interactionRadiusSqrd, interactionForce);
        //                 }
        //                 else if (m_attractInteraction)
        //                 {
        //                     m_particles[i]->addRepulsionForce(closestPointInContour, interactionRadiusSqrd, interactionForce);
        //                 }
        //                 else if (m_seekInteraction)
        //                 {
        //                     m_particles[i]->seek(closestPointInContour, interactionRadiusSqrd, interactionForce*10.f);
        //                 }
        //                 else if (m_gravityInteraction)
        //                 {
        //                     m_particles[i]->addForce(ofPoint(ofRandom(-100, 100), 500.f) * m_particles[i]->mass);
        //                     m_particles[i]->isTouched = true;
        //                 }
        //                 else if (m_bounceInteraction)
        //                 {
        //                     if (contourIdx != -1)
        //                     {
        //                         m_particles[i]->contourBounce(contour.contours[contourIdx]);
        //                     }
        //                 }
        //             }
        //             else if (m_gravityInteraction && m_particles[i]->isTouched)
        //             {
        //                 m_particles[i]->addForce(ofPoint(0.f, 500.f) * m_particles[i]->mass);
        //             }
        //         }
        //         if (fluidInteraction)
        //         {
        //             ofPoint frc = fluid.getFluidOffset(m_particles[i]->pos);
        //             m_particles[i]->addForce(frc*interactionForce);
        //         }
        //     }

        //     if (flock) // Flocking behavior
        //     {
        //         m_particles[i]->flockingRadiusSqrd = flockingRadiusSqrd;

        //         m_particles[i]->separationStrength = separationStrength;
        //         m_particles[i]->alignmentStrength = alignmentStrength;
        //         m_particles[i]->attractionStrength = attractionStrength;

        //         m_particles[i]->lowThresh = lowThresh;
        //         m_particles[i]->highThresh = highThresh;
        //         m_particles[i]->maxSpeed = maxSpeed;
        //     }

        //     if (m_returnToOrigin && particleMode == GRID && !m_gravityInteraction)
        //     {
        //         m_particles[i]->m_returnToOrigin(100.f, m_returnToOriginForce);
        //     }

        //     if (particleMode == ANIMATIONS && animation == SNOW)
        //     {
        //         float windX = ofSignedNoise(m_particles[i]->pos.x * 0.03f, m_particles[i]->pos.y * 0.06f, ofGetElapsedTimef() * 0.1f) * 3.f;
        //         ofPoint frc;
        //         frc.x = windX + ofSignedNoise(m_particles[i]->id, m_particles[i]->pos.y * 0.4) * 8.f;
        //         frc.y = ofSignedNoise(m_particles[i]->id, m_particles[i]->pos.x * 0.06f, ofGetElapsedTimef() * 0.2f) * 3.f;
        //         m_particles[i]->addForce(frc*m_particles[i]->mass);
        //     }
        // }

        // if (m_emit) // Create new particles
        // {
        // if (m_markersInput)
        // {
        //     for (unsigned int i = 0; i < markers.size(); ++i)
        //     {
        //         if (!markers[i].hasDisappeared)
        //         {
        //             if (m_emitInMovement)
        //             {
        //                 float n = ofMap(markers[i].velocity.lengthSquared(), 0.5, 300.f, 0.f, bornRate, true);
        //                 addParticles(n, markers[i]);
        //             }
        //             else
        //             {
//                         float range = ofMap(m_bornRate, 0.f, 150.f, 0.f, 20.f);
//                         if (m_bornRate > 0.1f)
//                         {
//                             addParticles(ofRandom(m_bornRate - range, m_bornRate + range), m_markers[i]);
//                         }
        //             }
        //         }
        //     }
        // }
        //     if (m_contourInput)
        //     {
        //         if (m_emitInMovement)
        //         {
        //             for (unsigned int i = 0; i < contour.vMaskContours.size(); ++i)
        //             {
        //                 // Create more particles if bigger area
        //                 float bornNum = bornRate * abs(contour.vMaskContours[i].getArea()) / 1500.f;
        //                 addParticles(bornNum, contour.vMaskContours[i], contour);
        //             }
        //         }
        //         else
        //         {
        //             for (unsigned int i = 0; i < contour.contours.size(); ++i)
        //             {
        //                 float range = ofMap(bornRate, 0.f, 150.f, 0.f, 30.f);
        //                 addParticles(ofRandom(bornRate-range, bornRate + range), contour.contours[i], contour);
        //             }
        //         }
        //     }
        // }

        // Keep adding particles if it is an animation (unless it is an explosion)
        // if (m_particleMode == ANIMATIONS && animation != EXPLOSION)
        // {
//             float range = ofMap(m_bornRate, 0.f, 60.f, 0.f, 15.f);
//             addParticles(ofRandom(m_bornRate - range, m_bornRate + range));
        // }

        // if (m_flock)
        // {
        //     flockParticles();
        // }
        // if (m_repulse)
        // {
        //     repulseParticles();
        // }

        // ---------- (3) Add some general behavior and update the particles
        for (int i = 0; i < m_particles.size(); ++i)
        {
//            m_particles[i]->addForce(m_gravity * m_particles[i]->mass());
//            m_particles[i]->addNoise(m_turbulence);

            // m_particles[i]->opacity = opacity;
            // m_particles[i]->friction = 1.f - friction / 1000.f;
            // m_particles[i]->bounces = bounce;
            // m_particles[i]->m_bounceDamping = m_bounceDamping;
            // m_particles[i]->steers = steer;
            // m_particles[i]->infiniteWalls = infiniteWalls;
            
            // update attributes also from immortal particle systems like GRID and BOIDS
            // if (m_immortal)
            // {
            //     if (m_particleMode != BOIDS)
            //     {
            //         m_particles[i]->radius = radius;
            //     }
            //     m_particles[i]->color = ofColor(red, green, blue);
            //     m_particles[i]->isEmpty = isEmpty;
            //     m_particles[i]->drawLine = drawLine;
            //     m_particles[i]->drawStroke = drawStroke;
            //     m_particles[i]->strokeWidth = strokeWidth;
            // }
            
            // if (m_gravityInteraction)
            // {
            //     m_particles[i]->bounces = true;
            // }

            m_particles[i]->update(dt);
        }
    }
    // else if (activeStarted)
    // {
    //     activeStarted = false;
    //     m_isFadingIn = false;
    //     m_isFadingOut = true;
    //     startFadeIn = false;
    //     startFadeOut = true;
    //     killParticles();
    // }
}

void ParticleSystem::draw()
{

    // if (m_isActive || m_isFadingOut)
    // {

    ofPushStyle();

    //Draw lines between near points
    if (m_drawConnections)
    {
        // sort particles so it is more effective to do particle/particle comparisons
        std::sort(m_particles.begin(), m_particles.end(), comparisonFunction);

        ofPushStyle();
        ofSetColor(ofColor(m_red, m_green, m_blue), m_opacity);
        ofSetLineWidth(m_connectWidth);
        for (int i = 1; i < m_particles.size(); ++i)
        {
            for (int j = i - 1; j >= 0; --j)
            {
                if ((m_particles[i]->position().x - m_particles[j]->position().x) > m_connectDist)
                {
                    break; // to speed the loop
                }
                if (m_particles[i]->position().squareDistance(m_particles[j]->position()) <= m_connectDistSqrd)
                {
                    ofDrawLine(m_particles[i]->position(), m_particles[j]->position());
                }
            }
        }
        ofPopStyle();
    }
    // Draw particles
    for (int i = 0; i < m_particles.size(); ++i)
    {
        m_particles[i]->draw();
    }

    ofPopStyle();
    // }
}

void ParticleSystem::addParticle(ofPoint pos, ofPoint vel, ofColor color, float radius, float lifetime)
{
    int id = m_numParticles;
    float friction = 1.f - m_friction / 1000.f;
    std::shared_ptr<Particle> newParticle = std::make_shared<Particle>(
        id, radius, lifetime, color, pos, vel, m_changeSizeWithAge,
        m_changeOpacityWithAge, m_changeColorWithAge, m_flickerWithAge,
        m_isEmpty, m_drawLine, m_drawContour, m_contourWidth, m_immortal,
        m_limitSpeed, m_friction, m_damping, m_width, m_height
    );
    
    // if (m_particleMode == GRID)
    // {
    //     newParticle->immortal = true;
    // }
    // else if (m_particleMode == BOIDS)
    // {
    //     newParticle->limitSpeed = true;
    //     newParticle->immortal = true;
    // }
    // else if (m_particleMode == ANIMATIONS)
    // {
    //     if (animation == SNOW)
    //     {
    //         newParticle->damping = 0.5f;
    //     }
    //     else
    //     {
    //         newParticle->damping = 0.2;
    //     }
    //     newParticle->bounceTop = false;
    // }

    m_particles.push_back(newParticle);

    m_numParticles++;
//    m_totalParticlesCreated++;
}

void ParticleSystem::addParticles(int numParticles)
{
    m_particles.reserve(numParticles);
    for (int i = 0; i < numParticles; ++i)
    {
        ofPoint pos = ofPoint(ofRandom(m_width), ofRandom(m_height));
        ofPoint vel = randomVector() * randomRange(m_velocityRnd, m_velocity);

        // if (m_particleMode == ANIMATIONS && (animation == RAIN || animation == SNOW))
        // {
        //     pos = ofPoint(ofRandom(width), ofRandom(-5.f * radius, -10.f * radius));
        //     vel.x = 0.f;
        //     vel.y = velocity + randomRange(velocityRnd, velocity); // make particles all be going down when born
        // }
        // else if (m_particleMode == ANIMATIONS && animation == EXPLOSION)
        // {
        //     pos = ofPoint(ofRandom(width), ofRandom(height, height + radius * 15.f));
        //     vel.x = 0.f;
        //     vel.y = -velocity-randomRange(velocityRnd, velocity); // make particles all be going up when born
        // }

        float initialRadius = randomRange(m_radiusRnd, m_radius);
        float lifetime = randomRange(m_lifetimeRnd, m_lifetime);

        addParticle(pos, vel, ofColor(m_red, m_green, m_blue), initialRadius, lifetime);
    }
}

// void ParticleSystem::addParticles(int numParticles, const irMarker& marker)
// {
//     for (int i = 0; i < n; ++i)
//     {
//         ofPoint pos;
//         if (emitAllTimeInside || m_emitInMovement)
//         {
//             pos = marker.smoothPos + randomVector()*ofRandom(0, emitterSize);
//         }
//         else if (emitAllTimeContour)
//         {
//             pos = marker.smoothPos + randomVector()*emitterSize;
//         }
//         ofPoint randomVel = randomVector() * randomRange(velocityRnd, velocity);
//         ofPoint motionVel = marker.velocity * (velocity / 5.f + randomRange(velocityRnd, velocity / 5.f)); // /5.f to reduce effect
//         ofPoint vel = randomVel*(velocityRnd / 100.f) + motionVel * (velocityMotion / 100.f);

//         float initialRadius = randomRange(radiusRnd, radius);
//         float lifetime = randomRange(lifetimeRnd, m_lifetime);

//         addParticle(pos, vel, ofColor(red, green, blue), initialRadius, lifetime);
//     }
// }

// void ParticleSystem::addParticles(int n, const ofPolyline& contour, Contour& flow)
// {
//     for (int i = 0; i < n; ++i)
//     {
//         ofPoint pos, randomVel, motionVel, vel;

//         // Create random particles inside contour polyline
//         if (emitAllTimeInside || m_emitInMovement)
//         {
//             ofRectangle box = contour.getBoundingBox(); // so it is easier that the particles are born inside contour
//             ofPoint center = box.getCenter();
//             pos.x = center.x + (ofRandom(1.f) - 0.5f) * box.getWidth();
//             pos.y = center.y + (ofRandom(1.f) - 0.5f) * box.getHeight();

//             while(!contour.inside(pos))
//             {
//                 pos.x = center.x + (ofRandom(1.f) - 0.5f) * box.getWidth();
//                 pos.y = center.y + (ofRandom(1.f) - 0.5f) * box.getHeight();
//             }

//             // set velocity to random vector direction with 'velocity' as magnitude
//             randomVel = randomVector() * (velocity + randomRange(velocityRnd, velocity));
//         }

//         // Create particles only on the contour polyline
//         else if (emitAllTimeContour)
//         {
//             float indexInterpolated = ofRandom(0, contour.size());
//             pos = contour.getPointAtIndexInterpolated(indexInterpolated);

//             // Use normal vector in surface as vel. direction so particle moves out of the contour
//             randomVel = -contour.getNormalAtIndexInterpolated(indexInterpolated) * 
//                 (velocity + randomRange(velocityRnd, velocity));
//         }

//         if (true)
//         { // get velocity vector in particle pos
//             motionVel = flow.getFlowOffset(pos) * (velocity*5.f+randomRange(velocityRnd, velocity*5.f));
//         }
//         else if (useContourVel)
//         { // slower and poorer result
//             motionVel = flow.getVelocityInPoint(pos) * (velocity*5.f+randomRange(velocityRnd, velocity*5.f));
//         }
//         vel = randomVel*(velocityRnd / 100.f) + motionVel*(velocityMotion / 100.f);
//         pos += randomVector()*emitterSize; // randomize position within a range of emitter size

//         float initialRadius = radius + randomRange(radiusRnd, radius);
//         float lifetime = m_lifetime + randomRange(lifetimeRnd, m_lifetime);

//         addParticle(pos, vel, ofColor(red, green, blue), initialRadius, lifetime);
//     }
// }

 void ParticleSystem::createParticleGrid()
 {
    const float numParticlesX_float = static_cast<float>(m_width) / m_gridRes;
    const float numParticlesY_float = static_cast<float>(m_height) / m_gridRes;

    const int numParticlesX = static_cast<int>(numParticlesX_float);
    const int numParticlesY = static_cast<int>(numParticlesY_float);

    // Compute offsets so the particles are centered on the screen
    const float offsetX = (numParticlesX_float - static_cast<float>(numParticlesX)) / 2.f;
    const float offsetY = (numParticlesY_float - static_cast<float>(numParticlesY)) / 2.f;

    const size_t numParticles = numParticlesX * numParticlesY;
    m_particles.reserve(numParticles);

    // Very important that the outer loop is the X axis and not Y
    // so particles are already sorted by X - we use this fact
    // throughout the code to make comparisons between particles
    // more efficent.
    for (int x = 0; x < numParticlesX; x++)
    {
        for (int y = 0; y < numParticlesY; y++)
        {
            int xi = (x + 0.5f + offsetX) * m_gridRes;
            int yi = (y + 0.5f + offsetY) * m_gridRes;
//            float initialRadius = m_gridRes / 2.f;
//            float initialRadius = ofRandom(1.f, 6.f);
//            float initialRadius = cos(yi * 0.1f) + sin(xi * 0.1f) + 2.f;
//            float initialRadius = (sin(yi * xi) + 1.f) * 2.f;
//            float xyOffset = sin( cos( sin( yi * 0.3183f ) + cos( xi * 0.3183f ) ) ) + 1.f;
//            float initialRadius = xyOffset * xyOffset * 1.8f;
            ofPoint vel = ofPoint(0.f, 0.f);
            addParticle(ofPoint(xi, yi), vel, ofColor(m_red, m_green, m_blue), m_radius, m_lifetime);
        }
    }
 }

// void ParticleSystem::removeParticles(int numParticles)
// {
//     numParticles = MIN(particles.size(), numParticles);
//     for (int i = 0; i < numParticles; ++i)
//     {
//         m_particles[i]->immortal = false;
//     }
// }

// void ParticleSystem::killParticles()
// {
//     for (int i = 0; i < particles.size(); ++i)
//     {
//         m_particles[i]->immortal = false;
//     }
// }

// void ParticleSystem::resetTouchedParticles()
// {
//     for (int i = 0; i < particles.size(); ++i)
//     {
//         m_particles[i]->isTouched = false;
//     }
// }

// void ParticleSystem::resetParticles()
// {
//     // Kill all the remaining particles before creating new ones
//     if (!particles.empty())
//     {
//         for (int i = 0; i < particles.size(); ++i)
//         {
//             m_particles[i]->isAlive = false;
//         }
//     }

// setup(m_particleMode, width, height); // resets the settings to default

//    if (m_particleMode == GRID)
//    {
//        createParticleGrid(width, height);
//    }
//    else if (m_particleMode == RANDOM || m_particleMode == BOIDS || m_particleMode == ANIMATIONS)
//    {
//        addParticles(m_numParticles);
//    }
// }

// void ParticleSystem::setAnimation(Animation animation)
// {
//     m_animation = animation;
// }

// void ParticleSystem::repulseParticles()
// {
//     float repulseDistSqrd = repulseDist*repulseDist;
//     for (int i = 1; i < particles.size(); ++i)
//     {
//         for (int j = i-1; j >= 0; j--)
//         {
//             if (fabs(m_particles[i]->pos.x - particles[j]->pos.x) > repulseDist) break; // to speed the loop
//             m_particles[i]->addRepulsionForce(*particles[j], repulseDistSqrd, 8.f);
// //            m_particles[i]->addRepulsionForce( *particles[j], 30.f);
//         }
//     }
// }

// void ParticleSystem::flockParticles()
// {
//     for (int i = 0; i < particles.size(); ++i)
//     {
//         for (int j = i-1; j >= 0; j--)
//         {
//             if (fabs(m_particles[i]->pos.x - particles[j]->pos.x) > flockingRadius) break;
//             m_particles[i]->addFlockingForces(*particles[j]);
//         }
//     }
// }

ofPoint ParticleSystem::randomVector()
{
    float angle = ofRandom((float)M_PI * 2.f);
    return ofPoint(cos(angle), sin(angle));
}

float ParticleSystem::randomRange(float percentage, float value)
{
    return ofRandom(value - value * (percentage / 100.f), value + value * (percentage / 100.f));
}

// irMarker* ParticleSystem::getClosestMarker(const Particle &particle, vector<irMarker> &markers,
//     float interactionRadiusSqrd)
// {
//     irMarker* closestMarker = NULL;
//     float minDistSqrd = interactionRadiusSqrd;

//     // Get closest marker to particle
//     for (int markerIndex = 0; markerIndex < markers.size(); markerIndex++)
//     {
//         if (!markers[markerIndex].hasDisappeared)
//         {
//             float markerDistSqrd = particle.pos.squareDistance(markers[markerIndex].smoothPos);
//             if (markerDistSqrd < minDistSqrd)
//             {
//                 minDistSqrd = markerDistSqrd;
//                 closestMarker = &markers[markerIndex];
//             }
//         }
//     }
//     return closestMarker;
// }

// // Closest marker without distance limit
// irMarker* ParticleSystem::getClosestMarker(const Particle &particle, vector<irMarker> &markers)
// {
//     irMarker* closestMarker = NULL;
//     float minDistSqrd = FLT_MAX;

//     // Get closest marker to particle
//     for (int markerIndex = 0; markerIndex < markers.size(); markerIndex++)
//     {
//         if (!markers[markerIndex].hasDisappeared)
//         {
//             float markerDistSqrd = particle.pos.squareDistance(markers[markerIndex].smoothPos);
//             if (markerDistSqrd < minDistSqrd)
//             {
//                 minDistSqrd = markerDistSqrd;
//                 closestMarker = &markers[markerIndex];
//             }
//         }
//     }
//     return closestMarker;
// }

// ofPoint ParticleSystem::getClosestPointInContour(const Particle& particle, const Contour& contour,
//     bool onlyInside, unsigned int* contourIdx)
// {
//     ofPoint closestPoint(-1, -1);
//     float minDistSqrd = FLT_MAX;

//     // Get closest point to particle from the different contours
//     for (unsigned int i = 0; i < contour.contours.size(); ++i)
//     {
//         if (!onlyInside || contour.contours[i].inside(particle.pos))
//         {
//             ofPoint candidatePoint = contour.contours[i].getClosestPoint(particle.pos);
//             float pointDistSqrd = particle.pos.squareDistance(candidatePoint);
//             if (pointDistSqrd < minDistSqrd)
//             {
//                 minDistSqrd = pointDistSqrd;
//                 closestPoint = candidatePoint;
//                 if (contourIdx != NULL)
//                 {
//                     *contourIdx = i; // save contour index
//                 }
//             }
//         }
//     }
//     return closestPoint;
// }

// void ParticleSystem::fadeIn(float dt)
// {
//     if (m_startFadeIn)
//     {
//         m_startFadeIn = false;
//         elapsedFadeTime = 0.f;
//         opacity = 0.f;
//     }
//     else
//     {
//         opacity = ofMap(elapsedFadeTime, 0.f, fadeTime, 0.f, maxOpacity, true);
//         elapsedFadeTime += dt;
//         if (elapsedFadeTime > fadeTime)
//         {
//             m_isFadingIn = false;
//             opacity = maxOpacity;
//         }
//     }
// }

// void ParticleSystem::fadeOut(float dt)
// {
//     if (startFadeOut)
//     {
//         startFadeOut = false;
//         elapsedFadeTime = 0.f;
//         opacity = maxOpacity;
//     }
//     else
//     {
//         opacity = ofMap(elapsedFadeTime, 0.f, fadeTime, maxOpacity, 0.f, true);
//         elapsedFadeTime += dt;
//         if (elapsedFadeTime > fadeTime)
//         {
//             m_isFadingOut = false;
//             opacity = 0.f;
//         }
//     }
// }
