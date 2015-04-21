#include "ofApp.h"

using namespace ofxCv;
using namespace cv;

//--------------------------------------------------------------
void ofApp::setup(){

    ofSetFrameRate(60);
//    ofSetVerticalSync(false);
    
    #ifdef SECOND_WINDOW
        // the arguments for the second window are its initial x and y position,
        // and its width and height. the last argument is whether the window
        // should be undecorated, i.e. title bar is visible. setting it to
        // true removes the title bar.
        // running the command below positions an undecorated window to display on a second
        // monitor or projector. this is a good way to set up a fullscreen display, while
        // retaining a control window in the primary monitor.
        
        secondWindow.setup("second window", ofGetScreenWidth()+100, -50, 1280, 720, true);
        // secondWindow.setup("second window", 0, 0, 1024, 768, true);
        // secondWindow.setup("second window", ofGetScreenWidth(), 0, 1024, 768, false);
    #endif

    ofHideCursor(); // trick to show the cursor icon (see mouseMoved())

    // Number of IR markers
    numMarkers = 1;

    // Using a live kinect?
    #ifdef KINECT_CONNECTED
        // OPEN KINECT
        kinect.init(true); // shows infrared instead of RGB video Image
        kinect.open();
        kinect.setLed(ofxKinect::LED_OFF);

    // Kinect not connected
    #else
        // Use xml sequence marker file
        #ifdef KINECT_SEQUENCE
            kinectSequence.setup(numMarkers);
            kinectSequence.load("sequences/sequenceT2.xml");
        #endif // KINECT_SEQUENCE

        // Load png files from file
        ofDirectory dir;                    // directory lister
        dir.allowExt("jpg");

        string depthFolder = "kinect/depth1/";
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

        string irFolder = "kinect/ir1/";
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

    time0 = ofGetElapsedTimef();

    // BACKGROUND COLOR
    red = 0; green = 0; blue = 0;
    bgGradient = false;

    // ALLOCATE IMAGES
    depthImage.allocate(kinect.width, kinect.height, OF_IMAGE_GRAYSCALE);
    depthOriginal.allocate(kinect.width, kinect.height, OF_IMAGE_GRAYSCALE);
    grayThreshNear.allocate(kinect.width, kinect.height, OF_IMAGE_GRAYSCALE);
    grayThreshFar.allocate(kinect.width, kinect.height, OF_IMAGE_GRAYSCALE);
    irImage.allocate(kinect.width, kinect.height, OF_IMAGE_GRAYSCALE);
    irOriginal.allocate(kinect.width, kinect.height, OF_IMAGE_GRAYSCALE);

    croppingMask = Mat::ones(kinect.height, kinect.width, CV_8UC1);
    leftMask = 0; rightMask = kinect.width; topMask = 0; bottomMask = kinect.height;

    // FILTER PARAMETERS DEPTH IMAGE
    numDilates = 4;
    numErodes = 2;
    blurValue = 7;

    // KINECT PARAMETERS
    flipKinect      = false;

    nearClipping    = 500;
    farClipping     = 4000;

    nearThreshold   = 255;
    farThreshold    = 165;
    minContourSize  = 20.0;
    maxContourSize  = 250.0;

    irThreshold     = 70;
    minMarkerSize   = 5.0;
    maxMarkerSize   = 80.0;
    irMarkerFinder.setMinAreaRadius(minMarkerSize);
    irMarkerFinder.setMaxAreaRadius(maxMarkerSize);

    trackerPersistence = 200;
    trackerMaxDistance = 300;
    tracker.setPersistence(trackerPersistence);     // wait for 'trackerPersistence' frames before forgetting something
    tracker.setMaximumDistance(trackerMaxDistance); // an object can move up to 'trackerMaxDistance' pixels per frame

    // MARKER PARTICLES
    emitterParticles = new ParticleSystem();
    emitterParticles->setup(EMITTER, kinect.width, kinect.height);

    // GRID PARTICLES
    gridParticles = new ParticleSystem();
    gridParticles->setup(GRID, kinect.width, kinect.height);

    // BOIDS PARTICLES
    boidsParticles = new ParticleSystem();
    boidsParticles->setup(BOIDS, kinect.width, kinect.height);

    // ANIMATIONS PARTICLES
    animationsParticles = new ParticleSystem();
    animationsParticles->animation = RAIN;
    animationsParticles->setup(ANIMATIONS, kinect.width, kinect.height);

    // VECTOR OF PARTICLE SYSTEMS
    particleSystems.push_back(emitterParticles);
    particleSystems.push_back(gridParticles);
    particleSystems.push_back(boidsParticles);
    particleSystems.push_back(animationsParticles);
    currentParticleSystem = 0;

    // DEPTH CONTOUR
    contour.setup(kinect.width, kinect.height);
    contour.setMinAreaRadius(minContourSize);
    contour.setMaxAreaRadius(maxContourSize);
    
    // FLUID
    fluid.setup(contour.getFlowWidth(), contour.getFlowHeight(), kinect.width, kinect.height);
    
    // Adding constant forces
//    fluid.addConstantForce(ofPoint(kinect.width*0.5,kinect.height*0.85), ofPoint(0,-2), ofFloatColor(fluidRed,fluidGreen,fluidBlue)*fluidOpacity, fluidRadius);

    // SEQUENCE
    sequence.setup(numMarkers);
    sequence.load("sequences/sequenceT2.xml");
    drawSequence = false;
    drawSequenceSegments = false;

    // MARKERS
    drawMarkers = false;
    drawMarkersPath = false;

    // SONG
    song.loadSound("songs/ASuitableEnsemble.mp3", true);

    // VMO SETUP
    dimensions = 2;
    slide = 1.0;
    decay = 0.25;

    initStatus = true;
    isTracking = false;
    isConv = false; //Don`t try this, too slow.

    if(isConv){
		numElements = (numMarkers*dimensions+1)*(numMarkers*dimensions)/2;
		savedObs.assign(sequence.numFrames, vector<float>(numMarkers*dimensions));
		vmoObs.assign(sequence.numFrames, vector<float>(numElements));
		for(int markerIndex = 0; markerIndex < numMarkers; markerIndex++){
			for(int frameIndex = 0; frameIndex < sequence.numFrames; frameIndex++){
				savedObs[frameIndex][markerIndex*dimensions] = sequence.markersPosition[markerIndex][frameIndex].x;
				savedObs[frameIndex][markerIndex*dimensions+1] = sequence.markersPosition[markerIndex][frameIndex].y;
			}
		}

		vmoObs = covarianceMat(savedObs, numMarkers, dimensions);

		// 2. Processing
		// 2.1 Load file into VMO
		//    int minLen = 1; // Temporary setting
		//    float start = 0.0, step = 0.05, stop = 10.0;

		float start = 0.0, step = 0.05, stop = 5.0;

		float t = vmo::findThreshold(vmoObs, numElements, start, step, stop); // Temporary threshold range and step
		int minLen = 10;

		cout << t << endl;
		seqVmo = vmo::buildOracle(vmoObs, numElements, t);
		// 2.2 Output pattern list
		pttrList = vmo::findPttr(seqVmo, minLen);
		sequence.loadPatterns(processPttr(seqVmo, savedObs, pttrList, numMarkers, dimensions));
		drawSequencePatterns = false;
        drawSequencePatternsSeparate = false;
		cout << sequence.patterns.size() << endl;
	}
	else{
		numElements = numMarkers*dimensions;
		savedObs.assign(sequence.numFrames, vector<float>(numElements));
		for(int markerIndex = 0; markerIndex < numMarkers; markerIndex++){
			for(int frameIndex = 0; frameIndex < sequence.numFrames; frameIndex++){
				savedObs[frameIndex][markerIndex*dimensions] = sequence.markersPosition[markerIndex][frameIndex].x;
				savedObs[frameIndex][markerIndex*dimensions+1] = sequence.markersPosition[markerIndex][frameIndex].y;
			}
		}

//        // 2. Processing
//        // 2.1 Load file into VMO
//        int minLen = 1; // Temporary setting
//        float start = 0.0, step = 0.05, stop = 10.0;

		float start = 0.0, step = 0.01, stop = 10.0;

//        float t = vmo::findThreshold(savedObs, numElements, start, step, stop); // Temporary threshold range and step
//        float t = vmo::findThreshold(obs, dimensions, numMarkers, start, step, stop); // Temporary threshold range and step
//        int minLen = 2; // sequence.xml
//        float t = 12.3; // for sequence.xml
//
//        int minLen = 7; // sequence3.xml
//        float t = 18.6; // for sequence2.xml
//        float t = 16.8; // for sequence3.xml
//
//        int minLen = 7;
//        float t = 4.5; // for sequence1marker1.xml
//        int minLen = 10;
//        float t = 5.7; // for sequence1marker2.xml
//        int minLen = 10;
//        float t = 6.0; // for sequence1marker3.xml
//        int minLen = 2;
//        float t = 3.6; // for simple5.xml
        int minLen = 2;
        float t = 4.8; // for sequenceT2.xml

		cout << t << endl;

		seqVmo = vmo::buildOracle(savedObs, numElements, t);

		// 2.2 Output pattern list
		pttrList = vmo::findPttr(seqVmo, minLen);
		sequence.loadPatterns(processPttr(seqVmo, savedObs, pttrList, numMarkers, dimensions));
		drawSequencePatterns = false;
        drawSequencePatternsSeparate = false;
		cout << sequence.patterns.size() << endl;
	}
    pastObs.assign(numMarkers*dimensions, 0.0);
//    pastFeatures.assign(numElements, 0.0);
    currentFeatures.assign(numElements, 0.0);

    currentBf = vmo::vmo::belief();
//    prevBf = vmo::vmo::belief();
//
//    cout << "pattern size: "<<sequence.patterns.size() << endl;
//    for (int i = 0; i < pttrList.size; i++) {
//        cout << "pattern "<< i+1 << endl;
//        for (int j = 0; j<pttrList.sfxPts[i].size(); j++){
//            cout << "	begin: "<< pttrList.sfxPts[i][j] - pttrList.sfxLen[i]<< endl;
//            cout << "	end  :"<< pttrList.sfxPts[i][j] << endl;
//        }
//    }
    
    // SETUP GUI
    setupGUI();
    
    // ALLOCATE FBO AND FILL WITH BG COLOR
    fbo.allocate(kinect.width, kinect.height, GL_RGB32F_ARB);
    fbo.begin();
    ofClear(red, green, blue);
    fbo.end();

    fadeAmount = 40;
    useFBO = false;

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
    
    #ifdef SECOND_WINDOW
        windowWidth = secondWindow.getWidth();
        windowHeight = secondWindow.getHeight();
    #else
        windowWidth = ofGetWindowWidth();
        windowHeight = ofGetWindowHeight();
    #endif
    
    // Compute rescale value to scale kinect image
    reScale = (float)windowHeight / (float)kinect.height;
//    reScale = ofVec2f((float)ofGetWidth()/(float)kinect.width, (float)ofGetHeight()/(float)kinect.height); // deforms the image a little bit
//    reScale = ofVec2f((float)ofGetHeight()/(float)kinect.height, (float)ofGetHeight()/(float)kinect.height);

    // Update the sound playing system
    ofSoundUpdate();

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
        if(flipKinect) depthOriginal.mirror(false, true);
        ofImage *irImg = savedIrImages.at(i);
        irOriginal.setFromPixels(irImg->getPixels(), irImg->getWidth(), irImg->getHeight(), OF_IMAGE_GRAYSCALE);
        if(flipKinect) irOriginal.mirror(false, true);
    #endif // KINECT_CONNECTED

    // Nothing will happen here if the kinect is unplugged
    kinect.update();
    if(kinect.isFrameNew()){
        depthOriginal.setFromPixels(kinect.getDepthPixels(), kinect.getWidth(), kinect.getHeight(), OF_IMAGE_GRAYSCALE);
        if(flipKinect) depthOriginal.mirror(false, true);
        irOriginal.setFromPixels(kinect.getPixels(), kinect.getWidth(), kinect.getHeight(), OF_IMAGE_GRAYSCALE);
        if(flipKinect) irOriginal.mirror(false, true);
    }

    copy(irOriginal, irImage);
    copy(depthOriginal, depthImage);
    copy(depthOriginal, grayThreshNear);
    copy(depthOriginal, grayThreshFar);

    // Filter the IR image
    erode(irImage); // delete small white dots
    dilate(irImage);
    dilate(irImage);
    dilate(irImage);
    blur(irImage, 21);
    threshold(irImage, irThreshold);

    // Treshold and filter depth image
    threshold(grayThreshNear, nearThreshold, true);
    threshold(grayThreshFar, farThreshold);
    bitwise_and(grayThreshNear, grayThreshFar, depthImage);
    for(int i = 0; i < numDilates; i++){
        dilate(depthImage);
    }
    for(int i = 0; i < numErodes; i++){
        erode(depthImage);
    }
    blur(depthImage, blurValue);

    // Crop images
    Mat depthMat = toCv(depthImage);
    Mat irMat = toCv(irImage);
    Mat depthCropped = Mat::zeros(kinect.height, kinect.width, CV_8UC1);
    Mat irCropped = Mat::zeros(kinect.height, kinect.width, CV_8UC1);

    depthCropped = depthMat.mul(croppingMask);
    irCropped = irMat.mul(croppingMask);

    copy(depthCropped, depthImage);
    copy(irCropped, irImage);

    // Update images
    irImage.update();
    depthImage.update();

    // Contour Finder + marker tracker in the IR Image
    irMarkerFinder.findContours(irImage);
    tracker.track(irMarkerFinder.getBoundingRects());

    // Track markers
    vector<irMarker>& markers           = tracker.getFollowers();   // TODO: assign dead labels to new labels and have a MAX number of markers
    vector<unsigned int> deadLabels     = tracker.getDeadLabels();
    vector<unsigned int> currentLabels  = tracker.getCurrentLabels();
    // vector<unsigned int> newLabels      = tracker.getNewLabels();

    // Update markers if we loose track of them
    for(unsigned int i = 0; i < markers.size(); i++){
        markers[i].updateLabels(deadLabels, currentLabels);
    }

    // Record sequence when recording button is true
    if(recordingSequence->getValue() == true) sequence.record(markers);

    // Update contour
    contour.update(dt, depthImage);
    
    // Update fluid
    fluid.update(dt, markers, contour, mouseX, mouseY);

    // Update particles
    emitterParticles->update(dt, markers, contour);
    gridParticles->update(dt, markers, contour);
//    boidsParticles->update(dt, markers, contour);
//    animationsParticles->update(dt, markers, contour);

    #ifdef KINECT_SEQUENCE

        if(isTracking){
            vector<float> obs(numMarkers*dimensions, 0.0); // Temporary code
            for(unsigned int i = 0; i < kinectSequence.getNumMarkers(); i++){
                ofPoint currentPoint = kinectSequence.getCurrentPoint(i);
				// Use the lowpass here??
				obs[i] = lowpass(currentPoint.x, pastObs[i], slide);
				obs[i+1] = lowpass(currentPoint.y, pastObs[i+1], slide);
				pastObs[i] = obs[i];
				pastObs[i+1] = obs[i+1];

                //obs[i] = currentPoint.x;
                //obs[i+1] = currentPoint.y;
            }

			if (isConv) currentFeatures = cov_cal(pastObs, obs, numElements);
			else currentFeatures = obs;

            if(initStatus){
				pastObs = obs;
//				pastFeatures.assign(numElements, 0.0);
                currentBf = vmo::tracking_init(seqVmo, currentBf, pttrList, currentFeatures);
                initStatus = false;
            }
            else{
//				prevBf = currentBf;
				currentBf = vmo::tracking(seqVmo, currentBf, pttrList, currentFeatures, decay);
                cout << "current index: " << currentBf.currentIdx << endl;
                currentPercent = sequence.getCurrentPercent(currentBf.currentIdx);
                cout << "current percent: " << currentPercent << endl;

                if(cueList.size() != 0) {
                    int cueSegment = currentCueIndex;
                    for(int i = 0; i < cueSliders.size(); i++){
                        float low = cueSliders.at(i).second->getValueLow()/100.0;
                        float high = cueSliders.at(i).second->getValueHigh()/100.0;
                        if (low <= currentPercent && currentPercent <= high){
                            cueSegment = i;
                            break;
                        }
                    }
                    // If tracking idx from sequence belongs to another cue different than the current
                    // we interpolate settings to this other cue
                    if(currentCueIndex != cueSegment){
                        currentCueIndex = cueSegment;
                        loadGUISettings(cueList[currentCueIndex], true, true);
                        string cueFileName = ofFilePath::getBaseName(cueList[currentCueIndex]);
                        cueIndexLabel->setLabel(ofToString(currentCueIndex)+".");
                        cueName->setTextString(cueFileName);
                    }
                }
            }
            gestureUpdate = getGestureUpdate(currentBf.currentIdx, seqVmo, pttrList, sequence);
            for (int i = 0; i < sequence.patterns.size(); i++) {
                if(gestureUpdate.find(i) != gestureUpdate.end()) {
                    cout << "key: "<< i << endl;
                    cout << "percent:"<< gestureUpdate[i] << endl;
                }
            }
        }

    #else
        if(markers.size()>0){
            if(isTracking){
                vector<float> obs(numMarkers*dimensions, 0.0); // Temporary code
                int numObs = 0;
                for(unsigned int i = 0; i < markers.size(); i++){
                    // If we have already filled numMarkers observations jump to tracking
                    if(numObs == numMarkers*dimensions) break;
                    // If marker has disappeared but we have more markers to fill numMarkers jump to the next marker
                    if(markers[i].hasDisappeared && (markers.size() - i) > numMarkers) continue;

                    ofPoint currentPoint = markers[i].smoothPos;

                    // Use the lowpass here??
                    obs[i] = lowpass(currentPoint.x, pastObs[i], slide);
                    obs[i+1] = lowpass(currentPoint.y, pastObs[i+1], slide);
                    pastObs[i] = obs[i];
                    pastObs[i+1] = obs[i+1];

                    numObs += dimensions;

                    //obs[i] = currentPoint.x;
                    //obs[i+1] = currentPoint.y;
                }

                if (isConv) currentFeatures = cov_cal(pastObs, obs, numElements);
                else currentFeatures = obs;

                if(initStatus){
                    pastObs = obs;
    //				pastFeatures.assign(numElements, 0.0);
                    currentBf = vmo::tracking_init(seqVmo, currentBf, pttrList, currentFeatures);
                    initStatus = false;
                }
                else{
    //				prevBf = currentBf;
                    currentBf = vmo::tracking(seqVmo, currentBf, pttrList, currentFeatures, decay);
                    cout << "current index: " << currentBf.currentIdx << endl;
                    currentPercent = sequence.getCurrentPercent(currentBf.currentIdx);
                    cout << "current percent: " << currentPercent << endl;

                    if(cueList.size() != 0) {
                        int cueSegment = currentCueIndex;
                        for(int i = 0; i < cueSliders.size(); i++){
                            float low = cueSliders.at(i).second->getValueLow()/100.0;
                            float high = cueSliders.at(i).second->getValueHigh()/100.0;
                            if (low <= currentPercent && currentPercent <= high){
                                cueSegment = i;
                                break;
                            }
                        }
                        // If tracking idx from sequence belongs to another cue different than the current
                        // we interpolate settings to this other cue
                        if(currentCueIndex != cueSegment){
                            currentCueIndex = cueSegment;
                            loadGUISettings(cueList[currentCueIndex], true, true);
                            string cueFileName = ofFilePath::getBaseName(cueList[currentCueIndex]);
                            cueIndexLabel->setLabel(ofToString(currentCueIndex)+".");
                            cueName->setTextString(cueFileName);
                        }

                    }
                }
                gestureUpdate = getGestureUpdate(currentBf.currentIdx, seqVmo, pttrList, sequence);
                for (int i = 0; i < sequence.patterns.size(); i++) {
                    if(gestureUpdate.find(i) != gestureUpdate.end()) {
                        cout << "key: "<< i << endl;
                        cout << "percent:"<< gestureUpdate[i] << endl;
                    }
                }
            }
        }

    #endif // KINECT_SEQUENCE
}

