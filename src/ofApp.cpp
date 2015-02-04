#include "ofApp.h"

using namespace ofxCv;
using namespace cv;

//--------------------------------------------------------------
void ofApp::setup(){

//    ofEnableBlendMode(OF_BLENDMODE_ADD);

    // OPEN KINECT
	kinect.init(true); // shows infrared instead of RGB video Image
	kinect.open();

	reScale = (float)ofGetWidth() / (float)kinect.width;
	time0 = ofGetElapsedTimef();

	// ALLOCATE IMAGES
	depthImage.allocate(kinect.width, kinect.height, OF_IMAGE_GRAYSCALE);
	depthOriginal.allocate(kinect.width, kinect.height, OF_IMAGE_GRAYSCALE);
	grayThreshNear.allocate(kinect.width, kinect.height, OF_IMAGE_GRAYSCALE);
	grayThreshFar.allocate(kinect.width, kinect.height, OF_IMAGE_GRAYSCALE);
	irImage.allocate(kinect.width, kinect.height, OF_IMAGE_GRAYSCALE);
	irOriginal.allocate(kinect.width, kinect.height, OF_IMAGE_GRAYSCALE);

	// BACKGROUND COLOR
	red = 0; green = 0; blue = 0;

	// KINECT PARAMETERS
	flipKinect      = false;

	nearClipping    = 500;
	farClipping     = 4000;

	nearThreshold   = 230;
	farThreshold    = 70;
	minContourSize  = 20.0;
	maxContourSize  = 4000.0;
	contourFinder.setMinAreaRadius(minContourSize);
	contourFinder.setMaxAreaRadius(maxContourSize);

	irThreshold     = 80;
	minMarkerSize   = 10.0;
	maxMarkerSize   = 1000.0;
	irMarkerFinder.setMinAreaRadius(minMarkerSize);
	irMarkerFinder.setMaxAreaRadius(maxMarkerSize);

	trackerPersistence = 70;
	trackerMaxDistance = 64;
	tracker.setPersistence(trackerPersistence);     // wait for 'trackerPersistence' frames before forgetting something
	tracker.setMaximumDistance(trackerMaxDistance); // an object can move up to 'trackerMaxDistance' pixels per frame

	// MARKER PARTICLES
	float bornRate       = 5;        // Number of particles born per frame
	float velocity       = 50;       // Initial velocity magnitude of newborn particles
	float velocityRnd    = 20;       // Magnitude randomness % of the initial velocity
	float velocityMotion = 50;       // Marker motion contribution to the initial velocity
	float emitterSize    = 8.0f;     // Size of the emitter area
	EmitterType type     = POINT;    // Type of emitter
	float lifetime       = 3;        // Lifetime of particles
	float lifetimeRnd    = 60;       // Randomness of lifetime
	float radius         = 5;        // Radius of the particles
	float radiusRnd      = 20;       // Randomness of radius

	bool  immortal       = false;    // Can particles die?
	bool  sizeAge        = true;     // Decrease size when particles get older?
	bool  opacityAge     = true;     // Decrease opacity when particles get older?
	bool  flickersAge    = true;     // Particle flickers opacity when about to die?
	bool  colorAge       = true;     // Change color when particles get older?
	bool  bounce         = true;     // Bounce particles with the walls of the window?

	float friction       = 0;        // Multiply this value by the velocity every frame
	float gravity        = 5.0f;     // Makes particles fall down in a natural way

	ofColor color(255);

	markersParticles.setup(bornRate, velocity, velocityRnd, velocityMotion, emitterSize, immortal, lifetime, lifetimeRnd,
						   color, radius, radiusRnd, 1-friction/1000, gravity, sizeAge, opacityAge, flickersAge, colorAge, bounce);

	// GRID PARTICLES
	bool sizeAge2     = false;
	bool opacityAge2  = false;
	bool flickersAge2 = false;
	bool colorAge2    = false;

	particles.setup(true, color, gravity, sizeAge2, opacityAge2, flickersAge2, colorAge2, bounce);

	// DEPTH CONTOUR
	smoothingSize = 0;

	// SETUP GUIs
    dim = 32;
    guiWidth = 240;

	setupGUI0();
	setupGUI1();
	setupGUI2();
    setupGUI3(0);
	setupGUI3();
	setupGUI4();
	setupGUI5();
	setupGUI6(0);

	//VMO Setup goes here//
	//1. Load xml files...
	obs = loadXML();
	initStatus = true;
	int minLen = 5; // Temporary setting
	float start = 0.0, step = 0.01, stop = 2.0;
	//2. Processing
	//2.1 Load file into VMO
	float t = vmo::findThreshold(obs, 4, start, step, stop); // Temporary threshold range and step
	seqVmo = vmo::buildOracle(obs, t);
	//2.2 Output pattern list
	pttrList = vmo::findPttr(seqVmo, minLen);
	patterns = vmo::processPttr(seqVmo, pttrList);
}

