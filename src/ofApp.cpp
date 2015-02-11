#include "ofApp.h"

using namespace ofxCv;
using namespace cv;

//--------------------------------------------------------------
void ofApp::setup(){

    // ofEnableBlendMode(OF_BLENDMODE_ADD);

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
    // smoothingSize = 0;
    // contour.setup();

    // SEQUENCE
    int maxMarkers = 2;
    sequence.setup(maxMarkers);
    sequence.load("sequences/sequence.xml");

//    //VMO Setup goes here//
    int dimensions = 2;
    obs.assign(sequence.numFrames, vector<float>(maxMarkers*dimensions));
    for (int markerIndex = 0; markerIndex < maxMarkers; markerIndex++){
        for (int frameIndex = 0; frameIndex < sequence.numFrames; frameIndex++){
            obs[frameIndex][markerIndex*dimensions] = sequence.markersPosition[markerIndex][frameIndex].x;
            obs[frameIndex][markerIndex*dimensions+1] = sequence.markersPosition[markerIndex][frameIndex].y;
        }
    }

    initStatus = true;
    stopTracking = true;
//    // gestureInd = -1;
//    // gestureCat = -1;
    // 2. Processing
    // 2.1 Load file into VMO
    int minLen = 1; // Temporary setting
    float start = 0.0, step = 0.05, stop = 5.0;
    
    // For sequence4.xml
//    int minLen = 4;
//    float start = 11.0 step = 0.01 stop = 14.0;
    
    float t = vmo::findThreshold(obs, dimensions, maxMarkers, start, step, stop); // Temporary threshold range and step
    seqVmo = vmo::buildOracle(obs, dimensions, maxMarkers, t);
    // 2.2 Output pattern list
    pttrList = vmo::findPttr(seqVmo, minLen);
    sequence.patterns = vmo::processPttr(seqVmo, pttrList); // double free error in linux

    cout << sequence.patterns.size() << endl;

    // SETUP GUIs
    dim = 32;
    guiWidth = 240;
    theme = OFX_UI_THEME_GRAYDAY;
    drawPatterns = false;
    drawSequence = false;

    setupGUI0();
    setupGUI1();
    setupGUI2();
    setupGUI3();
    setupGUI4();
    setupGUI5();
    setupGUI6(0);

    loadGUISettings("settings/lastSettings.xml");
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
        vector<irMarker>& tempMarkers       = tracker.getFollowers();   // TODO: assign dead labels to new labels and have a MAX number of markers
        vector<unsigned int> deadLabels     = tracker.getDeadLabels();
        vector<unsigned int> currentLabels  = tracker.getCurrentLabels();
        // vector<unsigned int> newLabels      = tracker.getNewLabels();

        // Update markers if we loose track of them
        for(unsigned int i = 0; i < tempMarkers.size(); i++){
            tempMarkers[i].updateLabels(deadLabels, currentLabels);
        }

        // Print currentLabels
        // cout << "Current:" << endl;
        // for(unsigned int i = 0; i < currentLabels.size(); i++){
        //     cout << currentLabels[i] << endl;
        // }

        // Print markers for debug
        // cout << "markers:" << endl;
        // for(unsigned int i = 0; i < tempMarkers.size(); i++){
        //    cout << tempMarkers[i].getLabel() << endl;
        // }

        // Update grid particles
        particles.update(dt, tempMarkers);

        // Update markers particles
        markersParticles.update(dt, tempMarkers);

        // Record sequence when recording button is true
        if(recordingSequence->getValue() == true) sequence.record(tempMarkers);

        if(drawSequence) sequence.update();

        //Gesture Tracking with VMO here?
        if (tempMarkers.size()>1){
            if (!stopTracking){
                vector<float> obs; // Temporary code
                for(unsigned int i = 0; i < 2; i++){
                    obs.push_back(tempMarkers[i].smoothPos.x);
                    obs.push_back(tempMarkers[i].smoothPos.y);
                }
                if(initStatus){
                    currentBf = vmo::tracking_init(seqVmo, pttrList, obs);
                    initStatus = false;
                }
                else{
                    prevBf = currentBf;
                    currentBf = vmo::tracking(seqVmo, pttrList, prevBf, obs);
                }
            }
        }
    }
}

