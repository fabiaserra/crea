#include "ofApp.h"

using namespace ofxCv;
using namespace cv;

//--------------------------------------------------------------
void ofApp::setup() {

//    ofEnableBlendMode(OF_BLENDMODE_ADD);

	kinect.init(true); // shows infrared instead of RGB video Image
	kinect.open();

    reScale = (float)ofGetWidth() / (float)kinect.width;
    time0 = ofGetElapsedTimef();

    //ALLOCATE IMAGES//
    depthImage.allocate(kinect.width, kinect.height, OF_IMAGE_GRAYSCALE);
    depthOriginal.allocate(kinect.width, kinect.height, OF_IMAGE_GRAYSCALE);
    grayThreshNear.allocate(kinect.width, kinect.height, OF_IMAGE_GRAYSCALE);
    grayThreshFar.allocate(kinect.width, kinect.height, OF_IMAGE_GRAYSCALE);
    irImage.allocate(kinect.width, kinect.height, OF_IMAGE_GRAYSCALE);
    irOriginal.allocate(kinect.width, kinect.height, OF_IMAGE_GRAYSCALE);

    //BACKGROUND COLOR//
    red = 0; green = 0; blue = 0;

    //KINECT PARAMETERS//
    flipKinect = false;

    nearClipping = 500;
    farClipping = 4000;

    nearThreshold = 230;
    farThreshold = 70;
    minContourSize = 20.0;
    maxContourSize = 4000.0;
    contourFinder.setMinAreaRadius(minContourSize);
    contourFinder.setMaxAreaRadius(maxContourSize);

    irThreshold = 80;
    minMarkerSize = 10.0;
    maxMarkerSize = 1000.0;
    irMarkerFinder.setMinAreaRadius(minMarkerSize);
    irMarkerFinder.setMaxAreaRadius(maxMarkerSize);

    trackerPersistence = 70;
    trackerMaxDistance = 64;
    tracker.setPersistence(trackerPersistence);     // wait for 'trackerPersistence' frames before forgetting something
    tracker.setMaximumDistance(trackerMaxDistance); // an object can move up to 'trackerMaxDistance' pixels per frame

    //MARKER PARTICLES//
    float bornRate = 5;               //Number of particles born per frame

    float velocity = 50;              //Initial velocity magnitude of newborn particles
    float velocityRnd = 20;           //Magnitude randomness % of the initial velocity
    float velocityMotion = 50;        //Marker motion contribution to the initial velocity

    float emitterSize = 8.0f;         //Size of the emitter area
    EmitterType emitterType = POINT;  //Type of emitter

    float lifetime = 3;               //Lifetime of particles
    float lifetimeRnd = 60;           //Randomness of lifetime

    float radius = 5;                 //Radius of the particles
    float radiusRnd = 20;             //Randomness of radius

    bool immortal = false;            //Can particles die?
    bool sizeAge = true;              //Decrease size when particles get older?
    bool opacityAge = true;           //Decrease opacity when particles get older?
    bool colorAge = true;             //Change color when particles get older?
    bool bounce = true;               //Bounce particles with the walls of the window?

    float friction = 0;               //Multiply this value by the velocity every frame
    float gravity = 5.0f;             //Makes particles fall down in a natural way

    markersParticles.setup(bornRate, velocity, velocityRnd, velocityMotion, emitterSize, immortal, lifetime, lifetimeRnd,
                           color, radius, radiusRnd, 1-friction/1000, gravity, sizeAge, opacityAge, colorAge, bounce);

    //GRID PARTICLES//
    bool sizeAge2 = false;
    bool opacityAge2 = false;
    bool colorAge2 = false;
    ofColor color(255);

    particles.setup(true, color, gravity, sizeAge2, opacityAge2, colorAge2, bounce);

    //DEPTH CONTOUR//
    smoothingSize = 0;

    //SETUP GUIs//
	setupGUI0();
	setupGUI1();
	setupGUI2();
    setupGUI3();
}

