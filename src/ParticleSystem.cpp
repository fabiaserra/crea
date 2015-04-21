#include "ParticleSystem.h"

bool comparisonFunction(Particle * a, Particle * b) {
    return a->pos.x < b->pos.x;
}

ParticleSystem::ParticleSystem(){
    isActive            = false;        // Particle system is active?
    activeStarted       = false;        // Active has started?
    
    // Fading in/out
    doFading            = false;        // Do opacity fading?
    isFadingIn          = false;        // Opacity fading in?
    isFadingOut         = false;        // Opacity fading out?
    startFadeIn         = false;        // Fade in has started?
    startFadeOut        = false;        // Fade out has started?
    elapsedFadeTime     = 0.0;          // Elapsed time of fade
    fadeTime            = 2.0;          // Transition time of fade
    
    // Opacity
    opacity             = 0.0;          // Actual opacity of the particles
    maxOpacity          = 255.0;        // Maximum opacity of particles

    // General properties
    immortal            = false;        // Can particles die?
    velocity            = 0.0;          // Initial velocity magnitude of newborn particles
    radius              = 3.0;          // Radius of the particles
    lifetime            = 5.0;          // Lifetime of particles
    red                 = 255.0;
    green               = 255.0;
    blue                = 255.0;
    color               = ofColor(red, green, blue); // Color of the particles
    
    // Specific properties
    nParticles          = 600;          // Number of particles born in the beginning
    bornRate            = 5.0;          // Number of particles born per frame

    // Emitter
    emitterSize         = 3.0;          // Size of the emitter area
    velocityRnd         = 20.0;         // Magnitude randomness % of the initial velocity
    velocityMotion      = 50.0;         // Marker motion contribution to the initial velocity
    lifetimeRnd         = 20.0;         // Randomness of lifetime
    radiusRnd           = 50.0;         // Randomness of radius

    // Grid
    gridRes             = 10;           // Resolution of the grid

    // Flocking
    lowThresh           = 0.1333;       // If dist. ratio lower than lowThresh separate
    highThresh          = 0.6867;       // Else if dist. ratio higher than highThresh attract. Else just align
    separationStrength  = 0.01f;        // Separation force
    attractionStrength  = 0.004f;       // Attraction force
    alignmentStrength   = 0.01f;        // Alignment force
    maxSpeed            = 50.0;         // Max speed particles
    flockingRadius      = 60.0;         // Radius of flocking

    // Graphic output
    sizeAge             = false;        // Decrease size when particles get older?
    opacityAge          = false;        // Decrease opacity when particles get older?
    colorAge            = false;        // Change color when particles get older?
    flickersAge         = false;        // Particle flickers opacity when about to die?
    isEmpty             = false;        // Draw only contours of the particles?
    drawLine            = false;        // Draw a line instead of a circle for the particle?
    drawConnections     = false;        // Draw a connecting line between close particles?
    connectDist         = 15.0;         // Maximum distance to connect particles with line

    // Physics
    friction            = 5.0;          // Friction to velocity 0~100
    gravity             = ofPoint(0, 0);// Makes particles react to gravity
    turbulence          = 0.0f;         // Turbulence perlin noise
    repulse             = false;        // Repulse particles between each other?
    bounce              = false;        // Bounce particles with the walls of the window?
    steer               = false;        // Steers direction before touching the walls of the window?
    infiniteWalls       = false;        // Infinite walls?
    bounceDamping       = true;         // Decrease velocity when particle bounces walls?
    repulseDist         = 5.0*radius;   // Repulse particle-particle distance

    // Behavior
    emit                = false;        // Born new particles in each frame?
    interact            = false;        // Can we interact with the particles?
    flock               = false;        // Particles have flocking behavior?
    repulseInteraction  = false;        // Repulse particles from input?
    attractInteraction  = false;        // Attract particles to input?
    gravityInteraction  = false;        // Apply gravity force to particles touched with input?
    returnToOrigin      = false;        // Make particles return to their born position?

    // Input
    markersInput        = false;        // Input are the IR markers?
    contourInput        = false;        // Input is the depth contour?
    markerRadius        = 50.0;         // Radius of interaction of the markers

    emitAllTimeInside   = true;         // Emit particles every frame inside all the defined area?
    emitAllTimeContour  = false;        // Emit particles every frame only on the contour of the defined area?
    emitInMovement      = false;        // Emit particles only in regions that there has been some movement?

    useFlow             = true;         // Use optical flow to get the motion velocity?
    useFlowRegion       = false;        // Use optical flow region to get the motion velocity?
    useContourArea      = false;        // Use contour area to interact with particles?
    useContourVel       = false;        // Use contour velocities to interact with particles?
}