//--------------------------------------------------------------
void ofApp::draw(){
    
    #ifdef SECOND_WINDOW
        secondWindow.begin();
    #endif
    
    ofPushMatrix();
    ofPushStyle();
    
    ofColor contourBg(red, green, blue);
    ofColor centerBg(red, green, blue);
    if(bgGradient){
        if(centerBg.getBrightness() > 0) contourBg.setBrightness(ofMap(centerBg.getBrightness(), 0.0, 255.0, 20.0, 130.0));
        ofBackgroundGradient(centerBg, contourBg);
    }
    else ofBackground(centerBg);
    
    ofRectangle canvasRect(0, 0, windowWidth, windowHeight);
    ofRectangle kinectRect(0, 0, kinect.width, kinect.height);
    kinectRect.scaleTo(canvasRect, OF_SCALEMODE_FIT);
    ofTranslate(kinectRect.x, kinectRect.y);
    ofScale(reScale, reScale);
    
//    ofScale(reScale.x, reScale.y);

//    // Kinect images
//    irImage.draw(0, 0);
//    depthImage.draw(0, 0);

    if(useFBO){
        fbo.begin();

        // Draw semi-transparent white rectangle to slightly clear buffer (depends on the history value)
        ofSetColor(red, green, blue, fadeAmount);
        ofFill();
        ofRectangle(0, 0, kinect.width, kinect.height);

        // Graphics
        ofSetColor(255);
        contour.draw();
        emitterParticles->draw();
        gridParticles->draw();
        boidsParticles->draw();
        animationsParticles->draw();

        fbo.end();

        // Draw buffer (graphics) on the screen
        ofSetColor(255);
        fbo.draw(0, 0);
    }
    else{
        // Draw Graphics
        contour.draw();
        fluid.draw();
        emitterParticles->draw();
        gridParticles->draw();
        boidsParticles->draw();
        animationsParticles->draw();

//        ofTexture& flowTexture = contour.getFlowTexture();
//        int w = flowTexture.getWidth();
//        int h = flowTexture.getHeight();
//        
//        ofSetColor(255, 255, 255);
//        if(flowTexture.isAllocated())
//            flowTexture.draw(0, 0);
//        
//        ofFloatPixels flowPixels;
//        flowTexture.readToPixels(flowPixels);
//        
//        ofSetLineWidth(1.5);
//        ofSetColor(255, 0, 0);
//        
//        // color pixels, use w and h to control red and green
//        for (int i = 0; i < w; i+=5){
//            for (int j = 0; j < h; j+=5){
//                ofVec2f vel;
//                vel.x = flowPixels[(j*w+i)*3 + 0]; // r
//                vel.y = flowPixels[(j*w+i)*3 + 1]; // g
//                ofLine(ofVec2f(i, j), ofVec2f(i, j)+vel);
//            }
//        }
        
    }

    if(drawMarkers || drawMarkersPath){
//        irMarkerFinder.draw();
        vector<irMarker>& markers = tracker.getFollowers();
        // Draw identified IR markers
        for (int i = 0; i < markers.size(); i++){
            if(drawMarkers) markers[i].draw();
            if(drawMarkersPath) markers[i].drawPath();
        }
    }

    #ifdef KINECT_SEQUENCE
        kinectSequence.draw();
    #endif // KINECT_SEQUENCE

    if(drawSequence) sequence.draw();
    if(drawSequenceSegments) sequence.drawSegments();
    if(drawSequencePatterns) sequence.drawPatterns(gestureUpdate);
    if(isTracking) sequence.drawTracking(currentBf.currentIdx);
    
    ofPopStyle();
    ofPopMatrix();
    
    #ifdef SECOND_WINDOW
        secondWindow.end();
        ofBackground(0);
    #endif
    
    if(drawSequencePatternsSeparate) sequence.drawPatternsSeparate(gestureUpdate);
}

//--------------------------------------------------------------
void ofApp::setupGUI(){
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
    
    dim = 32;
    guiWidth = 240;
    guiMargin = 2;
    
    setupHelperGUI();
    setupBasicsGUI();
    setupKinectGUI();
    setupGesturesGUI();
    setupCueListGUI();
    setupOpticalFlowGUI();
    setupFluidSolverGUI();
    setupFluidExtrasGUI();
    setupContourGUI();
    setupEmitterGUI();
    setupGridGUI();
    setupBoidsGUI();
    setupAnimationsGUI();
    
    interpolatingWidgets = false;
    nInterpolatedFrames = 0;
    maxTransitionFrames = 20;
    loadGUISettings("settings/lastSettings.xml", false, false);
}

//--------------------------------------------------------------
void ofApp::setupHelperGUI(){
    ofxUICanvas *guiHelper = new ofxUICanvas(0, 0, guiWidth, ofGetHeight());
    guiHelper->setUIColors(uiThemecb, uiThemeco, uiThemecoh, uiThemecf, uiThemecfh, uiThemecp, uiThemecpo);
    
    guiHelper->addLabel("MENU", OFX_UI_FONT_LARGE);

    guiHelper->addSpacer();
    guiHelper->addLabel("Press number 1 to 7 to", OFX_UI_FONT_SMALL);
    guiHelper->addLabel("switch between GUI panels", OFX_UI_FONT_SMALL);
    
    guiHelper->addSpacer();
    guiHelper->addLabel("Press 'f' to fullscreen.", OFX_UI_FONT_SMALL);
    guiHelper->addSpacer();
    
    guiHelper->addSpacer();
    guiHelper->addFPS(OFX_UI_FONT_SMALL);
    guiHelper->addSpacer();

    guiHelper->addSpacer();
    guiHelper->addLabel("1. BASICS + KINECT", OFX_UI_FONT_MEDIUM);
    guiHelper->addSpacer();
    
    guiHelper->addSpacer();
    guiHelper->addLabel("2. GESTURES + CUE LIST", OFX_UI_FONT_MEDIUM);
    guiHelper->addSpacer();

    guiHelper->addSpacer();
    guiHelper->addLabel("3. FLOW + FLUID SOLVER", OFX_UI_FONT_MEDIUM);
    guiHelper->addSpacer();
    
    guiHelper->addSpacer();
    guiHelper->addLabel("4. FLUID EXTRA + CONTOUR", OFX_UI_FONT_MEDIUM);
    guiHelper->addSpacer();
    
    guiHelper->addSpacer();
    guiHelper->addLabel("5. PARTICLE EMITTER", OFX_UI_FONT_MEDIUM);
    guiHelper->addSpacer();
    
    guiHelper->addSpacer();
    guiHelper->addLabel("6. PARTICLE GRID", OFX_UI_FONT_MEDIUM);
    guiHelper->addSpacer();
    
    guiHelper->addSpacer();
    guiHelper->addLabel("7. PARTICLE BOIDS", OFX_UI_FONT_MEDIUM);
    guiHelper->addSpacer();
    
    guiHelper->addSpacer();
    guiHelper->addLabel("8. PARTICLE ANIMATIONS", OFX_UI_FONT_MEDIUM);
    guiHelper->addSpacer();
    
//    guiHelper->autoSizeToFitWidgets();
    ofAddListener(guiHelper->newGUIEvent, this, &ofApp::guiEvent);
    guis.push_back(guiHelper);
}

