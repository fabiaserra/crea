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

    //Allocate images
    depthImage.allocate(kinect.width, kinect.height, OF_IMAGE_GRAYSCALE);
    depthOriginal.allocate(kinect.width, kinect.height, OF_IMAGE_GRAYSCALE);
    grayThreshNear.allocate(kinect.width, kinect.height, OF_IMAGE_GRAYSCALE);
    grayThreshFar.allocate(kinect.width, kinect.height, OF_IMAGE_GRAYSCALE);
    irImage.allocate(kinect.width, kinect.height, OF_IMAGE_GRAYSCALE);
    irOriginal.allocate(kinect.width, kinect.height, OF_IMAGE_GRAYSCALE);

    red = 0; green = 0; blue = 0;

    //Kinect parameters
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

    trackerPersistence = 240;
    trackerMaxDistance = 64;
    tracker.setPersistence(trackerPersistence);     // wait for 'trackerPersistence' frames before forgetting something
    tracker.setMaximumDistance(trackerMaxDistance); // an object can move up to 'trackerMaxDistance' pixels per frame

    //Particles parameters
    lifetime = 2;
    lifetimeRnd = 50;
    gravity = 5.0;
    emitterType = POINT;

    //Graphics parameters
    smoothingSize = 0;

//    //Init IR markers
//    nMarkers = 2;
//    for(int i = 1; i <= nMarkers; i++)
//    {
//        Marker newMarker;
//        newMarker.setup(i, ofPoint());
//        markers.push_back(newMarker);
//    }

//    //Kalman filter
//    kalman.init(1e-4, 1e-1); // invert of (smoothness, rapidness)
//    line.setMode(OF_PRIMITIVE_LINE_STRIP);
//    predicted.setMode(OF_PRIMITIVE_LINE_STRIP);
//    estimated.setMode(OF_PRIMITIVE_LINE_STRIP);
//    speed = 0.f;

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

//        for(int i = 0; i < deadLabels.size(); i++) {
//            particleSystems.erase(deadLabels[i]);
//        }

//        cout << "dead: ";
        for(int i = 0; i < deadLabels.size(); i++) {
//            cout << deadLabels[i] << ", ";
            particleSystems[deadLabels[i]].killParticles();
        }
//        cout << endl;

//        cout << "current: ";
        for(int i = 0; i < currentLabels.size(); i++) {
//            cout << currentLabels[i] << ", ";
            particleSystems[currentLabels[i]].bornParticles();
        }
//        cout << endl;

//        cout << "new: ";
        for(int i = 0; i < newLabels.size(); i++) {
//            cout << newLabels[i] << ", ";
            ParticleSystem newParticleSystem;
            newParticleSystem.setup(BORN_PARTICLES);
            particleSystems[newLabels[i]] = newParticleSystem;
        }
//        cout << endl;

//        cout << "markers: ";
        for(int i = 0; i < markers.size(); i++) {
            unsigned int label = markers[i].getLabel();
//            cout << label << ", ";
            particleSystems[label].update(dt, markers[i].smoothPos, markers[i].velocity);
        }
//        cout << endl;


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
	}
}

//--------------------------------------------------------------
void ofApp::draw() {

    ofPushMatrix();

//	ofScale(reScale, reScale);
    ofBackground(red, green, blue, 255);
    ofEnableBlendMode(OF_BLENDMODE_ALPHA);

    ofSetColor(255);
    irImage.draw(0, 0);
//    depthImage.draw(0, 0);

//    contourFinder.draw();
    irMarkerFinder.draw();

    vector<Marker>& markers = tracker.getFollowers();
    for(int i = 0; i < markers.size(); i++) {
        unsigned int label = markers[i].getLabel();
        particleSystems[label].draw();
    }

//    //Draw contour shape
//    for(int i = 0; i < contourFinder.size(); i++) {
//        ofFill();
//        ofSetColor(255);
//
////        ofRect(toOf(contourFinder.getBoundingRect(i)));
//
//		ofPolyline convexHull = toOf(contourFinder.getConvexHull(i));
//		convexHull = convexHull.getSmoothed(smoothingSize, 0.5);
////		convexHull.draw(); //if we only want the contour
//		ofBeginShape();
//            for(int i = 0; i < convexHull.getVertices().size(); i++)
//                ofVertex(convexHull.getVertices().at(i).x, convexHull.getVertices().at(i).y);
//		ofEndShape();
//
////        ofPolyline contour = contourFinder.getPolyline(i);
////        contour = contour.getSmoothed(5, 0.5);
////        contour.draw();
//	}

//    //Draw identified IR markers
//    for (int i = 0; i < markers.size(); i++)
//    {
//        markers[i].draw();
//    }

    ofPopMatrix();
}