ParticleSystem::~ParticleSystem(){
    // Delete all the particles
    for (int i = 0; i < particles.size(); i++) {
        delete particles.at(i);
        particles.at(i) = NULL;
    }
    particles.clear();
}


void ParticleSystem::setup(ParticleMode particleMode, int width , int height){

    this->particleMode = particleMode;

    this->width = width;
    this->height = height;

    if(particleMode == EMITTER){
        emit = true;
        sizeAge             = true;        // Decrease size when particles get older?
        opacityAge          = true;        // Decrease opacity when particles get older?
        colorAge            = true;        // Change color when particles get older?
        velocity            = 20;
    }

    else if(particleMode == GRID){
        interact            = true;
        returnToOrigin      = true;
        repulseInteraction  = true;
        immortal            = true;
        velocity            = 0.0;
        velocityRnd         = 0.0;
        gravity             = ofPoint(0, 0);
        friction            = 40;
        createParticleGrid(width, height);
    }

    else if(particleMode == RANDOM){
        interact            = true;
        immortal            = true;
        addParticles(nParticles);
    }

    else if(particleMode == BOIDS){ // TODO: BOIDS == RANDOM?
        flock               = true;
        interact            = true;
        markerRadius        = 600;
        radiusRnd           = 0;
        attractInteraction  = true;
//        repulseInteraction  = true;
        bounceDamping       = false;
        bounce              = true;
        immortal            = true;
        addParticles(nParticles);
    }

    else if(particleMode == ANIMATIONS){
        immortal            = false;
        radius              = 2.0;
        radiusRnd           = 50.0;

        velocity            = 10.0;
        velocityRnd         = 30.0;

        turbulence          = 0.0;
        gravity             = ofPoint(0, 1.5);
        friction            = 6.0;

        opacityAge          = true;
        flickersAge         = false;

        if(animation == SNOW){
            lifetime        = 6.0;
        }
        else if(animation == RAIN){
            lifetime        = 2.5;
            radius          = 0.8;
        }
        else if(animation == WIND){
            immortal        = true;
            velocity        = 50.0;
            turbulence      = 8.0;
            gravity         = ofVec2f(0, 0.0);
            addParticles(1500);
        }
        else if(animation == EXPLOSION){
            radius          = 6.0;
            radiusRnd       = 30.0;

            velocityRnd     = 0.0;

            friction        = 70.0;

            lifetime        = 3.0;
            lifetimeRnd     = 20.0;

            flickersAge     = true;
            opacityAge      = false;
            addParticles(1200);
        }
    }
}