void ofApp::setupBasicsGUI(){
    ofxUICanvas *guiBasics = new ofxUICanvas(guiWidth+guiMargin, 0, guiWidth, ofGetHeight());
    guiBasics->addLabel("BASICS", OFX_UI_FONT_LARGE);
    guiBasics->setUIColors(uiThemecb, uiThemeco, uiThemecoh, uiThemecf, uiThemecfh, uiThemecp, uiThemecpo);

    guiBasics->addSpacer();
    guiBasics->addLabel("Background", OFX_UI_FONT_MEDIUM);
    guiBasics->addSpacer();
    ofxUISlider *redSlider = guiBasics->addSlider("Red", 0.0, 255.0, &red);
    redSlider->setColorFill(ofColor(240, 30, 30));
    redSlider->setColorFillHighlight(ofColor(150, 30, 30));
    ofxUISlider *greenSlider = guiBasics->addSlider("Green", 0.0, 255.0, &green);
    greenSlider->setColorFill(ofColor(30, 240, 30));
    greenSlider->setColorFillHighlight(ofColor(30, 150, 30));
    ofxUISlider *blueSlider = guiBasics->addSlider("Blue", 0.0, 255.0, &blue);
    blueSlider->setColorFill(ofColor(30, 30, 240));
    blueSlider->setColorFillHighlight(ofColor(30, 30, 150));
    guiBasics->addToggle("Gradient", &bgGradient);

    guiBasics->addSpacer();
    guiBasics->addLabel("Settings", OFX_UI_FONT_MEDIUM);
    guiBasics->addSpacer();
    guiBasics->addImageButton("Save Settings", "icons/save.png", false, dim, dim)->setColorBack(ofColor(150, 255));
    guiBasics->setWidgetPosition(OFX_UI_WIDGET_POSITION_RIGHT);
    guiBasics->addImageButton("Load Settings", "icons/open.png", false, dim, dim)->setColorBack(ofColor(150, 255));
    guiBasics->addImageButton("Reset Settings", "icons/reset.png", false, dim, dim)->setColorBack(ofColor(150, 255));
    guiBasics->setWidgetPosition(OFX_UI_WIDGET_POSITION_DOWN);
    settingsFilename = guiBasics->addLabel("lastSettings.xml", OFX_UI_FONT_SMALL);

    guiBasics->addSpacer();
    guiBasics->addLabel("Interpolation", OFX_UI_FONT_MEDIUM);
    guiBasics->addSpacer();
    guiBasics->addIntSlider("Transition Frames", 0, 200, &maxTransitionFrames);
    
    guiBasics->addSpacer();
    guiBasics->addLabel("FBO", OFX_UI_FONT_MEDIUM);
    guiBasics->addSpacer();
    guiBasics->addToggle("Use FBO", &useFBO);
    guiBasics->addIntSlider("FBO fade amount", 0, 50, &fadeAmount);

    guiBasics->addSpacer();
    guiBasics->addLabel("Music", OFX_UI_FONT_MEDIUM);
    guiBasics->addSpacer();
    guiBasics->addImageButton("Play Song", "icons/play.png", false, dim, dim)->setColorBack(ofColor(150, 255));
    guiBasics->setWidgetPosition(OFX_UI_WIDGET_POSITION_RIGHT);
    guiBasics->addImageButton("Stop Song", "icons/delete.png", false, dim, dim)->setColorBack(ofColor(150, 255)); // TODO: create stop icon
    guiBasics->addImageToggle("Loop Song", "icons/reset.png", true, dim, dim)->setColorBack(ofColor(150, 255));
    guiBasics->addImageButton("Load Song", "icons/open.png", false, dim, dim)->setColorBack(ofColor(150, 255));
    guiBasics->setWidgetPosition(OFX_UI_WIDGET_POSITION_DOWN);
    songFilename = guiBasics->addLabel("ASuitableEnsemble.mp3", OFX_UI_FONT_SMALL);

    guiBasics->addSpacer();

    guiBasics->autoSizeToFitWidgets();
    ofAddListener(guiBasics->newGUIEvent, this, &ofApp::guiEvent);
    guis.push_back(guiBasics);
}

//--------------------------------------------------------------
void ofApp::setupKinectGUI(){
    ofxUICanvas *guiKinect_1 = new ofxUICanvas((guiWidth+guiMargin)*2, 0, guiWidth, ofGetHeight());
    guiKinect_1->addLabel("KINECT", OFX_UI_FONT_LARGE);
    guiKinect_1->setUIColors(uiThemecb, uiThemeco, uiThemecoh, uiThemecf, uiThemecfh, uiThemecp, uiThemecpo);

    guiKinect_1->addSpacer();
    guiKinect_1->addLabel("'Up' and 'down' keys to tilt", OFX_UI_FONT_SMALL);

    guiKinect_1->addSpacer();
    guiKinect_1->addLabelButton("Reset Kinect", &resetKinect);

    guiKinect_1->addSpacer();
    guiKinect_1->addLabelToggle("Flip Kinect", &flipKinect);

    guiKinect_1->addSpacer();
    guiKinect_1->addLabel("Depth Image", OFX_UI_FONT_MEDIUM);
    guiKinect_1->addSpacer();
    guiKinect_1->addRangeSlider("Clipping range", 500, 5000, &nearClipping, &farClipping);
    guiKinect_1->addRangeSlider("Threshold range", 0.0, 255.0, &farThreshold, &nearThreshold);
    guiKinect_1->addRangeSlider("Contour size", 0.0, 400.0, &minContourSize, &maxContourSize);

    guiKinect_1->addRangeSlider("Left/Right Crop", 0.0, 640.0, &leftMask, &rightMask);
    guiKinect_1->addRangeSlider("Top/Bottom Crop", 0.0, 480.0, &topMask, &bottomMask);

    guiKinect_1->addImage("Depth original", &depthOriginal, kinect.width/6, kinect.height/6, true);
    guiKinect_1->setWidgetPosition(OFX_UI_WIDGET_POSITION_RIGHT);
    guiKinect_1->addImage("Depth filtered", &depthImage, kinect.width/6, kinect.height/6, true);
    guiKinect_1->setWidgetPosition(OFX_UI_WIDGET_POSITION_DOWN);
    
    guiKinect_1->addIntSlider("Erode", 0, 8, &numErodes);
    guiKinect_1->addIntSlider("Dilate", 0, 8, &numDilates);
    guiKinect_1->addIntSlider("Blur", 0, 41, &blurValue);
    guiKinect_1->addSpacer();

    guiKinect_1->autoSizeToFitWidgets();
    ofAddListener(guiKinect_1->newGUIEvent, this, &ofApp::guiEvent);
    guis.push_back(guiKinect_1);
    
    ofxUICanvas *guiKinect_2 = new ofxUICanvas((guiWidth+guiMargin)*3, 0, guiWidth, ofGetHeight());
    guiKinect_2->setUIColors(uiThemecb, uiThemeco, uiThemecoh, uiThemecf, uiThemecfh, uiThemecp, uiThemecpo);
    guiKinect_2->addSpacer();
    guiKinect_2->addLabel("Infrared Image", OFX_UI_FONT_MEDIUM);
    guiKinect_2->addSpacer();
    guiKinect_2->addSlider("IR Threshold", 0.0, 255.0, &irThreshold);
    guiKinect_2->addRangeSlider("Markers size", 0.0, 150.0, &minMarkerSize, &maxMarkerSize);
    guiKinect_2->addSlider("Tracker persistence", 5.0, 500.0, &trackerPersistence);
    guiKinect_2->addSlider("Tracker max distance", 5.0, 500.0, &trackerMaxDistance);
    guiKinect_2->addImage("IR original", &irOriginal, kinect.width/6, kinect.height/6, true);
    guiKinect_2->setWidgetPosition(OFX_UI_WIDGET_POSITION_RIGHT);
    guiKinect_2->addImage("IR filtered", &irImage, kinect.width/6, kinect.height/6, true);
    guiKinect_2->setWidgetPosition(OFX_UI_WIDGET_POSITION_DOWN);
    
    guiKinect_2->addSpacer();
    guiKinect_2->addLabel("Debugging", OFX_UI_FONT_MEDIUM);
    guiKinect_2->addSpacer();
    guiKinect_2->addToggle("Show Markers", &drawMarkers);
    guiKinect_2->addToggle("Show Markers Path", &drawMarkersPath);
    guiKinect_2->addSpacer();

    guiKinect_2->autoSizeToFitWidgets();
    ofAddListener(guiKinect_2->newGUIEvent, this, &ofApp::guiEvent);
    guis.push_back(guiKinect_2);
}

//--------------------------------------------------------------
void ofApp::setupGesturesGUI(){
    ofxUICanvas *guiGestures_1 = new ofxUICanvas(guiWidth+guiMargin, 0, guiWidth, ofGetHeight());
    guiGestures_1->addLabel("GESTURE SEQUENCE", OFX_UI_FONT_LARGE);
    guiGestures_1->setUIColors(uiThemecb, uiThemeco, uiThemecoh, uiThemecf, uiThemecfh, uiThemecp, uiThemecpo);

    guiGestures_1->addSpacer();
    recordingSequence = guiGestures_1->addImageToggle("Record Sequence", "icons/record.png", false, dim, dim);
    recordingSequence->setColorBack(ofColor(150, 255));
    guiGestures_1->setWidgetPosition(OFX_UI_WIDGET_POSITION_RIGHT);
    guiGestures_1->addImageButton("Save Sequence", "icons/save.png", false, dim, dim)->setColorBack(ofColor(150, 255));
    guiGestures_1->addImageButton("Load Sequence", "icons/open.png", false, dim, dim)->setColorBack(ofColor(150, 255));
    guiGestures_1->addImageToggle("Play Sequence", "icons/play.png", &drawSequence, dim, dim)->setColorBack(ofColor(150, 255));
    guiGestures_1->setWidgetPosition(OFX_UI_WIDGET_POSITION_DOWN);
//    guiGestures_1->addIntSlider("Number of markers", 1, 4, &numMarkers);

    guiGestures_1->addSpacer();
    sequenceFilename = guiGestures_1->addLabel("Filename: "+sequence.filename, OFX_UI_FONT_SMALL);
    sequenceDuration = guiGestures_1->addLabel("Duration: "+ofToString(sequence.duration, 2) + " s", OFX_UI_FONT_SMALL);
    sequenceNumFrames = guiGestures_1->addLabel("Number of frames: "+ofToString(sequence.numFrames), OFX_UI_FONT_SMALL);
    sequenceNumMarkers = guiGestures_1->addLabel("Number of markers: "+ofToString(sequence.getNumMarkers()), OFX_UI_FONT_SMALL);

    guiGestures_1->addSpacer();
    guiGestures_1->addLabel("CUE MAPPING", OFX_UI_FONT_LARGE);
    guiGestures_1->addSpacer();
    guiGestures_1->addToggle("Show sequence segmentation", &drawSequenceSegments);
    guiGestures_1->addSpacer();
    
    guiGestures_1->autoSizeToFitWidgets();
    guiGestures_1->setVisible(false);
    ofAddListener(guiGestures_1->newGUIEvent, this, &ofApp::guiEvent);
    guis.push_back(guiGestures_1);

    ofxUICanvas *guiGestures_2 = new ofxUICanvas((guiWidth+guiMargin)*2, 0, guiWidth, ofGetHeight());
    guiGestures_2->addLabel("GESTURE TRACKER", OFX_UI_FONT_LARGE);
    guiGestures_2->setUIColors(uiThemecb, uiThemeco, uiThemecoh, uiThemecf, uiThemecfh, uiThemecp, uiThemecpo);
    guiGestures_2->addSpacer();
    guiGestures_2->addImageButton("Start vmo", "icons/play.png", false, dim, dim)->setColorBack(ofColor(150, 255));
    guiGestures_2->setWidgetPosition(OFX_UI_WIDGET_POSITION_RIGHT);
    guiGestures_2->addImageButton("Stop vmo", "icons/delete.png", false, dim, dim)->setColorBack(ofColor(150, 255));
    guiGestures_2->setWidgetPosition(OFX_UI_WIDGET_POSITION_DOWN);
    guiGestures_2->addSpacer();
    guiGestures_2->addLabel("File: " + sequence.filename, OFX_UI_FONT_SMALL);
    guiGestures_2->addLabel("Patterns: " + ofToString(sequence.patterns.size()), OFX_UI_FONT_SMALL);
    trackingInfoLabel = guiGestures_2->addLabel(" ", OFX_UI_FONT_SMALL);
    guiGestures_2->addSpacer();
    guiGestures_2->addSlider("Decay", 0.01, 1.0, &decay)->setLabelPrecision(2);
    guiGestures_2->addSlider("Slide", 1.0, 30.0, &slide);
    guiGestures_2->addSpacer();
    guiGestures_2->addToggle("Show patterns inside sequence", &drawSequencePatterns);
    guiGestures_2->addToggle("Show patterns in the side", &drawSequencePatternsSeparate);

    guiGestures_2->addSpacer();
    
    guiGestures_2->autoSizeToFitWidgets();
    guiGestures_2->setVisible(false);
    ofAddListener(guiGestures_2->newGUIEvent, this, &ofApp::guiEvent);
    guis.push_back(guiGestures_2);
}

