#include "ofApp.h"

using namespace ofxCv;
using namespace cv;

//--------------------------------------------------------------
void ofApp::setup(){

//    ofSetFrameRate(30);

    int maxMarkers = 2;
    // Using a live kinect?
    #ifdef KINECT_CONNECTED
        // OPEN KINECT
        kinect.init(true); // shows infrared instead of RGB video Image
        kinect.open();

    // Kinect not connected
    #else
        // Use xml sequence marker file
        #ifdef KINECT_SEQUENCE
            kinectSequence.setup(maxMarkers);
            kinectSequence.load("sequences/sequence2.xml");
        #endif // KINECT_SEQUENCE

        // Load png files from file
        ofDirectory dir;                    // directory lister
        dir.allowExt("jpg");

        string depthFolder = "depth1/";
        int totalImages = dir.listDir(depthFolder);
        dir.sort();
        savedDepthImages.resize(totalImages);

        // Load all recorded depth images in "data/depth01/"
        for(int i = 0; i < totalImages; i++){
            ofImage *img = new ofImage();
            img->loadImage(depthFolder + dir.getName(i));
            img->setImageType(OF_IMAGE_GRAYSCALE);
            savedDepthImages[i] = img;
        }

        string irFolder = "ir1/";
        totalImages = dir.listDir(irFolder);
        dir.sort();
        savedIrImages.resize(totalImages);

        // Load all recorded IR images in "data/ir01/"
        for(int i = 0; i < totalImages; i++){
            ofImage *img = new ofImage();
            img->loadImage(irFolder + dir.getName(i));
            img->setImageType(OF_IMAGE_GRAYSCALE);
            savedIrImages[i] = img;
        }

    #endif

    reScale = (float)ofGetHeight() / (float)kinect.height;
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

    nearThreshold   = 255;
    farThreshold    = 165;
    minContourSize  = 20.0;
    maxContourSize  = 4000.0;
    contourFinder.setMinAreaRadius(minContourSize);
    contourFinder.setMaxAreaRadius(maxContourSize);

    irThreshold     = 80;
    minMarkerSize   = 10.0;
    maxMarkerSize   = 1000.0;
    irMarkerFinder.setMinAreaRadius(minMarkerSize);
    irMarkerFinder.setMaxAreaRadius(maxMarkerSize);

    trackerPersistence = 150;
    trackerMaxDistance = 300;
    tracker.setPersistence(trackerPersistence);     // wait for 'trackerPersistence' frames before forgetting something
    tracker.setMaximumDistance(trackerMaxDistance); // an object can move up to 'trackerMaxDistance' pixels per frame

    // MARKER PARTICLES
    markerParticles = new ParticleSystem();
    markerParticles->setup(EMITTER, MARKERS, kinect.width, kinect.height);

    // CONTOUR PARTICLES
    contourParticles = new ParticleSystem();
    contourParticles->setup(EMITTER, CONTOUR, kinect.width, kinect.height);

    // GRID PARTICLES
//    gridParticles = new ParticleSystem();
//    gridParticles->radius = 2;
//    gridParticles->bounce = true;
//    gridParticles->setup(GRID, MARKERS, kinect.width, kinect.height);

    // BOIDS PARTICLES
    gridParticles = new ParticleSystem();
    gridParticles->bounce = false;
    gridParticles->setup(BOIDS, MARKERS, kinect.width, kinect.height);

    // VECTOR OF PARTICLE SYSTEMS
    particleSystems.push_back(markerParticles);
    particleSystems.push_back(contourParticles);
    particleSystems.push_back(gridParticles);
    currentParticleSystem = 2;

    // DEPTH CONTOUR
    // smoothingSize = 0;
    contour.setup();

    // SEQUENCE
    sequence.setup(maxMarkers);
//    sequence.load("sequences/sequence1marker2.xml");
    sequence.load("sequences/sequence2.xml");
    drawSequence = false;

    // MARKERS
    markers.resize(maxMarkers);
    drawMarkers = false;

    // VMO SETUP
    int dimensions = 2;
    obs.assign(sequence.numFrames, vector<float>(maxMarkers*dimensions));
    for(int markerIndex = 0; markerIndex < maxMarkers; markerIndex++){
        for(int frameIndex = 0; frameIndex < sequence.numFrames; frameIndex++){
            obs[frameIndex][markerIndex*dimensions] = sequence.markersPosition[markerIndex][frameIndex].x;
            obs[frameIndex][markerIndex*dimensions+1] = sequence.markersPosition[markerIndex][frameIndex].y;
        }
    }

    initStatus = true;
    stopTracking = true;
    // 2. Processing
    // 2.1 Load file into VMO
//    int minLen = 1; // Temporary setting
//    float start = 0.0, step = 0.05, stop = 10.0;

	float start = 10.0, step = 0.01, stop = 20.0;

//    float t = vmo::findThreshold(obs, dimensions, maxMarkers, start, step, stop); // Temporary threshold range and step
//	int minLen = 2; // sequence.xml
//	float t = 12.3; // for sequence.xml

	int minLen = 7; // sequence3.xml
 	float t = 18.6; // for sequence2.xml
//	float t = 16.8; // for sequence3.xml
//
//	int minLen = 7;
//	float t = 4.5; // for sequence1marker1.xml
//	int minLen = 10;
//	float t = 5.7; // for sequence1marker2.xml
//	int minLen = 10;
//	float t = 6.0; // for sequence1marker3.xml

	cout << t << endl;
	seqVmo = vmo::buildOracle(obs, dimensions, maxMarkers, t);
    // 2.2 Output pattern list
    pttrList = vmo::findPttr(seqVmo, minLen);
    sequence.loadPatterns(vmo::processPttr(seqVmo, pttrList));
    drawPatterns = false;
    cout << sequence.patterns.size() << endl;

	currentBf = vmo::vmo::belief();
	prevBf = vmo::vmo::belief();

//    cout << "pattern size: "<<sequence.patterns.size() << endl;
//	for (int i = 0; i < pttrList.size; i++) {
//		cout << "pattern "<< i+1 << endl;
//		for (int j = 0; j<pttrList.sfxPts[i].size(); j++){
//			cout << "	begin: "<< pttrList.sfxPts[i][j] - pttrList.sfxLen[i]<< endl;
//			cout << "	end  :"<< pttrList.sfxPts[i][j] << endl;
//		}
//	}

    // SETUP GUIs
    dim = 32;
    guiWidth = 240;

    // GUI COLORS
//    uiThemecb.set(128, 210), uiThemeco.set(192, 255), uiThemecoh.set(192, 255);
//    uiThemecf.set(255, 255); uiThemecfh.set(160, 255), uiThemecp.set(128, 192);
//    uiThemecpo.set(255, 192);

    uiThemecb.set(64, 192), uiThemeco.set(192, 192), uiThemecoh.set(128, 192);
    uiThemecf.set(240, 255); uiThemecfh.set(128, 255), uiThemecp.set(96, 192);
    uiThemecpo.set(255, 192);

//    uiThemecb.set(41, 34, 31, 192), uiThemeco.set(19, 116, 125, 192), uiThemecoh.set(41, 34, 31, 192);
//    uiThemecf.set(252, 53, 76, 255); uiThemecfh.set(252, 247, 197, 255), uiThemecp.set(10, 191, 188, 192);
//    uiThemecpo.set(19, 116, 125, 192);

    setupGUI0();
    setupGUI1();
    setupGUI2();
    setupGUI3();
    setupGUI4();
    setupGUI5();
    setupGUI6();
    setupGUI7();
    setupGUI8Marker();
    setupGUI8Contour();
    setupGUI8Grid();
//    setupgui8(MARKER_PARTICLES);

    interpolatingWidgets = false;
    loadGUISettings("settings/lastSettings.xml", false, false);


    // CREATE DIRECTORIES IN /DATA IF THEY DONT EXIST
    string directory[3] = {"sequences", "settings", "cues"};
    for(int i = 0; i < 3; i++){
        if(!ofDirectory::doesDirectoryExist(directory[i])){ // relative to /data folder
            ofDirectory::createDirectory(directory[i]);
        }
    }
}