void ParticleSystem::update(float dt, vector<irMarker> &markers, Contour& contour){
    // if is active or we are fading out
    if(isActive || (doFading && isFadingOut)){
        // if it is the first frame where isActive is true and we are not fading out (trick to fix a bug)
        // start fadeIn and change activeStarted to true so we dont enter anymore
        if(!activeStarted && !isFadingOut){
            activeStarted = true;
            isFadingIn = true;
            isFadingOut = false;
            startFadeIn = true;
            startFadeOut = false;
            bornParticles();
        }
        if(doFading){
            if(isFadingIn) fadeIn(dt);
            else if(isFadingOut) fadeOut(dt);
            else opacity = maxOpacity;
        }
        else opacity = maxOpacity;
        
        // sort particles so it is more effective to do particle/particle interactions
        sort(particles.begin(), particles.end(), comparisonFunction);

        // ---------- (1) Delete inactive particles
        int i = 0;
        while (i < particles.size()){
            if (!particles[i]->isAlive){
                delete particles.at(i);
                particles.erase(particles.begin() + i);
                numParticles--;
            }
            else{
                i++;
            }
        }

        // ---------- (2) Calculate specific particle system behavior
        for(int i = 0; i < particles.size(); i++){
            if(interact){ // Interact particles with input
                if(markersInput){
                    // Get closest marker to particle
                    ofPoint closestMarker = getClosestMarker(*particles[i], markers, markerRadius);

                    // Get direction vector to closest marker
                    // dir = closestMarker.smoothPos - particles[i]->pos;
                    // dir.normalize();

                    if(closestMarker != ofPoint(-1, -1)){
                        if(repulseInteraction) particles[i]->addRepulsionForce(closestMarker.x, closestMarker.y, markerRadius*markerRadius, 10.0);
                        if(attractInteraction) particles[i]->addAttractionForce(closestMarker.x, closestMarker.y, markerRadius*markerRadius, 8.0);
                        if(gravityInteraction){
                            particles[i]->addForce(ofPoint(0, 9.8)*particles[i]->mass);
                            particles[i]->isTouched = true;
                        }
                    }
                }
                if(contourInput){
                    ofPoint closestPointInContour = getClosestPointInContour(*particles[i], contour);
                    ofPoint frc = contour.getFlowOffset(particles[i]->pos);
                    particles[i]->addForce(frc);

                    if(closestPointInContour != ofPoint(-1, -1) || (gravityInteraction && particles[i]->isTouched)){
//                        ofPoint frc = contour.getFlowOffset(particles[i]->pos);
//                        particles[i]->addForce(frc);

                        if(attractInteraction){
                            particles[i]->addAttractionForce(closestPointInContour.x, closestPointInContour.y, 8000, 15.0);
                        }
                        else if(gravityInteraction){
                            particles[i]->addForce(ofPoint(0, 9.8)*particles[i]->mass);
                            particles[i]->isTouched = true;
                        }
                    }
//                    else if(useContourVel){
//                        ofPoint frc = -contour.getVelocityInPoint(particles[i]->pos) * 0.1;
//                        particles[i]->addForce(frc);
//                    }
                }
            }

            if(flock){ // Flocking behavior
                particles[i]->flockingRadiusSqrd    =   flockingRadius * flockingRadius;

                particles[i]->separationStrength    =   separationStrength;
                particles[i]->alignmentStrength     =   alignmentStrength;
                particles[i]->attractionStrength    =   attractionStrength;

                particles[i]->lowThresh             =   lowThresh;
                particles[i]->highThresh            =   highThresh;
                particles[i]->maxSpeed              =   maxSpeed;
            }

            if(returnToOrigin && particleMode == GRID && !gravityInteraction) particles[i]->xenoToOrigin(0.06);

            if(particleMode == ANIMATIONS && (animation == SNOW || animation == WIND)){
                ofPoint windForce(ofRandom(-0.1, 0.1), ofRandom(-0.08, 0.06));
                particles[i]->addForce(windForce*particles[i]->mass);

                float windX = ofSignedNoise(particles[i]->pos.x * 0.003, particles[i]->pos.y * 0.006, ofGetElapsedTimef() * 0.6) * 0.6;
                ofPoint frc;
                frc.x = windX * 0.25 + ofSignedNoise(particles[i]->id, particles[i]->pos.y * 0.04) * 0.6;
                if(animation == SNOW) frc.y = ofSignedNoise(particles[i]->id, particles[i]->pos.x * 0.006, ofGetElapsedTimef() * 0.2) * 0.1 + 0.25;
                particles[i]->addForce(frc*particles[i]->mass/2.0);
            }
            else if(animation == RAIN){
                ofPoint vel(0, ofSignedNoise(particles[i]->id, ofGetElapsedTimef() * 0.2)*0.3 + 1.2);
                particles[i]->vel += vel;
            }
        }

        if(emit){ // Born new particles
            if(markersInput){
                for(unsigned int i = 0; i < markers.size(); i++){
                    if (!markers[i].hasDisappeared){
                        if(emitInMovement){
                            float n = ofMap(markers[i].velocity.lengthSquared(), 0.5, 400.0, 0.0, bornRate);
                            addParticles(n, markers[i]);
                        }
                        else{
                            addParticles(ofRandom(bornRate-3.0, bornRate+3.0), markers[i]);
                        }
                    }
                }
            }
            if(contourInput){
                if(emitInMovement){
                    for(unsigned int i = 0; i < contour.diffContours.size(); i++){
//                        addParticles(bornRate, contour.diffContours[i], contour);

                        // born more particles if bigger area
                        float bornNum = bornRate * abs(contour.diffContours[i].getArea())/1500.0;
                        addParticles(bornNum, contour.diffContours[i], contour);
                    }
                }
                else{
                    for(unsigned int i = 0; i < contour.contours.size(); i++){
                        addParticles(bornRate, contour.contours[i], contour);
                    }
                }
            }
        }

        // Keep adding particles if it is an animation
        if(particleMode == ANIMATIONS && animation != EXPLOSION && animation != WIND){
            addParticles(ofRandom(15,25));
        }

        if(flock) flockParticles();
        if(repulse) repulseParticles();

        // ---------- (3) Add some general behavior and update the particles
        for(int i = 0; i < particles.size(); i++){
            particles[i]->addForce(gravity*particles[i]->mass);
            particles[i]->addNoise(15.0, turbulence, dt);

            particles[i]->immortal = immortal;
            particles[i]->opacity = opacity;
            particles[i]->friction = 1-friction/1000;
            particles[i]->bounces = bounce;
            particles[i]->bounceDamping = bounceDamping;
            particles[i]->steers = steer;
            particles[i]->infiniteWalls = infiniteWalls;

            particles[i]->update(dt);
        }
    }
    
    else if(activeStarted){
        activeStarted = false;
        isFadingIn = false;
        isFadingOut = true;
        startFadeIn = false;
        startFadeOut = true;
        killParticles();
    }
}