//--------------------------------------------------------------
void ofApp::update() {

    //Compute dt
	float time = ofGetElapsedTimef();
	float dt = ofClamp(time - time0, 0, 0.1);
	time0 = time;

	kinect.update();
	if(kinect.isFrameNew())
    {
        kinect.setDepthClipping(nearClipping, farClipping);
        depthOriginal.setFromPixels(kinect.getDepthPixels(), kinect.width, kinect.height, OF_IMAGE_GRAYSCALE);
        if(flipKinect) depthOriginal.mirror(false, true);

        copy(depthOriginal, depthImage);
        copy(depthOriginal, grayThreshNear);
        copy(depthOriginal, grayThreshFar);

        irOriginal.setFromPixels(kinect.getPixels(), kinect.width, kinect.height, OF_IMAGE_GRAYSCALE);
        if(flipKinect) irOriginal.mirror(false, true);

        copy(irOriginal, irImage);

        //Filter the IR image
        erode(irImage);
        blur(irImage, 21);
        dilate(irImage);
        threshold(irImage, irThreshold);

        //Treshold and filter depth image
        threshold(grayThreshNear, nearThreshold, true);
        threshold(grayThreshFar, farThreshold);
        bitwise_and(grayThreshNear, grayThreshFar, depthImage);
//        erode(depthImage);
//        blur(depthImage, 21);
//        dilate(depthImage);

        //Update images
		irImage.update();
		depthImage.update();

        //Contour Finder + marker tracker in the IR Image
		irMarkerFinder.findContours(irImage);
		tracker.track(irMarkerFinder.getBoundingRects());

        //Contour Finder in the depth Image
        contourFinder.findContours(depthImage);

        //Track markers
        vector<Marker>& markers = tracker.getFollowers();
        vector<unsigned int> deadLabels = tracker.getDeadLabels();
        vector<unsigned int> newLabels = tracker.getNewLabels();
        vector<unsigned int> currentLabels = tracker.getCurrentLabels();

        //update markers if we loose track of them
        for(unsigned int i = 0; i < markers.size(); i++) {
            markers[i].update(deadLabels, currentLabels);
        }

        //update grid particles
//        particles.update(dt, markers);

        //update markers particles
        markersParticles.update(dt, markers);
//        //Identify IR markers with hungarian algorithm
//        if(irMarkerFinder.size() > 0)
//        {
//            costMatrix = computeCostMatrix(); //stores the cost matrix in the variable costMatrix
//
//            //initialize the hungarian_problem using the cost matrix
//            Hungarian hungarian(costMatrix, nMarkers, irMarkerFinder.size(), HUNGARIAN_MODE_MINIMIZE_COST);
//
//            fprintf(stderr, "cost-matrix:");
//            hungarian.print_cost();
//
//            //solve the assignement problem
//            hungarian.solve();
//
//            fprintf(stderr, "assignment:");
//            hungarian.print_assignment();
//
//            //Assignment matrix of the non identified markers with the existing markers
//            vector< vector<int> > assignment = hungarian.assignment();
//
//            //Update each of the n markers positions with the assigned IDs
//            for (int i = 0; i < nMarkers; i++)
//            {
//                //markers[i].update(dt);
//                for(int j = 0; j < irMarkerFinder.size(); j++)
//                {
//                    if(assignment[i][j] == 1)
//                    {
//                        markers[i].updatePosition(toOf(irMarkerFinder.getCentroid(j)));
//                        break;
//                    }
//                }
//            }
//        }
//
//        //Kalman Filter
//        if(irMarkerFinder.size() > 0)
//        {
//            ofVec2f curPoint(toOf(irMarkerFinder.getCentroid(0)));
//            line.addVertex(curPoint);
//
//            kalman.update(curPoint);
//
//            point = kalman.getPrediction(); // prediction before measurement
//            predicted.addVertex(point);
//
//            estimated.addVertex(kalman.getEstimation()); // corrected estimation after measurement
//
//            speed = kalman.getVelocity().length();
//            int alpha = ofMap(speed, 0, 20, 50, 255, true);
//            line.addColor(ofColor(255, 255, 255, alpha));
//            predicted.addColor(ofColor(255, 0, 0, alpha));
//            estimated.addColor(ofColor(0, 255, 0, alpha));
//        }
*/
	}
}