//--------------------------------------------------------------
void ofApp::update(){

    // Compute dt
    float time = ofGetElapsedTimef();
    float dt = ofClamp(time - time0, 0, 0.1);
    time0 = time;

    // Compute rescale value to scale kinect image
    reScale = (float)ofGetHeight() / (float)kinect.height;

    // Interpolate GUI widget values
    if(interpolatingWidgets) interpolateWidgetValues();

    // Update sequence playhead to draw gesture
    if(drawSequence) sequence.update();

    // Load a saved image for playback
    #ifndef KINECT_CONNECTED

        #ifdef KINECT_SEQUENCE
            kinectSequence.update();
        #endif // KINECT_SEQUENCE

        // Get the size of the image sequence
        int n = savedDepthImages.size();

        // Calculate sequence duration assuming 30 fps
        float duration = n / 30.0;

        // Calculate playing percent in sequence
        float percent = time / duration;

        // Convert percent in the frame number
        if(percent < 0.0 || percent > 1.0) percent -= floor(percent);
        int i = MIN((int)(percent*n), n-1);

        ofImage *depthImg = savedDepthImages.at(i);
        depthOriginal.setFromPixels(depthImg->getPixels(), depthImg->getWidth(), depthImg->getHeight(), OF_IMAGE_GRAYSCALE);

        ofImage *irImg = savedIrImages.at(i);
        irOriginal.setFromPixels(irImg->getPixels(), irImg->getWidth(), irImg->getHeight(), OF_IMAGE_GRAYSCALE);

    #endif // KINECT_CONNECTED

    // Nothing will happen here if the kinect is unplugged
    kinect.update();
    if(kinect.isFrameNew()){
        kinect.setDepthClipping(nearClipping, farClipping);
        depthOriginal.setFromPixels(kinect.getDepthPixels(), kinect.width, kinect.height, OF_IMAGE_GRAYSCALE);
        if(flipKinect) depthOriginal.mirror(false, true);

        irOriginal.setFromPixels(kinect.getPixels(), kinect.width, kinect.height, OF_IMAGE_GRAYSCALE);
        if(flipKinect) irOriginal.mirror(false, true);
    }

    copy(irOriginal, irImage);

    copy(depthOriginal, depthImage);
    copy(depthOriginal, grayThreshNear);
    copy(depthOriginal, grayThreshFar);

    // Filter the IR image
    erode(irImage);
    blur(irImage, 21);
    dilate(irImage);
    threshold(irImage, irThreshold);

    // Treshold and filter depth image
    threshold(grayThreshNear, nearThreshold, true);
    threshold(grayThreshFar, farThreshold);
    bitwise_and(grayThreshNear, grayThreshFar, depthImage);

    // Update images
    irImage.update();
    depthImage.update();

    // Contour Finder + marker tracker in the IR Image
    irMarkerFinder.findContours(irImage);
    tracker.track(irMarkerFinder.getBoundingRects());

    // Contour Finder in the depth Image
    contourFinder.findContours(depthImage);

    // Track markers
    vector<irMarker>& tempMarkers       = tracker.getFollowers();   // TODO: assign dead labels to new labels and have a MAX number of markers
    vector<unsigned int> deadLabels     = tracker.getDeadLabels();
    vector<unsigned int> currentLabels  = tracker.getCurrentLabels();
    // vector<unsigned int> newLabels      = tracker.getNewLabels();

    // Update markers if we loose track of them
    for(unsigned int i = 0; i < tempMarkers.size(); i++){
        tempMarkers[i].updateLabels(deadLabels, currentLabels);
    }

    // Record sequence when recording button is true
    if(recordingSequence->getValue() == true) sequence.record(tempMarkers);

    // Update contour
    contour.update(contourFinder);

    // Update grid particles
    gridParticles->update(dt, tempMarkers, contour);

    // Update markers particles
    markerParticles->update(dt, tempMarkers, contour);

    // Update contour particles
    contourParticles->update(dt, tempMarkers, contour);

    #ifdef KINECT_SEQUENCE

        if (!stopTracking){
            vector<float> obs; // Temporary code
            for(unsigned int i = 0; i < kinectSequence.maxMarkers; i++){
                ofPoint currentPoint = kinectSequence.getCurrentPoint(i);
                obs.push_back(currentPoint.x);
                obs.push_back(currentPoint.y);
            }
            if(initStatus){
                currentBf = vmo::tracking_init(seqVmo, currentBf, pttrList, obs);
                initStatus = false;
            }
            else{
                prevBf = currentBf;
                currentBf = vmo::tracking(seqVmo, pttrList, prevBf, obs);
                cout << "current index: " << currentBf.currentIdx << endl;

                float currentPercent = ofMap(currentBf.currentIdx, 0, sequence.numFrames, 0, 100, true);
                cout << "current percent: " << currentPercent << endl;
                if(cues.size() != 0) {
                    int cueSegment = currentCueIndex;
                    for(int i = 0; i < cueSliders.size(); i++){
                        float low = cueSliders.at(i).second->getValueLow();
                        float high = cueSliders.at(i).second->getValueHigh();
                        cout << "Low: " << low << endl;
                        cout << "High: " << high << endl;
                        if (low <= currentPercent && currentPercent <= high){
                            cueSegment = i;
                            break;
                        }
                    }
                    // If tracking idx from sequence belongs to another cue different than the current
                    // we interpolate settings to this other cue
                    if(currentCueIndex != cueSegment){
                        currentCueIndex = cueSegment;
                        loadGUISettings(cues[currentCueIndex], true, true);
                        string cueFileName = ofFilePath::getBaseName(cues[currentCueIndex]);
                        cueIndexLabel->setLabel(ofToString(currentCueIndex)+".");
                        cueName->setTextString(cueFileName);
                    }
                }
            }
            gestureUpdate = seqVmo.getGestureUpdate(currentBf.currentIdx, pttrList);
            for (int i = 0; i < sequence.patterns.size(); i++) {
//                if(gestureUpdate.find(i) != gestureUpdate.end()) {
//                    cout << "key: "<< i << endl;
//                    cout << "percent:"<< gestureUpdate[i] << endl;
//                }
            }
        }

    #else

        // Gesture Tracking with VMO here?
        if (tempMarkers.size()>1){
            if (!stopTracking){
                vector<float> obs; // Temporary code
                for(unsigned int i = 0; i < sequence.maxMarkers; i++){
                    obs.push_back(tempMarkers[i].smoothPos.x);
                    obs.push_back(tempMarkers[i].smoothPos.y);
                }
                if(initStatus){
                    currentBf = vmo::tracking_init(seqVmo, currentBf, pttrList, obs);
                    initStatus = false;
                }
                else{
                    prevBf = currentBf;
                    currentBf = vmo::tracking(seqVmo, pttrList, prevBf, obs);
                    cout << "current index: " << currentBf.currentIdx << endl;
                    // We need the min/max of currentIdx
                    float currentPercent = ofMap(currentBf.currentIdx, 0, sequence.numFrames, 0, 100, true);
                    cout << "current percent: " << currentPercent << endl;
                    if(cues.size() != 0) {
                        int cueSegment = currentCueIndex;
                        for(int i = 0; i < cueSliders.size(); i++){
                            float low = cueSliders.at(currentCueIndex).second->getValueHigh();
                            float high = cueSliders.at(currentCueIndex).second->getValueHigh();
                            if (low <= currentPercent && currentPercent <= high){
                                cueSegment = i;
                                break;
                            }
                        }
                        // If tracking idx from sequence belongs to another cue different than the current
                        // we interpolate settings to this other cue
                        if(currentCueIndex != cueSegment){
                            currentCueIndex = cueSegment;
                            loadGUISettings(cues[currentCueIndex], true, true);
                            string cueFileName = ofFilePath::getBaseName(cues[currentCueIndex]);
                            cueIndexLabel->setLabel(ofToString(currentCueIndex)+".");
                            cueName->setTextString(cueFileName);
                        }
                    }
                }
                gestureUpdate = seqVmo.getGestureUpdate(currentBf.currentIdx, pttrList);
                for (int i = 0; i < sequence.patterns.size(); i++) {
//                    if(gestureUpdate.find(i) != gestureUpdate.end()) {
//                        cout << "key: "<< i << endl;
//                        cout << "percent:"<< gestureUpdate[i] << endl;
//                    }
                }
            }
        }

    #endif // KINECT_SEQUENCE
}