void ParticleSystem::draw(){
    if(isActive || (doFading && isFadingOut)){
        ofPushStyle();
        for(int i = 0; i < particles.size(); i++){
            particles[i]->draw();
        }

        //Draw lines between near points
        if(drawConnections){
            float connectDistSqrd = connectDist*connectDist;
            ofSetColor(ofColor(red, green, blue), opacity);
            for(int i = 0; i < particles.size(); i++){
                for(int j = i-1; j >= 0; j--){
                    if (fabs(particles[j]->pos.x - particles[i]->pos.x) > connectDist*2) break; // to speed the loop
                    if(particles[i]->pos.squareDistance(particles[j]->pos) < connectDistSqrd){
                        ofLine(particles[i]->pos, particles[j]->pos);
                    }
                }
            }
        }
        ofPopStyle();
    }
}

void ParticleSystem::addParticle(ofPoint pos, ofPoint vel, ofColor color, float radius, float lifetime){
    Particle * newParticle = new Particle();
    float id = totalParticlesCreated;
    
//    float initialRadius = radius + randomRange(radiusRnd, radius);
//    float lifetime = this->lifetime + randomRange(lifetimeRnd, this->lifetime);
//    ofColor color(red, green, blue);

    newParticle->sizeAge        = sizeAge;
    newParticle->opacityAge     = opacityAge;
    newParticle->flickersAge    = flickersAge;
    newParticle->colorAge       = colorAge;
    newParticle->isEmpty        = isEmpty;
    newParticle->drawLine       = drawLine;

    newParticle->width = width;
    newParticle->height = height;

    if(particleMode == BOIDS) newParticle->limitSpeed = true;

    newParticle->setup(id, pos, vel, color, radius, lifetime);
    particles.push_back(newParticle);

    numParticles++;
    totalParticlesCreated++;
}

void ParticleSystem::addParticles(int n){
    for(int i = 0; i < n; i++){
        ofPoint pos = ofPoint(ofRandom(width), ofRandom(height));
        ofPoint vel = randomVector()*(velocity+randomRange(velocityRnd, velocity));

        if(particleMode == ANIMATIONS && animation == RAIN){
            pos = ofPoint(ofRandom(width), ofRandom(-2*radius, -20*radius));
            vel.x = 0;
            vel.y = fabs(vel.y) * 20.0; // make particles all be going down when born
        }
        else if(particleMode == ANIMATIONS && animation == SNOW){
            pos = ofPoint(ofRandom(width), ofRandom(-2*radius, -20*radius));
            vel.y = fabs(vel.y) * 6.0; // make particles all be going down when born
        }
        else if(particleMode == ANIMATIONS && animation == EXPLOSION){
            pos = ofPoint(ofRandom(width), ofRandom(height+radius, height+radius*50));
            vel = ofPoint(0, -velocity*ofRandom(80, 180)); // make particles all be going up when born
        }

        float initialRadius = radius + randomRange(radiusRnd, radius);
        float lifetime = this->lifetime + randomRange(lifetimeRnd, this->lifetime);

        addParticle(pos, vel, ofColor(red, green, blue), initialRadius, lifetime);
    }
}