//--------------------------------------------------------------
//vector< vector<int> > ofApp::computeCostMatrix()
//{
//    vector< vector<int> > C;
//    C.resize(nMarkers, vector<int>(irMarkerFinder.size(), 0));
//
//    for(int i = 0; i < nMarkers; i++)
//    {
//        for(int j = 0; j < irMarkerFinder.size(); j++)
//        {
//            C[i][j] = markers[i].distance(toOf(irMarkerFinder.getCentroid(j)));
//        }
//    }
//
//    return C;
//}

//--------------------------------------------------------------
void ofApp::setupGUI1()
{
    gui1 = new ofxUISuperCanvas("1: PANEL 1: BASICS");
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
    ofAddListener(gui1->newGUIEvent,this,&ofApp::guiEvent);
    gui1->loadSettings("gui1Settings.xml");
}

//--------------------------------------------------------------
void ofApp::setupGUI2()
{
    gui2 = new ofxUISuperCanvas("2: KINECT");
    gui2->addSpacer();
    gui2->addLabel("Press '2' to hide panel", OFX_UI_FONT_SMALL);

    gui2->addSpacer();
    gui2->addToggle("Flip Kinect", &flipKinect);

    gui2->addSpacer();
    gui2->addLabel("DEPTH IMAGE");
    gui2->addRangeSlider("Clipping range", 500, 5000, &nearClipping, &farClipping);
    gui2->addRangeSlider("Threshold range", 0.0, 255.0, &farThreshold, &nearThreshold);
    gui2->addRangeSlider("Contour Size", 0.0, (kinect.width * kinect.height)/4, minContourSize, maxContourSize);
    gui2->addImage("Depth original", &depthOriginal, kinect.width/6, kinect.height/6, true);
    gui2->setWidgetPosition(OFX_UI_WIDGET_POSITION_RIGHT);
    gui2->addImage("Depth filtered", &depthImage, kinect.width/6, kinect.height/6, true);
    gui2->setWidgetPosition(OFX_UI_WIDGET_POSITION_DOWN);


    gui2->addSpacer();
    gui2->addLabel("INFRARED IMAGE");
    gui2->addSlider("IR Threshold", 0.0, 255.0, &irThreshold);
    gui2->addRangeSlider("Markers size", 0.0, 350, minMarkerSize, maxMarkerSize);
    gui2->addSlider("Tracker persistence", 0.0, 500.0, trackerPersistence);
    gui2->addSlider("Tracker max distance", 5.0, 400.0, trackerMaxDistance);
    gui2->addImage("IR original", &irOriginal, kinect.width/6, kinect.height/6, true);
    gui2->setWidgetPosition(OFX_UI_WIDGET_POSITION_RIGHT);
    gui2->addImage("IR filtered", &irImage, kinect.width/6, kinect.height/6, true);
    gui2->setWidgetPosition(OFX_UI_WIDGET_POSITION_DOWN);
    gui2->addSpacer();

    gui2->autoSizeToFitWidgets();
    ofAddListener(gui2->newGUIEvent,this,&ofApp::guiEvent);
    gui2->loadSettings("gui1Settings.xml");
}