//--------------------------------------------------------------
void ofApp::setupCueListGUI(){
    ofxUICanvas *guiCueList = new ofxUICanvas((guiWidth+guiMargin)*3, 0, guiWidth, ofGetHeight());
    guiCueList->addLabel("CUE LIST", OFX_UI_FONT_LARGE);
    guiCueList->setUIColors(uiThemecb, uiThemeco, uiThemecoh, uiThemecf, uiThemecfh, uiThemecp, uiThemecpo);

    guiCueList->addSpacer();
    guiCueList->addLabel("Press arrow keys to navigate", OFX_UI_FONT_SMALL);
    guiCueList->addLabel("through cues and space key", OFX_UI_FONT_SMALL);
    guiCueList->addLabel("to trigger next cue in the list.", OFX_UI_FONT_SMALL);

    guiCueList->addSpacer();
    guiCueList->setWidgetPosition(OFX_UI_WIDGET_POSITION_DOWN, OFX_UI_ALIGN_CENTER);
    currentCueIndex = -1;
    cueIndexLabel = guiCueList->addLabel("", OFX_UI_FONT_MEDIUM);
    guiCueList->setWidgetFontSize(OFX_UI_FONT_MEDIUM);
    cueName = guiCueList->addTextInput("Cue Name", "");
    cueName->setAutoClear(false);
    if(cueList.size() == 0) cueName->setVisible(false);

    guiCueList->setWidgetPosition(OFX_UI_WIDGET_POSITION_DOWN);
    guiCueList->addSpacer();
    guiCueList->addImageButton("New Cue", "icons/add.png", false, dim, dim)->setColorBack(ofColor(150, 255));
    guiCueList->setWidgetPosition(OFX_UI_WIDGET_POSITION_RIGHT);
    guiCueList->addImageButton("Save Cue", "icons/save.png", false, dim, dim)->setColorBack(ofColor(150, 255));

    ofxUIButton *previous;
    previous = guiCueList->addImageButton("Previous Cue", "icons/previous.png", false, dim, dim);
//    previous->bindToKey(OF_KEY_LEFT);
    previous->setColorBack(ofColor(150, 255));

    ofxUIButton *next;
    next = guiCueList->addImageButton("Next Cue", "icons/play.png", false, dim, dim);
//    next->bindToKey(OF_KEY_RIGHT);
    next->setColorBack(ofColor(150, 255));

    guiCueList->addImageButton("Load Cue", "icons/open.png", false, dim, dim)->setColorBack(ofColor(150, 255));
    guiCueList->addImageButton("Delete Cue", "icons/delete.png", false, dim, dim)->setColorBack(ofColor(150, 255));

    guiCueList->setWidgetPosition(OFX_UI_WIDGET_POSITION_DOWN, OFX_UI_ALIGN_CENTER);

    guiCueList->addSpacer();
//    guiCueList->addLabelButton("GO", false, 230, 40)->bindToKey(' ');
    guiCueList->addLabelButton("GO", false, 230, 40);

    guiCueList->addSpacer();

    guiCueList->autoSizeToFitWidgets();
    guiCueList->setVisible(false);
    ofAddListener(guiCueList->newGUIEvent, this, &ofApp::guiEvent);
    guis.push_back(guiCueList);
}

void ofApp::setupOpticalFlowGUI(){
    ofxUICanvas *guiFlow = new ofxUICanvas(guiWidth+guiMargin, 0, guiWidth, ofGetHeight());
    guiFlow->addLabel("OPTICAL FLOW", OFX_UI_FONT_LARGE);
    guiFlow->setUIColors(uiThemecb, uiThemeco, uiThemecoh, uiThemecf, uiThemecfh, uiThemecp, uiThemecpo);
    
    guiFlow->addSpacer();
    ofxUIImageToggle *active;
    active = guiFlow->addImageToggle("Activate Flow", "icons/show.png", &contour.doOpticalFlow, dim, dim);
    active->setColorBack(ofColor(150, 255));
    
    guiFlow->addSpacer();
    guiFlow->addSlider("Flow Strength", 0.0, 100.0, &contour.flowStrength);
    guiFlow->addIntSlider("Offset", 1, 10, &contour.flowOffset);
    guiFlow->addSlider("Lambda", 0.0, 0.1, &contour.flowLambda);
    guiFlow->addSlider("Threshold", 0.0, 0.2, &contour.flowThreshold);
    
    guiFlow->addToggle("Inverse X", &contour.flowInverseX);
    guiFlow->addToggle("Inverse Y", &contour.flowInverseY);
    
    guiFlow->addSpacer();
    guiFlow->addToggle("Time Blur Active", &contour.flowTimeBlurActive);
    guiFlow->addSlider("Time Blur Decay", 0.0, 1.0, &contour.flowTimeBlurDecay);
    guiFlow->addSlider("Time Blur Radius", 0.0, 10.0, &contour.flowTimeBlurRadius);
    
    guiFlow->addSpacer();
    guiFlow->addLabel("Debug", OFX_UI_FONT_MEDIUM);
    guiFlow->addSpacer();
    guiFlow->addToggle("Show Optical Flow", &contour.drawFlow);
    guiFlow->addToggle("Show Scalar", &contour.drawFlowScalar);
    guiFlow->addSpacer();
    
    guiFlow->addLabel("VELOCITY MASK", OFX_UI_FONT_LARGE);
    guiFlow->addSpacer();
    guiFlow->addSlider("Mask Strength", 0.0, 10.0, &contour.vMaskStrength);
    guiFlow->addIntSlider("Blur Passes", 0, 10, &contour.vMaskBlurPasses);
    guiFlow->addSlider("Blur Radius", 0.0, 10.0, &contour.vMaskBlurRadius);
    guiFlow->addSpacer();
    
    guiFlow->addLabel("Debug", OFX_UI_FONT_MEDIUM);
    guiFlow->addSpacer();
    guiFlow->addToggle("Show difference", &contour.drawDiffImage);
    guiFlow->addToggle("Show difference contour", &contour.drawDiff);
    guiFlow->addToggle("Show velocities from contour", &contour.drawVelocities);
    guiFlow->addSpacer();
    
    guiFlow->autoSizeToFitWidgets();
    guiFlow->setVisible(false);
    ofAddListener(guiFlow->newGUIEvent, this, &ofApp::guiEvent);
    guis.push_back(guiFlow);
}


//--------------------------------------------------------------
void ofApp::setupFluidSolverGUI(){
    ofxUICanvas *guiFluid_1 = new ofxUICanvas((guiWidth+guiMargin)*2, 0, guiWidth, ofGetHeight());
    guiFluid_1->addLabel("FLUID", OFX_UI_FONT_LARGE);
    guiFluid_1->setUIColors(uiThemecb, uiThemeco, uiThemecoh, uiThemecf, uiThemecfh, uiThemecp, uiThemecpo);
    
    guiFluid_1->addSpacer();
    ofxUIImageToggle *active;
    active = guiFluid_1->addImageToggle("Activate Fluid", "icons/show.png", &fluid.isActive, dim, dim);
    active->setColorBack(ofColor(150, 255));
    
    guiFluid_1->setWidgetPosition(OFX_UI_WIDGET_POSITION_RIGHT);
    ofxUIImageButton *reset;
    reset = guiFluid_1->addImageButton("Reset Fluid", "icons/reset.png", false, dim, dim);
    reset->setColorBack(ofColor(150, 255));
    guiFluid_1->setWidgetPosition(OFX_UI_WIDGET_POSITION_DOWN);
    
    guiFluid_1->addSpacer();
    ofxUISlider *redSlider = guiFluid_1->addSlider("Red", 0.0, 255.0, &fluid.red);
    redSlider->setColorFill(ofColor(240, 30, 30));
    redSlider->setColorFillHighlight(ofColor(150, 30, 30));
    ofxUISlider *greenSlider = guiFluid_1->addSlider("Green", 0.0, 255.0, &fluid.green);
    greenSlider->setColorFill(ofColor(30, 240, 30));
    greenSlider->setColorFillHighlight(ofColor(30, 150, 30));
    ofxUISlider *blueSlider = guiFluid_1->addSlider("Blue", 0.0, 255.0, &fluid.blue);
    blueSlider->setColorFill(ofColor(30, 30, 240));
    blueSlider->setColorFillHighlight(ofColor(30, 30, 150));
    guiFluid_1->addSlider("Opacity", 0.0, 255.0, &fluid.opacity);
    
    guiFluid_1->addSpacer();
    guiFluid_1->addLabel("Solver", OFX_UI_FONT_MEDIUM);
    guiFluid_1->addSpacer();
    guiFluid_1->addSlider("Speed", 0.0, 100.0, &fluid.speed);
    guiFluid_1->addSlider("Cell Size", 0.0, 2.0, &fluid.cellSize);
    guiFluid_1->addIntSlider("Num Jacobi Iterations", 1, 100., &fluid.numJacobiIterations);
    guiFluid_1->addSlider("Viscosity", 0.0, 1.0, &fluid.viscosity);
    guiFluid_1->addSlider("Vorticity", 0.0, 1.0, &fluid.vorticity);
    guiFluid_1->addSlider("Dissipation", 0.0, 0.02, &fluid.dissipation)->setLabelPrecision(2);
    
    guiFluid_1->addSpacer();
    guiFluid_1->addLabel("Advanced dissipation", OFX_UI_FONT_MEDIUM);
    guiFluid_1->addSpacer();
    guiFluid_1->addSlider("Velocity offset", -0.01, 0.01, &fluid.dissipationVelocityOffset)->setLabelPrecision(3);
    guiFluid_1->addSlider("Density offset", -0.01, 0.01, &fluid.dissipationDensityOffset)->setLabelPrecision(3);
    guiFluid_1->addSlider("Temperature offset", -0.01, 0.01, &fluid.dissipationTemperatureOffset)->setLabelPrecision(3);
    
    guiFluid_1->addSpacer();
    guiFluid_1->addLabel("Smoke buoyancy", OFX_UI_FONT_MEDIUM);
    guiFluid_1->addSpacer();
    guiFluid_1->addSlider("Sigma", 0.0, 1.0, &fluid.smokeSigma);
    guiFluid_1->addSlider("Weight", 0.0, 1.0, &fluid.smokeWeight);
    guiFluid_1->addSlider("Ambient temperature", 0.0, 1.0, &fluid.ambientTemperature);
    
    guiFluid_1->autoSizeToFitWidgets();
    guiFluid_1->setVisible(false);
    ofAddListener(guiFluid_1->newGUIEvent, this, &ofApp::guiEvent);
    guis.push_back(guiFluid_1);
    
    ofxUICanvas *guiFluid_2 = new ofxUICanvas((guiWidth+guiMargin)*3, 0, guiWidth, ofGetHeight());
    guiFluid_2->setUIColors(uiThemecb, uiThemeco, uiThemecoh, uiThemecf, uiThemecfh, uiThemecp, uiThemecpo);
    
    guiFluid_2->addSpacer();
    guiFluid_2->addSlider("Gravity X", -10.0, 10.0, &fluid.gravity.x);
    guiFluid_2->addSlider("Gravity Y", -10.0, 10.0, &fluid.gravity.y);
    
    guiFluid_2->addSpacer();
    guiFluid_2->addLabel("Maximum", OFX_UI_FONT_MEDIUM);
    guiFluid_2->addSpacer();
    guiFluid_2->addSlider("Clamp Force", 0.0, 0.1, &fluid.clampForce)->setLabelPrecision(2);
    guiFluid_2->addSlider("Density", 0.0, 5.0, &fluid.maxDensity);
    guiFluid_2->addSlider("Velocity", 0.0, 10.0, &fluid.maxVelocity);
    guiFluid_2->addSlider("Temperature", 0.0, 5.0, &fluid.maxTemperature);
    
    guiFluid_2->addSpacer();
    guiFluid_2->addSlider("Density from Pressure", -0.1, 0.1, &fluid.densityFromPressure);
    guiFluid_2->addSlider("Density from Vorticity", -0.5, 0.5, &fluid.densityFromVorticity);
    guiFluid_2->addSpacer();
    
    guiFluid_2->addLabel("Debug", OFX_UI_FONT_MEDIUM);
    guiFluid_2->addSpacer();
    guiFluid_2->addToggle("Show Fluid Velocities", &fluid.drawVelocity);
    guiFluid_2->addToggle("Show Fluid Velocities Scalar", &fluid.drawVelocityScalar);
    guiFluid_2->addToggle("Show Temperature", &fluid.drawTemperature);
    
    guiFluid_2->autoSizeToFitWidgets();
    guiFluid_2->setVisible(false);
    ofAddListener(guiFluid_2->newGUIEvent, this, &ofApp::guiEvent);
    guis.push_back(guiFluid_2);
}