//--------------------------------------------------------------
void ofApp::update(){

	// Compute dt
	float time = ofGetElapsedTimef();
	float dt = ofClamp(time - time0, 0, 0.1);
	time0 = time;

	kinect.update();
	if(kinect.isFrameNew()){
		kinect.setDepthClipping(nearClipping, farClipping);
		depthOriginal.setFromPixels(kinect.getDepthPixels(), kinect.width, kinect.height, OF_IMAGE_GRAYSCALE);
		if(flipKinect) depthOriginal.mirror(false, true);

		copy(depthOriginal, depthImage);
		copy(depthOriginal, grayThreshNear);
		copy(depthOriginal, grayThreshFar);

		irOriginal.setFromPixels(kinect.getPixels(), kinect.width, kinect.height, OF_IMAGE_GRAYSCALE);
		if(flipKinect) irOriginal.mirror(false, true);

		copy(irOriginal, irImage);

		// Filter the IR image
		erode(irImage);
		blur(irImage, 21);
		dilate(irImage);
		threshold(irImage, irThreshold);

		// Treshold and filter depth image
		threshold(grayThreshNear, nearThreshold, true);
		threshold(grayThreshFar, farThreshold);
		bitwise_and(grayThreshNear, grayThreshFar, depthImage);
		// erode(depthImage);
		// blur(depthImage, 21);
		// dilate(depthImage);

		// Update images
		irImage.update();
		depthImage.update();

		// Contour Finder + marker tracker in the IR Image
		irMarkerFinder.findContours(irImage);
		tracker.track(irMarkerFinder.getBoundingRects());

		// Contour Finder in the depth Image
		contourFinder.findContours(depthImage);

		// Track markers
		vector<Marker>& markers             = tracker.getFollowers();
		vector<unsigned int> deadLabels     = tracker.getDeadLabels();
		vector<unsigned int> currentLabels  = tracker.getCurrentLabels();
		// vector<unsigned int> newLabels      = tracker.getNewLabels();

		// Update markers if we loose track of them
		for(unsigned int i = 0; i < markers.size(); i++){
			markers[i].update(deadLabels, currentLabels);
		}

		// Update grid particles
		particles.update(dt, markers);

		// Update markers particles
		// markersParticles.update(dt, markers);

		//Gesture Tracking with VMO here?

        //update markers particles
//        markersParticles.update(dt, markers);
		
		//Gesture Tracking with VMO here?
		vector<float> firstObs; // Temporary code
		if(initStatus){
			currentBf = vmo::tracking_init(pttrList, seqVmo, firstObs);
			initStatus = false;
		}else{
			vector<float> obs;
			prevBf = currentBf;
			currentBf = vmo::tracking(pttrList, seqVmo, prevBf, obs);
		}
		
	}
}

//--------------------------------------------------------------
void ofApp::draw(){

	ofPushMatrix();
    ofTranslate(guiWidth+10, 0);
//	ofScale(reScale, reScale);
	ofBackground(red, green, blue, 255);
	// ofEnableBlendMode(OF_BLENDMODE_ALPHA);

//	ofSetColor(255);
//	irImage.draw(0, 0);
//	depthImage.draw(0, 0);

	// contourFinder.draw();
	// irMarkerFinder.draw();

//	particles.draw();
//	markersParticles.draw();

	// // Draw contour shape
	// for(int i = 0; i < contourFinder.size(); i++){
	// 	ofFill();
	// 	ofSetColor(255);

	// 	ofRect(toOf(contourFinder.getBoundingRect(i)));

	// 	ofPolyline convexHull = toOf(contourFinder.getConvexHull(i));
	// 	convexHull = convexHull.getSmoothed(smoothingSize, 0.5);


	// 	ofSetColor(255);
	// 	ofBeginShape();
	// 		for(int i = 0; i < convexHull.getVertices().size(); i++){
	// 			ofVertex(convexHull.getVertices().at(i).x, convexHull.getVertices().at(i).y);
	// 		}
	// 	ofEndShape();

	// 	ofSetColor(255, 0, 0);
	// 	ofSetLineWidth(3);
	// 	convexHull.draw(); //if we only want the contour

	// 	ofSetColor(0);
	// 	ofPolyline contour = contourFinder.getPolyline(i);
	// 	contour = contour.getSmoothed(5, 0.5);
	// 	contour.draw();
 	// }

	// // Draw identified IR markers
	// for (int i = 0; i < markers.size(); i++){
	// 	markers[i].draw();
	// }

	ofPopMatrix();
}

