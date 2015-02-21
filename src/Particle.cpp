#include "Particle.h"

Particle::Particle(){
    isAlive         = true;
    isTouched       = false;
    immortal        = false;
    bounces         = false;
    sizeAge         = false;
    opacityAge      = false;
    colorAge        = false;
    flickersAge     = false;
    isEmpty         = false;
    age             = 0;
    width           = ofGetWidth();
    height          = ofGetHeight();
}

void Particle::setup(float id, ofPoint pos, ofPoint vel, ofColor color, float initialRadius, float lifetime){
    this->id = id;
    this->pos = pos;
    this->vel = vel;
    this->color = color;
    this->initialRadius = initialRadius;
    this->radius = initialRadius;
    this->lifetime = lifetime;

    this->mass = initialRadius * initialRadius * 0.005f;
    this->originalHue = color.getHue();
    this->prevPos = pos;
}

void Particle::update(float dt){
    if(isAlive){
        // Perlin noise
//         noise = ofNoise(pos.x*0.005f, pos.y*0.005f, dt*0.1f);
//         float angle = noise*30.0f;
//         acc = ofPoint(cos(angle), sin(angle)) * age * 0.1;

        // Update position
        acc += frc;
        vel += acc;
        pos += vel*dt;
        vel *= friction;
        acc.set(0, 0);

        // Update age and check if particle has to die
        if(!immortal){
            age += dt;
            if(age >= lifetime) isAlive = false;
        }

        // Decrease particle radius with age
        if (sizeAge) radius = initialRadius * (1.0f - (age/lifetime));

        // Decrease particle opacity with age
        opacity = 255;
        if (opacityAge) opacity *= (1.0f - (age/lifetime));
        if (flickersAge && (age/lifetime) > 0.94f && ofRandomf() > 0.3) opacity *= 0.2;

        // Change particle color with age
        if (colorAge){
            float saturation = ofMap(age, 0, lifetime, 255, 128);
            float hue = ofMap(age, 0, lifetime, originalHue, originalHue-100);
            color.setSaturation(saturation);
            color.setHue(hue);
        }

        // Bounce particle with the window margins
        if(bounces){
            bool hasCollided = false;
            bool collisionDamping = true;
            if(pos.x > width-radius){
                pos.x = width-radius;
                vel.x *= -1.0;
                hasCollided = true;
            }
            if(pos.x < radius){
                pos.x = radius;
                vel.x *= -1.0;
                hasCollided = true;
            }
            if(pos.y > height-radius){
                pos.y = height-radius;
                vel.y *= -1.0;
                hasCollided = true;
            }
            if(pos.y < radius){
                pos.y = radius;
                vel.y *= -1.0;
                hasCollided = true;
            }
            if (hasCollided && collisionDamping){
                vel *= 0.3;
            }
        }
    }
}

void Particle::draw(){
    if(isAlive){
        ofPushStyle();

        ofSetColor(color, opacity);

        if(isEmpty){
            ofNoFill();
            ofSetLineWidth(1);
        }
        else{
            ofFill();
        }

        ofCircle(pos, radius);

//        ofLine(pos, prevPos);
//        prevPos = pos;

        // // Draw arrows
        // if (markerDist == 0){
        //     ofCircle(pos, 2);
        // }
        // else{
        //     float length = 15.0f;
        //     ofPoint p1(pos);
        //     ofPoint p2(pos + dir*length);
        //     ofLine(p1, p2);
        // }

        ofPopStyle();
    }
}

void Particle::addForce(ofPoint force){
    frc += force/mass;
}

void Particle::addRepulsionForce(Particle &p, float radius, float scale){
    
    // ----------- (1) make a vector of where this particle p is:
    ofPoint posOfForce;
    posOfForce.set(p.pos.x,p.pos.y);
    
    // ----------- (2) calculate the difference & length
    
    ofVec2f diff    = pos - posOfForce;
    float length    = pos.squareDistance(posOfForce); // faster than length or distance (no square root)
    
    // ----------- (3) check close enough
    
    bool closeEnough = true;
    if (radius > 0){
        if (length > radius){
            closeEnough = false;
        }
    }
    
    // ----------- (4) if so, update force
    
    if (closeEnough == true){
        float pct = 1 - (length / radius);  // stronger on the inside
        diff.normalize();
        frc.x = frc.x + diff.x * scale * pct;
        frc.y = frc.y + diff.y * scale * pct;
        p.frc.x = p.frc.x - diff.x * scale * pct;
        p.frc.y = p.frc.y - diff.y * scale * pct;
    }
}

void Particle::addRepulsionForce(float x, float y, float radius, float scale){

    // ----------- (1) make a vector of where this position is:
	ofPoint posOfForce;
	posOfForce.set(x, y);

    // ----------- (2) calculate the difference & length

	ofPoint diff	= pos - posOfForce;
	float length	= pos.squareDistance(posOfForce); // faster than length or distance (no square root)

    // ----------- (3) check close enough

    bool closeEnough = true;
    if (radius > 0){
        if (length > radius){
            closeEnough = false;
        }
    }

    // ----------- (4) if so, update force
    if (closeEnough == true){
		float pct = 1 - (length / radius);  // stronger on the inside
        diff.normalize();
        frc.x = frc.x + diff.x * scale * pct;
        frc.y = frc.y + diff.y * scale * pct;
    }
}

void Particle::addAttractionForce(Particle &p, float radius, float scale){
    
    // ----------- (1) make a vector of where this particle p is:
    ofPoint posOfForce;
    posOfForce.set(p.pos.x,p.pos.y);
    
    // ----------- (2) calculate the difference & length
    
    ofVec2f diff    = pos - posOfForce;
    float length    = pos.squareDistance(posOfForce); // faster than length or distance (no square root)
    
    // ----------- (3) check close enough
    
    bool closeEnough = true;
    if (radius > 0){
        if (length > radius){
            closeEnough = false;
        }
    }
    
    // ----------- (4) if so, update force
    
    if (closeEnough == true){
        float pct = 1 - (length / radius);  // stronger on the inside
        diff.normalize();
        frc.x = frc.x - diff.x * scale * pct;
        frc.y = frc.y - diff.y * scale * pct;
        p.frc.x = p.frc.x + diff.x * scale * pct;
        p.frc.y = p.frc.y + diff.y * scale * pct;
    }
}

void Particle::addAttractionForce(float x, float y, float radius, float scale){

    // ----------- (1) make a vector of where this position is:
	ofPoint posOfForce;
	posOfForce.set(x, y);

    // ----------- (2) calculate the difference & length

	ofPoint diff	= pos - posOfForce;
	float length	= pos.squareDistance(posOfForce); // faster than length or distance (no square root)

    // ----------- (3) check close enough

    bool closeEnough = true;
    if (radius > 0){
        if (length > radius){
            closeEnough = false;
        }
    }

    // ----------- (4) if so, update force
    if (closeEnough == true){
		float pct = 1 - (length / radius);  // stronger on the inside
        diff.normalize();
        frc.x = frc.x - diff.x * scale * pct;
        frc.y = frc.y - diff.y * scale * pct;
    }
}

void Particle::kill(){
    isAlive = false;
}