void ofApp::setupFluidExtrasGUI(){
    ofxUICanvas *guiFluidMarkers = new ofxUICanvas(guiWidth+guiMargin, 0, guiWidth, ofGetHeight());
    guiFluidMarkers->setUIColors(uiThemecb, uiThemeco, uiThemecoh, uiThemecf, uiThemecfh, uiThemecp, uiThemecpo);
    guiFluidMarkers->addLabel("FLUID MARKER", OFX_UI_FONT_LARGE);
    
    guiFluidMarkers->addSpacer();
    ofxUIImageToggle *activeMarkers;
    activeMarkers = guiFluidMarkers->addImageToggle("Activate marker forces", "icons/show.png",  &fluid.markersInput, dim, dim);
    activeMarkers->setColorBack(ofColor(150, 255));
    guiFluidMarkers->setWidgetPosition(OFX_UI_WIDGET_POSITION_RIGHT);
    ofxUIImageButton *reset;
    reset = guiFluidMarkers->addImageButton("Reset marker draw forces", "icons/reset.png", false, dim, dim);
    reset->setColorBack(ofColor(150, 255));
    guiFluidMarkers->setWidgetPosition(OFX_UI_WIDGET_POSITION_DOWN);
    
    guiFluidMarkers->addSpacer();
    guiFluidMarkers->addLabel("Force 1", OFX_UI_FONT_MEDIUM);
    guiFluidMarkers->addSpacer();
    guiFluidMarkers->addIntSlider("F1 Type", 0, 5, &fluid.markerForceTypes[0]);
    guiFluidMarkers->addSlider("F1 Strength", 0.0, 5.0, &fluid.markerForceStrengths[0]);
    guiFluidMarkers->addSlider("F1 Radius", 0.0, 0.1, &fluid.markerForceRadiuses[0])->setLabelPrecision(2);
    guiFluidMarkers->addSlider("F1 Edge", 0.0, 1.0, &fluid.markerForceEdges[0]);
    guiFluidMarkers->addSpacer();
    guiFluidMarkers->addSpacer();
    guiFluidMarkers->addLabel("Force 2", OFX_UI_FONT_MEDIUM);
    guiFluidMarkers->addSpacer();
    guiFluidMarkers->addIntSlider("F2 Type", 0, 5, &fluid.markerForceTypes[1]);
    guiFluidMarkers->addSlider("F2 Strength", 0.0, 5.0, &fluid.markerForceStrengths[1]);
    guiFluidMarkers->addSlider("F2 Radius", 0.0, 0.1, &fluid.markerForceRadiuses[1])->setLabelPrecision(2);
    guiFluidMarkers->addSlider("F2 Edge", 0.0, 1.0, &fluid.markerForceEdges[1]);
    guiFluidMarkers->addSpacer();
    guiFluidMarkers->addSpacer();
    guiFluidMarkers->addLabel("Force 3", OFX_UI_FONT_MEDIUM);
    guiFluidMarkers->addSpacer();
    guiFluidMarkers->addIntSlider("F3 Type", 0, 5, &fluid.markerForceTypes[2]);
    guiFluidMarkers->addSlider("F3 Strength", 0.0, 5.0, &fluid.markerForceStrengths[2]);
    guiFluidMarkers->addSlider("F3 Radius", 0.0, 0.1, &fluid.markerForceRadiuses[2])->setLabelPrecision(2);
    guiFluidMarkers->addSlider("F3 Edge", 0.0, 1.0, &fluid.markerForceEdges[2]);
    guiFluidMarkers->addSpacer();
    
    guiFluidMarkers->autoSizeToFitWidgets();
    guiFluidMarkers->setVisible(false);
    ofAddListener(guiFluidMarkers->newGUIEvent, this, &ofApp::guiEvent);
    guis.push_back(guiFluidMarkers);
    
    ofxUICanvas *guiFluidParticles = new ofxUICanvas((guiWidth+guiMargin)*2, 0, guiWidth, ofGetHeight());
    guiFluidParticles->setUIColors(uiThemecb, uiThemeco, uiThemecoh, uiThemecf, uiThemecfh, uiThemecp, uiThemecpo);
    guiFluidParticles->addLabel("FLUID PARTICLES", OFX_UI_FONT_LARGE);
    
    guiFluidParticles->addSpacer();
    ofxUIImageToggle *active;
    active = guiFluidParticles->addImageToggle("Activate Particles", "icons/show.png",  &fluid.particlesActive, dim, dim);
    active->setColorBack(ofColor(150, 255));
    
    guiFluidParticles->addSpacer();
    guiFluidParticles->addToggle("Marker", &fluid.markersInput);
    guiFluidParticles->setWidgetPosition(OFX_UI_WIDGET_POSITION_RIGHT);
    guiFluidParticles->setWidgetSpacing(35);
    guiFluidParticles->addToggle("Contour", &fluid.contourInput);
    guiFluidParticles->setWidgetPosition(OFX_UI_WIDGET_POSITION_DOWN);
    guiFluidParticles->setWidgetSpacing(3);
    
    guiFluidParticles->addSpacer();
    guiFluidParticles->addLabel("Particle", OFX_UI_FONT_MEDIUM);
    guiFluidParticles->addSpacer();
    guiFluidParticles->addSlider("Birth Chance", 0.0, 1.0, &fluid.particlesBirthChance);
    guiFluidParticles->addSlider("Birth Velocity Chance", 0.0, 5.0, &fluid.particlesBirthVelocityChance);
    guiFluidParticles->addSlider("Lifetime", 0.0, 10.0, &fluid.particlesLifetime);
    guiFluidParticles->addSlider("Lifetime Random", 0.0, 1.0, &fluid.particlesLifetimeRnd);
    guiFluidParticles->addSlider("Mass", 0.0, 2.0, &fluid.particlesMass);
    guiFluidParticles->addSlider("Mass Random", 0.0, 1.0, &fluid.particlesMassRnd);
    guiFluidParticles->addSlider("Size", 0.0, 10.0, &fluid.particlesSize);
    guiFluidParticles->addSlider("Size Random", 0.0, 1.0, &fluid.particlesSizeRnd);
    guiFluidParticles->addSpacer();
    
    guiFluidParticles->autoSizeToFitWidgets();
    guiFluidParticles->setVisible(false);
    ofAddListener(guiFluidParticles->newGUIEvent, this, &ofApp::guiEvent);
    guis.push_back(guiFluidParticles);
}

//--------------------------------------------------------------
void ofApp::setupContourGUI(){
    ofxUICanvas *guiContour = new ofxUICanvas((guiWidth+guiMargin)*3, 0, guiWidth, ofGetHeight());
    guiContour->addLabel("DEPTH CONTOUR", OFX_UI_FONT_LARGE);
    guiContour->setUIColors(uiThemecb, uiThemeco, uiThemecoh, uiThemecf, uiThemecfh, uiThemecp, uiThemecpo);
    
    guiContour->addSpacer();
    ofxUIImageToggle *active;
    active = guiContour->addImageToggle("Activate Contour", "icons/show.png", &contour.isActive, dim, dim);
    active->setColorBack(ofColor(150, 255));
    
    guiContour->addSpacer();
    ofxUISlider *redSlider = guiContour->addSlider("Red", 0.0, 255.0, &contour.red);
    redSlider->setColorFill(ofColor(240, 30, 30));
    redSlider->setColorFillHighlight(ofColor(150, 30, 30));
    ofxUISlider *greenSlider = guiContour->addSlider("Green", 0.0, 255.0, &contour.green);
    greenSlider->setColorFill(ofColor(30, 240, 30));
    greenSlider->setColorFillHighlight(ofColor(30, 150, 30));
    ofxUISlider *blueSlider = guiContour->addSlider("Blue", 0.0, 255.0, &contour.blue);
    blueSlider->setColorFill(ofColor(30, 30, 240));
    blueSlider->setColorFillHighlight(ofColor(30, 30, 150));
    guiContour->addSlider("Opacity", 0.0, 255.0, &contour.maxOpacity);

    guiContour->addSpacer();
    guiContour->addLabel("Contours", OFX_UI_FONT_MEDIUM);
    guiContour->addSpacer();
    guiContour->addToggle("Bounding Rectangle", &contour.drawBoundingRect);
    guiContour->addToggle("Convex Hull", &contour.drawConvexHull);
    guiContour->addToggle("Convex Hull Line", &contour.drawConvexHullLine);
    guiContour->addToggle("Contour", &contour.drawContour);
    guiContour->addToggle("Contour Line", &contour.drawContourLine);
    guiContour->addToggle("Quads Line", &contour.drawQuads);
    guiContour->addSlider("Smoothing Size", 0.0, 40.0, &contour.smoothingSize);
    guiContour->addSlider("Line Width", 0.5, 10.0, &contour.lineWidth);
    guiContour->addSlider("Scale", 1.0, 1.6, &contour.scaleContour);

    guiContour->addSpacer();
    guiContour->addLabel("Debug", OFX_UI_FONT_MEDIUM);
    guiContour->addSpacer();
    guiContour->addToggle("Show Difference", &contour.drawDiff);
    guiContour->addToggle("Show Difference Image", &contour.drawDiffImage);
    guiContour->addToggle("Show Contour Velocities", &contour.drawVelocities);
    guiContour->addSpacer();

    guiContour->autoSizeToFitWidgets();
    guiContour->setVisible(false);
    ofAddListener(guiContour->newGUIEvent, this, &ofApp::guiEvent);
    guis.push_back(guiContour);
}

//--------------------------------------------------------------
void ofApp::setupEmitterGUI(){    
    ofxUICanvas *gui8Emitter_1 = new ofxUICanvas(guiWidth+guiMargin, 0, guiWidth, ofGetHeight());
    gui8Emitter_1->addLabel("PARTICLE EMITTER", OFX_UI_FONT_LARGE);
    gui8Emitter_1->setUIColors(uiThemecb, uiThemeco, uiThemecoh, uiThemecf, uiThemecfh, uiThemecp, uiThemecpo);

    addParticleBasicsGUI(gui8Emitter_1, emitterParticles);
    addParticleInteractionGUI(gui8Emitter_1, emitterParticles);

    gui8Emitter_1->addLabel("Emitter", OFX_UI_FONT_MEDIUM);
    gui8Emitter_1->addSpacer();
    gui8Emitter_1->addSlider("Particles/sec", 0.0, 150.0, &emitterParticles->bornRate);
    gui8Emitter_1->addSlider("Velocity", 0.0, 100.0, &emitterParticles->velocity);
    gui8Emitter_1->addSlider("Velocity Random[%]", 0.0, 100.0, &emitterParticles->velocityRnd);
    gui8Emitter_1->addSlider("Velocity from Motion[%]", 0.0, 100.0, &emitterParticles->velocityMotion);
    gui8Emitter_1->addSlider("Emitter size", 0.0, 60.0, &emitterParticles->emitterSize);
    gui8Emitter_1->addSpacer();
    vector<string> emitters;
    emitters.push_back("Emit all time");
    emitters.push_back("Emit all time on contour");
    emitters.push_back("Emit only if movement");
    gui8Emitter_1->addRadio("Emitters", emitters, OFX_UI_ORIENTATION_VERTICAL)->setTriggerType(OFX_UI_TRIGGER_ALL);//->activateToggle("Emit all time");
    
    gui8Emitter_1->addSpacer();

    gui8Emitter_1->autoSizeToFitWidgets();
    gui8Emitter_1->setVisible(false);
    ofAddListener(gui8Emitter_1->newGUIEvent, this, &ofApp::guiEvent);
    guis.push_back(gui8Emitter_1);
    
    ofxUICanvas *gui8Emitter_2 = new ofxUICanvas((guiWidth+guiMargin)*2, 0, guiWidth, ofGetHeight());
    gui8Emitter_2->setUIColors(uiThemecb, uiThemeco, uiThemecoh, uiThemecf, uiThemecfh, uiThemecp, uiThemecpo);
    gui8Emitter_2->addSpacer();
    addParticlePropertiesGUI(gui8Emitter_2, emitterParticles);

    gui8Emitter_2->addLabel("Time behavior", OFX_UI_FONT_MEDIUM);
    gui8Emitter_2->addSpacer();
    gui8Emitter_2->addToggle("Size", &emitterParticles->sizeAge);
    gui8Emitter_2->setWidgetPosition(OFX_UI_WIDGET_POSITION_RIGHT);
    gui8Emitter_2->setWidgetSpacing(10);
    gui8Emitter_2->addToggle("Alpha", &emitterParticles->opacityAge);
    gui8Emitter_2->addToggle("Flickers", &emitterParticles->flickersAge);
    gui8Emitter_2->setWidgetPosition(OFX_UI_WIDGET_POSITION_DOWN);
    gui8Emitter_2->setWidgetSpacing(3);
    gui8Emitter_2->addToggle("Color", &emitterParticles->colorAge);
    gui8Emitter_2->addSpacer();

    addParticlePhysicsGUI(gui8Emitter_2, emitterParticles);

    gui8Emitter_2->autoSizeToFitWidgets();
    gui8Emitter_2->setVisible(false);
    ofAddListener(gui8Emitter_2->newGUIEvent, this, &ofApp::guiEvent);
    guis.push_back(gui8Emitter_2);
}

//--------------------------------------------------------------
void ofApp::setupGridGUI(){
//    gui8Grid = new ofxUISuperCanvas("8: PARTICLES", 0, 0, guiWidth, ofGetHeight());
    ofxUICanvas *gui8Grid = new ofxUICanvas(guiWidth+guiMargin, 0, guiWidth, ofGetHeight());
    gui8Grid->addLabel("PARTICLE GRID", OFX_UI_FONT_LARGE);
    gui8Grid->setUIColors(uiThemecb, uiThemeco, uiThemecoh, uiThemecf, uiThemecfh, uiThemecp, uiThemecpo);

    addParticleBasicsGUI(gui8Grid, gridParticles);
    
    gui8Grid->addLabel("Grid", OFX_UI_FONT_MEDIUM);
    gui8Grid->addSpacer();
    gui8Grid->addSlider("Radius", 0.1, 25.0, &gridParticles->radius);
    gui8Grid->addIntSlider("Resolution", 1, 20, &gridParticles->gridRes)->setStickyValue(1.0);
    
    gui8Grid->addSpacer();
    gui8Grid->addLabel("Interaction", OFX_UI_FONT_MEDIUM);
    gui8Grid->addSpacer();
    gui8Grid->addToggle("Interact", &gridParticles->interact);
    gui8Grid->addSpacer();
    gui8Grid->addSlider("Marker interaction radius", 5.0, 150.0, &gridParticles->markerRadius);
    gui8Grid->addToggle("Gravity Interaction", &gridParticles->gravityInteraction);
    gui8Grid->addToggle("Attract Interaction", &gridParticles->attractInteraction);
    gui8Grid->addSpacer();

    addParticlePhysicsGUI(gui8Grid, gridParticles);

    gui8Grid->autoSizeToFitWidgets();
    gui8Grid->setVisible(false);
    ofAddListener(gui8Grid->newGUIEvent, this, &ofApp::guiEvent);
    guis.push_back(gui8Grid);
}