void ParticleSystem::addParticles(int n, const irMarker &marker){
    for(int i = 0; i < n; i++){
        ofPoint pos;
        if(emitAllTimeInside || emitInMovement){
            pos = marker.smoothPos + randomVector()*ofRandom(0, emitterSize);
        }
        else if(emitAllTimeContour){
            pos = marker.smoothPos + randomVector()*emitterSize;
        }
        ofPoint vel = randomVector()*(velocity+5.0*randomRange(velocityRnd, velocity));
        vel += marker.velocity*(velocityMotion/100)*5.0;

//        if(emitInMovement && marker.velocity.lengthSquared() < 25.0) break;

        float initialRadius = radius + randomRange(radiusRnd, radius);
        float lifetime = this->lifetime + randomRange(lifetimeRnd, this->lifetime);

//        addParticle(pos, vel, marker.color, initialRadius, lifetime);
        addParticle(pos, vel, ofColor(red, green, blue), initialRadius, lifetime);
    }
}

void ParticleSystem::addParticles(int n, const ofPolyline &contour, Contour &flow){
    for(int i = 0; i < n; i++){

        ofPoint pos, vel;

        // Create random particles inside contour polyline
        if(emitAllTimeInside || emitInMovement){
            ofRectangle box = contour.getBoundingBox();
            ofPoint center = box.getCenter();
            pos.x = center.x + (ofRandom(1.0f) - 0.5f) * box.getWidth();
            pos.y = center.y + (ofRandom(1.0f) - 0.5f) * box.getHeight();

            while(!contour.inside(pos)){
                pos.x = center.x + (ofRandom(1.0f) - 0.5f) * box.getWidth();
                pos.y = center.y + (ofRandom(1.0f) - 0.5f) * box.getHeight();
            }

            // random vector
            vel = randomVector()*(velocity+2*randomRange(velocityRnd, velocity));
        }

        // Create particles only on the contour polyline
        else if(emitAllTimeContour){
            float indexInterpolated = ofRandom(0, contour.size());
            pos = contour.getPointAtIndexInterpolated(indexInterpolated);

            // use point normal vector as velocity
            vel = contour.getNormalAtIndexInterpolated(indexInterpolated)*(velocity+2*randomRange(velocityRnd, velocity))*-1;
        }

//        if(useFlow){
        if(true){
            ofPoint motionVel = flow.getFlowOffset(pos);
            vel += motionVel*(velocityMotion/100);
        }
        else if(useContourVel){
            ofPoint motionVel = flow.getVelocityInPoint(pos);
            vel += motionVel*(velocityMotion/100)*6;
        }

        pos.x += ofRandom(-emitterSize, emitterSize);
        pos.y += ofRandom(-emitterSize, emitterSize);

        float initialRadius = radius + randomRange(radiusRnd, radius);
        float lifetime = this->lifetime + randomRange(lifetimeRnd, this->lifetime);

        addParticle(pos, vel, ofColor(red, green, blue), initialRadius, lifetime);
    }
}

void ParticleSystem::createParticleGrid(int width, int height){
    for(int y = 0; y < height/gridRes; y++){
        for(int x = 0; x < width/gridRes; x++){
            int xi = (x + 0.5f) * gridRes;
            int yi = (y + 0.5f) * gridRes;
//            float initialRadius = (float)gridRes / 2.0f;
//            float initialRadius = ofRandom(1.0f, 6.0f);
//            float initialRadius = cos(yi * 0.1f) + sin(xi * 0.1f) + 2.0f;
//            float initialRadius = (sin(yi * xi) + 1.0f) * 2.0f;
//            float xyOffset = sin( cos( sin( yi * 0.3183f ) + cos( xi * 0.3183f ) ) ) + 1.0f;
//            float initialRadius = xyOffset * xyOffset * 1.8f;
            ofPoint vel = ofPoint(0, 0);
            addParticle(ofPoint(xi, yi), vel, ofColor(red, green, blue), radius, lifetime);
        }
    }
}

void ParticleSystem::removeParticles(int n){
    n = MIN(particles.size(), n);
    for(int i = 0; i < n; i++){
        particles[i]->immortal = false;
    }
}

void ParticleSystem::killParticles(){
    for(int i = 0; i < particles.size(); i++){
        particles[i]->immortal = false;
    }
}

