#include "ParticleSystem.h"

bool comparisonFunction(Particle * a, Particle * b) {
    return a->pos.x < b->pos.x;
}

ParticleSystem::ParticleSystem(){
    isActive            = false;        // Particle system is active?

    opacity             = 255.0;          // Opacity of the particles

    // General properties
    immortal            = false;        // Can particles die?
    velocity            = 0.0;          // Initial velocity magnitude of newborn particles
    radius              = 3.0;          // Radius of the particles
    color               = ofColor(255); // Color of the particles
    lifetime            = 5.0;          // Lifetime of particles

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

    // Physics
    friction            = 5.0;          // Friction to velocity 0~100
    gravity             = 0.0f;         // Makes particles fall down in a natural way
    turbulence          = 0.2f;         // Turbulence perlin noise
    repulse             = false;        // Repulse particles between each other?
    bounce              = false;        // Bounce particles with the walls of the window?
    steer               = false;        // Steers direction before touching the walls of the window?
    infiniteWalls       = false;        // Infinite walls?
    bounceDamping       = true;         // Decrease velocity when particle bounces walls?

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

    emitInsideContour   = false;        // Emit particles inside all the area of the contour?
    useFlow             = false;        // Use optical flow to get the motion velocity?
    useFlowRegion       = false;        // Use optical flow region to get the motion velocity?
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
        sizeAge             = true;        // Decrease size when particles get older?
        opacityAge          = true;        // Decrease opacity when particles get older?
        colorAge            = true;        // Change color when particles get older?
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
        interact = true;
        markerRadius = 600;
        radiusRnd = 0;
        attractInteraction = true;
//        repulseInteraction = true;
        bounceDamping = false;
        bounce = true;
        immortal = true;
        addParticles(nParticles);
    }

    else if(particleMode == ANIMATIONS){
        immortal = true;
        drawLine = false;
        bounce = false;
        infiniteWalls = false;
        flickersAge = false;
        opacity = 150.0;
        radius = 2.0;
        radiusRnd = 20.0;
        turbulence = 0.0;
        gravity = 0.0;
        numParticles = 1500;
        friction = 4.0;
        velocity = 10.0;
        velocityRnd = 30.0;

        if(animation == SNOW){
//            velocity = 8.0;
            infiniteWalls = true;
            addParticles(nParticles);
        }
        else if(animation == RAIN){
//            velocity = 15.0;
            friction = 6.0;
            infiniteWalls = true;
            radius = 0.8;
            addParticles(nParticles);
        }
        else if(animation == WIND){
            infiniteWalls = true;
            velocity = 30.0;
            velocityRnd = 80.0;
            turbulence = 8.0;
            addParticles(nParticles);
        }
        else if(animation == EXPLOSION){
//            bounce = true;
            radius = 6.0;
            radiusRnd = 60.0;
            friction = 50.0;
//            velocity = 6.0;
            immortal = false;
            lifetime = 3.0;
            lifetimeRnd = 10.0;
            flickersAge = true;
            addParticles(nParticles);
        }
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
//                    ofPoint closestMarker = getClosestMarker(*particles[i], markers, markerRadius);
                    ofPoint closestMarker = getClosestMarker(*particles[i], markers, 20000);

                    // Get direction vector to closest marker
                    // dir = closestMarker - particles[i]->pos;
                    // dir.normalize();

                    if(closestMarker != ofPoint(-1, -1)){
                        if(repulseInteraction) particles[i]->addRepulsionForce(closestMarker.x, closestMarker.y, markerRadius*markerRadius, 10.0);
                        if(attractInteraction) particles[i]->addAttractionForce(closestMarker.x, closestMarker.y, markerRadius*markerRadius, 8.0);
                        if(gravityInteraction) particles[i]->addForce(ofPoint(0, gravity*particles[i]->mass));
                        particles[i]->isTouched = true;
                    }
                }
                if(contourInput){
//                    ofPoint frc = contour.getFlowOffset(particles[i]->pos);
//                    particles[i]->addForce(frc);
                    float dimRegion = 10.0;
                    ofRectangle flowRegion(particles[i]->pos.x-dimRegion/2.0, particles[i]->pos.y-dimRegion/2.0, dimRegion, dimRegion);
                    ofPoint frc = contour.getAverageFlowInRegion(flowRegion);
                    particles[i]->addForce(frc);
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

            if(animation == SNOW || animation == WIND){
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
//                    if (markers[i].hasDisappeared) bornRate -= 1.0;
//                    else markers[i].bornRate = bornRate;
//                    addParticles(markers[i].bornRate, markers[i]);
                    if (!markers[i].hasDisappeared) addParticles(bornRate, markers[i]);
                }
            }
            if(contourInput){
                for(unsigned int i = 0; i < contour.contours.size(); i++){
                    addParticles(bornRate, contour.contours[i], contour);
                }
            }
        }

        if(flock) flockParticles();
        if(repulse) repulseParticles();

        // ---------- (3) Add some general behavior and update the particles
        for(int i = 0; i < particles.size(); i++){
            ofPoint gravityForce(0, gravity);
//            ofPoint gravityForce(0, gravity);
            particles[i]->addForce(gravityForce*particles[i]->mass);

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
}