//--------------------------------------------------------------
void ofApp::draw() {

    ofPushMatrix();

//	ofScale(reScale, reScale);
    ofBackground(red, green, blue, 255);
//    ofEnableBlendMode(OF_BLENDMODE_ALPHA);

    ofSetColor(255);
    irImage.draw(0, 0);
//    depthImage.draw(0, 0);

//    contourFinder.draw();
    irMarkerFinder.draw();

//    particles.draw();
    markersParticles.draw();

//    //Draw contour shape
//    for(int i = 0; i < contourFinder.size(); i++) {
//        ofFill();
//        ofSetColor(255);
//
////        ofRect(toOf(contourFinder.getBoundingRect(i)));
//
//		ofPolyline convexHull = toOf(contourFinder.getConvexHull(i));
//		convexHull = convexHull.getSmoothed(smoothingSize, 0.5);
//
//
//		ofSetColor(255);
//		ofBeginShape();
//            for(int i = 0; i < convexHull.getVertices().size(); i++)
//                ofVertex(convexHull.getVertices().at(i).x, convexHull.getVertices().at(i).y);
//		ofEndShape();
//		ofSetColor(255, 0, 0);
//		ofSetLineWidth(3);
//        convexHull.draw(); //if we only want the contour
//
//        ofSetColor(0);
//        ofPolyline contour = contourFinder.getPolyline(i);
//        contour = contour.getSmoothed(5, 0.5);
//        contour.draw();
//	}

    //Draw identified IR markers
//    for (int i = 0; i < markers.size(); i++)
//    {
//        markers[i].draw();
//    }

    ofPopMatrix();
}

//--------------------------------------------------------------
void ofApp::setupGUI0()
{
    gui0 = new ofxUISuperCanvas("MAIN WINDOW", false);
    gui0->addSpacer();
    gui0->addLabel("Press panel number to open", OFX_UI_FONT_SMALL);
    gui0->addSpacer();
    gui0->addLabel("Press 'h' to hide GUI", OFX_UI_FONT_SMALL);
    gui0->addSpacer();
    gui0->addLabel("Press 'f' to fullscreen", OFX_UI_FONT_SMALL);

    gui0->addSpacer();
    gui0->addLabel("1: BASICS");
    gui0->addSpacer();

    gui0->addSpacer();
	gui0->addLabel("2: KINECT");
    gui0->addSpacer();

    gui0->addSpacer();
	gui0->addLabel("3: PARTICLES");
	gui0->addSpacer();

    gui0->autoSizeToFitWidgets();
    gui0->setPosition(795, 6);
    ofAddListener(gui0->newGUIEvent,this,&ofApp::guiEvent);
    gui0->loadSettings("gui0Settings.xml");
}

//--------------------------------------------------------------
void ofApp::setupGUI1()
{
    gui1 = new ofxUISuperCanvas("1: BASICS");
    gui1->addSpacer();
    gui1->addLabel("Press '1' to hide panel", OFX_UI_FONT_SMALL);

    gui1->addSpacer();
    gui1->addFPS(OFX_UI_FONT_SMALL);

    gui1->addSpacer();
	gui1->addLabel("BACKGROUND");
	gui1->addSlider("Red", 0.0, 255.0, &red);
	gui1->addSlider("Green", 0.0, 255.0, &green);
	gui1->addSlider("Blue", 0.0, 255.0, &blue);

    gui1->autoSizeToFitWidgets();
    gui1->setPosition(795, 6);
    gui1->setVisible(false);
    ofAddListener(gui1->newGUIEvent,this,&ofApp::guiEvent);
    gui1->loadSettings("gui1Settings.xml");
}

