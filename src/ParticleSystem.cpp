#include "ParticleSystem.h"

bool comparisonFunction(Particle * a, Particle * b) {
    return a->pos.x < b->pos.x;
}

ParticleSystem::ParticleSystem(){
    isActive            = false;        // Particle system is active?

    // General properties
    immortal            = false;        // Can particles die?
    velocity            = 0.0;          // Initial velocity magnitude of newborn particles
    radius              = 3.0;          // Radius of the particles
    color               = ofColor(255); // Color of the particles
    lifetime            = 5.0;          // Lifetime of particles

    // Specific properties
    nParticles          = 1000;         // Number of particles born in the beginning
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
    sizeAge             = true;         // Decrease size when particles get older?
    opacityAge          = true;         // Decrease opacity when particles get older?
    colorAge            = true;        // Change color when particles get older?
    flickersAge         = false;        // Particle flickers opacity when about to die?
    isEmpty             = false;        // Draw only contours of the particles?
    drawLine            = false;        // Draw a line instead of a circle for the particle?

    // Physics
    friction            = 5.0;          // Friction to velocity 0~100
    gravity             = 0.0f;         // Makes particles fall down in a natural way
    turbulence          = 0.2f;         // Turbulence perlin noise
    repulse             = false;        // Repulse particles between each other?
    bounce              = false;        // Bounce particles with the walls of the window?
    steer               = false;        // Steers direction before touching the walls of the window?

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
}

ParticleSystem::~ParticleSystem(){
    // Delete all the particles
    for (int i = 0; i < particles.size(); i++) {
        delete particles.at(i);
        particles.at(i) = NULL;
    }
    particles.clear();
}


void ParticleSystem::setup(ParticleMode particleMode, InputSource inputSource, int width , int height){

    this->particleMode = particleMode;

    this->width = width;
    this->height = height;

    if(inputSource == MARKERS) markersInput = true;
    else if(inputSource == CONTOUR) contourInput = true;

    if(particleMode == EMITTER){
        emit = true;
        velocity = 20;
    }

    else if(particleMode == GRID){
        interact = true;
        returnToOrigin = true;
        repulseInteraction = true;
        immortal = true;
        friction = 30;
        createParticleGrid(width, height);
    }

    else if(particleMode == RANDOM){
        interact = true;
        immortal = true;
        addParticles(nParticles);
    }

    else if(particleMode == BOIDS){ // TODO: BOIDS == RANDOM?
        flock = true;
        immortal = true;
        steer = true;
        addParticles(nParticles);
    }
}

void ParticleSystem::update(float dt, vector<irMarker> &markers, Contour& contour){
    if(isActive){
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
                    // dir = closestMarker - particles[i]->pos;
                    // dir.normalize();

                    if(closestMarker != ofPoint(-1, -1)){
                        if(repulseInteraction) particles[i]->addRepulsionForce(closestMarker.x, closestMarker.y, markerRadius*markerRadius, 5.0);
                        if(attractInteraction) particles[i]->addAttractionForce(closestMarker.x, closestMarker.y, markerRadius*markerRadius, 5.0);
                        if(gravityInteraction) particles[i]->addForce(ofPoint(0, gravity*particles[i]->mass));
                        particles[i]->isTouched = true;
                    }
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

            if(returnToOrigin) particles[i]->xenoToOrigin(0.03);
        }

        if(emit){ // Born new particles
            if(markersInput){
                for(unsigned int i = 0; i < markers.size(); i++){
                    if (markers[i].hasDisappeared) markers[i].bornRate -= 0.5;
                    else markers[i].bornRate = bornRate;
                    addParticles(markers[i].bornRate, markers[i]);
                }
            }
            if(contourInput){
                for(unsigned int i = 0; i < contour.contours.size(); i++){
                    addParticles(bornRate, contour.contours[i]);
                }
            }
        }

        if(flock) flockParticles();
        if(repulse) repulseParticles();

        // ---------- (3) Add some general behavior and update the particles
        for(int i = 0; i < particles.size(); i++){
            ofPoint gravityForce(0, gravity*particles[i]->mass/10);
            particles[i]->addForce(gravityForce);

//            ofPoint windForce(ofRandom(-0.1, 0.1), ofRandom(-0.08, 0.06));
//            particles[i]->addForce(windForce*particles[i]->mass);

            particles[i]->addNoise(15.0, turbulence, dt);

            particles[i]->immortal = immortal;
            particles[i]->friction = 1-friction/1000;
            particles[i]->bounces = bounce;
            particles[i]->steers = steer;

            particles[i]->update(dt);
        }
    }
}