//--------------------------------------------------------------
void ofApp::setupGUI0(){
	gui0 = new ofxUISuperCanvas("MAIN WINDOW", 0, 0, guiWidth, ofGetHeight());
    gui0->setTheme(OFX_UI_THEME_GRAYDAY);

	gui0->addSpacer();
	gui0->addLabel("Press panel number to switch", OFX_UI_FONT_SMALL);
	gui0->addLabel("between menus.", OFX_UI_FONT_SMALL);
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
	gui0->addLabel("3: GESTURE SEQUENCE");
	gui0->addSpacer();

	gui0->addSpacer();
	gui0->addLabel("4: GESTURE TRACKER");
	gui0->addSpacer();

    gui0->addSpacer();
	gui0->addLabel("5: FLUID SOLVER");
	gui0->addSpacer();

	gui0->addSpacer();
	gui0->addLabel("6: PARTICLES");
	gui0->addSpacer();

    gui0->addSpacer();
    gui0->addImageButton("Save", "GUI/icons/save.png", false, dim, dim);
    gui0->setWidgetPosition(OFX_UI_WIDGET_POSITION_RIGHT);
	gui0->addImageButton("Load", "GUI/icons/open.png", false, dim, dim);
	gui0->addImageButton("Reset", "GUI/icons/reset.png", false, dim, dim);
	gui0->setWidgetPosition(OFX_UI_WIDGET_POSITION_DOWN);

    gui0->addSpacer();

	gui0->autoSizeToFitWidgets();
	ofAddListener(gui0->newGUIEvent, this, &ofApp::guiEvent);
}

//--------------------------------------------------------------
void ofApp::setupGUI1(){
	gui1 = new ofxUISuperCanvas("1: BASICS", 0, 0, guiWidth, ofGetHeight());
	gui1->setTheme(OFX_UI_THEME_GRAYDAY);

	gui1->addSpacer();
	gui1->addLabel("Press '1' to hide panel", OFX_UI_FONT_SMALL);

	gui1->addSpacer();
	gui1->addFPS(OFX_UI_FONT_SMALL);

	gui1->addSpacer();
	gui1->addLabel("BACKGROUND");
	gui1->addSlider("Red", 0.0, 255.0, &red);
	gui1->addSlider("Green", 0.0, 255.0, &green);
	gui1->addSlider("Blue", 0.0, 255.0, &blue);

    gui1->addSpacer();

	gui1->autoSizeToFitWidgets();
	gui1->setVisible(false);
	ofAddListener(gui1->newGUIEvent, this, &ofApp::guiEvent);
	gui1->loadSettings("GUI/gui1Settings.xml");
}

//--------------------------------------------------------------
void ofApp::setupGUI2(){
	gui2 = new ofxUISuperCanvas("2: KINECT", 0, 0, guiWidth, ofGetHeight());
	gui2->setTheme(OFX_UI_THEME_GRAYDAY);

	gui2->addSpacer();
	gui2->addLabel("Press '2' to hide panel", OFX_UI_FONT_SMALL);

    gui2->addLabelButton("Reset Kinect", &resetKinect);

	gui2->addSpacer();
	gui2->addLabelToggle("Flip Kinect", &flipKinect);

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
	gui2->setVisible(false);
	ofAddListener(gui2->newGUIEvent, this, &ofApp::guiEvent);
	gui2->loadSettings("GUI/gui2Settings.xml");
}

//--------------------------------------------------------------
void ofApp::setupGUI3(){
	gui3 = new ofxUISuperCanvas("3: GESTURE SEQUENCE", 0, 0, guiWidth, ofGetHeight());
    gui3->setTheme(OFX_UI_THEME_GRAYDAY);

	gui3->addSpacer();
	gui3->addLabel("Press '3' to hide panel", OFX_UI_FONT_SMALL);

	gui3->addSpacer();
	gui3->addImageButton("Record", "GUI/icons/record.png", false, 32, 32);
	gui3->setWidgetPosition(OFX_UI_WIDGET_POSITION_RIGHT);
//	gui3->addImageButton("Stop", "GUI/icons/record.png", false, 32, 32);
	gui3->addImageButton("Load", "GUI/icons/open.png", false, 32, 32);
	gui3->addImageButton("Save", "GUI/icons/save.png", false, 32, 32);
	gui3->addImageButton("Delete", "GUI/icons/delete.png", false, 32, 32);
	gui3->setWidgetPosition(OFX_UI_WIDGET_POSITION_DOWN);

    gui3->addSpacer();

	gui3->autoSizeToFitWidgets();
	gui3->setVisible(false);
	ofAddListener(gui3->newGUIEvent, this, &ofApp::guiEvent);
	gui3->loadSettings("GUI/gui3Settings.xml");
}