//--------------------------------------------------------------
void ofApp::setupGUI2()
{
    gui2 = new ofxUISuperCanvas("2: KINECT");
    gui2->setVisible(false);
    gui2->addSpacer();
    gui2->addLabel("Press '2' to hide panel", OFX_UI_FONT_SMALL);

    gui2->addSpacer();
    gui2->addToggle("Flip Kinect", &flipKinect);

    gui2->addSpacer();
    gui2->addLabel("DEPTH IMAGE");
    gui2->addRangeSlider("Clipping range", 500, 5000, &nearClipping, &farClipping);
    gui2->addRangeSlider("Threshold range", 0.0, 255.0, &farThreshold, &nearThreshold);
    gui2->addRangeSlider("Contour Size", 0.0, (kinect.width * kinect.height)/4, &minContourSize, &maxContourSize);
    gui2->addImage("Depth original", &depthOriginal, kinect.width/6, kinect.height/6, true);
    gui2->setWidgetPosition(OFX_UI_WIDGET_POSITION_RIGHT);
    gui2->addImage("Depth filtered", &depthImage, kinect.width/6, kinect.height/6, true);
    gui2->setWidgetPosition(OFX_UI_WIDGET_POSITION_DOWN);


    gui2->addSpacer();
    gui2->addLabel("INFRARED IMAGE");
    gui2->addSlider("IR Threshold", 0.0, 255.0, &irThreshold);
    gui2->addRangeSlider("Markers size", 0.0, 350, &minMarkerSize, &maxMarkerSize);
    gui2->addSlider("Tracker persistence", 0.0, 500.0, &trackerPersistence);
    gui2->addSlider("Tracker max distance", 5.0, 400.0, &trackerMaxDistance);
    gui2->addImage("IR original", &irOriginal, kinect.width/6, kinect.height/6, true);
    gui2->setWidgetPosition(OFX_UI_WIDGET_POSITION_RIGHT);
    gui2->addImage("IR filtered", &irImage, kinect.width/6, kinect.height/6, true);
    gui2->setWidgetPosition(OFX_UI_WIDGET_POSITION_DOWN);
    gui2->addSpacer();

    gui2->autoSizeToFitWidgets();
    gui2->setPosition(795, 6);
    gui2->setVisible(false);
    ofAddListener(gui2->newGUIEvent,this,&ofApp::guiEvent);
    gui2->loadSettings("gui2Settings.xml");
}

//--------------------------------------------------------------
void ofApp::setupGUI3()
{
    gui3 = new ofxUISuperCanvas("3: PARTICLES");
    gui3->addSpacer();
    gui3->addLabel("Press '3' to hide panel", OFX_UI_FONT_SMALL);

    gui3->addSpacer();
    gui3->addLabel("Emitter");
    gui3->addSlider("Particles/sec", 0.0, 20.0, &markersParticles.bornRate);

//    vector<string> types;
//	types.push_back("Point");
//	types.push_back("Grid");
//	types.push_back("Contour");
//    gui3->addLabel("Emitter type:", OFX_UI_FONT_SMALL);
//    gui3->addRadio("Emitter type", types, OFX_UI_ORIENTATION_VERTICAL);

    gui3->addSlider("Velocity", 0.0, 100.0, &markersParticles.velocity);
    gui3->addSlider("Velocity Random[%]", 0.0, 100.0, &markersParticles.velocityRnd);
    gui3->addSlider("Velocity from Motion[%]", 0.0, 100.0, &markersParticles.velocityMotion);

    gui3->addSlider("Emitter size", 0.0, 50.0, &markersParticles.emitterSize);

    gui3->addSpacer();
    gui3->addLabel("Particle");
    gui3->addToggle("Immortal", &markersParticles.immortal);
    gui3->addSlider("Lifetime", 0.0, 20.0, &markersParticles.lifetime);
    gui3->addSlider("Life Random[%]", 0.0, 100.0, &markersParticles.lifetimeRnd);
    gui3->addSlider("Radius", 1.0, 15.0, &markersParticles.radius);
    gui3->addSlider("Radius Random[%]", 0.0, 100.0, &markersParticles.radiusRnd);

    gui3->addSpacer();
    gui3->addLabel("Time behaviour");
    gui3->addToggle("Size", &markersParticles.sizeAge);
    gui3->addToggle("Opacity", &markersParticles.opacityAge);
    gui3->addToggle("Color", &markersParticles.colorAge);
    gui3->addToggle("Bounce", &markersParticles.bounce);

    gui3->addSpacer();
    gui3->addLabel("Physics");
    gui3->addSlider("Friction", 0, 100, &markersParticles.friction);
    gui3->addSlider("Gravity", 0.0, 20.0, &markersParticles.gravity);

    gui3->addSpacer();

//    gui3->addToggle("OPEN KINECT", bKinectOpen);
    gui3->setGlobalCanvasWidth(OFX_UI_GLOBAL_CANVAS_WIDTH);

    gui3->autoSizeToFitWidgets();
    gui3->setPosition(795, 6);
    gui3->setVisible(false);
    ofAddListener(gui3->newGUIEvent,this,&ofApp::guiEvent);
    gui3->loadSettings("gui3Settings.xml");
}