//--------------------------------------------------------------
void ofApp::setupBoidsGUI(){
    ofxUICanvas *gui8Boids_1 = new ofxUICanvas(guiWidth+guiMargin, 0, guiWidth, ofGetHeight());
    gui8Boids_1->addLabel("PARTICLE BOIDS", OFX_UI_FONT_LARGE);
    gui8Boids_1->setUIColors(uiThemecb, uiThemeco, uiThemecoh, uiThemecf, uiThemecfh, uiThemecp, uiThemecpo);

    addParticleBasicsGUI(gui8Boids_1, boidsParticles);

    gui8Boids_1->addLabel("Flocking", OFX_UI_FONT_MEDIUM);
    gui8Boids_1->addSpacer();
    gui8Boids_1->addSlider("Flocking Radius", 10.0, 100.0, &boidsParticles->flockingRadius);
    lowThresh = gui8Boids_1->addSlider("Lower Threshold", 0.025, 1.0, &boidsParticles->lowThresh);
    lowThresh->setLabelPrecision(3);
    highThresh = gui8Boids_1->addSlider("Higher Threshold", 0.025, 1.0, &boidsParticles->highThresh);
    highThresh->setLabelPrecision(3);
    gui8Boids_1->addSlider("Max speed", 1.0, 100.0, &boidsParticles->maxSpeed);
    gui8Boids_1->addSlider("Separation Strength", 0.001, 0.1, &boidsParticles->separationStrength)->setLabelPrecision(3);
    gui8Boids_1->addSlider("Attraction Strength", 0.001, 0.1, &boidsParticles->attractionStrength)->setLabelPrecision(3);
    gui8Boids_1->addSlider("Alignment Strength", 0.001, 0.1, &boidsParticles->alignmentStrength)->setLabelPrecision(3);
    
    gui8Boids_1->addSpacer();
    gui8Boids_1->addLabel("Interaction", OFX_UI_FONT_MEDIUM);
    gui8Boids_1->addSpacer();
    gui8Boids_1->addToggle("Interact", &boidsParticles->interact);
    gui8Boids_1->addSlider("Marker interaction radius", 5.0, 150.0, &boidsParticles->markerRadius);
    gui8Boids_1->addSpacer();
    
    gui8Boids_1->autoSizeToFitWidgets();
    gui8Boids_1->setVisible(false);
    ofAddListener(gui8Boids_1->newGUIEvent, this, &ofApp::guiEvent);
    guis.push_back(gui8Boids_1);

    ofxUICanvas *gui8Boids_2 = new ofxUICanvas((guiWidth+guiMargin)*2, 0, guiWidth, ofGetHeight());
    gui8Boids_2->setUIColors(uiThemecb, uiThemeco, uiThemecoh, uiThemecf, uiThemecfh, uiThemecp, uiThemecpo);
    gui8Boids_2->addSpacer();

    addParticlePropertiesGUI(gui8Boids_2, boidsParticles);
    addParticlePhysicsGUI(gui8Boids_2, boidsParticles);

    gui8Boids_2->autoSizeToFitWidgets();
    gui8Boids_2->setVisible(false);
    ofAddListener(gui8Boids_2->newGUIEvent, this, &ofApp::guiEvent);
    guis.push_back(gui8Boids_2);
}

//--------------------------------------------------------------
void ofApp::setupAnimationsGUI(){
//    gui8Animations = new ofxUISuperCanvas("8: PARTICLES", 0, 0, guiWidth, ofGetHeight());
    ofxUICanvas *gui8Animations_1 = new ofxUICanvas(guiWidth+guiMargin, 0, guiWidth, ofGetHeight());
    gui8Animations_1->addLabel("PARTICLE ANIMATION", OFX_UI_FONT_LARGE);
    gui8Animations_1->setUIColors(uiThemecb, uiThemeco, uiThemecoh, uiThemecf, uiThemecfh, uiThemecp, uiThemecpo);

    addParticleBasicsGUI(gui8Animations_1, animationsParticles);

    vector<string> animations;
	animations.push_back("Rain");
	animations.push_back("Snow");
	animations.push_back("Wind");
	animations.push_back("Explosion");
	gui8Animations_1->addRadio("Animations", animations, OFX_UI_ORIENTATION_HORIZONTAL);
    
    gui8Animations_1->addSpacer();
    gui8Animations_1->addLabel("Interaction", OFX_UI_FONT_MEDIUM);
    gui8Animations_1->addSpacer();
    gui8Animations_1->addToggle("Interact", &animationsParticles->interact);
    gui8Animations_1->addSpacer();
    gui8Animations_1->addSlider("Marker interaction radius", 5.0, 150.0, &animationsParticles->markerRadius);
    
    gui8Animations_1->addSpacer();
    addParticlePropertiesGUI(gui8Animations_1, animationsParticles);
    
    gui8Animations_1->autoSizeToFitWidgets();
    gui8Animations_1->setVisible(false);
    ofAddListener(gui8Animations_1->newGUIEvent, this, &ofApp::guiEvent);
    guis.push_back(gui8Animations_1);
    
    ofxUICanvas *gui8Animations_2 = new ofxUICanvas((guiWidth+guiMargin)*2, 0, guiWidth, ofGetHeight());
    gui8Animations_2->setUIColors(uiThemecb, uiThemeco, uiThemecoh, uiThemecf, uiThemecfh, uiThemecp, uiThemecpo);
    
    gui8Animations_2->addSpacer();
    addParticlePhysicsGUI(gui8Animations_2, animationsParticles);
    
    gui8Animations_2->autoSizeToFitWidgets();
    gui8Animations_2->setVisible(false);
    ofAddListener(gui8Animations_2->newGUIEvent, this, &ofApp::guiEvent);
    guis.push_back(gui8Animations_2);
}

void ofApp::addParticleBasicsGUI(ofxUICanvas* gui, ParticleSystem* ps){
    gui->addSpacer();
    ofxUIImageToggle *active;
    active = gui->addImageToggle("Activate Particles", "icons/show.png", &ps->isActive, dim, dim);
    active->setColorBack(ofColor(150, 255));
    
    gui->addSpacer();
    ofxUISlider *redSlider = gui->addSlider("Red", 0.0, 255.0, &ps->red);
    redSlider->setColorFill(ofColor(240, 30, 30));
    redSlider->setColorFillHighlight(ofColor(150, 30, 30));
    ofxUISlider *greenSlider = gui->addSlider("Green", 0.0, 255.0, &ps->green);
    greenSlider->setColorFill(ofColor(30, 240, 30));
    greenSlider->setColorFillHighlight(ofColor(30, 150, 30));
    ofxUISlider *blueSlider = gui->addSlider("Blue", 0.0, 255.0, &ps->blue);
    blueSlider->setColorFill(ofColor(30, 30, 240));
    blueSlider->setColorFillHighlight(ofColor(30, 30, 150));
    gui->addSlider("Opacity", 0.0, 255.0, &ps->maxOpacity);
    gui->addSpacer();
    
    gui->addToggle("Marker", &ps->markersInput);
    gui->setWidgetPosition(OFX_UI_WIDGET_POSITION_RIGHT);
    gui->setWidgetSpacing(35);
    gui->addToggle("Contour", &ps->contourInput);
    gui->setWidgetPosition(OFX_UI_WIDGET_POSITION_DOWN);
    gui->setWidgetSpacing(3);
    gui->addSpacer();
}

//--------------------------------------------------------------
void ofApp::addParticleInteractionGUI(ofxUICanvas* gui, ParticleSystem* ps){
    gui->addLabel("Interaction", OFX_UI_FONT_MEDIUM);
    gui->addSpacer();

    gui->addToggle("Interact", &ps->interact);
    gui->addSlider("Marker interaction radius", 5.0, 150.0, &ps->markerRadius);

//    vector<string> interactions;
//    interactions.push_back("Repulsion");
//    interactions.push_back("Attraction");
//    interactions.push_back("Gravity");
//    gui->addRadio("Interactions", interactions, OFX_UI_ORIENTATION_VERTICAL)->activateToggle("Repulsion");
    gui->addSpacer();
}

//--------------------------------------------------------------
void ofApp::addParticlePropertiesGUI(ofxUICanvas* gui, ParticleSystem* ps){
    gui->addLabel("Particle", OFX_UI_FONT_MEDIUM);
    gui->addSpacer();
    gui->addToggle("Empty", &ps->isEmpty);
    gui->setWidgetPosition(OFX_UI_WIDGET_POSITION_RIGHT);
    gui->setWidgetSpacing(10);
    gui->addToggle("Line", &ps->drawLine);
    gui->addToggle("Connected", &ps->drawConnections);
    gui->setWidgetPosition(OFX_UI_WIDGET_POSITION_DOWN);
    gui->setWidgetSpacing(3);
    gui->addSlider("Connect Dist", 5.0, 100.0, &ps->connectDist);
    gui->addSpacer();
    gui->addToggle("Immortal", &ps->immortal);
    gui->addSlider("Lifetime", 0.1, 20.0, &ps->lifetime);
    gui->addSlider("Life Random[%]", 0.0, 100.0, &ps->lifetimeRnd);
    gui->addSpacer();
    gui->addSlider("Radius", 0.1, 25.0, &ps->radius);
    gui->addSlider("Radius Random[%]", 0.0, 100.0, &ps->radiusRnd);
    gui->addSpacer();
}

//--------------------------------------------------------------
void ofApp::addParticlePhysicsGUI(ofxUICanvas* gui, ParticleSystem* ps){
    gui->addLabel("Physics", OFX_UI_FONT_MEDIUM);
    gui->addSpacer();
    gui->addSlider("Friction", 0, 100, &ps->friction);
    gui->addSlider("Gravity X", -15.0, 15.0, &ps->gravity.x);
    gui->addSlider("Gravity Y", -15.0, 15.0, &ps->gravity.y);
    gui->addSlider("Turbulence", 0.0, 20.0, &ps->turbulence);
    gui->addSpacer();
    gui->addToggle("Bounces", &ps->bounce);
    gui->setWidgetPosition(OFX_UI_WIDGET_POSITION_RIGHT);
    gui->setWidgetSpacing(10);
    gui->addToggle("Steers", &ps->steer);
    gui->addToggle("Infinite", &ps->infiniteWalls);
    gui->setWidgetPosition(OFX_UI_WIDGET_POSITION_DOWN);
    gui->setWidgetSpacing(3);
    gui->addToggle("Repulse", &ps->repulse);
    gui->addSlider("Repulse Dist", 1.0, 50.0, &ps->repulseDist);
    gui->addSpacer();
}

// GUIS VECTOR
//  0: guiHelper
//  1: guiBasics
//  2: guiKinect_1
//  3: guiKinect_2
//  4: guiGestures_1
//  5: guiGestures_2
//  6: guiCueList
//  7: guiFlow
//  8: guiFluid_1
//  9: guiFluid_2
// 10: guiFluidParticles
// 11: guiFluidMarkers
// 12: guiContour
// 13: guiEmitter_1
// 14: guiEmitter_2
// 15: guiGrid
// 16: guiBoids_1
// 17: guiBoids_2
// 18: guiAnimations_1
// 19: guiAnimations_2