//--------------------------------------------------------------
void ofApp::draw(){

    ofPushMatrix();
    ofTranslate(guiWidth+10, 0);
    // ofScale(reScale, reScale);
    ofScale(1.2, 1.2);
    ofBackground(red, green, blue, 255);
    // ofEnableBlendMode(OF_BLENDMODE_ALPHA);

    ofSetColor(255);

    // Kinect images
    // irImage.draw(0, 0);
    // depthImage.draw(0, 0);

    // OpenCV contour detection
    // contourFinder.draw();
    irMarkerFinder.draw();

    // Graphics
    // particles.draw();
    markersParticles.draw();
    // contour.draw();

     vector<irMarker>& tempMarkers         = tracker.getFollowers();
     // Draw identified IR markers
     for (int i = 0; i < tempMarkers.size(); i++){
         tempMarkers[i].draw();
     }

    if(drawSequence) sequence.draw();

    ofPopMatrix();

//	gestureUpdate = seqVmo.getGestureUpdate(currentBf.currentIdx, pttrList);

    // Draw gesture patterns
    ofSetColor(255, 0, 0);
    ofSetLineWidth(3);
    for(int patternIndex = 0; patternIndex < sequence.patterns.size(); patternIndex++){
        for(int markerIndex = 0; markerIndex < sequence.patterns[patternIndex].size(); markerIndex++){
//            cout << sequence.patterns[patternIndex][markerIndex].size() << endl;
            sequence.patterns[patternIndex][markerIndex].draw();
        }
    }


    map<int, float> currentPatterns; // Use "gestureUpdate" above!!!!!!!!!!
    currentPatterns[1] = 0.35;
    currentPatterns[3] = 0.75;
    currentPatterns[4] = 0.95;
//    if(drawPatterns) sequence.drawPatterns(currentPatterns);

}

//--------------------------------------------------------------
void ofApp::setupGUI0(){
    gui0 = new ofxUISuperCanvas("0: MAIN WINDOW", 0, 0, guiWidth, ofGetHeight());
    gui0->setTheme(theme);

    gui0->addSpacer();
    gui0->addLabel("Press panel number 0 to 7 to", OFX_UI_FONT_SMALL);
    gui0->addLabel("switch between panels and hide", OFX_UI_FONT_SMALL);
    gui0->addLabel("them.", OFX_UI_FONT_SMALL);
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
    gui0->addLabel("5: CUE LIST");
    gui0->addSpacer();

    gui0->addSpacer();
    gui0->addLabel("6: FLUID SOLVER");
    gui0->addSpacer();

    gui0->addSpacer();
    gui0->addLabel("7: PARTICLES");
    gui0->addSpacer();

    gui0->addSpacer();
    gui0->addLabel("8: DEPTH CONTOUR");
    gui0->addSpacer();

    gui0->autoSizeToFitWidgets();
    ofAddListener(gui0->newGUIEvent, this, &ofApp::guiEvent);
    guis.push_back(gui0);
}