void ParticleSystem::draw(){
    if(isActive){
        for(int i = 0; i < particles.size(); i++){
            particles[i]->draw();
        }
    }
}

void ParticleSystem::addParticle(ofPoint pos, ofPoint vel, ofColor color, float radius, float lifetime){
    Particle * newParticle = new Particle();
    float id = totalParticlesCreated;

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

        float initialRadius = radius + randomRange(radiusRnd, radius);
        float lifetime = this->lifetime + randomRange(lifetimeRnd, this->lifetime);

        addParticle(pos, vel, color, initialRadius, lifetime);
    }
}

void ParticleSystem::addParticles(int n, const irMarker &marker){
    for(int i = 0; i < n; i++){
        ofPoint pos = marker.smoothPos + randomVector()*ofRandom(0, emitterSize);
        ofPoint vel = randomVector()*(velocity+randomRange(velocityRnd, velocity));
        vel += marker.velocity*(velocityMotion/100)*6;

        float initialRadius = radius + randomRange(radiusRnd, radius);
        float lifetime = this->lifetime + randomRange(lifetimeRnd, this->lifetime);

        addParticle(pos, vel, marker.color, initialRadius, lifetime);
    }
}

void ParticleSystem::addParticles(int n, const ofPolyline &contour){
    for(int i = 0; i < n; i++){
        // Create particles only inside contour polyline
        ofRectangle box = contour.getBoundingBox();
        ofPoint center = box.getCenter();
        ofPoint pos;
        pos.x = center.x + (ofRandom(1.0f) - 0.5f) * box.getWidth();
        pos.y = center.y + (ofRandom(1.0f) - 0.5f) * box.getHeight();

        while(!contour.inside(pos)){
            pos.x = center.x + (ofRandom(1.0f) - 0.5f) * box.getWidth();
            pos.y = center.y + (ofRandom(1.0f) - 0.5f) * box.getHeight();
        }

        ofPoint vel = randomVector()*(velocity+randomRange(velocityRnd, velocity));
//        vel += contour.velocity*(velocityMotion/100)*6;

        float initialRadius = radius + randomRange(radiusRnd, radius);
        float lifetime = this->lifetime + randomRange(lifetimeRnd, this->lifetime);

        addParticle(pos, vel, color, initialRadius, lifetime);
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
            addParticle(ofPoint(xi, yi), vel, color, radius, lifetime);
        }
    }
}

void ParticleSystem::removeParticles(int n){
    for(int i = 0; i < n; i++){
        particles[i]->immortal = false;
    }
}

void ParticleSystem::killParticles(){
    // TODO: fadeOut when we desactivate the ps
    for(int i = 0; i < particles.size(); i++){
        particles[i]->immortal = false;
    }
}

void ParticleSystem::bornParticles(){
    // Kill all the remaining particles before creating new ones
    for(int i = 0; i < particles.size(); i++){
        particles[i]->isAlive = false;
    }

    if(particleMode == GRID){
        createParticleGrid(width, height);
    }

    else if(particleMode == RANDOM || particleMode == BOIDS){
        addParticles(nParticles);
    }
}

void ParticleSystem::repulseParticles(){
    for(int i = 0; i < particles.size(); i++){
        for(int j = i-1; j >= 0; j--){
            if (fabs(particles[j]->pos.x - particles[i]->pos.x) > radius*2) break; // to speed the loop
            particles[i]->addRepulsionForce( *particles[j], radius*radius, 1.0);
//            particles[i]->addRepulsionForce( *particles[j], 1.0);
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
