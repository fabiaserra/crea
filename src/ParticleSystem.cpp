#include "ParticleSystem.h"

ParticleSystem::ParticleSystem(){
    isActive = true;
}

void ParticleSystem::setup(bool immortal, ofColor color, float gravity, bool sizeAge, bool opacityAge,
							bool flickersAge, bool colorAge, bool isEmpty, bool bounce){

	particleMode            = GRID_PARTICLES;

	numParticles            = 0;
	totalParticlesCreated   = 0;

	this->bornRate          = 0;
	this->velocity          = 0;
	this->velocityRnd       = 0;
	this->velocityMotion    = 0;
	this->emitterSize       = 0;
	this->immortal          = immortal;
	this->lifetime          = 6;
	this->lifetimeRnd       = 30;
	this->color             = color;
	this->radius            = 0;
	this->radiusRnd         = 0;
	this->friction          = 0;

	this->gravity           = gravity;

	this->sizeAge           = sizeAge;
	this->opacityAge        = opacityAge;
	this->flickersAge       = flickersAge;
	this->colorAge          = colorAge;
	this->isEmpty           = isEmpty;
	this->bounce            = bounce;

	createParticleGrid(ofGetWidth(), ofGetHeight(), 10);
}

void ParticleSystem::setup(float bornRate, float velocity, float velocityRnd, float velocityMotion,
							float emitterSize, bool immortal, float lifetime, float lifetimeRnd,
							ofColor color, float radius, float radiusRnd, float friction, float gravity,
							bool sizeAge, bool opacityAge, bool flickersAge, bool colorAge, bool isEmpty,
							bool bounce){

	particleMode            = BORN_PARTICLES;

	numParticles            = 0;
	totalParticlesCreated   = 0;

	this->bornRate          = bornRate;
	this->velocity          = velocity;
	this->velocityRnd       = velocityRnd;
	this->velocityMotion    = velocityMotion;
	this->emitterSize       = emitterSize;
	this->immortal          = immortal;
	this->lifetime          = lifetime;
	this->lifetimeRnd       = lifetimeRnd;
	this->color             = color;
	this->radius            = radius;
	this->radiusRnd         = radiusRnd;
	this->friction          = friction;

	this->gravity           = gravity;

	this->sizeAge           = sizeAge;
	this->opacityAge        = opacityAge;
	this->colorAge          = colorAge;
	this->isEmpty         = isEmpty;
	this->bounce            = bounce;
}

void ParticleSystem::update(float dt, vector<irMarker>& markers){

	if(particleMode == GRID_PARTICLES){
		// Update the particles
		for(int i = 0; i < particles.size(); i++){
			particles[i].update(dt, markers);
		}
	}

	else if(particleMode == BORN_PARTICLES){
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
                if (markers[i].hasDisappeared){
                    markers[i].bornRate -= 0.5;
                }
                else{
                    markers[i].bornRate = bornRate;
                }
                addParticles(markers[i].bornRate, markers[i].smoothPos, markers[i].velocity, markers[i].color);
            }
		}

		// Update the particles
		for(int i = 0; i < particles.size(); i++){
			particles[i].update(dt);
		}
	}

}

void ParticleSystem::draw(){
	for(int i = 0; i < particles.size(); i++){
		particles[i].draw();
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
			float xyOffset = sin( cos( sin( yi * 0.3183f ) + cos( xi * 0.3183f ) ) ) + 1.0f;
			float initialRadius = xyOffset * xyOffset * 1.8f;
			addParticle(xi, yi, initialRadius);
		}
	}
}

void ParticleSystem::addParticle(int x, int y, float initialRadius){
	Particle newParticle;
	float id = totalParticlesCreated;

	ofPoint pos = ofPoint(x, y);
	ofPoint vel = randomVector()*(velocity+randomRange(velocityRnd, velocity));

	newParticle.setup(id, pos, vel, color, initialRadius, immortal, lifetime, 1-friction/1000);
	newParticle.sizeAge = sizeAge;
	newParticle.opacityAge = opacityAge;
	newParticle.flickersAge = flickersAge;
	newParticle.colorAge = colorAge;
	newParticle.isEmpty = isEmpty;
	newParticle.bounces = bounce;
	particles.push_back(newParticle);

	numParticles++;
	totalParticlesCreated++;
}

void ParticleSystem::addParticles(int n){
	for(int i = 0; i < n; i++){
		Particle newParticle;
		float id = totalParticlesCreated;

		ofPoint pos = ofPoint(ofRandom(ofGetWidth()), ofRandom(ofGetHeight()));
		ofPoint vel = randomVector()*(velocity+randomRange(velocityRnd, velocity));

		float initialRadius = radius + randomRange(radiusRnd, radius);
		float lifetime = this->lifetime + randomRange(lifetimeRnd, this->lifetime);

		newParticle.setup(id, pos, vel, color, initialRadius, immortal, lifetime, 1-friction/1000);
		newParticle.sizeAge = sizeAge;
		newParticle.opacityAge = opacityAge;
		newParticle.flickersAge = flickersAge;
		newParticle.colorAge = colorAge;
		newParticle.isEmpty = isEmpty;
		newParticle.bounces = bounce;
		particles.push_back(newParticle);

		numParticles++;
		totalParticlesCreated++;
	}
}

void ParticleSystem::addParticles(int n, const ofPoint &markerPos, const ofPoint &markerVel, const ofColor &markerColor){
	for(int i = 0; i < n; i++){
		Particle newParticle;
		float id = totalParticlesCreated;

		ofPoint pos = markerPos + randomVector()*ofRandom(0, emitterSize);
		ofPoint vel = randomVector()*(velocity+randomRange(velocityRnd, velocity));
		vel += markerVel*(velocityMotion/100)*6;

		float initialRadius = radius + randomRange(radiusRnd, radius);
		float lifetime = this->lifetime + randomRange(lifetimeRnd, this->lifetime);

		newParticle.setup(id, pos, vel, markerColor, initialRadius, immortal, lifetime, 1-friction/1000);
		newParticle.sizeAge = sizeAge;
		newParticle.opacityAge = opacityAge;
		newParticle.flickersAge = flickersAge;
		newParticle.colorAge = colorAge;
		newParticle.isEmpty = isEmpty;
		newParticle.bounces = bounce;
		particles.push_back(newParticle);

		numParticles++;
		totalParticlesCreated++;
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
