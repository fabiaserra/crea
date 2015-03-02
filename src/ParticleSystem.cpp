#include "ParticleSystem.h"

bool comparisonFunction(Particle * a, Particle * b) {
    return a->pos.x < b->pos.x;
}

ParticleSystem::ParticleSystem(){
    isActive        = true;         // Particle system is active?

    bornRate        = 0;            // Number of particles born per frame
    velocity        = 0;            // Initial velocity magnitude of newborn particles
    velocityRnd     = 0;            // Magnitude randomness % of the initial velocity
    velocityMotion  = 0;            // Marker motion contribution to the initial velocity
    emitterSize     = 1.0f;         // Size of the emitter area
    lifetime        = 5;            // Lifetime of particles
    lifetimeRnd     = 0;            // Randomness of lifetime
    color           = ofColor(255); // Color of the particles
    radius          = 3;            // Radius of the particles
    radiusRnd       = 0;            // Randomness of radius

    immortal        = false;        // Can particles die?
    sizeAge         = false;        // Decrease size when particles get older?
    opacityAge      = false;        // Decrease opacity when particles get older?
    flickersAge     = false;        // Particle flickers opacity when about to die?
    colorAge        = false;        // Change color when particles get older?
    isEmpty         = false;        // Draw only contours of the particles?
    drawLine        = false;        // Draw a line instead of a circle for the particle?
    bounce          = false;        // Bounce particles with the walls of the window?

    friction        = 20;           // Friction to velocity 0~100
    gravity         = 0.0f;         // Makes particles fall down in a natural way
}

ParticleSystem::~ParticleSystem(){
    for (int i = 0; i < particles.size(); i++) {
        delete particles.at(i);
        particles.at(i) = NULL;
    }
    particles.clear();
}


