#include "ParticleSystem.h"

ParticleSystem::ParticleSystem() {

}

void ParticleSystem::setup(ParticleMode particleMode = BORN_PARTICLES) {
    numParticles = 0;
    totalParticlesCreated = 0;
    this->particleMode = particleMode;
    color.setHsb(ofRandom(0, 255), 255, 255);

    switch(particleMode) {
        case GRID_PARTICLES:
            createParticleGrid(ofGetWidth(), ofGetHeight(), 10);
            break;
        case BORN_PARTICLES:
            killingParticles = false;
            bornRate = 5;
            break;
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
        if (killingParticles) bornRate -= 0.5;
        else bornRate = 5;
        addParticles(bornRate, markerPos, markerVel);
    }

    //Update the particles
    for(int i = 0; i < particles.size(); i++) {
        particles[i].update(dt, markerPos);
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


void ParticleSystem::addParticles(int n) {
    for(int i = 0; i < n; i++) {
        Particle newParticle;
        float id = totalParticlesCreated;
        ofPoint pos(ofRandom(ofGetWidth()), ofRandom(ofGetHeight()));
        ofPoint vel(randomVector()*5.0f);
        float initialRadius = ofRandom(5.0f, 8.0f);
        float lifetime = ofRandom(1, 4);
        float friction = ofRandom(0.95f, 0.998f);

        newParticle.setup(id, pos, vel, color, initialRadius, lifetime, friction);
        particles.push_back(newParticle);

        numParticles++;
        totalParticlesCreated++;
    }
}

void ParticleSystem::addParticles(int n, const ofPoint &pos_, const ofPoint &vel_) {
    for(int i = 0; i < n; i++) {
        Particle newParticle;
        float id = totalParticlesCreated;
        ofPoint pos(pos_ + randomVector()*8.0f);
        ofPoint velOffset = randomVector()*ofRandom(30.0f, 80.0f);
        ofPoint vel(vel_*ofRandom(3.0f, 6.0f) + velOffset);
        float initialRadius = 5.0f;
        float lifetime = ofRandom(1.5, 4.2);
        float friction = ofRandom(0.99f, 0.9999f);

        newParticle.setup(id, pos, vel, color, initialRadius, lifetime, friction);
        particles.push_back(newParticle);

        numParticles++;
        totalParticlesCreated++;
    }
}

void ParticleSystem::addParticle(int x, int y, float initialRadius) {
    Particle newParticle;
    float id = totalParticlesCreated;
    ofPoint pos(x, y);
    ofPoint vel(0);
    float lifetime = 0;
    float friction = 0;

    newParticle.setup(id, pos, vel, color, initialRadius, lifetime, friction);
    particles.push_back(newParticle);

    numParticles++;
    totalParticlesCreated++;
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

                particles[i].acc += dir * ( F / particles[i].mass );
                particles[j].acc -= dir * ( F / particles[j].mass );
            }
        }
    }
}

ofPoint ParticleSystem::randomVector() {
    float angle = ofRandom((float)M_PI * 2.0f);
    return ofPoint(cos(angle), sin(angle));
}