//--------------------------------------------------------------
void ofApp::saveGUISettings(const string path, const bool isACue){

    // Create directory if this doesnt exist
    ofFile file(path);
    if(!ofDirectory::doesDirectoryExist(file.getEnclosingDirectory())){
        ofDirectory::createDirectory(file.getEnclosingDirectory());
    }

    ofxXmlSettings *XML = new ofxXmlSettings();

    // Save settings
    for(vector<ofxUICanvas *>::iterator it = guis.begin(); it != guis.end(); ++it){
        ofxUICanvas *g = *it;
        int guiIndex = XML->addTag("GUI");
        
        //  2: guiKinect_1
        //  3: guiKinect_2
        //  4: guiGestures_1
        //  5: guiGestures_2
        //  6: guiCueList
        // panels 2, 3, 4 and 5 and 6 are settings that for now we dont want to save in a cue file
        if(isACue && guiIndex >= 2 && guiIndex <= 6) continue;

        XML->pushTag("GUI", guiIndex);
        vector<ofxUIWidget*> widgets = g->getWidgets();
        for(int i = 0; i < widgets.size(); i++){
            // Don't want to save transition frames for cues
            if(isACue && widgets[i]->getName() == "Transition Frames") continue;
            // kind number 20 is ofxUIImageToggle
            // kind number 12 is ofxUITextInput, for which we don't want to save the state
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

    // Save Cues if it is not a cue
    if(!isACue){
        XML->addTag("CUES");
        XML->pushTag("CUES", 0);
        XML->setValue("Active", currentCueIndex, 0);
        for(int i = 0; i < cueList.size(); i++){
            XML->setValue("Cue", cueList[i], i);
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
    
    // if interpolate we do opacity fading in graphics
    if(interpolate){
        for(int i = 0; i < particleSystems.size(); i++){
            particleSystems[i]->doFading = true;
        }
        contour.doFading = true;
    }
    else{
        for(int i = 0; i < particleSystems.size(); i++){
            particleSystems[i]->doFading = false;
        }
        contour.doFading = false;
    }

    int guiIndex = 0;
    for(vector<ofxUICanvas *>::iterator it = guis.begin(); it != guis.end(); ++it){
        ofxUICanvas *g = *it;
        
        //  2: guiKinect_1
        //  3: guiKinect_2
        //  4: guiGestures_1
        //  5: guiGestures_2
        //  6: guiCueList
        // For now we never want to load the settings from panel 5
        if(guiIndex == 5){
            guiIndex++;
            continue;
        }
        // Panel 2, 3, 4, 5 and 6 are settings we dont want to load from a cue file
        if(isACue && guiIndex >= 2 && guiIndex <= 6){
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
                    nInterpolatedFrames = 0;
                    vector<float> values;
                    if(widget->getKind() == 6){ // kind 6 is a range slider widget
                        ofxUIRangeSlider *rangeSlider = (ofxUIRangeSlider *) widget;
                        values.push_back(rangeSlider->getValueLow());
                        values.push_back(rangeSlider->getValueHigh());
                        values.push_back(XML->getValue("LowValue", -1.0, 0));
                        values.push_back(XML->getValue("HighValue", -1.0, 0));
                        widgetsToUpdate[widget] = values;
                    }
                    else{
                        ofxXmlSettings *tmpXML = new ofxXmlSettings();
                        widget->saveState(tmpXML);
                        float currentValue = tmpXML->getValue("Value", -1.0, 0);
                        values.push_back(currentValue);
                        values.push_back(XML->getValue("Value", -1.0, 0));
                        widgetsToUpdate[widget] = values;
                        delete tmpXML;
                    }
                }
                else{
                    widget->loadState(XML);
                    g->triggerEvent(widget);
                    interpolatingWidgets = false;
                    nInterpolatedFrames = 0;
                    // Delete all widgets from the map
                    map<ofxUIWidget *, vector<float> >::iterator it = widgetsToUpdate.begin();
                    while(it != widgetsToUpdate.end()){
                        delete it->first;
                        widgetsToUpdate.erase(it);
                    }
                    widgetsToUpdate.clear();
                }
            }
            XML->popTag();
        }
        guiIndex++;
        XML->popTag();
    }

    // Load cue list if it is not a cue
    if(!isACue){
        XML->pushTag("CUES");
        int numCues = XML->getNumTags("Cue");
        cueList.clear();
        currentCueIndex = XML->getValue("Active", -1, 0);
        for(int i = 0; i < numCues; i++){
            string name = XML->getValue("Cue", "NULL", i);
            // Check if path corresponds to a cue file, if not, we dont add it
            if(ofFile::doesFileExist(name)){
                cueList.push_back(name);
            }
            else{
                ofLogWarning("File " + ofFilePath::getFileName(name) + " not found.");
            }
        }
        XML->popTag();

        if(cueList.size() > 0){
            if(cueList.size() <= currentCueIndex) currentCueIndex = 0;
            cueIndexLabel->setLabel(ofToString(currentCueIndex)+".");
            string cueFileName = ofFilePath::getBaseName(cueList[currentCueIndex]);
            cueName->setTextString(cueFileName);
            cueName->setVisible(true);
            loadGUISettings(cueList[currentCueIndex], false, true);
            
            vector<ofxUICanvas *>::iterator it = guis.begin();
            ofxUICanvas *guiGestures_1 = *(it+4);

            // Delete all existing sequence segmentation cueSliders
            for(int cueIdx = 0; cueIdx < cueSliders.size(); cueIdx++){
                guiGestures_1->removeWidget(cueSliders.at(cueIdx).first);
                guiGestures_1->removeWidget(cueSliders.at(cueIdx).second);
            }
            cueSliders.erase(cueSliders.begin(), cueSliders.end());

            // Create sequence segmentation range sliders
            float n = cueList.size();
            for(int i = 0; i < cueList.size(); i++){
                ofxUILabel *label;
                label = guiGestures_1->addLabel(ofFilePath::getBaseName(cueList[i]));
                string cueName = "Sequence percent";
                float low = (float)i/n*100;
                float high = ((float)i+1.0)/n*100;
                ofxUIRangeSlider *slider;
                slider = guiGestures_1->addRangeSlider(cueName, 0, 100, low, high);
                pair<ofxUILabel *, ofxUIRangeSlider*> cue(label, slider);
                cueSliders.push_back(cue);
            }
            guiGestures_1->autoSizeToFitWidgets();
        }
        else currentCueIndex = -1;
    }

    delete XML;
}

//--------------------------------------------------------------
void ofApp::interpolateWidgetValues(){

    nInterpolatedFrames++;

    map<ofxUIWidget *, vector<float> >::iterator it = widgetsToUpdate.begin();
    while (it != widgetsToUpdate.end()){
        ofxUIWidget * w = it->first;
        vector<float> values = it->second;
        ofxXmlSettings *XML = new ofxXmlSettings();
        bool canDelete = false;
        // if widget is a range slider we have two values to interpolate to
        if(w->getKind() == 6){ // kind 6 is a range slider widget
            w->saveState(XML);
            float initialLowValue = values.at(0);
            float initialHighValue = values.at(1);
            float targetLowValue = values.at(2);
            float targetHighValue = values.at(3);
            float currentLowValue = ofMap(nInterpolatedFrames*(1.0/(float)maxTransitionFrames), 0.0, 1.0, initialLowValue, targetLowValue, true);;
            float currentHighValue = ofMap(nInterpolatedFrames*(1.0/(float)maxTransitionFrames), 0.0, 1.0, initialHighValue, targetHighValue, true);;
            XML->setValue("HighValue", currentHighValue, 0);
            XML->setValue("LowValue", currentLowValue, 0);
            w->loadState(XML);
        }
        else{
            w->saveState(XML);
            float initialValue = values.at(0);
            float targetValue = values.at(1);
            float currentValue = ofMap(nInterpolatedFrames*(1.0/(float)maxTransitionFrames), 0.0, 1.0, initialValue, targetValue, true);
            XML->setValue("Value", currentValue, 0);
            w->loadState(XML);
            // kind 2 is a toggle and 20 is ofxUIImageToggle, so they only can have 0 or 1 value
            // We switch the value in half maxTransitionFrames frames
            if(currentValue == targetValue){
                canDelete = true;
            }
            else if((w->getKind() == 2 || w->getKind() == 20) && nInterpolatedFrames > maxTransitionFrames/2.0){
                XML->setValue("Value", targetValue, 0);
                w->loadState(XML);
                canDelete = true;
            }
        }

        // If values are already target value we delete them from the map
        if(canDelete){
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
    //-------------------------------------------------------------
    // BASICS
    //-------------------------------------------------------------
    if(e.getName() == "Save Settings"){
        ofxUIImageButton *button = (ofxUIImageButton *) e.widget;
        if(button->getValue() == true){
            ofFileDialogResult result = ofSystemSaveDialog("settings.xml", "Save current settings");
            if(result.bSuccess){
                saveGUISettings(result.getPath(), false);
                settingsFilename->setLabel(ofFilePath::getFileName(result.getPath()));
            }
        }
    }
    if(e.getName() == "Load Settings"){
        ofxUIImageButton *button = (ofxUIImageButton *) e.widget;
        if(button->getValue() == true){
            ofFileDialogResult result = ofSystemLoadDialog("Select settings xml file.", false, ofToDataPath("settings/"));
            if(result.bSuccess){
                loadGUISettings(result.getPath(), false, false);
                settingsFilename->setLabel(ofFilePath::getFileName(result.getPath()));
            }
        }
    }
    if(e.getName() == "Reset Settings"){
        ofxUIImageButton *button = (ofxUIImageButton *) e.widget;
        if(button->getValue() == true){
            loadGUISettings("settings/defaultSettings.xml", false, false);
        }
    }
    if(e.getName() == "Load Song"){
        ofxUIImageButton *button = (ofxUIImageButton *) e.widget;
        if(button->getValue() == true){
            ofFileDialogResult result = ofSystemLoadDialog("Select an audio file.", false, ofToDataPath("songs/"));
            if(result.bSuccess){
                song.loadSound(result.getPath(), true);
                songFilename->setLabel(ofFilePath::getFileName(result.getPath()));
            }
        }
    }
    if(e.getName() == "Play Song"){
        ofxUIImageButton *button = (ofxUIImageButton *) e.widget;
        if(button->getValue() == true) song.play();
    }
    if(e.getName() == "Stop Song"){
        ofxUIImageButton *button = (ofxUIImageButton *) e.widget;
        if(button->getValue() == true) song.stop();
    }
    if(e.getName() == "Loop Song"){
        ofxUIImageToggle *toggle = (ofxUIImageToggle *) e.widget;
        if(toggle->getValue() == true) song.setLoop(true);
        else song.setLoop(false);
    }
    //-------------------------------------------------------------
    // KINECT
    //-------------------------------------------------------------
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
    if(e.getName() == "Clipping range"){
        kinect.setDepthClipping(nearClipping, farClipping);
    }
    if(e.getName() == "Contour size"){
        contour.setMinAreaRadius(minContourSize);
        contour.setMaxAreaRadius(maxContourSize);
    }
    if(e.getName() == "Left/Right Crop" || e.getName() == "Top/Bottom Crop"){
        croppingMask = Mat::ones(480, 640, CV_8UC1);
        for(int i = 0; i < (int)leftMask; i++){
            for(int j = 0; j < croppingMask.rows; j++){
                croppingMask.at<uchar>(cv::Point(i,j)) = 0;
            }
        }
        for(int i = 0; i < croppingMask.cols; i++){
            for(int j = 0; j < (int)topMask; j++){
                croppingMask.at<uchar>(cv::Point(i,j)) = 0;
            }
        }
        for(int i = (int)rightMask-1; i < croppingMask.cols; i++){
            for(int j = 0; j < croppingMask.rows; j++){
                croppingMask.at<uchar>(cv::Point(i,j)) = 0;
            }
        }
        for(int i = 0; i < croppingMask.cols; i++){
            for(int j = (int)bottomMask-1; j < croppingMask.rows; j++){
                croppingMask.at<uchar>(cv::Point(i,j)) = 0;
            }
        }
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
    if(e.getName() == "Show Markers Path"){
        ofxUIImageToggle *toggle = (ofxUIImageToggle *) e.widget;
        if(toggle->getValue() == true){
            vector<irMarker>& markers = tracker.getFollowers();
            // Delete path from markers
            for (int i = 0; i < markers.size(); i++){
                markers[i].clearPath();
            }
        }
    }
    //-------------------------------------------------------------
    // GESTURE SEQUENCE
    //-------------------------------------------------------------
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
            if(result.bSuccess) sequence.save(result.getPath());
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
                sequenceNumMarkers->setLabel("Number of markers: "+ofToString(sequence.getNumMarkers()));
                numMarkers = sequence.getNumMarkers();
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
    if(e.getName() == "Sequence percent"){
        // Update segments polylines in sequence
        vector< pair<float, float> > segmentsPcts;
        for (int i = 0; i < cueSliders.size(); i++){
            pair<float, float> pcts;
            pcts.first = cueSliders.at(i).second->getValueLow();
            pcts.second = cueSliders.at(i).second->getValueHigh();
            if((i < cueSliders.size()-1) && (pcts.second > cueSliders.at(i+1).second->getValueLow())){
                cueSliders.at(i).second->setValueHigh(cueSliders.at(i+1).second->getValueLow());
            }
            segmentsPcts.push_back(pcts);
        }
        sequence.updateSegments(segmentsPcts);
    }
    if(e.getName() == "Show sequence segmentation"){
        ofxUIImageToggle *toggle = (ofxUIImageToggle *) e.widget;
        // Update segments polylines in sequence
        if(toggle->getValue() == true){
            vector< pair<float, float> > segmentsPcts;
            for (int i = 0; i < cueSliders.size(); i++){
                pair<float, float> pcts;
                pcts.first = cueSliders.at(i).second->getValueLow();
                pcts.second = cueSliders.at(i).second->getValueHigh();
                if((i < cueSliders.size()-1) && (pcts.second > cueSliders.at(i+1).second->getValueLow())){
                    cueSliders.at(i).second->setValueHigh(cueSliders.at(i+1).second->getValueLow());
                }
                segmentsPcts.push_back(pcts);
            }
            sequence.updateSegments(segmentsPcts);
        }
    }
    //-------------------------------------------------------------
    // GESTURE TRACKER
    //-------------------------------------------------------------
    if(e.getName() == "Start vmo"){
        ofxUIImageButton *button = (ofxUIImageButton *) e.widget;
        if(button->getValue() == true){
            initStatus = true;
            isTracking = true;
            trackingInfoLabel->setLabel("tracking...");
        }
    }
    if(e.getName() == "Stop vmo"){
        ofxUIImageButton *button = (ofxUIImageButton *) e.widget;
        if(button->getValue() == true){
            isTracking = false;
            trackingInfoLabel->setLabel("");
        }
    }
    //-------------------------------------------------------------
    // CUE LIST
    //-------------------------------------------------------------
    if(e.getName() == "Cue Name"){
        ofxUITextInput *ti = (ofxUITextInput *) e.widget;
        if(ti->getInputTriggerType() == OFX_UI_TEXTINPUT_ON_UNFOCUS || ti->getInputTriggerType() == OFX_UI_TEXTINPUT_ON_ENTER){
            string cuePath = "cueList/"+cueName->getTextString()+".xml";
            cueList[currentCueIndex] = cuePath;
            cueSliders[currentCueIndex].first->setLabel(ofFilePath::getBaseName(cuePath));
        }
    }
    if(e.getName() == "New Cue"){
        ofxUIImageButton *button = (ofxUIImageButton *) e.widget;
        if(button->getValue() == true){
            if(currentCueIndex >= 0) saveGUISettings(cueList[currentCueIndex], true);
            currentCueIndex++;
            string cueFileName = "newCue"+ofToString(currentCueIndex);
            string cuePath = "cues/"+cueFileName+".xml";
            while(find(cueList.begin(), cueList.end(), cuePath) != cueList.end()){
                cueFileName += ".1";
                cuePath = "cues/"+cueFileName+".xml";
            }
            cueList.insert(cueList.begin()+currentCueIndex, cuePath);
            cueIndexLabel->setLabel(ofToString(currentCueIndex)+".");
            cueName->setTextString(cueFileName);
            cueName->setVisible(true);
            
            vector<ofxUICanvas *>::iterator it = guis.begin();
            ofxUICanvas *guiGestures_1 = *(it+4);
            
            // Delete all existing sequence segmentation cueSliders widgets...
            for(int cueIdx = 0; cueIdx < cueSliders.size(); cueIdx++){
                guiGestures_1->removeWidget(cueSliders.at(cueIdx).first);
                guiGestures_1->removeWidget(cueSliders.at(cueIdx).second);
            }
            cueSliders.erase(cueSliders.begin(), cueSliders.end());

            // ...and create them all again
            float n = cueList.size();
            for(int i = 0; i < cueList.size(); i++){
                ofxUILabel *label;
                label = guiGestures_1->addLabel(ofFilePath::getBaseName(cueList[i]));
                string cueName = "Sequence percent";
                float low = (float)i/n*100;
                float high = ((float)i+1.0)/n*100;
                ofxUIRangeSlider *slider;
                slider = guiGestures_1->addRangeSlider(cueName, 0, 100, low, high);
                pair<ofxUILabel *, ofxUIRangeSlider*> cue(label, slider);
                cueSliders.push_back(cue);
            }
            guiGestures_1->autoSizeToFitWidgets();
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
            if(cueList.size() == 0) return;
            if(!interpolatingWidgets) saveGUISettings(cueList[currentCueIndex], true);
            if(currentCueIndex-1 >= 0) currentCueIndex--;
            else if(currentCueIndex-1 < 0) currentCueIndex = cueList.size()-1;
            loadGUISettings(cueList[currentCueIndex], false, true);
            string cueFileName = ofFilePath::getBaseName(cueList[currentCueIndex]);
            cueIndexLabel->setLabel(ofToString(currentCueIndex)+".");
            cueName->setTextString(cueFileName);
        }
    }
    if(e.getName() == "Next Cue"){
        ofxUIImageButton *button = (ofxUIImageButton *) e.widget;
        if(button->getValue() == true){
            if(cueList.size() == 0) return;
            if(!interpolatingWidgets) saveGUISettings(cueList[currentCueIndex], true);
            if(currentCueIndex+1 < cueList.size()) currentCueIndex++;
            else if(currentCueIndex+1 == cueList.size()) currentCueIndex = 0;
            loadGUISettings(cueList[currentCueIndex], false, true);
            string cueFileName = ofFilePath::getBaseName(cueList[currentCueIndex]);
            cueIndexLabel->setLabel(ofToString(currentCueIndex)+".");
            cueName->setTextString(cueFileName);
        }
    }
    if(e.getName() == "Load Cue"){
        ofxUIImageButton *button = (ofxUIImageButton *) e.widget;
        if(button->getValue() == true){
            ofFileDialogResult result = ofSystemLoadDialog("Select cue file.", false, ofToDataPath("cues/"));
            if(result.bSuccess){
                if(cueList.size() == 0) currentCueIndex = 0;
                loadGUISettings(result.getPath(), false, true);
                string cuePath = "cues/"+ofFilePath::getFileName(result.getPath());
                saveGUISettings(cuePath, true);
                cueList[currentCueIndex] = cuePath;
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
            if(cueList.size() == 0) return;

            // Delete cue string from vector
            cueList.erase(cueList.begin()+currentCueIndex);

            // Show previous cue
            if(currentCueIndex-1 >= 0) currentCueIndex--;
            else if(currentCueIndex-1 < 0) currentCueIndex = cueList.size()-1;
            if(cueList.size() == 0){
                currentCueIndex = -1; // if it enters here it already has this value
                cueIndexLabel->setLabel("");
                cueName->setTextString("");
                cueName->setVisible(false);
            }
            else{
                loadGUISettings(cueList[currentCueIndex], false, true);
                string cueFileName = ofFilePath::getBaseName(cueList[currentCueIndex]);
                cueIndexLabel->setLabel(ofToString(currentCueIndex)+".");
                cueName->setTextString(cueFileName);
            }
            
            vector<ofxUICanvas *>::iterator it = guis.begin();
            ofxUICanvas *guiGestures_1 = *(it+4);
            
            // Delete all existing sequence segmentation cueSliders widgets...
            for(int cueIdx = 0; cueIdx < cueSliders.size(); cueIdx++){
                guiGestures_1->removeWidget(cueSliders.at(cueIdx).first);
                guiGestures_1->removeWidget(cueSliders.at(cueIdx).second);
            }
            cueSliders.erase(cueSliders.begin(), cueSliders.end());

            // ...and create them all again
            float n = cueList.size();
            for(int i = 0; i < cueList.size(); i++){
                ofxUILabel *label;
                label = guiGestures_1->addLabel(ofFilePath::getBaseName(cueList[i]));
                string cueName = "Sequence percent";
                float low = (float)i/n*100;
                float high = ((float)i+1.0)/n*100;
                ofxUIRangeSlider *slider;
                slider = guiGestures_1->addRangeSlider(cueName, 0, 100, low, high);
                pair<ofxUILabel *, ofxUIRangeSlider*> cue(label, slider);
                cueSliders.push_back(cue);
            }
            guiGestures_1->autoSizeToFitWidgets();
        }
    }
    if(e.getName() == "GO"){
        ofxUILabelButton *button = (ofxUILabelButton *) e.widget;
        if(button->getValue() == true){
            if(cueList.size() == 0) return;
            if(!interpolatingWidgets) saveGUISettings(cueList[currentCueIndex], true);
            if(currentCueIndex+1 < cueList.size()) currentCueIndex++;
            else if(currentCueIndex+1 == cueList.size()) currentCueIndex = 0;
            loadGUISettings(cueList[currentCueIndex], true, true);
            string cueFileName = ofFilePath::getBaseName(cueList[currentCueIndex]);
            cueIndexLabel->setLabel(ofToString(currentCueIndex)+".");
            cueName->setTextString(cueFileName);
        }
    }
    //-------------------------------------------------------------
    // FLUID SOLVER
    //-------------------------------------------------------------
    if(e.getName() == "Reset Fluid"){
        ofxUIImageToggle *toggle = (ofxUIImageToggle *) e.widget;
        if(toggle->getValue() == true){
            fluid.reset();
        }
    }
    if(e.getName() == "Reset marker draw forces"){
        ofxUIImageToggle *toggle = (ofxUIImageToggle *) e.widget;
        if(toggle->getValue() == true){
            fluid.resetDrawForces();
        }
    }
    //-------------------------------------------------------------
    // PARTICLES
    //-------------------------------------------------------------
//    if(e.getName() == "Interactions"){
//        ofxUIRadio *radio = (ofxUIRadio *) e.widget;
//        if(radio->getActiveName() == "Repulsion"){
//            gridParticles->repulseInteraction    = true;
//            gridParticles->attractInteraction    = false;
//            gridParticles->gravityInteraction    = false;
//            gridParticles->friction              = 40.0;
//        }
//        else if(radio->getActiveName() == "Gravity"){
//            gridParticles->repulseInteraction    = false;
//            gridParticles->attractInteraction    = false;
//            gridParticles->gravityInteraction    = true;
//            gridParticles->friction              = 5.0;
//        }
//    }
    // EMITTER SPECIFIC
    if(e.getName() == "Emit all time"){
        ofxUIImageToggle *toggle = (ofxUIImageToggle *) e.widget;
        if(toggle->getValue() == true){
            emitterParticles->emitAllTimeInside     = true;
            emitterParticles->emitAllTimeContour    = false;
            emitterParticles->emitInMovement        = false;
        }
    }
    if(e.getName() == "Emit all time only in contour"){
        ofxUIImageToggle *toggle = (ofxUIImageToggle *) e.widget;
        if(toggle->getValue() == true){
            emitterParticles->emitAllTimeInside     = false;
            emitterParticles->emitAllTimeContour    = true;
            emitterParticles->emitInMovement        = false;
        }
    }
    if(e.getName() == "Emit only if movement"){
        ofxUIImageToggle *toggle = (ofxUIImageToggle *) e.widget;
        if(toggle->getValue() == true){
            emitterParticles->emitAllTimeInside     = false;
            emitterParticles->emitAllTimeContour    = false;
            emitterParticles->emitInMovement        = true;
        }
    }
    // GRID SPECIFIC
    
    // BOIDS SPECIFIC
    if(e.getName() == "Lower Threshold" || e.getName() == "Higher Threshold"){
        if(lowThresh->getValue() > highThresh->getValue()){
            highThresh->setValue(lowThresh->getValue());
        }
    }
    // ANIMATIONS SPECIFIC
    if(e.getName() == "Rain"){
        ofxUIImageToggle *toggle = (ofxUIImageToggle *) e.widget;
        if(toggle->getValue() == true){
            animationsParticles->setAnimation(RAIN);
            animationsParticles->bornParticles();
        }
    }
    if(e.getName() == "Snow"){
        ofxUIImageToggle *toggle = (ofxUIImageToggle *) e.widget;
        if(toggle->getValue() == true){
            animationsParticles->setAnimation(SNOW);
            animationsParticles->bornParticles();
        }
    }
    if(e.getName() == "Wind"){
        ofxUIImageToggle *toggle = (ofxUIImageToggle *) e.widget;
        if(toggle->getValue() == true){
            animationsParticles->setAnimation(WIND);
            animationsParticles->bornParticles();
        }
    }
    if(e.getName() == "Explosion"){
        ofxUIImageToggle *toggle = (ofxUIImageToggle *) e.widget;
        if(toggle->getValue() == true){
            animationsParticles->setAnimation(EXPLOSION);
            animationsParticles->bornParticles();
        }
    }
}

//--------------------------------------------------------------
void ofApp::exit(){
    kinect.close();
    kinect.clear();

    if(!interpolatingWidgets && cueList.size()) saveGUISettings(cueList[currentCueIndex], false);
    saveGUISettings("settings/lastSettings.xml", false);
    
    // Delete particle systems
    for (int i=0; i<particleSystems.size(); i++) {
        delete particleSystems.at(i);
        particleSystems.at(i) = NULL;
    }
    particleSystems.clear();

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
    
    for(int i = 0; i < guis.size(); i++){
        ofxUICanvas *g = guis[i];
        delete g;
    }
    guis.clear();
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
    if(!cueName->isFocused()){
        if(key == 'f'){
            ofToggleFullscreen();
        }
        else if(key == 'h'){
            for(vector<ofxUICanvas *>::iterator it = guis.begin(); it != guis.end(); ++it){
                (*it)->setVisible(false);
            }
        }
        else if(key == '1'){
            int idx = 0;
            for(vector<ofxUICanvas *>::iterator it = guis.begin(); it != guis.end(); ++it){
                if(idx == 0 || idx == 1 || idx == 2 || idx == 3) (*it)->setVisible(true);
                else (*it)->setVisible(false);
                idx++;
            }
        }
        else if(key == '2'){
            int idx = 0;
            for(vector<ofxUICanvas *>::iterator it = guis.begin(); it != guis.end(); ++it){
                if(idx == 0 || idx == 4 || idx == 5 || idx == 6) (*it)->setVisible(true);
                else (*it)->setVisible(false);
                idx++;
            }
        }
        else if(key == '3'){
            int idx = 0;
            for(vector<ofxUICanvas *>::iterator it = guis.begin(); it != guis.end(); ++it){
                if(idx == 0 || idx == 7 || idx == 8 || idx == 9) (*it)->setVisible(true);
                else (*it)->setVisible(false);
                idx++;
            }
        }
        else if(key == '4'){
            int idx = 0;
            for(vector<ofxUICanvas *>::iterator it = guis.begin(); it != guis.end(); ++it){
                if(idx == 0 || idx == 10 || idx == 11 || idx == 12) (*it)->setVisible(true);
                else (*it)->setVisible(false);
                idx++;
            }
        }
        else if(key == '5'){
            currentParticleSystem = 0;
            int idx = 0;
            for(vector<ofxUICanvas *>::iterator it = guis.begin(); it != guis.end(); ++it){
                if(idx == 0 || idx == 13 || idx == 14) (*it)->setVisible(true);
                else (*it)->setVisible(false);
                idx++;
            }
        }
        else if(key == '6'){
            currentParticleSystem = 1;
            int idx = 0;
            for(vector<ofxUICanvas *>::iterator it = guis.begin(); it != guis.end(); ++it){
                if(idx == 0 || idx == 15) (*it)->setVisible(true);
                else (*it)->setVisible(false);
                idx++;
            }
        }
        else if(key == '7'){
            currentParticleSystem = 2;
            int idx = 0;
            for(vector<ofxUICanvas *>::iterator it = guis.begin(); it != guis.end(); ++it){
                if(idx == 0 || idx == 16 || idx == 17) (*it)->setVisible(true);
                else (*it)->setVisible(false);
                idx++;
            }
        }
        else if(key == '8'){
            currentParticleSystem = 3;
            int idx = 0;
            for(vector<ofxUICanvas *>::iterator it = guis.begin(); it != guis.end(); ++it){
                if(idx == 0 || idx == 18 || idx == 19) (*it)->setVisible(true);
                else (*it)->setVisible(false);
                idx++;
            }
        }
        else if(key == ' '){
            vector<ofxUICanvas *>::iterator it = guis.begin();
            ofxUICanvas *guiCueList = *(it+6);
            ofxUILabelButton *button = (ofxUILabelButton *) guiCueList->getWidget("GO");
            button->setValue(true);
            guiCueList->triggerEvent(button);
            button->setValue(false);
        }
        else if(key == OF_KEY_UP){
            angle++;
			if(angle>30) angle=30;
			kinect.setCameraTiltAngle(angle);
        }
        else if(key == OF_KEY_DOWN){
            angle--;
			if(angle<-30) angle=-30;
			kinect.setCameraTiltAngle(angle);
        }
        else if(key == OF_KEY_RIGHT){
            vector<ofxUICanvas *>::iterator it = guis.begin();
            ofxUICanvas *guiCueList = *(it+6);
            ofxUIImageButton *button = (ofxUIImageButton *) guiCueList->getWidget("Next Cue");
            button->setValue(true);
            guiCueList->triggerEvent(button);
            button->setValue(false);
        }
        else if(key == OF_KEY_LEFT){
            vector<ofxUICanvas *>::iterator it = guis.begin();
            ofxUICanvas *guiCueList = *(it+6);
            ofxUIImageButton *button = (ofxUIImageButton *) guiCueList->getWidget("Previous Cue");
            button->setValue(true);
            guiCueList->triggerEvent(button);
            button->setValue(false);
        }
    }
}
//--------------------------------------------------------------
void ofApp::keyReleased(int key){
}
//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){
    ofShowCursor();
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