void ParticleSystem::setup(ParticleMode particleMode, InputSource inputSource, int width , int height){

    this->particleMode = particleMode;
    this->inputSource = inputSource;
    this->width = width;
    this->height = height;

    if(particleMode == GRID){
        immortal = true;
        gridRes = 10;
        createParticleGrid(width, height);
    }

    else if(particleMode == RANDOM){
        immortal = true;
        nParticles = 1500;
        addParticles(nParticles);
    }

    else if(particleMode == BOIDS){ // TODO: BOIDS == RANDOM?
        nParticles = 500;
        separationStrength = 1.5;
        cohesionStrength = 1.0;
        alignmentStrength = 1.0;
        
        separationDistance = radius*2;
        alignmentDistance = 50;
        cohesionDistance = 50;
        
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

        // TODO: not do the calculations depending on particle creation mode but on which interactions we want
        // being able to combine flocking with emitter?

        // ---------- (2) Calculate specific particle system behavior
        if(particleMode == GRID){
            float scale = 5;
            for(int i = 0; i < particles.size(); i++){
                if(inputSource == MARKERS){
                    float markerRadius = 50;

                    // Get closest marker to particle
                    ofPoint closestMarker = getClosestMarker(*particles[i], markers, markerRadius);

                    // Get direction vector to closest marker
//                    dir = closestMarker - particles[i]->pos;
//                    dir.normalize();

                    if(closestMarker != ofPoint(-1, -1)){
                        particles[i]->addRepulsionForce(closestMarker.x, closestMarker.y, markerRadius*markerRadius, scale);
//                      ofPoint gravityForce(0, gravity*particles[i]->mass);
//                      particles[i]->addForce(gravityForce);
//                      particles[i]->isTouched = true;
                    }
                }
                particles[i]->xenoToOrigin(0.03);
            }
        }

        else if(particleMode == BOIDS){
            float scale = 5;
            float markerRadius = 50;
            for(int i = 0; i < particles.size(); i++){
                particles[i]->separation.distSqrd   =   separationDistance * separationDistance;
                particles[i]->cohesion.distSqrd     =   cohesionDistance * cohesionDistance;
                particles[i]->alignment.distSqrd    =   alignmentDistance * alignmentDistance;
                
                particles[i]->separation.strength   =   separationStrength;
                particles[i]->cohesion.strength     =   cohesionStrength;
                particles[i]->alignment.strength    =   alignmentStrength;
                
                // Get closest marker to particle
                ofPoint closestMarker = getClosestMarker(*particles[i], markers, markerRadius);
                if(closestMarker != ofPoint(-1, -1))
                    particles[i]->addRepulsionForce(closestMarker.x, closestMarker.y, markerRadius*markerRadius, scale);
                
            }
            
            flockParticles();
            
            for(int i = 0; i < particles.size(); i++){
                particles[i]->addFlockingForces();
            }
        }

        else if(particleMode == EMITTER){

            // Born new particles
            if(inputSource == MARKERS){
                for(unsigned int i = 0; i < markers.size(); i++){
                    if (markers[i].hasDisappeared) markers[i].bornRate -= 0.5;
                    else markers[i].bornRate = bornRate;
                    addParticles(markers[i].bornRate, markers[i]);
                }
            }
            else if(inputSource == CONTOUR){
                for(unsigned int i = 0; i < contour.contours.size(); i++){
                    addParticles(bornRate, contour.contours[i]);
                }
            }
        }

//        if(repulseParticles) repulseParticles();

        // Update the particles
        for(int i = 0; i < particles.size(); i++){
            ofPoint gravityForce(0, gravity*particles[i]->mass/10);
            particles[i]->addForce(gravityForce);

//            particles[i]->addNoise(15.0, 0.5, dt);

//            ofPoint windForce(0.05, -0.02); // TODO: add some turbulence
//            ofPoint windForce(ofRandom(-0.1, 0.1), ofRandom(-0.08, 0.06)); // TODO: add some turbulence
//            particles[i]->addForce(windForce*particles[i]->mass);

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

    newParticle->immortal       = immortal;
    newParticle->sizeAge        = sizeAge;
    newParticle->opacityAge     = opacityAge;
    newParticle->flickersAge    = flickersAge;
    newParticle->colorAge       = colorAge;
    newParticle->isEmpty        = isEmpty;
    newParticle->drawLine       = drawLine;
    newParticle->bounces        = bounce;
    newParticle->friction       = 1-friction/1000; // so we have a range between 0.90 and 1

    newParticle->width = width;
    newParticle->height = height;

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
        numParticles--;
    }
}

void ParticleSystem::killParticles(){
    // TODO: fadeOut when we desactivate the ps
    for(int i = 0; i < particles.size(); i++){
        particles[i]->immortal = false;
        numParticles--; // should be 0 at the end
    }
}

void ParticleSystem::bornParticles(){
    // Kill all the remaining particles before creating new ones
    for(int i = 0; i < particles.size(); i++){
        particles[i]->isAlive = false;
    }

    if(particleMode == RANDOM){
        immortal = true;
        nParticles = 1500;
        addParticles(nParticles);
    }
    else if(particleMode == BOIDS){
        nParticles = 1000;
        immortal = true;
        addParticles(nParticles);
    }
    else if(particleMode == GRID){
        immortal = true;
        createParticleGrid(width, height);
    }
}

void ParticleSystem::repulseParticles(){
    for(int i = 0; i < particles.size(); i++){
        for(int j = i-1; j >= 0; j--){
            if ( fabs(particles[j]->pos.x - particles[i]->pos.x) > radius*3) break; // to speed the loop
//            particles[i]->addRepulsionForce( *particles[j], radius, 1.0);
            particles[i]->addRepulsionForce( *particles[j], 1.0);
        }
    }
}

void ParticleSystem::flockParticles(){
    int flockRegionRadius = 100;
    for(int i = 0; i < particles.size(); i++){
        for(int j = i-1; j >= 0; j--){
//            if ( fabs(particles[j]->pos.x - particles[i]->pos.x) > flockRegionRadius) break;
            particles[i]->addForFlocking(*particles[j]);
        }
    }
    
//    for (int i = 0; i < particles.size(); i++){
//        for (int j = 0; j < particles.size(); j++){
//            if (i != j){
//                particles[i]->addForFlocking(*particles[j]);
//            }
//        }
//    }
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