//--------------------------------------------------------------
void ofApp::draw(){

    ofPushMatrix();
//    ofSetRectMode(OF_RECTMODE_CENTER);
//    ofTranslate(ofGetWidth()/2, ofGetHeight()/2);  // Translate to the center of the screen
    ofScale(reScale, reScale);
    ofBackground(red, green, blue);
//    depthOriginal.draw(0,0); // Pre-recorded depth image
//    ofEnableBlendMode(OF_BLENDMODE_ALPHA);
//    ofEnableBlendMode(OF_BLENDMODE_ADD);

    ofSetColor(255);

//    // Kinect images
//    irImage.draw(0, 0);
//    depthImage.draw(0, 0);

    #ifdef KINECT_SEQUENCE
        kinectSequence.draw();
    #endif // KINECT_SEQUENCE

    // Graphics
    contour.draw();
    gridParticles->draw();
    markerParticles->draw();
    contourParticles->draw();

    if(drawMarkers){
        irMarkerFinder.draw();
        vector<irMarker>& tempMarkers = tracker.getFollowers();
        // Draw identified IR markers
        for (int i = 0; i < tempMarkers.size(); i++){
            tempMarkers[i].draw();
        }
    }

    if(drawSequence) sequence.draw();

    #ifdef KINECT_SEQUENCE
        sequence.drawPatternsInSequence(gestureUpdate);
    #endif // KINECT_SEQUENCE

    ofPopMatrix();

    if(drawPatterns) sequence.drawPatterns(gestureUpdate);
}

//--------------------------------------------------------------
void ofApp::setupGUI0(){

    gui0 = new ofxUISuperCanvas("0: MAIN WINDOW", 0, 0, guiWidth, ofGetHeight());
    gui0->setUIColors(uiThemecb, uiThemeco, uiThemecoh, uiThemecf, uiThemecfh, uiThemecp, uiThemecpo);

    gui0->addSpacer();
    gui0->addLabel("Press panel number 0 to 7 to", OFX_UI_FONT_SMALL);
    gui0->addLabel("switch between panels and hide", OFX_UI_FONT_SMALL);
    gui0->addLabel("them.", OFX_UI_FONT_SMALL);
    gui0->addSpacer();
    gui0->addLabel("Press 'h' to hide GUIs.", OFX_UI_FONT_SMALL);
    gui0->addLabel("Press 'f' to fullscreen.", OFX_UI_FONT_SMALL);
    gui0->addSpacer();

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
    gui0->addLabel("5: CUE LIST");
    gui0->addSpacer();

    gui0->addSpacer();
    gui0->addLabel("6: FLUID SOLVER");
    gui0->addSpacer();

    gui0->addSpacer();
    gui0->addLabel("7: DEPTH CONTOUR");
    gui0->addSpacer();

    gui0->addSpacer();
    gui0->addLabel("8: PARTICLES");
    gui0->addSpacer();

    gui0->autoSizeToFitWidgets();
    ofAddListener(gui0->newGUIEvent, this, &ofApp::guiEvent);
    guis.push_back(gui0);
}

//--------------------------------------------------------------
void ofApp::setupGUI1(){
    gui1 = new ofxUISuperCanvas("1: BASICS", 0, 0, guiWidth, ofGetHeight());
    gui1->setUIColors(uiThemecb, uiThemeco, uiThemecoh, uiThemecf, uiThemecfh, uiThemecp, uiThemecpo);

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
    gui1->addLabel("SETTINGS");
    gui1->addImageButton("Save Settings", "icons/save.png", false, dim, dim)->setColorBack(ofColor(150, 255));
    gui1->setWidgetPosition(OFX_UI_WIDGET_POSITION_RIGHT);
    gui1->addImageButton("Load Settings", "icons/open.png", false, dim, dim)->setColorBack(ofColor(150, 255));
    gui1->setWidgetPosition(OFX_UI_WIDGET_POSITION_DOWN);

    gui1->addSpacer();

    gui1->autoSizeToFitWidgets();
    gui1->setVisible(false);
    ofAddListener(gui1->newGUIEvent, this, &ofApp::guiEvent);
    guis.push_back(gui1);
}

//--------------------------------------------------------------
void ofApp::setupGUI2(){
    gui2 = new ofxUISuperCanvas("2: KINECT", 0, 0, guiWidth, ofGetHeight());
    gui2->setUIColors(uiThemecb, uiThemeco, uiThemecoh, uiThemecf, uiThemecfh, uiThemecp, uiThemecpo);

    gui2->addSpacer();
    gui2->addLabel("Press '2' to hide panel", OFX_UI_FONT_SMALL);

    gui2->addSpacer();
    gui2->addFPS(OFX_UI_FONT_SMALL);

    gui2->addSpacer();
    gui2->addLabelButton("Reset Kinect", &resetKinect);

    gui2->addSpacer();
    gui2->addLabelToggle("Flip Kinect", &flipKinect);

    gui2->addSpacer();
    gui2->addLabel("DEPTH IMAGE");
    gui2->addRangeSlider("Clipping range", 500, 5000, &nearClipping, &farClipping);
    gui2->addRangeSlider("Threshold range", 0.0, 255.0, &farThreshold, &nearThreshold);
    gui2->addRangeSlider("Contour Size", 0.0, (kinect.width * kinect.height)/8, &minContourSize, &maxContourSize);
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
    gui2->addToggle("Show Markers", &drawMarkers);

    gui2->addSpacer();

    gui2->autoSizeToFitWidgets();
    gui2->setVisible(false);
    ofAddListener(gui2->newGUIEvent, this, &ofApp::guiEvent);
    guis.push_back(gui2);
}

//--------------------------------------------------------------
void ofApp::setupGUI3(){
    gui3 = new ofxUISuperCanvas("3: GESTURE SEQUENCE", 0, 0, guiWidth, ofGetHeight());
    gui3->setUIColors(uiThemecb, uiThemeco, uiThemecoh, uiThemecf, uiThemecfh, uiThemecp, uiThemecpo);

    gui3->addSpacer();
    gui3->addLabel("Press '3' to hide panel", OFX_UI_FONT_SMALL);

    gui3->addSpacer();
    gui3->addFPS(OFX_UI_FONT_SMALL);

    gui3->addSpacer();
    recordingSequence = gui3->addImageToggle("Record Sequence", "icons/record.png", false, dim, dim);
    recordingSequence->setColorBack(ofColor(150, 255));
    gui3->setWidgetPosition(OFX_UI_WIDGET_POSITION_RIGHT);
    gui3->addImageButton("Save Sequence", "icons/save.png", false, dim, dim)->setColorBack(ofColor(150, 255));
    gui3->addImageButton("Load Sequence", "icons/open.png", false, dim, dim)->setColorBack(ofColor(150, 255));
    gui3->addImageToggle("Play Sequence", "icons/play.png", &drawSequence, dim, dim)->setColorBack(ofColor(150, 255));
    gui3->setWidgetPosition(OFX_UI_WIDGET_POSITION_DOWN);

    gui3->addSpacer();
    sequenceFilename = gui3->addLabel("Filename: "+sequence.filename, OFX_UI_FONT_SMALL);
    sequenceDuration = gui3->addLabel("Duration: "+ofToString(sequence.duration, 2) + " s", OFX_UI_FONT_SMALL);
    sequenceNumFrames = gui3->addLabel("Number of frames: "+ofToString(sequence.numFrames), OFX_UI_FONT_SMALL);

    gui3->addSpacer();

    gui3->autoSizeToFitWidgets();
    gui3->setVisible(false);
    ofAddListener(gui3->newGUIEvent, this, &ofApp::guiEvent);
    guis.push_back(gui3);
}

//--------------------------------------------------------------
void ofApp::setupGUI4(){
    gui4 = new ofxUISuperCanvas("4: GESTURE TRACKER", 0, 0, guiWidth, ofGetHeight());
    gui4->setUIColors(uiThemecb, uiThemeco, uiThemecoh, uiThemecf, uiThemecfh, uiThemecp, uiThemecpo);

    gui4->addSpacer();
    gui4->addLabel("Press '4' to hide panel", OFX_UI_FONT_SMALL);

    gui4->addSpacer();
    gui4->addFPS(OFX_UI_FONT_SMALL);

    gui4->addSpacer();
    gui4->addImageButton("Start vmo", "icons/play.png", false, dim, dim)->setColorBack(ofColor(150, 255));
    gui4->setWidgetPosition(OFX_UI_WIDGET_POSITION_RIGHT);
    gui4->addImageButton("Stop vmo", "icons/delete.png", false, dim, dim)->setColorBack(ofColor(150, 255));
    gui4->addImageToggle("Show gesture patterns", "icons/show.png", &drawPatterns, dim, dim)->setColorBack(ofColor(150, 255));
    gui4->setWidgetPosition(OFX_UI_WIDGET_POSITION_DOWN);

    gui4->addSpacer();

    gui4->autoSizeToFitWidgets();
    gui4->setVisible(false);
    ofAddListener(gui4->newGUIEvent, this, &ofApp::guiEvent);
    guis.push_back(gui4);
}