void ofApp::guiEvent(ofxUIEventArgs &e)
{
    if(e.getName() == "Size range")
    {
        contourFinder.setMinAreaRadius(minContourSize);
        contourFinder.setMaxAreaRadius(maxContourSize);
    }

    if(e.getName() == "Markers size")
    {
        irMarkerFinder.setMinAreaRadius(minMarkerSize);
        irMarkerFinder.setMaxAreaRadius(maxMarkerSize);
    }

    if(e.getName() == "Tracker persistence")
    {
        tracker.setPersistence(trackerPersistence); // wait for 'trackerPersistence' frames before forgetting something
    }

    if(e.getName() == "Tracker max distance")
    {
        tracker.setMaximumDistance(trackerMaxDistance); // an object can move up to 'trackerMaxDistance' pixels per frame
    }

//    if(e.getName() == "Emitter type")
//    {
//        ofxUIRadio *radio = (ofxUIRadio *) e.widget;
//        cout << radio->getName() << " value: " << radio->getValue() << " active name: " << radio->getActiveName() << endl;
//    }

    if(e.getName() == "Immortal")
    {
        ofxUIToggle *toggle = (ofxUIToggle *) e.widget;
        if (toggle->getValue() == false) markersParticles.killParticles();
    }

}

//--------------------------------------------------------------
void ofApp::exit()
{
	kinect.close();

    gui0->saveSettings("gui0Settings.xml");
    gui1->saveSettings("gui1Settings.xml");
    gui2->saveSettings("gui2Settings.xml");
    gui3->saveSettings("gui3Settings.xml");

    delete gui0;
    delete gui1;
    delete gui2;
    delete gui3;
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key) {
    switch (key)
    {
        case 'h':
            gui0->toggleVisible();
            gui1->toggleVisible();
            gui2->toggleVisible();
            gui3->toggleVisible();
            break;

        case 'f':
            ofToggleFullscreen();
            reScale = (float)ofGetWidth() / (float)kinect.width;
            break;

        case '0':
            gui0->toggleVisible();
            gui1->setVisible(false);
            gui2->setVisible(false);
            gui3->setVisible(false);
            break;

        case '1':
            gui1->toggleVisible();
            gui0->setVisible(false);
            gui2->setVisible(false);
            gui3->setVisible(false);
            break;

        case '2':
            gui2->toggleVisible();
            gui0->setVisible(false);
            gui1->setVisible(false);
            gui3->setVisible(false);
            break;

        case '3':
            gui3->toggleVisible();
            gui0->setVisible(false);
            gui1->setVisible(false);
            gui2->setVisible(false);
            break;

        default:
            break;
    }
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button) {

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){

}