void ParticleSystem::bornParticles(){
    // Kill all the remaining particles before creating new ones
    if(!particles.empty()){
        for(int i = 0; i < particles.size(); i++){
            particles[i]->isAlive = false;
        }
    }
    
    setup(particleMode, width, height); // resets the settings to default

//    if(particleMode == GRID){
//        createParticleGrid(width, height);
//    }
//
//    else if(particleMode == RANDOM || particleMode == BOIDS || particleMode == ANIMATIONS){
//        addParticles(nParticles);
//    }
}

void ParticleSystem::setAnimation(Animation animation){
    this->animation = animation;
}

void ParticleSystem::repulseParticles(){
    float repulseDistSqrd = repulseDist*repulseDist;
    for(int i = 0; i < particles.size(); i++){
        for(int j = i-1; j >= 0; j--){
            if (fabs(particles[j]->pos.x - particles[i]->pos.x) > repulseDist) break; // to speed the loop
            particles[i]->addRepulsionForce( *particles[j], repulseDistSqrd, 15.0);
//            particles[i]->addRepulsionForce( *particles[j], 5.0);
        }
    }
}

void ParticleSystem::flockParticles(){
    for(int i = 0; i < particles.size(); i++){
        for(int j = i-1; j >= 0; j--){
//            if (fabs(particles[j]->pos.x - particles[i]->pos.x) > flockingRadius) break;
            particles[i]->addFlockingForces(*particles[j]);
        }
    }
}

ofPoint ParticleSystem::randomVector(){
    float angle = ofRandom((float)M_PI * 2.0f);
    return ofPoint(cos(angle), sin(angle));
}

float ParticleSystem::randomRange(float percentage, float value){
    return ofRandom(-(percentage/100)*value, (percentage/100)*value);
}

ofPoint ParticleSystem::getClosestMarker(const Particle &particle, const vector<irMarker> &markers, float markerRadius){
    ofPoint closestMarker(-1, -1);
    float minDistSqrd = markerRadius*markerRadius;

    // Get closest marker to particle
    for(int markerIndex = 0; markerIndex < markers.size(); markerIndex++){
        if (!markers[markerIndex].hasDisappeared){
            float markerDistSqrd = particle.pos.squareDistance(markers[markerIndex].smoothPos);
            if(markerDistSqrd < minDistSqrd){
                minDistSqrd = markerDistSqrd;
                closestMarker = markers[markerIndex].smoothPos;
            }
        }
    }
    return closestMarker;
}

//irMarker ParticleSystem::getClosestMarker(const Particle &particle, const vector<irMarker> &markers, float markerRadius){
//    irMarker closestMarker;
//    float minDistSqrd = markerRadius*markerRadius;
//
//    // Get closest marker to particle
//    for(int markerIndex = 0; markerIndex < markers.size(); markerIndex++){
//        if (!markers[markerIndex].hasDisappeared){
//            float markerDistSqrd = particle.pos.squareDistance(markers[markerIndex].smoothPos);
//            if(markerDistSqrd < minDistSqrd){
//                minDistSqrd = markerDistSqrd;
//                closestMarker = markers[markerIndex];
//            }
//        }
//    }
//    return closestMarker;
//}

ofPoint ParticleSystem::getClosestPointInContour(const Particle &particle, const Contour &contour){
    ofPoint closestPoint(-1, -1);

    // Get closest point in the contour to particle
    for(unsigned int i = 0; i < contour.contours.size(); i++){
        if(contour.contours[i].inside(particle.pos)){
            closestPoint = contour.contours[i].getClosestPoint(particle.pos);
        }
    }

    return closestPoint;
}

void ParticleSystem::fadeIn(float dt){
    if(startFadeIn){
        startFadeIn = false;
        elapsedFadeTime = 0.0;
        opacity = 0.0;
    }
    else{
        opacity = ofMap(elapsedFadeTime, 0.0, fadeTime, 0.0, maxOpacity, true);
        elapsedFadeTime += dt;
        if(elapsedFadeTime > fadeTime){
            isFadingIn = false;
            opacity = maxOpacity;
        }
    }
}

void ParticleSystem::fadeOut(float dt){
    if(startFadeOut){
        startFadeOut = false;
        elapsedFadeTime = 0.0;
        opacity = maxOpacity;
    }
    else{
        opacity = ofMap(elapsedFadeTime, 0.0, fadeTime, maxOpacity, 0.0, true);
        elapsedFadeTime += dt;
        if(elapsedFadeTime > fadeTime){
            isFadingOut = false;
            opacity = 0.0;
        }
    }
}