//--------------------------------------------------------------
void ofApp::setupGUI5(){
    gui5 = new ofxUISuperCanvas("5: CUE LIST", 0, 0, guiWidth, ofGetHeight());
    gui5->setUIColors(uiThemecb, uiThemeco, uiThemecoh, uiThemecf, uiThemecfh, uiThemecp, uiThemecpo);

    gui5->addSpacer();
    gui5->addLabel("Press '5' to hide panel", OFX_UI_FONT_SMALL);
    gui5->addSpacer();
    gui5->addLabel("Press arrow keys to navigate", OFX_UI_FONT_SMALL);
    gui5->addLabel("through cues and space key", OFX_UI_FONT_SMALL);
    gui5->addLabel("to trigger next cue in the list.", OFX_UI_FONT_SMALL);
    gui5->addSpacer();

    gui5->addSpacer();
    gui5->addFPS(OFX_UI_FONT_SMALL);

    gui5->addSpacer();
    gui5->setWidgetPosition(OFX_UI_WIDGET_POSITION_DOWN, OFX_UI_ALIGN_CENTER);
    currentCueIndex = -1;
    cueIndexLabel = gui5->addLabel("", OFX_UI_FONT_MEDIUM);
    gui5->setWidgetFontSize(OFX_UI_FONT_MEDIUM);
    cueName = gui5->addTextInput("Cue Name", "");
    cueName->setAutoClear(false);
    if(cues.size() == 0) cueName->setVisible(false);

    gui5->setWidgetPosition(OFX_UI_WIDGET_POSITION_DOWN);
    gui5->addSpacer();
    gui5->addImageButton("New Cue", "icons/add.png", false, dim, dim)->setColorBack(ofColor(150, 255));
    gui5->setWidgetPosition(OFX_UI_WIDGET_POSITION_RIGHT);
    gui5->addImageButton("Save Cue", "icons/save.png", false, dim, dim)->setColorBack(ofColor(150, 255));

    ofxUIButton *previous;
    previous = gui5->addImageButton("Previous Cue", "icons/previous.png", false, dim, dim);
    previous->bindToKey(OF_KEY_LEFT);
    previous->setColorBack(ofColor(150, 255));

    ofxUIButton *next;
    next = gui5->addImageButton("Next Cue", "icons/play.png", false, dim, dim);
    next->bindToKey(OF_KEY_RIGHT);
    next->setColorBack(ofColor(150, 255));

    gui5->addImageButton("Load Cue", "icons/open.png", false, dim, dim)->setColorBack(ofColor(150, 255));
    gui5->addImageButton("Delete Cue", "icons/delete.png", false, dim, dim)->setColorBack(ofColor(150, 255));

    gui5->setWidgetPosition(OFX_UI_WIDGET_POSITION_DOWN, OFX_UI_ALIGN_CENTER);

    gui5->addSpacer();
    gui5->addLabelButton("GO", false, 230, 40)->bindToKey(' ');

    gui5->addSpacer();

    gui5->autoSizeToFitWidgets();
    gui5->setVisible(false);
    ofAddListener(gui5->newGUIEvent, this, &ofApp::guiEvent);
    guis.push_back(gui5);
}

//--------------------------------------------------------------
void ofApp::setupGUI6(){
    gui6 = new ofxUISuperCanvas("6: FLUID SOLVER", 0, 0, guiWidth, ofGetHeight());
    gui6->setUIColors(uiThemecb, uiThemeco, uiThemecoh, uiThemecf, uiThemecfh, uiThemecp, uiThemecpo);

    gui6->addSpacer();
    gui6->addLabel("Press '6' to hide panel", OFX_UI_FONT_SMALL);

    gui6->addSpacer();
    gui6->addFPS(OFX_UI_FONT_SMALL);

    gui6->addSpacer();
    gui6->addLabel("Physics");

    gui6->addSpacer();

    gui6->autoSizeToFitWidgets();
    gui6->setVisible(false);
    ofAddListener(gui6->newGUIEvent, this, &ofApp::guiEvent);
    guis.push_back(gui6);
}

//--------------------------------------------------------------
void ofApp::setupGUI7(){
    gui7 = new ofxUISuperCanvas("7: DEPTH CONTOUR", 0, 0, guiWidth, ofGetHeight());
    gui7->setUIColors(uiThemecb, uiThemeco, uiThemecoh, uiThemecf, uiThemecfh, uiThemecp, uiThemecpo);

    gui7->addSpacer();
    gui7->addLabel("Press '7' to hide panel", OFX_UI_FONT_SMALL);

    gui7->addSpacer();
    gui7->addFPS(OFX_UI_FONT_SMALL);

    gui7->addSpacer();
    gui7->addImageToggle("Show Contour", "icons/show.png", &contour.isActive, dim, dim);

    gui7->addSpacer();
    gui7->addToggle("Bounding Rectangle", &contour.drawBoundingRect);
    gui7->addToggle("Convex Hull", &contour.drawConvexHull);
    gui7->addToggle("Convex Hull Line", &contour.drawConvexHullLine);
    gui7->addToggle("Contour Line", &contour.drawContourLine);
    gui7->addSlider("Smoothing Size", 0.0, 40.0, &contour.smoothingSize);


    gui7->autoSizeToFitWidgets();
    gui7->setVisible(false);
    ofAddListener(gui7->newGUIEvent, this, &ofApp::guiEvent);
    guis.push_back(gui7);
}

//--------------------------------------------------------------
void ofApp::setupGUI8Marker(){
    gui8Marker = new ofxUISuperCanvas("8: PARTICLES", 0, 0, guiWidth, ofGetHeight());
    gui8Marker->setUIColors(uiThemecb, uiThemeco, uiThemecoh, uiThemecf, uiThemecfh, uiThemecp, uiThemecpo);

    gui8Marker->addSpacer();
    gui8Marker->addLabel("Press '8' to hide panel", OFX_UI_FONT_SMALL);

    gui8Marker->addSpacer();
    gui8Marker->addFPS(OFX_UI_FONT_SMALL);

    gui8Marker->addSpacer();
    gui8Marker->addImageToggle("Particles Active", "icons/show.png", &markerParticles->isActive, dim, dim);
    gui8Marker->setWidgetPosition(OFX_UI_WIDGET_POSITION_RIGHT);

    ofxUIImageButton *previous;
    previous = gui8Marker->addImageButton("Previous Particle System", "icons/previous.png", false, dim, dim);
    previous->setColorBack(ofColor(150, 255));

    ofxUIImageButton *next;
    next = gui8Marker->addImageButton("Next Particle System", "icons/play.png", false, dim, dim);
    next->setColorBack(ofColor(150, 255));

    gui8Marker->setWidgetPosition(OFX_UI_WIDGET_POSITION_DOWN);

    gui8Marker->addLabel("MARKER", OFX_UI_FONT_LARGE);
    gui8Marker->addSpacer();
    gui8Marker->addLabel("Emitter");
    gui8Marker->addSlider("Particles/sec", 0.0, 20.0, &markerParticles->bornRate);

    gui8Marker->addSlider("Velocity", 0.0, 100.0, &markerParticles->velocity);
    gui8Marker->addSlider("Velocity Random[%]", 0.0, 100.0, &markerParticles->velocityRnd);
    gui8Marker->addSlider("Velocity from Motion[%]", 0.0, 100.0, &markerParticles->velocityMotion);

    gui8Marker->addSlider("Emitter size", 0.0, 60.0, &markerParticles->emitterSize);

    gui8Marker->addSpacer();
    gui8Marker->addLabel("Particle");
    gui8Marker->addToggle("Immortal", &markerParticles->immortal);
    gui8Marker->setWidgetPosition(OFX_UI_WIDGET_POSITION_RIGHT);
    gui8Marker->addToggle("Empty", &markerParticles->isEmpty);
    gui8Marker->addToggle("Draw Line", &markerParticles->drawLine);
    gui8Marker->setWidgetPosition(OFX_UI_WIDGET_POSITION_DOWN);
    gui8Marker->addSlider("Lifetime", 0.0, 20.0, &markerParticles->lifetime);
    gui8Marker->addSlider("Life Random[%]", 0.0, 100.0, &markerParticles->lifetimeRnd);
    gui8Marker->addSlider("Radius", 0.1, 25.0, &markerParticles->radius);
    gui8Marker->addSlider("Radius Random[%]", 0.0, 100.0, &markerParticles->radiusRnd);

    gui8Marker->addSpacer();
    gui8Marker->addLabel("Time behaviour");
    gui8Marker->addToggle("Size", &markerParticles->sizeAge);
    gui8Marker->setWidgetPosition(OFX_UI_WIDGET_POSITION_RIGHT);
    gui8Marker->addToggle("Opacity", &markerParticles->opacityAge);
    gui8Marker->addToggle("Flickers", &markerParticles->flickersAge);
    gui8Marker->setWidgetPosition(OFX_UI_WIDGET_POSITION_DOWN);
    gui8Marker->addToggle("Color", &markerParticles->colorAge);

    gui8Marker->addSpacer();
    gui8Marker->addLabel("Physics");
    gui8Marker->addSlider("Friction", 0, 100, &markerParticles->friction);
    gui8Marker->addSlider("Gravity", 0.0, 15.0, &markerParticles->gravity);
    gui8Marker->addToggle("Bounces", &markerParticles->bounce);


    gui8Marker->addSpacer();

    gui8Marker->autoSizeToFitWidgets();
    gui8Marker->setVisible(false);
    ofAddListener(gui8Marker->newGUIEvent, this, &ofApp::guiEvent);
    guis.push_back(gui8Marker);
    particleGuis.push_back(gui8Marker);
}

