#include "ParticleSystem.h"

ParticleSystem::ParticleSystem(){
    isActive        = false;        // Particle system is active?

    bornRate        = 5;            // Number of particles born per frame
    velocity        = 50;           // Initial velocity magnitude of newborn particles
    velocityRnd     = 20;           // Magnitude randomness % of the initial velocity
    velocityMotion  = 50;           // Marker motion contribution to the initial velocity
    emitterSize     = 8.0f;         // Size of the emitter area
    lifetime        = 3;            // Lifetime of particles
    lifetimeRnd     = 60;           // Randomness of lifetime
    color           = ofColor(255); // Color of the particles
    radius          = 5;            // Radius of the particles
    radiusRnd       = 20;           // Randomness of radius

    immortal        = false;        // Can particles die?
    sizeAge         = true;         // Decrease size when particles get older?
    opacityAge      = true;         // Decrease opacity when particles get older?
    flickersAge     = true;         // Particle flickers opacity when about to die?
    colorAge        = true;         // Change color when particles get older?
    isEmpty         = true;         // Draw only contours of the particles?
    bounce          = true;         // Bounce particles with the walls of the window?

    friction        = 0;            // Multiply this value by the velocity every frame
    gravity         = 1.0f;         // Makes particles fall down in a natural way
}

void ParticleSystem::setup(ParticleMode particleMode, int width , int height){

	this->particleMode = particleMode;
	this->width = width;
	this->height = height;

    if(particleMode == GRID_PARTICLES){
        immortal = true;
        createParticleGrid(width, height, 10);
    }
}

void ParticleSystem::update(float dt, vector<irMarker> &markers){

	if(particleMode == GRID_PARTICLES){
		// Update the particles
		for(int i = 0; i < particles.size(); i++){
			particles[i].update(dt, markers);
		}
	}

	else if(particleMode == MARKER_PARTICLES){
		// Delete inactive particles
		int i = 0;
		while (i < particles.size()){
			if (!particles[i].isAlive){
				particles.erase(particles.begin() + i);
				numParticles--;
			}
			else{
				i++;
			}
		}

		// Born new particles
		if(isActive){
            for(unsigned int i = 0; i < markers.size(); i++){
                if (markers[i].hasDisappeared) markers[i].bornRate -= 0.5;
                else markers[i].bornRate = bornRate;
                addParticles(markers[i].bornRate, markers[i]);
            }
		}

		// Update the particles
		for(int i = 0; i < particles.size(); i++){
            ofPoint gravityForce(0, gravity*particles[i].mass);
            particles[i].applyForce(gravityForce);
			particles[i].update(dt);
		}
	}
}


void ParticleSystem::update(float dt, Contour& contour){
    if(particleMode == CONTOUR_PARTICLES){
        // Delete inactive particles
		int i = 0;
		while (i < particles.size()){
			if (!particles[i].isAlive){
				particles.erase(particles.begin() + i);
				numParticles--;
			}
			else{
				i++;
			}
		}

		// Born new particles
		if(isActive){
            for(unsigned int i = 0; i < contour.contours.size(); i++){
                addParticles(bornRate, contour.contours[i]);
            }
		}

		// Update the particles
		for(int i = 0; i < particles.size(); i++){
            ofPoint windForce(0.2, -0.1);
            particles[i].applyForce(windForce);
			particles[i].update(dt);
		}
    }
}

void ParticleSystem::draw(){
	for(int i = 0; i < particles.size(); i++){
		particles[i].draw();
	}
}

void ParticleSystem::addParticle(ofPoint pos, ofPoint vel, ofColor color, float radius, float lifetime){
	Particle newParticle;
	float id = totalParticlesCreated;

	newParticle.setup(id, pos, vel, color, radius, lifetime);
	newParticle.immortal = immortal;
	newParticle.sizeAge = sizeAge;
	newParticle.opacityAge = opacityAge;
	newParticle.flickersAge = flickersAge;
	newParticle.colorAge = colorAge;
	newParticle.isEmpty = isEmpty;
	newParticle.bounces = bounce;
	newParticle.friction = 1-friction/1000;

	newParticle.width = width;
	newParticle.height = height;

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

void ParticleSystem::createParticleGrid(int width, int height, int res){
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
		particles[i].immortal = false;
	}
}

void ParticleSystem::repulseParticles(){
	for(int i = 0; i < particles.size(); i++){
		for(int j = i+1; j < particles.size(); j++){
			ofVec2f dir = particles[i].pos - particles[j].pos;
			float distSqrd = particles[i].pos.squareDistance(particles[j].pos);

			if( distSqrd > 0.0f ){
				dir.normalize();
				float F = 1.0f/distSqrd;

				particles[i].acc += dir * (F/particles[i].mass);
				particles[j].acc -= dir * (F/particles[j].mass);
			}
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