//--------------------------------------------------------------
void ofApp::setupGUI4(){
	gui4 = new ofxUISuperCanvas("4: GESTURE TRACKER", 0, 0, guiWidth, ofGetHeight());
	gui4->setTheme(OFX_UI_THEME_GRAYDAY);

	gui4->addSpacer();
	gui4->addLabel("Press '4' to hide panel", OFX_UI_FONT_SMALL);

	gui4->addSpacer();
	gui4->addLabelButton("Start", false);
	gui4->addLabelButton("Stop", false);

    gui4->addSpacer();

	gui4->autoSizeToFitWidgets();
	gui4->setVisible(false);
	ofAddListener(gui4->newGUIEvent, this, &ofApp::guiEvent);
	gui4->loadSettings("GUI/gui4Settings.xml");
}

//--------------------------------------------------------------
void ofApp::setupGUI5(){
	gui5 = new ofxUISuperCanvas("5: FLUID SOLVER", 0, 0, guiWidth, ofGetHeight());
    gui5->setTheme(OFX_UI_THEME_GRAYDAY);

	gui5->addSpacer();
	gui5->addLabel("Press '5' to hide panel", OFX_UI_FONT_SMALL);

	gui5->addSpacer();
	gui5->addLabel("Physics");
	gui5->addSlider("Friction", 0, 100, &markersParticles.friction);

	gui5->addSpacer();

	gui5->autoSizeToFitWidgets();
	gui5->setVisible(false);
	ofAddListener(gui5->newGUIEvent, this, &ofApp::guiEvent);
	gui5->loadSettings("GUI/gui5Settings.xml");
}

//--------------------------------------------------------------
void ofApp::setupGUI6(int i){
	gui6 = new ofxUISuperCanvas("6: PARTICLES", 0, 0, guiWidth, ofGetHeight());
	gui6->setTheme(OFX_UI_THEME_GRAYDAY);
//	gui6->setTheme(OFX_UI_THEME_MINBLACK);

	gui6->addSpacer();
	gui6->addLabel("Press '6' to hide panel", OFX_UI_FONT_SMALL);

	gui6->addSpacer();
	gui6->addLabel("Emitter");
	gui6->addSlider("Particles/sec", 0.0, 20.0, &markersParticles.bornRate);

//    vector<string> types;
//    types.push_back("Point");
//    types.push_back("Grid");
//    types.push_back("Contour");
//    gui6->addLabel("Emitter type:", OFX_UI_FONT_SMALL);
//    gui6->addRadio("Emitter type", types, OFX_UI_ORIENTATION_VERTICAL);

	gui6->addSlider("Velocity", 0.0, 100.0, &markersParticles.velocity);
	gui6->addSlider("Velocity Random[%]", 0.0, 100.0, &markersParticles.velocityRnd);
	gui6->addSlider("Velocity from Motion[%]", 0.0, 100.0, &markersParticles.velocityMotion);

	gui6->addSlider("Emitter size", 0.0, 60.0, &markersParticles.emitterSize);

	gui6->addSpacer();
	gui6->addLabel("Particle");
	gui6->addLabelToggle("Immortal", &markersParticles.immortal);
	gui6->addSlider("Lifetime", 0.0, 20.0, &markersParticles.lifetime);
	gui6->addSlider("Life Random[%]", 0.0, 100.0, &markersParticles.lifetimeRnd);
	gui6->addSlider("Radius", 1.0, 15.0, &markersParticles.radius);
	gui6->addSlider("Radius Random[%]", 0.0, 100.0, &markersParticles.radiusRnd);

	gui6->addSpacer();
	gui6->addLabel("Time behaviour");
	gui6->addLabelToggle("Size", &markersParticles.sizeAge);
	gui6->addLabelToggle("Opacity", &markersParticles.opacityAge);
	gui6->addLabelToggle("Flickers", &markersParticles.flickersAge);
	gui6->addLabelToggle("Color", &markersParticles.colorAge);
	gui6->addLabelToggle("Bounce", &markersParticles.bounce);

	gui6->addSpacer();
	gui6->addLabel("Physics");
	gui6->addSlider("Friction", 0, 100, &markersParticles.friction);
	gui6->addSlider("Gravity", 0.0, 20.0, &markersParticles.gravity);

	gui6->addSpacer();

	gui6->autoSizeToFitWidgets();
	gui6->setVisible(false);
	ofAddListener(gui6->newGUIEvent, this, &ofApp::guiEvent);
	gui6->loadSettings("GUI/gui6Settings.xml");
}