//--------------------------------------------------------------
void ofApp::setupGUI8Contour(){
    gui8Contour = new ofxUISuperCanvas("8: PARTICLES", 0, 0, guiWidth, ofGetHeight());
    gui8Contour->setUIColors(uiThemecb, uiThemeco, uiThemecoh, uiThemecf, uiThemecfh, uiThemecp, uiThemecpo);

    gui8Contour->addSpacer();
    gui8Contour->addLabel("Press '8' to hide panel", OFX_UI_FONT_SMALL);

    gui8Contour->addSpacer();
    gui8Contour->addFPS(OFX_UI_FONT_SMALL);

    gui8Contour->addSpacer();
    gui8Contour->addImageToggle("Particles Active", "icons/show.png", &contourParticles->isActive, dim, dim);
    gui8Contour->setWidgetPosition(OFX_UI_WIDGET_POSITION_RIGHT);

    ofxUIImageButton *previous;
    previous = gui8Contour->addImageButton("Previous Particle System", "icons/previous.png", false, dim, dim);
    previous->setColorBack(ofColor(150, 255));

    ofxUIImageButton *next;
    next = gui8Contour->addImageButton("Next Particle System", "icons/play.png", false, dim, dim);
    next->setColorBack(ofColor(150, 255));

    gui8Contour->setWidgetPosition(OFX_UI_WIDGET_POSITION_DOWN);

    gui8Contour->addLabel("CONTOUR", OFX_UI_FONT_LARGE);
    gui8Contour->addSpacer();
    gui8Contour->addLabel("Emitter");
    gui8Contour->addSlider("Particles/sec", 0.0, 20.0, &contourParticles->bornRate);

    gui8Contour->addSlider("Velocity", 0.0, 100.0, &contourParticles->velocity);
    gui8Contour->addSlider("Velocity Random[%]", 0.0, 100.0, &contourParticles->velocityRnd);
    gui8Contour->addSlider("Velocity from Motion[%]", 0.0, 100.0, &contourParticles->velocityMotion);

    gui8Contour->addSlider("Emitter size", 0.0, 60.0, &contourParticles->emitterSize);

    gui8Contour->addSpacer();
    gui8Contour->addLabel("Particle");
    gui8Contour->addToggle("Immortal", &contourParticles->immortal);
    gui8Contour->setWidgetPosition(OFX_UI_WIDGET_POSITION_RIGHT);
    gui8Contour->addToggle("Empty", &contourParticles->isEmpty);
    gui8Contour->addToggle("Draw Line", &contourParticles->drawLine);
    gui8Contour->setWidgetPosition(OFX_UI_WIDGET_POSITION_DOWN);
    gui8Contour->addSlider("Lifetime", 0.0, 20.0, &contourParticles->lifetime);
    gui8Contour->addSlider("Life Random[%]", 0.0, 100.0, &contourParticles->lifetimeRnd);
    gui8Contour->addSlider("Radius", 0.1, 25.0, &contourParticles->radius);
    gui8Contour->addSlider("Radius Random[%]", 0.0, 100.0, &contourParticles->radiusRnd);

    gui8Contour->addSpacer();
    gui8Contour->addLabel("Time behaviour");
    gui8Contour->addToggle("Size", &contourParticles->sizeAge);
    gui8Contour->setWidgetPosition(OFX_UI_WIDGET_POSITION_RIGHT);
    gui8Contour->addToggle("Opacity", &contourParticles->opacityAge);
    gui8Contour->addToggle("Flickers", &contourParticles->flickersAge);
    gui8Contour->setWidgetPosition(OFX_UI_WIDGET_POSITION_DOWN);
    gui8Contour->addToggle("Color", &contourParticles->colorAge);

    gui8Contour->addSpacer();
    gui8Contour->addLabel("Physics");
    gui8Contour->addSlider("Friction", 0, 100, &contourParticles->friction);
    gui8Contour->addSlider("Gravity", 0.0, 15.0, &contourParticles->gravity);
    gui8Contour->addToggle("Bounces", &contourParticles->bounce);

    gui8Contour->addSpacer();

    gui8Contour->autoSizeToFitWidgets();
    gui8Contour->setVisible(false);
    ofAddListener(gui8Contour->newGUIEvent, this, &ofApp::guiEvent);
    guis.push_back(gui8Contour);
    particleGuis.push_back(gui8Contour);
}

//--------------------------------------------------------------
void ofApp::setupGUI8Grid(){
    gui8Grid = new ofxUISuperCanvas("8: PARTICLES", 0, 0, guiWidth, ofGetHeight());
    gui8Grid->setUIColors(uiThemecb, uiThemeco, uiThemecoh, uiThemecf, uiThemecfh, uiThemecp, uiThemecpo);

    gui8Grid->addSpacer();
    gui8Grid->addLabel("Press '8' to hide panel", OFX_UI_FONT_SMALL);

    gui8Grid->addSpacer();
    gui8Grid->addFPS(OFX_UI_FONT_SMALL);

    gui8Grid->addSpacer();
    gui8Grid->addImageToggle("Particles Active", "icons/show.png", &gridParticles->isActive, dim, dim);
    gui8Grid->setWidgetPosition(OFX_UI_WIDGET_POSITION_RIGHT);

    ofxUIImageButton *previous;
    previous = gui8Grid->addImageButton("Previous Particle System", "icons/previous.png", false, dim, dim);
//    previous->bindToKey(OF_KEY_LEFT);
//    previous->setTriggerType(OFX_UI_TRIGGER_BEGIN);
    previous->setColorBack(ofColor(150, 255));

    ofxUIImageButton *next;
    next = gui8Grid->addImageButton("Next Particle System", "icons/play.png", false, dim, dim);
    next->setColorBack(ofColor(150, 255));

    gui8Grid->setWidgetPosition(OFX_UI_WIDGET_POSITION_DOWN);

    gui8Grid->addLabel("GRID", OFX_UI_FONT_LARGE);
    gui8Grid->addSpacer();

    gui8Grid->addSlider("Radius", 0.1, 25.0, &gridParticles->radius);
    gui8Grid->addIntSlider("Resolution", 1, 20, &gridParticles->gridRes)->setStickyValue(1.0);

    gui8Grid->addSpacer();
    gui8Grid->addLabel("Physics");
    gui8Grid->addSlider("Friction", 0, 100, &gridParticles->friction);
    gui8Grid->addSlider("Gravity", 0.0, 15.0, &gridParticles->gravity);
    gui8Grid->addToggle("Bounces", &gridParticles->bounce);

    gui8Grid->addSlider("Flocking Radius", 10.0, 100.0, &gridParticles->flockingRadius);
    lowThresh = gui8Grid->addSlider("Lower Threshold", 0.025, 1.0, &gridParticles->lowThresh);
    lowThresh->setLabelPrecision(3);
    highThresh = gui8Grid->addSlider("Higher Threshold", 0.025, 1.0, &gridParticles->highThresh);
    highThresh->setLabelPrecision(3);

    gui8Grid->addSlider("Separation Strength", 0.001, 0.1, &gridParticles->separationStrength)->setLabelPrecision(3);
    gui8Grid->addSlider("Attraction Strength", 0.001, 0.1, &gridParticles->attractionStrength)->setLabelPrecision(3);
    gui8Grid->addSlider("Alignment Strength", 0.001, 0.1, &gridParticles->alignmentStrength)->setLabelPrecision(3);

    gui8Grid->addSpacer();

    gui8Grid->autoSizeToFitWidgets();
    gui8Grid->setVisible(false);
    ofAddListener(gui8Grid->newGUIEvent, this, &ofApp::guiEvent);
    guis.push_back(gui8Grid);
    particleGuis.push_back(gui8Grid);
}