//--------------------------------------------------------------
void ofApp::setupGUI3()
{
    gui3 = new ofxUISuperCanvas("3: PARTICLES");
    gui3->addSpacer();
    gui3->addLabel("Press '3' to hide panel", OFX_UI_FONT_SMALL);

    gui3->addSpacer();
    gui3->addLabel("Emitter");
    gui3->addSlider("Particles/sec", 0.0, 255.0, &bornRate);

    vector<string> types;
	types.push_back("Point");
	types.push_back("Sphere");
	types.push_back("Box");
	types.push_back("Grid");
	types.push_back("Contour");

//    ddl = gui3->addDropDownList("DROP DOWN LIST", types);
//    ddl->setAllowMultiple(false);

    gui3->addRadio("Emitter type", types, OFX_UI_ORIENTATION_HORIZONTAL);
    gui3->addSlider("Emitter Size", 0.0, 50.0, &emitterSize);

    gui3->addSpacer();
    gui3->addLabel("Particle");
    gui3->addSlider("Lifetime", 0.0, 150.0, &lifetime);
    gui3->addSlider("Life Random[%]", 0.0, 100.0, &lifetimeRnd);

    gui3->addSpacer();
    gui3->addLabel("Physics");
    gui3->addSlider("Gravity", 0.0, 20.0, &gravity);

    gui3->addSpacer();

//    gui3->addToggle("OPEN KINECT", bKinectOpen);
    gui3->setGlobalCanvasWidth(OFX_UI_GLOBAL_CANVAS_WIDTH);

    gui3->autoSizeToFitWidgets();
    ofAddListener(gui3->newGUIEvent,this,&ofApp::guiEvent);
    gui3->loadSettings("gui2Settings.xml");
}

void ofApp::guiEvent(ofxUIEventArgs &e)
{
    if(e.getName() == "Size range")
    {
        ofxUIRangeSlider *slider = (ofxUIRangeSlider *) e.widget;
        minContourSize = slider->getScaledValueLow() ;
        maxContourSize = slider->getScaledValueHigh() ;
        contourFinder.setMinAreaRadius(minContourSize);
        contourFinder.setMaxAreaRadius(maxContourSize);
    }

    if(e.getName() == "Markers size")
    {
        ofxUIRangeSlider *slider = (ofxUIRangeSlider *) e.widget;
        minMarkerSize = slider->getScaledValueLow() ;
        maxMarkerSize = slider->getScaledValueHigh() ;
        irMarkerFinder.setMinAreaRadius(minMarkerSize);
        irMarkerFinder.setMaxAreaRadius(maxMarkerSize);
    }

    if(e.getName() == "Tracker persistence")
    {
        ofxUISlider *slider = (ofxUISlider *) e.widget;
        trackerPersistence = slider->getScaledValue() ;
        tracker.setPersistence(trackerPersistence); // wait for 'trackerPersistence' frames before forgetting something
    }

    if(e.getName() == "Tracker max distance")
    {
        ofxUISlider *slider = (ofxUISlider *) e.widget;
        trackerMaxDistance = slider->getScaledValue() ;
        tracker.setMaximumDistance(trackerMaxDistance); // an object can move up to 'trackerMaxDistance' pixels per frame
    }

    if(e.getName() == "Emitter type")
    {
        ofxUIRadio *radio = (ofxUIRadio *) e.widget;
        cout << radio->getName() << " value: " << radio->getValue() << " active name: " << radio->getActiveName() << endl;
    }

}

//--------------------------------------------------------------
void ofApp::exit()
{
	kinect.close();

    gui1->saveSettings("gui1Settings.xml");
    gui2->saveSettings("gui2Settings.xml");
    gui3->saveSettings("gui3Settings.xml");

    delete gui1;
    delete gui2;
    delete gui3;
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key) {
    switch (key)
    {
        case 'h':
            gui1->toggleVisible();
            gui2->toggleVisible();
            gui3->toggleVisible();
            break;

        case '1':
            gui1->toggleVisible();
            break;

        case '2':
            gui2->toggleVisible();
            break;

        case '3':
            gui3->toggleVisible();
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