//--------------------------------------------------------------
void ofApp::setupGUI1(){
    gui1 = new ofxUISuperCanvas("1: BASICS", 0, 0, guiWidth, ofGetHeight());
    gui1->setTheme(theme);
    gui1->setTriggerWidgetsUponLoad(true);

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
    gui1->addImageButton("Save Settings", "gui/icons/save.png", false, dim, dim);
    gui1->setWidgetPosition(OFX_UI_WIDGET_POSITION_RIGHT);
    gui1->addImageButton("Load Settings", "gui/icons/open.png", false, dim, dim);
    gui1->setWidgetPosition(OFX_UI_WIDGET_POSITION_DOWN);

    gui1->addSpacer();

    gui1->addLabel("GUI THEME");
    vector<string> themes;
    themes.push_back("DEFAULT");
    themes.push_back("HIPSTER");
    themes.push_back("GRAYDAY");
    themes.push_back("RUSTIC");
    themes.push_back("LIMESTONE");
    themes.push_back("VEGAN");
    themes.push_back("BLUEBLUE");
    themes.push_back("COOLCLAY");
    themes.push_back("SPEARMINT");
    themes.push_back("PEPTOBISMOL");
    themes.push_back("MIDNIGHT");
    themes.push_back("BERLIN");

    ofxUIRadio *guiThemes;
    guiThemes = gui1->addRadio("GUI Theme", themes, OFX_UI_ORIENTATION_VERTICAL);
    guiThemes->activateToggle("GRAYDAY");

    gui1->addSpacer();

    gui1->autoSizeToFitWidgets();
    gui1->setVisible(false);
    ofAddListener(gui1->newGUIEvent, this, &ofApp::guiEvent);
    guis.push_back(gui1);
}

//--------------------------------------------------------------
void ofApp::setupGUI2(){
    gui2 = new ofxUISuperCanvas("2: KINECT", 0, 0, guiWidth, ofGetHeight());
    gui2->setTheme(theme);

    gui2->addSpacer();
    gui2->addLabel("Press '2' to hide panel", OFX_UI_FONT_SMALL);

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

    gui2->autoSizeToFitWidgets();
    gui2->setVisible(false);
    ofAddListener(gui2->newGUIEvent, this, &ofApp::guiEvent);
    guis.push_back(gui2);
}

//--------------------------------------------------------------
void ofApp::setupGUI3(){
    gui3 = new ofxUISuperCanvas("3: GESTURE SEQUENCE", 0, 0, guiWidth, ofGetHeight());
    gui3->setTheme(theme);

    gui3->addSpacer();
    gui3->addLabel("Press '3' to hide panel", OFX_UI_FONT_SMALL);

    gui3->addSpacer();
    recordingSequence = gui3->addImageToggle("Record Sequence", "gui/icons/record.png", false, dim, dim);
    gui3->setWidgetPosition(OFX_UI_WIDGET_POSITION_RIGHT);
    gui3->addImageButton("Save Sequence", "gui/icons/save.png", false, dim, dim);
    gui3->addImageButton("Load Sequence", "gui/icons/open.png", false, dim, dim);
    gui3->addImageToggle("Play Sequence", "gui/icons/play.png", &drawSequence, dim, dim);
    gui3->setWidgetPosition(OFX_UI_WIDGET_POSITION_DOWN);

    sequenceFilename = gui3->addLabel("Filename: "+sequence.filename, OFX_UI_FONT_SMALL);
    sequenceDuration = gui3->addLabel("Duration: "+ofToString(sequence.duration, 2) + " s", OFX_UI_FONT_SMALL);
    sequenceNumFrames = gui3->addLabel("Number of frames: "+ofToString(sequence.numFrames), OFX_UI_FONT_SMALL);


    gui3->addSpacer();
    gui3->addToggle("Show gesture patterns", &drawPatterns);

    gui3->addSpacer();

    gui3->autoSizeToFitWidgets();
    gui3->setVisible(false);
    ofAddListener(gui3->newGUIEvent, this, &ofApp::guiEvent);
    guis.push_back(gui3);
}

//--------------------------------------------------------------
void ofApp::setupGUI4(){
    gui4 = new ofxUISuperCanvas("4: GESTURE TRACKER", 0, 0, guiWidth, ofGetHeight());
    gui4->setTheme(theme);

    gui4->addSpacer();
    gui4->addLabel("Press '4' to hide panel", OFX_UI_FONT_SMALL);

    gui4->addSpacer();
    gui4->addImageButton("Start vmo", "gui/icons/play.png", false, dim, dim);
    gui4->setWidgetPosition(OFX_UI_WIDGET_POSITION_RIGHT);
    gui4->addImageButton("Stop vmo", "gui/icons/delete.png", false, dim, dim);
    gui4->setWidgetPosition(OFX_UI_WIDGET_POSITION_DOWN);

    gui4->addSpacer();
    gui4->addToggle("Show gesture patterns", &drawPatterns);

    gui4->addSpacer();

    gui4->autoSizeToFitWidgets();
    gui4->setVisible(false);
    ofAddListener(gui4->newGUIEvent, this, &ofApp::guiEvent);
    guis.push_back(gui4);
}

