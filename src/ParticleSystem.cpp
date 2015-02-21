#include "ParticleSystem.h"

bool comparisonFunction(Particle * a, Particle * b) {
	return a->pos.x < b->pos.x;
}

ParticleSystem::ParticleSystem(){
    isActive        = true;         // Particle system is active?

    bornRate        = 3;            // Number of particles born per frame
    velocity        = 50;           // Initial velocity magnitude of newborn particles
    velocityRnd     = 20;           // Magnitude randomness % of the initial velocity
    velocityMotion  = 50;           // Marker motion contribution to the initial velocity
    emitterSize     = 8.0f;         // Size of the emitter area
    lifetime        = 5;            // Lifetime of particles
    lifetimeRnd     = 20;           // Randomness of lifetime
    color           = ofColor(255); // Color of the particles
    radius          = 5;            // Radius of the particles
    radiusRnd       = 20;           // Randomness of radius

    immortal        = false;        // Can particles die?
    sizeAge         = false;        // Decrease size when particles get older?
    opacityAge      = false;        // Decrease opacity when particles get older?
    flickersAge     = false;        // Particle flickers opacity when about to die?
    colorAge        = false;        // Change color when particles get older?
    isEmpty         = false;        // Draw only contours of the particles?
    bounce          = false;        // Bounce particles with the walls of the window?

    friction        = 30;           // Decrease of the velocity
    gravity         = 1.0f;         // Makes particles fall down in a natural way
}

ParticleSystem::~ParticleSystem(){
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

    if(particleMode == GRID_PARTICLES){
        immortal = true;
        createParticleGrid(width, height);
    }
    
    ofSetCircleResolution(6);
}

void ParticleSystem::update(float dt, vector<irMarker> &markers){
    if(isActive){
        if(particleMode == GRID_PARTICLES){

            sort( particles.begin(), particles.end(), comparisonFunction );

//            ofPoint dir;
            ofPoint closestPos;
            bool closeEnough = false;
            float radius = 50;
            float minDist = radius;
            float scale = 1;

//            repulseParticles();

            for(int i = 0; i < particles.size(); i++){
                // Get closest marker to particle
                for(int markerIndex = 0; markerIndex < markers.size(); markerIndex++){
                    if (!markers[markerIndex].hasDisappeared){
                        float markerDist = particles[i]->pos.squareDistance(markers[markerIndex].smoothPos);
                        if(markerDist < minDist){
                            closeEnough = true;
//                            dir = markers[markerIndex].smoothPos - particles[i]->pos;
//                            dir.normalize();
                            minDist = markerDist;
                            closestPos = markers[markerIndex].smoothPos;
    //                        color = markers[markerIndex].color;
                        }
                    }
                }
                if(closeEnough){
                    particles[i]->addRepulsionForce(closestPos.x, closestPos.y, radius, scale);
                    particles[i]->isTouched = true;
                }
                if(particles[i]->isTouched){
                    ofPoint gravityForce(0, gravity*particles[i]->mass);
                    particles[i]->addForce(gravityForce);
                }
                
                particles[i]->xenoToPoint(0.1);
                
                particles[i]->update(dt);
            }
        }

        else if(particleMode == MARKER_PARTICLES){
            // Delete inactive particles
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

            // Born new particles
            for(unsigned int i = 0; i < markers.size(); i++){
                if (markers[i].hasDisappeared) markers[i].bornRate -= 0.5;
                else markers[i].bornRate = bornRate;
                addParticles(markers[i].bornRate, markers[i]);
            }

            // Update the particles
            for(int i = 0; i < particles.size(); i++){
                ofPoint gravityForce(0, gravity*particles[i]->mass);
                particles[i]->addForce(gravityForce);
                particles[i]->update(dt);
            }
        }
    }
}


void ParticleSystem::update(float dt, Contour& contour){
    if(isActive){
        if(particleMode == CONTOUR_PARTICLES){
            // Delete inactive particles
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

            // Born new particles
            for(unsigned int i = 0; i < contour.contours.size(); i++){
                addParticles(bornRate, contour.contours[i]);
            }

            // Update the particles
            for(int i = 0; i < particles.size(); i++){
                ofPoint windForce(0.5, -0.1);
                particles[i]->addForce(windForce*particles[i]->mass);
                particles[i]->update(dt);
            }
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

	newParticle->setup(id, pos, vel, color, radius, lifetime);
	newParticle->immortal = immortal;
	newParticle->sizeAge = sizeAge;
	newParticle->opacityAge = opacityAge;
	newParticle->flickersAge = flickersAge;
	newParticle->colorAge = colorAge;
	newParticle->isEmpty = isEmpty;
	newParticle->bounces = bounce;
	newParticle->friction = 1-friction/1000; // so we have a range between 0.90 and 1

	newParticle->width = width;
	newParticle->height = height;

	particles.push_back(newParticle);

	numParticles++;
	totalParticlesCreated++;
}

void ParticleSystem::addParticles(int n){
    for(int i = 0; i < n; i++){
        ofPoint pos = ofPoint(ofRandom(ofGetWidth()), ofRandom(ofGetHeight()));
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
	int res = 10;
    float radius = 3;
	for(int y = 0; y < height/res; y++){
		for(int x = 0; x < width/res; x++){
			int xi = (x + 0.5f) * res;
			int yi = (y + 0.5f) * res;
//            float initialRadius = (float)res / 2.0f;
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
		particles.pop_back();
	}
}

void ParticleSystem::killParticles(){
	for(int i = 0; i < particles.size(); i++){
		particles[i]->immortal = false;
	}
}

void ParticleSystem::bornParticles(){
    if(particleMode == GRID_PARTICLES){
        immortal = true;
        createParticleGrid(width, height);
    }
}

void ParticleSystem::repulseParticles(){
	for(int i = 0; i < particles.size(); i++){
		for(int j = i-1; j >= 0; j--){
            if ( fabs(particles[j]->pos.x - particles[i]->pos.x) >	5) break; // to speed the loop
            particles[i]->addRepulsionForce( *particles[j], 5, 1.1f);
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
