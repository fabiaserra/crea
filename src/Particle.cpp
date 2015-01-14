#include "Particle.h"

Particle::Particle() {
    isAlive = true;
    bounces = true;
    age = 0;
}

void Particle::setup(float id, ofPoint pos, ofPoint vel, ofColor color, float initialRadius, float lifetime, float friction) {
    this->id = id;
    this->pos = pos;
    this->vel = vel;
    this->color = color;
    this->initialRadius = initialRadius;
    this->lifetime = lifetime;
    this->friction = friction;
    this->mass = initialRadius * initialRadius * 0.005f;

    //alter color for each particle
    //    color.setHsb((ofGetFrameNum() % 255), 255, 255);
    originalHue = color.getHue();
    float hue = originalHue - ofRandom(-20, 20);
    color.setHue(hue);
}

void Particle::setup(float id, ofPolyline contour, ofPoint vel, ofColor color, float initialRadius, float lifetime, float friction) {
    this->id = id;
    this->contour = contour;
    this->vel = vel;
    this->color = color;
    this->initialRadius = initialRadius;
    this->lifetime = lifetime;
    this->friction = friction;
    this->mass = initialRadius * initialRadius * 0.005f;

    //create particles only inside contour polyline
    ofRectangle box = contour.getBoundingBox();
	ofPoint center = box.getCenter();
	pos.x = center.x + (ofRandom(1.0f) - 0.5f) * box.getWidth();
	pos.y = center.y + (ofRandom(1.0f) - 0.5f) * box.getHeight();

	while(!contour.inside(pos))
	{
		pos.x = center.x + (ofRandom(1.0f) - 0.5f) * box.getWidth();
		pos.y = center.y + (ofRandom(1.0f) - 0.5f) * box.getHeight();
	}
}

void Particle::update(float dt, const ofPoint &markerPos) {
    if(isAlive)
    {
        //Perlin noise
        noise = ofNoise(pos.x*0.005f, pos.y*0.005f, dt*0.1f);

        //Update position
        float angle = noise*30.0f;
        acc = ofPoint(cos(angle), sin(angle)) * age * 0.1;
        vel += acc;
//        acc.set(0, 0);
        pos += vel*dt;
        vel *= friction;

        //Direction and distance to particular point
//        dir = markerPos - pos;
//        float dist = dir.length();
//        dir.normalize();

        //Update age and check if particle has to die
        age += dt;
        if(age >= lifetime) {
            isAlive = false;
        }

        //Decrease particle radius with the age
        radius = initialRadius * (1.0f - (age/lifetime));

        //Bounce with the window margins
        if(bounces)
        {
            if(pos.x > ofGetWidth()-radius) {
            pos.x = ofGetWidth()-radius;
            vel.x *= -1.0;
            }
            if(pos.x < radius){
                pos.x = radius;
                vel.x *= -1.0;
            }
            if(pos.y > ofGetHeight()-radius) {
                pos.y = ofGetHeight()-radius;
                vel.y *= -1.0;
            }
            if(pos.y < radius) {
                pos.y = radius;
                vel.y *= -1.0;
            }
        }
    }
}

void Particle::draw() {
    if(isAlive)
    {
        float saturation = ofMap(age, 0, lifetime, 255, 0);
        float hue = ofMap(age, 0, lifetime, originalHue, originalHue-100);
        color.setSaturation(saturation);
        color.setHue(hue);
        ofSetColor(color);
        ofCircle(pos, radius);
//        float length = 15.0f;
//        ofPoint p1(pos);
//        ofPoint p2(pos + dir*length);
//        ofLine(p1, p2);
    }
}

void Particle::kill() {
    isAlive = false;
}