//--------------------------------------------------------------
void ofApp::setupGUI5(){
    gui5 = new ofxUISuperCanvas("5: FLUID SOLVER", 0, 0, guiWidth, ofGetHeight());
    gui5->setTheme(theme);

    gui5->addSpacer();
    gui5->addLabel("Press '5' to hide panel", OFX_UI_FONT_SMALL);

    gui5->addSpacer();
    gui5->addLabel("Physics");
    gui5->addSlider("Friction", 0, 100, &markersParticles.friction);

    gui5->addSpacer();

    gui5->autoSizeToFitWidgets();
    gui5->setVisible(false);
    ofAddListener(gui5->newGUIEvent, this, &ofApp::guiEvent);
    guis.push_back(gui5);
}

//--------------------------------------------------------------
void ofApp::setupGUI6(int i){
    gui6 = new ofxUISuperCanvas("6: PARTICLES", 0, 0, guiWidth, ofGetHeight());
    gui6->setTheme(theme);

    gui6->addSpacer();
    gui6->addLabel("Press '6' to hide panel", OFX_UI_FONT_SMALL);

    gui6->addSpacer();
    gui6->addLabel("Emitter");
    gui6->addSlider("Particles/sec", 0.0, 20.0, &markersParticles.bornRate);

    // vector<string> types;
    // types.push_back("Point");
    // types.push_back("Grid");
    // types.push_back("Contour");
    // gui6->addLabel("Emitter type:", OFX_UI_FONT_SMALL);
    // gui6->addRadio("Emitter type", types, OFX_UI_ORIENTATION_VERTICAL);

    gui6->addSlider("Velocity", 0.0, 100.0, &markersParticles.velocity);
    gui6->addSlider("Velocity Random[%]", 0.0, 100.0, &markersParticles.velocityRnd);
    gui6->addSlider("Velocity from Motion[%]", 0.0, 100.0, &markersParticles.velocityMotion);

    gui6->addSlider("Emitter size", 0.0, 60.0, &markersParticles.emitterSize);

    gui6->addSpacer();
    gui6->addLabel("Particle");
    gui6->addToggle("Immortal", &markersParticles.immortal);
    gui6->addSlider("Lifetime", 0.0, 20.0, &markersParticles.lifetime);
    gui6->addSlider("Life Random[%]", 0.0, 100.0, &markersParticles.lifetimeRnd);
    gui6->addSlider("Radius", 1.0, 15.0, &markersParticles.radius);
    gui6->addSlider("Radius Random[%]", 0.0, 100.0, &markersParticles.radiusRnd);

    gui6->addSpacer();
    gui6->addLabel("Time behaviour");
    gui6->addToggle("Size", &markersParticles.sizeAge);
    gui6->addToggle("Opacity", &markersParticles.opacityAge);
    gui6->addToggle("Flickers", &markersParticles.flickersAge);
    gui6->addToggle("Color", &markersParticles.colorAge);
    gui6->addToggle("Bounce", &markersParticles.bounce);

    gui6->addSpacer();
    gui6->addLabel("Physics");
    gui6->addSlider("Friction", 0, 100, &markersParticles.friction);
    gui6->addSlider("Gravity", 0.0, 20.0, &markersParticles.gravity);

    gui6->addSpacer();

    gui6->autoSizeToFitWidgets();
    gui6->setVisible(false);
    ofAddListener(gui6->newGUIEvent, this, &ofApp::guiEvent);
    guis.push_back(gui6);
}

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
        if (button->getValue() == true){
            ofFileDialogResult result = ofSystemSaveDialog("sequence.xml", "Save sequence file");
            if (result.bSuccess){
                saveGUISettings(result.getPath());
            }
        }
    }

    if(e.getName() == "Load Settings"){
        ofxUIImageButton *button = (ofxUIImageButton *) e.widget;
        if (button->getValue() == true){
            ofFileDialogResult result = ofSystemLoadDialog("Select settings xml file.", false, "settings/");
            if (result.bSuccess){
                loadGUISettings(result.getPath());
            }
        }
    }

    if(e.getName() == "Record Sequence"){
        if (recordingSequence->getValue() == true){
            drawSequence = false;
            sequence.startRecording();
        }
    }

    if(e.getName() == "Save Sequence"){
        ofxUIImageButton *button = (ofxUIImageButton *) e.widget;
        if (button->getValue() == true){
            recordingSequence->setValue(false);
            drawSequence = false;
            ofFileDialogResult result = ofSystemSaveDialog("sequence.xml", "Save sequence file");
            if (result.bSuccess){
                sequence.save(result.getPath());
            }
        }
    }

    if(e.getName() == "Load Sequence"){
        ofxUIImageButton *button = (ofxUIImageButton *) e.widget;
        if (button->getValue() == true){
            recordingSequence->setValue(false);
            drawSequence = false;
            ofFileDialogResult result = ofSystemLoadDialog("Select sequence xml file.", false, "sequences/");
            if (result.bSuccess){
                sequence.load(result.getPath());
                sequenceFilename->setLabel("Filename: "+sequence.filename);
                sequenceDuration->setLabel("Duration: "+ofToString(sequence.duration, 2) + " s");
                sequenceNumFrames->setLabel("Number of frames: "+ofToString(sequence.numFrames));
            }
        }
    }

    if(e.getName() == "Play Sequence"){
        ofxUIImageToggle *toggle = (ofxUIImageToggle *) e.widget;
        if (toggle->getValue() == true){
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
        if (button->getValue() == true){
            initStatus = true;
            stopTracking = false;
        }
    }

    if(e.getName() == "Stop vmo"){
        ofxUIImageButton *button = (ofxUIImageButton *) e.widget;
        if (button->getValue() == true){
            stopTracking = true;
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

     if(e.getName() == "GUI Theme"){
        ofxUIRadio *radio = (ofxUIRadio *) e.widget;

        string name = radio->getActiveName();
        if (name == "DEFAULT")      theme = OFX_UI_THEME_DEFAULT;
        if (name == "HIPSTER")      theme = OFX_UI_THEME_HIPSTER;
        if (name == "GRAYDAY")      theme = OFX_UI_THEME_GRAYDAY;
        if (name == "RUSTIC")       theme = OFX_UI_THEME_RUSTIC;
        if (name == "LIMESTONE")    theme = OFX_UI_THEME_LIMESTONE;
        if (name == "VEGAN")        theme = OFX_UI_THEME_VEGAN;
        if (name == "BLUEBLUE")     theme = OFX_UI_THEME_BLUEBLUE;
        if (name == "COOLCLAY")     theme = OFX_UI_THEME_COOLCLAY;
        if (name == "SPEARMINT")    theme = OFX_UI_THEME_SPEARMINT;
        if (name == "PEPTOBISMOL")  theme = OFX_UI_THEME_PEPTOBISMOL;
        if (name == "MIDNIGHT")     theme = OFX_UI_THEME_MIDNIGHT;
        if (name == "BERLIN")       theme = OFX_UI_THEME_BERLIN;

        gui0->setTheme(theme);
        gui1->setTheme(theme);
        gui2->setTheme(theme);
        gui3->setTheme(theme);
        gui4->setTheme(theme);
        gui5->setTheme(theme);
        gui6->setTheme(theme);
     }

    if(e.getName() == "Immortal"){
        ofxUIToggle *toggle = (ofxUIToggle *) e.widget;
        if (toggle->getValue() == false) markersParticles.killParticles();
    }
}

//--------------------------------------------------------------
void ofApp::saveGUISettings(const string path){

    ofxXmlSettings *XML = new ofxXmlSettings();

    for(vector<ofxUISuperCanvas *>::iterator it = guis.begin(); it != guis.end(); ++it)
    {
        ofxUICanvas *g = *it;
        int guiIndex = XML->addTag("GUI");
        XML->pushTag("GUI", guiIndex);
        vector<ofxUIWidget*> widgets = g->getWidgets();
        for(int i = 0; i < widgets.size(); i++)
        {
            // kind number 20 is ofxUIImageToggle, for which we don't want to save the state
            if(widgets[i]->hasState() && widgets[i]->getKind() != 20){
                int index = XML->addTag("Widget");
                if(XML->pushTag("Widget", index))
                {
                    XML->setValue("Kind", widgets[i]->getKind(), 0);
                    XML->setValue("Name", widgets[i]->getName(), 0);
                    widgets[i]->saveState(XML);
                }
                XML->popTag();
            }
        }
        XML->popTag();
    }

    XML->saveFile(path);
    delete XML;
}

//--------------------------------------------------------------
void ofApp::loadGUISettings(const string path){
    ofxXmlSettings *XML = new ofxXmlSettings();
    if(!XML->loadFile(path)) return;
    int guiIndex = 0;

    for(vector<ofxUISuperCanvas *>::iterator it = guis.begin(); it != guis.end(); ++it)
    {
        ofxUICanvas *g = *it;
        XML->pushTag("GUI", guiIndex);
        int widgetTags = XML->getNumTags("Widget");
        for(int i = 0; i < widgetTags; i++)
        {
            XML->pushTag("Widget", i);
            string name = XML->getValue("Name", "NULL", 0);
            ofxUIWidget *widget = g->getWidget(name);
            if(widget != NULL && widget->hasState())
            {
                widget->loadState(XML);
                g->triggerEvent(widget);
            }
            XML->popTag();
        }
        guiIndex++;
        XML->popTag();
    }
    delete XML;
}

//--------------------------------------------------------------
void ofApp::exit(){
    kinect.close();
    kinect.clear();

    saveGUISettings("settings/lastSettings.xml");

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

            ofToggleFullscreen();
        case 'f':
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
//            drawPatterns = false;
            break;

        case '1':
            gui0->setVisible(false);
            gui1->toggleVisible();
            gui2->setVisible(false);
            gui3->setVisible(false);
            gui4->setVisible(false);
            gui5->setVisible(false);
            gui6->setVisible(false);
//            drawPatterns = false;
            break;

        case '2':
            gui0->setVisible(false);
            gui1->setVisible(false);
            gui2->toggleVisible();
            gui3->setVisible(false);
            gui4->setVisible(false);
            gui5->setVisible(false);
            gui6->setVisible(false);
//            drawPatterns = false;
            break;

        case '3':
            gui0->setVisible(false);
            gui1->setVisible(false);
            gui2->setVisible(false);
            gui3->toggleVisible();
            gui4->setVisible(false);
            gui5->setVisible(false);
            gui6->setVisible(false);
//            if(gui3->isVisible()) drawPatterns = true;
//            else drawPatterns = false;
            break;

        case '4':
            gui0->setVisible(false);
            gui1->setVisible(false);
            gui2->setVisible(false);
            gui3->setVisible(false);
            gui4->toggleVisible();
            gui5->setVisible(false);
            gui6->setVisible(false);
//            if(gui4->isVisible()) drawPatterns = true;
//            else drawPatterns = false;
            break;

        case '5':
            gui0->setVisible(false);
            gui1->setVisible(false);
            gui2->setVisible(false);
            gui3->setVisible(false);
            gui4->setVisible(false);
            gui5->toggleVisible();
            gui6->setVisible(false);
//            drawPatterns = false;
            break;

        case '6':
            gui0->setVisible(false);
            gui1->setVisible(false);
            gui2->setVisible(false);
            gui3->setVisible(false);
            gui4->setVisible(false);
            gui5->setVisible(false);
            gui6->toggleVisible();
//            drawPatterns = false;
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