//--------------------------------------------------------------
void ofApp::saveGUISettings(const string path, const bool isACue){

    // Create directory if this doesnt exist
    ofFile file(path);
    if(!ofDirectory::doesDirectoryExist(file.getEnclosingDirectory())){
        ofDirectory::createDirectory(file.getEnclosingDirectory());
    }

    ofxXmlSettings *XML = new ofxXmlSettings();

    // Save settings
    for(vector<ofxUISuperCanvas *>::iterator it = guis.begin(); it != guis.end(); ++it){
        ofxUICanvas *g = *it;
        int guiIndex = XML->addTag("GUI");

        // panel 2, 3, 4 and 5 are settings that for now we dont want to save in a cue file
        if(isACue && guiIndex >= 2 && guiIndex <= 5) continue;

        XML->pushTag("GUI", guiIndex);
        vector<ofxUIWidget*> widgets = g->getWidgets();
        for(int i = 0; i < widgets.size(); i++){
            // kind number 20 is ofxUIImageToggle, for which we don't want to save the state
            // kind number 12 is ofxUITextInput, for which we don't want to save the state
//            if(widgets[i]->hasState() && widgets[i]->getKind() != 20 && widgets[i]->getKind() != 12){
            if(widgets[i]->hasState() && widgets[i]->getKind() != 12){
                int index = XML->addTag("Widget");
                if(XML->pushTag("Widget", index)){
                    XML->setValue("Kind", widgets[i]->getKind(), 0);
                    XML->setValue("Name", widgets[i]->getName(), 0);
                    widgets[i]->saveState(XML);
                }
                XML->popTag();
            }
        }
        XML->popTag();
    }

    // Save active particles panel
    XML->addTag("PARTICLES");
    XML->pushTag("PARTICLES", 0);
    XML->setValue("Active", currentParticleSystem, 0);
    XML->popTag();

    // Save Cues if it is not a cue
    if(!isACue){
        XML->addTag("CUES");
        XML->pushTag("CUES", 0);
        XML->setValue("Active", currentCueIndex, 0);
        for(int i = 0; i < cues.size(); i++){
            XML->setValue("Cue", cues[i], i);
        }
        XML->popTag();
    }

    XML->saveFile(path);
    delete XML;
}

//--------------------------------------------------------------
void ofApp::loadGUISettings(const string path, const bool interpolate, const bool isACue){
    ofxXmlSettings *XML = new ofxXmlSettings();
    if(!XML->loadFile(path)){
        ofLogWarning("File " + ofFilePath::getFileName(path) + " not found.");
        return;
    }

    widgetsToUpdate.clear();

    int guiIndex = 0;
    for(vector<ofxUISuperCanvas *>::iterator it = guis.begin(); it != guis.end(); ++it){
        ofxUICanvas *g = *it;
        // For now we never want to load the settings from panel 4
        if(guiIndex == 4){
            guiIndex++;
            continue;
        }
        // Panel 2, 3, 4 and 5 are settings we dont want to load from a cue file
        if(isACue && guiIndex >= 2 && guiIndex <= 5){
           guiIndex++;
           continue;
        }
        XML->pushTag("GUI", guiIndex);
        int widgetTags = XML->getNumTags("Widget");
        for(int i = 0; i < widgetTags; i++){
            XML->pushTag("Widget", i);
            string name = XML->getValue("Name", "NULL", 0);
            ofxUIWidget *widget = g->getWidget(name);
            if(widget != NULL && widget->hasState()){
                if(interpolate){ // interpolate new values with previous ones
                    interpolatingWidgets = true;
                    vector<float> values;
                    if(widget->getKind() == 6){ // kind 6 is a range slider widget
                        values.push_back(XML->getValue("HighValue", -1, 0));
                        values.push_back(XML->getValue("LowValue", -1, 0));
                        widgetsToUpdate[widget] = values;
                    }
                    else{
                        values.push_back(XML->getValue("Value", -1, 0));
                        widgetsToUpdate[widget] = values;
                    }
                }
                else{
                    interpolatingWidgets = false;
                    widgetsToUpdate.clear();
                    widget->loadState(XML);
                    g->triggerEvent(widget);
                }
            }
            XML->popTag();
        }
        guiIndex++;
        XML->popTag();
    }

    // Load active particles panel
    XML->pushTag("PARTICLES", 0);
    currentParticleSystem = XML->getValue("Active", 0, 0);
    XML->popTag();

    // Load cue list if it is not a cue
    if(!isACue){
        XML->pushTag("CUES");
        int numCues = XML->getNumTags("Cue");
        cues.clear();
        currentCueIndex = XML->getValue("Active", -1, 0);
        for(int i = 0; i < numCues; i++){
            string name = XML->getValue("Cue", "NULL", i);
            // Check if path corresponds to a cue file, if not, we dont add it
            if(ofFile::doesFileExist(name)){
                cues.push_back(name);
            }
            else{
                ofLogWarning("File " + ofFilePath::getFileName(name) + " not found.");
            }
        }
        XML->popTag();

        if(cues.size() > 0){
            if(cues.size() <= currentCueIndex) currentCueIndex = 0;
            cueIndexLabel->setLabel(ofToString(currentCueIndex)+".");
            string cueFileName = ofFilePath::getBaseName(cues[currentCueIndex]);
            cueName->setTextString(cueFileName);
            cueName->setVisible(true);
            loadGUISettings(cues[currentCueIndex], false, true);

            // Create map of the sliders so we can get the values outside
            float n = cues.size();
            for(int i = 0; i < cues.size(); i++){
                ofxUILabel *label;
                label = gui3->addLabel(ofFilePath::getBaseName(cues[i]));
                string cueName = "Sequence percent";
                float low = (float)i/n*100;
                float high = ((float)i+1.0)/n*100;
                ofxUIRangeSlider *slider;
                slider = gui3->addRangeSlider(cueName, 0, 100, low, high);
                pair<ofxUILabel *, ofxUIRangeSlider*> cue(label, slider);
                cueSliders.push_back(cue);
            }
            gui3->autoSizeToFitWidgets();

        }
        else currentCueIndex = -1;
    }

    delete XML;
}

//--------------------------------------------------------------
void ofApp::interpolateWidgetValues(){
    map<ofxUIWidget *, vector<float> >::iterator it = widgetsToUpdate.begin();
    while (it != widgetsToUpdate.end()) {
        ofxUIWidget * w = it->first;
        vector<float> values = it->second;
        ofxXmlSettings *XML = new ofxXmlSettings();
        bool canDelete = false;
        // if widget is a range slider we have two values to interpolate to
        if(w->getKind() == 6){ // kind 6 is a range slider widget
            w->saveState(XML);
            float targetHighValue = values.at(0);
            float targetLowValue = values.at(1);
            float currentHighValue = XML->getValue("HighValue", targetHighValue, 0);
            float currentLowValue = XML->getValue("LowValue", targetLowValue, 0);
            float highDifference = targetHighValue-currentHighValue;
            float lowDifference = targetLowValue-currentLowValue;
            // if the difference is small we assign the target value
            if(abs(highDifference) < 1 && abs(lowDifference) < 1){
                canDelete = true;
                XML->setValue("HighValue", targetHighValue, 0);
                XML->setValue("LowValue", targetLowValue, 0);
                w->loadState(XML);
            }
            else{
                float highIncrement = highDifference/100.0; // TODO: use float ofLerp(float start, float stop, float amt)
                float lowIncrement = lowDifference/100.0;
                XML->setValue("HighValue", currentHighValue+highIncrement, 0);
                XML->setValue("LowValue", currentLowValue+lowIncrement, 0);
                w->loadState(XML);
            }
        }
        else{
            w->saveState(XML);
            float targetValue = values.front();
            float currentValue = XML->getValue("Value", targetValue, 0);
            float difference = targetValue-currentValue;
            if(abs(difference) < 1 || w->getKind() == 2 || w->getKind() == 20){ // kind 2 is a toggle and 20 is ofxUIImageToggle
                canDelete = true;
                XML->setValue("Value", targetValue, 0);
                w->loadState(XML);
            }
            else{
                float increment = difference/100.0;
                XML->setValue("Value", currentValue+increment, 0);
                w->loadState(XML);
            }
        }

        // If values already interpolated we delete widget from the map
        if (canDelete){
            map<ofxUIWidget *, vector<float> >::iterator toErase = it;
            ++it;
            widgetsToUpdate.erase(toErase);
        }
        else{
            ++it;
        }
        // Delete XML where we load and save values
        delete XML;
    }

    if(widgetsToUpdate.empty()) interpolatingWidgets = false;
}