void ParticleSystem::draw(){
    if(isActive){
        ofPushStyle();
        for(int i = 0; i < particles.size(); i++){
            particles[i]->draw();
        }

        //Draw lines between near points
        if(drawConnections){
            float dist = 15; //Threshold parameter of distance
            float distSqrd = dist*dist;
            ofSetColor(color, opacity);
            for(int i = 0; i < particles.size(); i++){
                for(int j = i-1; j >= 0; j--){
                    if (fabs(particles[j]->pos.x - particles[i]->pos.x) > dist) break; // to speed the loop
                    if(particles[i]->pos.squareDistance(particles[j]->pos) < distSqrd){
                        ofLine(particles[i]->pos, particles[j]->pos);
                    }
                }
            }
            ofPopStyle();
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

        if(particleMode == ANIMATIONS && animation == RAIN){
            vel.x = 0;
            vel.y = fabs(vel.y) * 30.0; // make particles all be going down when born
        }
        else if(particleMode == ANIMATIONS && animation == SNOW){
            vel.y = fabs(vel.y) * 17.0; // make particles all be going down when born
        }
        else if(particleMode == ANIMATIONS && animation == EXPLOSION){
            pos = ofPoint(ofRandom(width), ofRandom(height+radius, height*2));
            vel.y = fabs(vel.y) * -200.0; // make particles all be going up when born
        }

        float initialRadius = radius + randomRange(radiusRnd, radius);
        float lifetime = this->lifetime + randomRange(lifetimeRnd, this->lifetime);

        addParticle(pos, vel, color, initialRadius, lifetime);
    }
}

void ParticleSystem::addParticles(int n, const irMarker &marker){
    for(int i = 0; i < n; i++){
        ofPoint pos = marker.smoothPos + randomVector()*ofRandom(0, emitterSize);
        ofPoint vel = randomVector()*(velocity+2*randomRange(velocityRnd, velocity));
        vel += marker.velocity*(velocityMotion/100)*6;

        float initialRadius = radius + randomRange(radiusRnd, radius);
        float lifetime = this->lifetime + randomRange(lifetimeRnd, this->lifetime);

        addParticle(pos, vel, marker.color, initialRadius, lifetime);
    }
}

void ParticleSystem::addParticles(int n, const ofPolyline &contour, Contour &flow){
    for(int i = 0; i < n; i++){

        ofPoint pos, vel;

        // Create random particles inside contour polyline
        if(emitInsideContour){
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
        else{
            float indexInterpolated = ofRandom(0, contour.size());
            pos = contour.getPointAtIndexInterpolated(indexInterpolated);

            // use point normal vector as velocity
            vel = contour.getNormalAtIndexInterpolated(indexInterpolated)*(velocity+2*randomRange(velocityRnd, velocity))*-1;
        }

        if(useFlow){
            ofPoint motionVel = flow.getFlowOffset(pos);
            vel += motionVel*(velocityMotion/100)*150;
        }
        else if(useFlowRegion){
            float dimRegion = 5.0;
            ofRectangle flowRegion(pos.x-dimRegion/2.0, pos.y-dimRegion/2.0, dimRegion, dimRegion);
            ofPoint motionVel = flow.getAverageFlowInRegion(flowRegion);
            vel += motionVel*(velocityMotion/100)*150;
        }
        else{
            ofPoint motionVel = flow.getVelocityInPoint(pos);
            vel += motionVel*(velocityMotion/100)*6;
        }

        pos.x += ofRandom(-emitterSize, emitterSize);
        pos.y += ofRandom(-emitterSize, emitterSize);

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

    InputSource inputSource = MARKERS;
    if(contourInput == true) inputSource = CONTOUR;
    setup(particleMode, inputSource, width, height); // resets the settings to default

//    if(particleMode == GRID){
//        createParticleGrid(width, height);
//    }
//
//    else if(particleMode == RANDOM || particleMode == BOIDS || particleMode == ANIMATIONS){
//        addParticles(nParticles);
//    }
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

void ParticleSystem::setAnimation(Animation animation){
    this->animation = animation;
}
