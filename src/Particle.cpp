#include "Particle.h"

Particle::Particle(){
    immortal        = false;
    isAlive         = true;
    bounces         = true;
    sizeAge         = true;
    opacityAge      = true;
    colorAge        = true;
    flickersAge     = true;
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
    this->lifetime = lifetime;

    this->mass = initialRadius * initialRadius * 0.005f;
    this->originalHue = color.getHue();
}

void Particle::update(float dt){
    if(isAlive){
        // Perlin noise
        // noise = ofNoise(pos.x*0.005f, pos.y*0.005f, dt*0.1f);
        // float angle = noise*30.0f;
        // acc = ofPoint(cos(angle), sin(angle)) * age * 0.1;

        // Update position
        vel += acc;
        pos += vel*dt;
        vel *= friction;
        acc.set(0, 0);

        // Update age and check if particle has to die
        age += dt;
        if(age >= lifetime){
            if (immortal) age = 0;
            else isAlive = false;
        }

        // Decrease particle radius with age
        if (sizeAge) radius = initialRadius * (1.0f - (age/lifetime));

        // Decrease particle opacity with age
        opacity = 255;
        if (opacityAge){
            opacity *= 1.0f - (age/lifetime);
        }
        if (flickersAge && (age/lifetime) > 0.94f && ofRandomf() > 0.3){
            opacity *= 0.2;
        }

        // Change particle color with age
        if (colorAge){
            float saturation = ofMap(age, 0, lifetime, 255, 128);
            float hue = ofMap(age, 0, lifetime, originalHue, originalHue-100);
            color.setSaturation(saturation);
            color.setHue(hue);
        }

        // Bounce particle with the window margins
        if(bounces){
            if(pos.x > width-radius){
                pos.x = width-radius;
                vel.x *= -1.0;
            }
            if(pos.x < radius){
                pos.x = radius;
                vel.x *= -1.0;
            }
            if(pos.y > height-radius){
                pos.y = height-radius;
                vel.y *= -1.0;
            }
            if(pos.y < radius){
                pos.y = radius;
                vel.y *= -1.0;
            }
        }
    }
}

void Particle::update(float dt, vector<irMarker>& markers){

    update(dt);

    // Find closest marker to particle
    float minDist = 5000;
    markerDist = 0;
    ofPoint markerPos;
    color.set(255);

    for(int i = 0; i < markers.size(); i++){
        if (!markers[i].hasDisappeared){
            markerDist = pos.squareDistance(markers[i].smoothPos);
            if(markerDist < minDist){
                dir = markers[i].smoothPos - pos;
                minDist = markerDist;
                markerPos = markers[i].smoothPos;
                color = markers[i].color;
                dir.normalize();
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

void Particle::applyForce(ofPoint force){
    force /= mass;
    acc += force;
}

void Particle::kill(){
    isAlive = false;
}