//--------------------------------------------------------------
void ofApp::guiEvent(ofxUIEventArgs &e){
    if(e.getName() == "Reset Kinect"){
        if(resetKinect){
            kinect.close();
            kinect.clear();
        }
        else{
            kinect.init(true); // shows infrared instead of RGB video Image
            kinect.open();
        }
    }

    if(e.getName() == "Save Settings"){
        ofxUIImageButton *button = (ofxUIImageButton *) e.widget;
        if(button->getValue() == true){
            ofFileDialogResult result = ofSystemSaveDialog("sequence.xml", "Save current settings");
            if(result.bSuccess){
                saveGUISettings(result.getPath(), false);
            }
        }
    }

    if(e.getName() == "Load Settings"){
        ofxUIImageButton *button = (ofxUIImageButton *) e.widget;
        if(button->getValue() == true){
            ofFileDialogResult result = ofSystemLoadDialog("Select settings xml file.", false, ofToDataPath("settings/"));
            if(result.bSuccess){
                loadGUISettings(result.getPath(), false, false);
            }
        }
    }

    if(e.getName() == "Record Sequence"){
        if(recordingSequence->getValue() == true){
            drawSequence = false;
            sequence.startRecording();
        }
    }

    if(e.getName() == "Save Sequence"){
        ofxUIImageButton *button = (ofxUIImageButton *) e.widget;
        if(button->getValue() == true){
            recordingSequence->setValue(false);
            drawSequence = false;
            ofFileDialogResult result = ofSystemSaveDialog("sequence.xml", "Save sequence file");
            if(result.bSuccess){
                sequence.save(result.getPath());
            }
        }
    }

    if(e.getName() == "Load Sequence"){
        ofxUIImageButton *button = (ofxUIImageButton *) e.widget;
        if(button->getValue() == true){
            recordingSequence->setValue(false);
            drawSequence = false;
            ofFileDialogResult result = ofSystemLoadDialog("Select sequence xml file.", false, ofToDataPath("sequences/"));
            if(result.bSuccess){
                sequence.load(result.getPath());
                sequenceFilename->setLabel("Filename: "+sequence.filename);
                sequenceDuration->setLabel("Duration: "+ofToString(sequence.duration, 2) + " s");
                sequenceNumFrames->setLabel("Number of frames: "+ofToString(sequence.numFrames));
            }
        }
    }

    if(e.getName() == "Play Sequence"){
        ofxUIImageToggle *toggle = (ofxUIImageToggle *) e.widget;
        if(toggle->getValue() == true){
            recordingSequence->setValue(false);
            drawSequence = true;
        }
        else{
            sequence.clearPlayback();
            drawSequence = false;
        }
    }

    if(e.getName() == "Start vmo"){
        ofxUIImageButton *button = (ofxUIImageButton *) e.widget;
        if(button->getValue() == true){
            initStatus = true;
            stopTracking = false;
        }
    }

    if(e.getName() == "Stop vmo"){
        ofxUIImageButton *button = (ofxUIImageButton *) e.widget;
        if(button->getValue() == true){
            stopTracking = true;
        }
    }

    if(e.getName() == "Cue Name"){
        ofxUITextInput *ti = (ofxUITextInput *) e.widget;
        if(ti->getInputTriggerType() == OFX_UI_TEXTINPUT_ON_UNFOCUS || ti->getInputTriggerType() == OFX_UI_TEXTINPUT_ON_ENTER){
            string cuePath = "cues/"+cueName->getTextString()+".xml";
            cues[currentCueIndex] = cuePath;
            cueSliders[currentCueIndex].first->setLabel(ofFilePath::getBaseName(cuePath));
        }
    }

    if(e.getName() == "New Cue"){
        ofxUIImageButton *button = (ofxUIImageButton *) e.widget;
        if(button->getValue() == true){
            if(currentCueIndex >= 0) saveGUISettings(cues[currentCueIndex], true);
            currentCueIndex++;
            string cueFileName = "newCue"+ofToString(currentCueIndex);
            string cuePath = "cues/"+cueFileName+".xml";
            while(find(cues.begin(), cues.end(), cuePath) != cues.end()){
                cueFileName += ".1";
                cuePath = "cues/"+cueFileName+".xml";
            }
            cues.insert(cues.begin()+currentCueIndex, cuePath);
            cueIndexLabel->setLabel(ofToString(currentCueIndex)+".");
            cueName->setTextString(cueFileName);
            cueName->setVisible(true);

            // Add new empty cue in the end and modify all of them
            ofxUILabel *label;
            label = gui3->addLabel(" ");
            string cueName = "Sequence percent";
            ofxUIRangeSlider *slider;
            slider = gui3->addRangeSlider(cueName, 0, 100, 0, 100);
            pair<ofxUILabel *, ofxUIRangeSlider*> cue(label, slider);
            cueSliders.push_back(cue);

            float n = cueSliders.size();
            for (int i = 0; i < cueSliders.size(); i++){
                cueSliders.at(i).first->setLabel(ofFilePath::getBaseName(cues[i]));
                cueSliders.at(i).first->setVisible(true);
                float low = (float)i/n*100;
                float high = ((float)i+1.0)/n*100;
                cueSliders.at(i).second->setValueLow(low);
                cueSliders.at(i).second->setValueHigh(high);
            }
            gui3->autoSizeToFitWidgets();
        }
    }

    if(e.getName() == "Save Cue"){
        ofxUIImageButton *button = (ofxUIImageButton *) e.widget;
        if(button->getValue() == true){
            string cuePath = "cues/"+cueName->getTextString()+".xml";
            saveGUISettings(cuePath, true);
        }
    }

    if(e.getName() == "Previous Cue"){
        ofxUIImageButton *button = (ofxUIImageButton *) e.widget;
        if(button->getValue() == true){
            if(cues.size() == 0) return;
            if(!interpolatingWidgets) saveGUISettings(cues[currentCueIndex], true);
            if(currentCueIndex-1 >= 0) currentCueIndex--;
            else if(currentCueIndex-1 < 0) currentCueIndex = cues.size()-1;
            loadGUISettings(cues[currentCueIndex], false, true);
            string cueFileName = ofFilePath::getBaseName(cues[currentCueIndex]);
            cueIndexLabel->setLabel(ofToString(currentCueIndex)+".");
            cueName->setTextString(cueFileName);
        }
    }

    if(e.getName() == "Next Cue"){
        ofxUIImageButton *button = (ofxUIImageButton *) e.widget;
        if(button->getValue() == true){
            if(cues.size() == 0) return;
            if(!interpolatingWidgets) saveGUISettings(cues[currentCueIndex], true);
            if(currentCueIndex+1 < cues.size()) currentCueIndex++;
            else if(currentCueIndex+1 == cues.size()) currentCueIndex = 0;
            loadGUISettings(cues[currentCueIndex], false, true);
            string cueFileName = ofFilePath::getBaseName(cues[currentCueIndex]);
            cueIndexLabel->setLabel(ofToString(currentCueIndex)+".");
            cueName->setTextString(cueFileName);
        }
    }

    if(e.getName() == "Load Cue"){
        ofxUIImageButton *button = (ofxUIImageButton *) e.widget;
        if(button->getValue() == true){
            ofFileDialogResult result = ofSystemLoadDialog("Select cue file.", false, ofToDataPath("cues/"));
            if(result.bSuccess){
                if(cues.size() == 0) currentCueIndex = 0;
                loadGUISettings(result.getPath(), false, true);
                string cuePath = "cues/"+ofFilePath::getFileName(result.getPath());
                saveGUISettings(cuePath, true);
                cues[currentCueIndex] = cuePath;
                string cueFileName = ofFilePath::getBaseName(cuePath);
                cueIndexLabel->setLabel(ofToString(currentCueIndex)+".");
                cueName->setTextString(cueFileName);
                cueName->setVisible(true);
            }
        }
    }

    if(e.getName() == "Delete Cue"){
        ofxUIImageButton *button = (ofxUIImageButton *) e.widget;
        if(button->getValue() == true){
            if(cues.size() == 0) return;

            // Delete cue string from vector
            cues.erase(cues.begin()+currentCueIndex);

            // Delete cue slider
            gui3->removeWidget(cueSliders.at(currentCueIndex).first);
            gui3->removeWidget(cueSliders.at(currentCueIndex).second);
            cueSliders.erase(cueSliders.begin()+currentCueIndex);

            // Show previous cue
            if(currentCueIndex-1 >= 0) currentCueIndex--;
            else if(currentCueIndex-1 < 0) currentCueIndex = cues.size()-1;
            if(cues.size() == 0){
                currentCueIndex = -1; // if it enters here it already has this value
                cueIndexLabel->setLabel("");
                cueName->setTextString("");
                cueName->setVisible(false);
            }
            else{
                loadGUISettings(cues[currentCueIndex], false, true);
                string cueFileName = ofFilePath::getBaseName(cues[currentCueIndex]);
                cueIndexLabel->setLabel(ofToString(currentCueIndex)+".");
                cueName->setTextString(cueFileName);
            }

            // Modify all cue sliders after deleting the current one
            float n = cueSliders.size();
            for (int i = 0; i < cueSliders.size(); i++){
                cueSliders.at(i).first->setLabel(ofFilePath::getBaseName(cues[i]));
                float low = (float)i/n*100;
                float high = ((float)i+1.0)/n*100;
                cueSliders.at(i).second->setValueLow(low);
                cueSliders.at(i).second->setValueHigh(high);
            }
            gui3->autoSizeToFitWidgets();
        }
    }

    if(e.getName() == "GO"){
        ofxUILabelButton *button = (ofxUILabelButton *) e.widget;
        if(button->getValue() == true){
            if(cues.size() == 0) return;
            if(!interpolatingWidgets) saveGUISettings(cues[currentCueIndex], true);
            if(currentCueIndex+1 < cues.size()) currentCueIndex++;
            else if(currentCueIndex+1 == cues.size()) currentCueIndex = 0;
            loadGUISettings(cues[currentCueIndex], true, true);
            string cueFileName = ofFilePath::getBaseName(cues[currentCueIndex]);
            cueIndexLabel->setLabel(ofToString(currentCueIndex)+".");
            cueName->setTextString(cueFileName);
            button->setValue(false);
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
    //  ofxUIRadio *radio = (ofxUIRadio *) e.widget;
    //  cout << radio->getName() << " value: " << radio->getValue() << " active name: " << radio->getActiveName() << endl;
    // }

    if(e.getName() == "Previous Particle System"){
        ofxUIImageButton *button = (ofxUIImageButton *) e.widget;
        if(button->getValue() == true){
            particleGuis.at(currentParticleSystem)->setVisible(false);
            if(currentParticleSystem > 0) currentParticleSystem--;
            else currentParticleSystem = particleSystems.size()-1;
            particleGuis.at(currentParticleSystem)->setVisible(true);
        }
    }

    if(e.getName() == "Next Particle System"){
        ofxUIImageButton *button = (ofxUIImageButton *) e.widget;
        if(button->getValue() == true){
            particleGuis.at(currentParticleSystem)->setVisible(false);
            if(currentParticleSystem < particleSystems.size()-1) currentParticleSystem++;
            else currentParticleSystem = 0;
            particleGuis.at(currentParticleSystem)->setVisible(true);
        }
    }

    if(e.getName() == "Particles Active"){
        ofxUIImageToggle *toggle = (ofxUIImageToggle *) e.widget;
        if(toggle->getValue() == true) particleSystems[currentParticleSystem]->bornParticles();
        else particleSystems[currentParticleSystem]->killParticles();
    }

    if(e.getName() == "Lower Threshold" || e.getName() == "Higher Threshold"){
        if(lowThresh->getValue() > highThresh->getValue()){
            highThresh->setValue(lowThresh->getValue());
//            lowThresh->setValue(highThresh->getValue());
        }
    }
}

//--------------------------------------------------------------
void ofApp::exit(){
    kinect.close();
    kinect.clear();

    if(!interpolatingWidgets && cues.size()) saveGUISettings(cues[currentCueIndex], false);
    saveGUISettings("settings/lastSettings.xml", false);

    delete contourParticles;
    delete markerParticles;
    delete gridParticles;
    particleSystems.clear();

//    for (int i=0; i<particleSystems.size(); i++) {
//        delete particleSystems.at(i);
//        particleSystems.at(i) = NULL;
//    }
//    particleSystems.clear();

    // Delete cue sliders map
    cueSliders.clear();

    // Cleanup any loaded images
    for(int i = 0; i < savedDepthImages.size(); i++){
        ofImage *img = savedDepthImages[i];
        delete img;
    }
    savedDepthImages.clear();

    for(int i = 0; i < savedIrImages.size(); i++){
        ofImage *img = savedIrImages[i];
        delete img;
    }
    savedIrImages.clear();

    delete gui0;
    delete gui1;
    delete gui2;
    delete gui3;
    delete gui4;
    delete gui5;
    delete gui6;
    delete gui7;
    delete gui8Marker;
    delete gui8Contour;
    delete gui8Grid;
    guis.clear();
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
    if(!cueName->isFocused()){
        switch (key){
            case 'f':
                ofToggleFullscreen();
                break;

            case 'h':
                gui0->setVisible(false);
                gui1->setVisible(false);
                gui2->setVisible(false);
                gui3->setVisible(false);
                gui4->setVisible(false);
                gui5->setVisible(false);
                gui6->setVisible(false);
                gui7->setVisible(false);
                particleGuis.at(currentParticleSystem)->setVisible(false);
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
                gui7->setVisible(false);
                particleGuis.at(currentParticleSystem)->setVisible(false);
                break;

            case '1':
                gui0->setVisible(false);
                gui1->toggleVisible();
                gui2->setVisible(false);
                gui3->setVisible(false);
                gui4->setVisible(false);
                gui5->setVisible(false);
                gui6->setVisible(false);
                gui7->setVisible(false);
                particleGuis.at(currentParticleSystem)->setVisible(false);
                break;

            case '2':
                gui0->setVisible(false);
                gui1->setVisible(false);
                gui2->toggleVisible();
                gui3->setVisible(false);
                gui4->setVisible(false);
                gui5->setVisible(false);
                gui6->setVisible(false);
                gui7->setVisible(false);
                particleGuis.at(currentParticleSystem)->setVisible(false);
                break;

            case '3':
                gui0->setVisible(false);
                gui1->setVisible(false);
                gui2->setVisible(false);
                gui3->toggleVisible();
                gui4->setVisible(false);
                gui5->setVisible(false);
                gui6->setVisible(false);
                gui7->setVisible(false);
                particleGuis.at(currentParticleSystem)->setVisible(false);
                break;

            case '4':
                gui0->setVisible(false);
                gui1->setVisible(false);
                gui2->setVisible(false);
                gui3->setVisible(false);
                gui4->toggleVisible();
                gui5->setVisible(false);
                gui6->setVisible(false);
                gui7->setVisible(false);
                particleGuis.at(currentParticleSystem)->setVisible(false);
                break;

            case '5':
                gui0->setVisible(false);
                gui1->setVisible(false);
                gui2->setVisible(false);
                gui3->setVisible(false);
                gui4->setVisible(false);
                gui5->toggleVisible();
                gui6->setVisible(false);
                gui7->setVisible(false);
                particleGuis.at(currentParticleSystem)->setVisible(false);
                break;

            case '6':
                gui0->setVisible(false);
                gui1->setVisible(false);
                gui2->setVisible(false);
                gui3->setVisible(false);
                gui4->setVisible(false);
                gui5->setVisible(false);
                gui6->toggleVisible();
                gui7->setVisible(false);
                particleGuis.at(currentParticleSystem)->setVisible(false);
                break;

            case '7':
                gui0->setVisible(false);
                gui1->setVisible(false);
                gui2->setVisible(false);
                gui3->setVisible(false);
                gui4->setVisible(false);
                gui5->setVisible(false);
                gui6->setVisible(false);
                gui7->toggleVisible();
                particleGuis.at(currentParticleSystem)->setVisible(false);
                break;

            case '8':
                gui0->setVisible(false);
                gui1->setVisible(false);
                gui2->setVisible(false);
                gui3->setVisible(false);
                gui4->setVisible(false);
                gui5->setVisible(false);
                gui6->setVisible(false);
                gui7->setVisible(false);
                particleGuis.at(currentParticleSystem)->toggleVisible();
                break;

            default:
                break;
        }
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