void ofApp::guiEvent(ofxUIEventArgs &e){
	if(e.getName() == "Reset Kinect"){
        if(resetKinect){
            kinect.close();
        }
        else{
            kinect.open();
        }
	}

	if(e.getName() == "Size range"){
		contourFinder.setMinAreaRadius(minContourSize);
		contourFinder.setMaxAreaRadius(maxContourSize);
	}

	if(e.getName() == "Markers size"){
		irMarkerFinder.setMinAreaRadius(minMarkerSize);
		irMarkerFinder.setMaxAreaRadius(maxMarkerSize);
	}

	if(e.getName() == "Tracker persistence"){
		tracker.setPersistence(trackerPersistence); // wait for 'trackerPersistence' frames before forgetting something
	}

	if(e.getName() == "Tracker max distance"){
		tracker.setMaximumDistance(trackerMaxDistance); // an object can move up to 'trackerMaxDistance' pixels per frame
	}

	// if(e.getName() == "Emitter type"){
	// 	ofxUIRadio *radio = (ofxUIRadio *) e.widget;
	// 	cout << radio->getName() << " value: " << radio->getValue() << " active name: " << radio->getActiveName() << endl;
	// }

	if(e.getName() == "Immortal"){
		ofxUIToggle *toggle = (ofxUIToggle *) e.widget;
		if (toggle->getValue() == false) markersParticles.killParticles();
	}

}

//--------------------------------------------------------------
void ofApp::exit(){
	kinect.close();

	gui0->saveSettings("GUI/gui0Settings.xml");
	gui1->saveSettings("GUI/gui1Settings.xml");
	gui2->saveSettings("GUI/gui2Settings.xml");
	gui3->saveSettings("GUI/gui3Settings.xml");
	gui4->saveSettings("GUI/gui4Settings.xml");
	gui5->saveSettings("GUI/gui5Settings.xml");
	gui6->saveSettings("GUI/gui6Settings.xml");

	delete gui0;
	delete gui1;
	delete gui2;
	delete gui3;
	delete gui4;
	delete gui5;
	delete gui6;
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
	switch (key){
		case 'h':
			gui0->toggleVisible();
			gui1->toggleVisible();
			gui2->toggleVisible();
			gui3->toggleVisible();
			gui4->toggleVisible();
			gui5->toggleVisible();
			gui6->toggleVisible();
			break;

		case 'f':
			ofToggleFullscreen();
			reScale = (float)ofGetWidth() / (float)kinect.width;
			break;

		case '0':
        case '`':
			gui0->toggleVisible();
			gui1->setVisible(false);
			gui2->setVisible(false);
			gui3->setVisible(false);
			gui4->setVisible(false);
			gui5->setVisible(false);
			gui6->setVisible(false);
			break;

		case '1':
			gui0->setVisible(false);
			gui1->toggleVisible();
			gui2->setVisible(false);
			gui3->setVisible(false);
			gui4->setVisible(false);
			gui5->setVisible(false);
			gui6->setVisible(false);
			break;

		case '2':
			gui0->setVisible(false);
			gui1->setVisible(false);
			gui2->toggleVisible();
			gui3->setVisible(false);
			gui4->setVisible(false);
			gui5->setVisible(false);
			gui6->setVisible(false);
			break;

		case '3':
			gui0->setVisible(false);
			gui1->setVisible(false);
			gui2->setVisible(false);
			gui3->toggleVisible();
			gui4->setVisible(false);
			gui5->setVisible(false);
			gui6->setVisible(false);
			break;

		case '4':
			gui0->setVisible(false);
			gui1->setVisible(false);
			gui2->setVisible(false);
			gui3->setVisible(false);
			gui4->toggleVisible();
			gui5->setVisible(false);
			gui6->setVisible(false);
			break;

		case '5':
			gui0->setVisible(false);
			gui1->setVisible(false);
			gui2->setVisible(false);
			gui3->setVisible(false);
			gui4->setVisible(false);
			gui5->toggleVisible();
			gui6->setVisible(false);
			break;

		case '6':
			gui0->setVisible(false);
			gui1->setVisible(false);
			gui2->setVisible(false);
			gui3->setVisible(false);
			gui4->setVisible(false);
			gui5->setVisible(false);
			gui6->toggleVisible();
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
void ofApp::mousePressed(int x, int y, int button){

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
