#include "ParticleSystem.h"

ParticleSystem::ParticleSystem() {

}

void ParticleSystem::setup(bool immortal, ofColor color, float gravity, bool sizeAge, bool opacityAge, bool colorAge, bool bounce) {
    numParticles = 0;
    totalParticlesCreated = 0;

    particleMode = GRID_PARTICLES;

    this->immortal = immortal;
    this->color = color;

    this->gravity = gravity;

    this->sizeAge = sizeAge;
    this->opacityAge = opacityAge;
    this->colorAge = colorAge;
    this->bounce = bounce;

    killingParticles = false;
    createParticleGrid(ofGetWidth(), ofGetHeight(), 10);
}

void ParticleSystem::setup(float bornRate, float velocity, float velocityRnd, float velocityMotion, float emitterSize, bool immortal, float lifetime,
    float lifetimeRnd, ofColor color, float radius, float radiusRnd, float friction, float gravity, bool sizeAge, bool opacityAge, bool colorAge, bool bounce) {

    particleMode = BORN_PARTICLES;

    numParticles = 0;
    totalParticlesCreated = 0;

    this->bornRate = bornRate;
    this->velocity = velocity;
    this->velocityRnd = velocityRnd;
    this->velocityMotion = velocityMotion;
    this->emitterSize = emitterSize;
    this->immortal = immortal;
    this->lifetime = lifetime;
    this->lifetimeRnd = lifetimeRnd;
    this->color = color;
    this->radius = radius;
    this->radiusRnd = radiusRnd;
    this->friction = friction;

    this->gravity = gravity;

    this->sizeAge = sizeAge;
    this->opacityAge = opacityAge;
    this->colorAge = colorAge;
    this->bounce = bounce;

    killingParticles = false;
}

void ParticleSystem::update(float dt, vector<Marker>& markers) {
    //Update the particles
    for(int i = 0; i < particles.size(); i++) {
        particles[i].update(dt, markers);
    }
}

void ParticleSystem::update(float dt, const ofPoint &markerPos, const ofPoint &markerVel) {
    //Delete inactive particles
	int i = 0;
	while (i < particles.size()) {
		if (!particles[i].isAlive) {
			particles.erase(particles.begin() + i);
			numParticles--;
		}
		else {
			i++;
		}
	}

    //Born new particles
    if(particleMode == BORN_PARTICLES) {
        if (killingParticles) currentBornRate -= 0.5;
        else currentBornRate = bornRate;
        addParticles(currentBornRate, markerPos, markerVel);
    }

    //Update the particles
    for(int i = 0; i < particles.size(); i++) {
        particles[i].update(dt);
    }
}

void ParticleSystem::draw() {
    for(int i = 0; i < particles.size(); i++) {
        particles[i].draw();
    }
}

void ParticleSystem::createParticleGrid(int width, int height, int res) {
    for(int y = 0; y < height/res; y++) {
        for(int x = 0; x < width/res; x++) {
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

void ParticleSystem::addParticle(int x, int y, float initialRadius) {
    Particle newParticle;
    float id = totalParticlesCreated;
    ofPoint pos(x, y);
    ofPoint vel(0);
    float lifetime = 1;
    float friction = 0;

    newParticle.setup(id, pos, vel, color, initialRadius, immortal, lifetime, friction);
    newParticle.bounces = bounce;
    newParticle.sizeAge = sizeAge;
    newParticle.opacityAge = opacityAge;
    newParticle.colorAge = colorAge;
    particles.push_back(newParticle);

    numParticles++;
    totalParticlesCreated++;
}

void ParticleSystem::addParticles(int n) {
    for(int i = 0; i < n; i++) {
        Particle newParticle;
        float id = totalParticlesCreated;
        ofPoint pos(ofRandom(ofGetWidth()), ofRandom(ofGetHeight()));
        ofPoint vel = randomVector()*(velocity+randomRange(velocityRnd, velocity));
        float initialRadius = radius + randomRange(radiusRnd, radius);
        float lifetime = this->lifetime + randomRange(lifetimeRnd, this->lifetime);
        float friction = ofRandom(0.95f, 0.998f);

        newParticle.setup(id, pos, vel, color, initialRadius, immortal, lifetime, friction);
        newParticle.bounces = bounce;
        newParticle.sizeAge = sizeAge;
        newParticle.opacityAge = opacityAge;
        newParticle.colorAge = colorAge;
        particles.push_back(newParticle);

        numParticles++;
        totalParticlesCreated++;
    }
}

void ParticleSystem::addParticles(int n, const ofPoint &position, const ofPoint &markerVel) {
    for(int i = 0; i < n; i++) {
        Particle newParticle;
        float id = totalParticlesCreated;
        ofPoint pos = position + randomVector()*ofRandom(0, emitterSize);
        ofPoint vel = randomVector()*(velocity+randomRange(velocityRnd, velocity));
        vel += markerVel*(velocityMotion/100)*6;
        float initialRadius = radius + randomRange(radiusRnd, radius);
        float lifetime = this->lifetime + randomRange(lifetimeRnd, this->lifetime);

        newParticle.setup(id, pos, vel, color, initialRadius, immortal, lifetime, friction);
        newParticle.bounces = bounce;
        newParticle.sizeAge = sizeAge;
        newParticle.opacityAge = opacityAge;
        newParticle.colorAge = colorAge;
        particles.push_back(newParticle);

        numParticles++;
        totalParticlesCreated++;
    }
}

void ParticleSystem::removeParticles(int n) {
    for(int i = 0; i < n; i++) {
        particles.pop_back();
    }
}

void ParticleSystem::killParticles() {
    killingParticles = true;
}

void ParticleSystem::bornParticles() {
    killingParticles = false;
}

void ParticleSystem::repulseParticles() {
    for(int i = 0; i < particles.size(); i++) {
        for(int j = i+1; j < particles.size(); j++) {
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

ofPoint ParticleSystem::randomVector() {
    float angle = ofRandom((float)M_PI * 2.0f);
    return ofPoint(cos(angle), sin(angle));
}

float ParticleSystem::randomRange(float percentage, float value) {
    return ofRandom(-(percentage/100)*value, (percentage/100)*value);
}
