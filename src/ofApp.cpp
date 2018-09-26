/*
 * Copyright (C) 2018 Fabia Serra Arrizabalaga
 *
 * This file is part of CREA
 *
 * CREA is free software: you can redistribute it and/or modify it under
 * the terms of the GNU Affero General Public License as published by the Free
 * Software Foundation (FSF), either version 3 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the Affero GNU General Public License
 * version 3 along with this program.  If not, see http://www.gnu.org/licenses/
 */

#include "ofApp.h"

//using namespace ofxCv;
//using namespace cv;

//--------------------------------------------------------------
void ofApp::setup()
{
    ofSetFrameRate(60);
    // ofSetVerticalSync(false);

    m_windowWidth = ofGetWindowWidth();
    m_windowHeight = ofGetWindowHeight();

    // Number of IR markers
    // m_numMarkers = 1;

    // // Using a live kinect?
    // #ifdef KINECT_CONNECTED
    // // OPEN KINECT
    // m_kinect.init(true); // shows infrared instead of RGB video Image
    // m_kinect.open();
    // m_kinect.setLed(ofxKinect::LED_OFF);

    // // Kinect not connected
    // #else
    // #ifdef KINECT_SEQUENCE // Use xml sequence marker file
    // m_kinectSequence.setup(numMarkers);
    // m_kinectSequence.load("sequences/sequenceT2.xml");
    // #endif // KINECT_SEQUENCE

    // // Load png files from file
    // ofDirectory dir; // directory lister
    // dir.allowExt("jpg");

    // std::string depthFolder = "kinect/depth1/";
    // int totalImages = dir.listDir(depthFolder);
    // dir.sort();
    // m_savedDepthImages.resize(totalImages);

    // // Load all recorded depth images in "data/depth01/"
    // for (int i = 0; i < totalImages; ++i)
    // {
    //     ofImage img = ofImage();
    //     img.loadImage(depthFolder + dir.getName(i));
    //     img.setImageType(OF_IMAGE_GRAYSCALE);
    //     m_savedDepthImages[i] = img;
    // }

    // std::string irFolder = "kinect/ir1/";
    // totalImages = dir.listDir(irFolder);
    // dir.sort();
    // m_savedIrImages.resize(totalImages);

    // // Load all recorded IR images in "data/ir01/"
    // for (int i = 0; i < totalImages; ++i)
    // {
    //     ofImage img = ofImage(irFolder + dir.getName(i));
    //     ofImageLoadSettings settings();
    //     settings.grayscale = true;
    //     // img.setImageType(OF_IMAGE_GRAYSCALE);
    //     m_savedIrImages[i] = img;
    // }
    // #endif

    m_time0 = ofGetElapsedTimef();

    // BACKGROUND COLOR
    m_red = 0;
    m_green = 0;
    m_blue = 0;
    m_bgGradient = false;

    // ALLOCATE IMAGES
    // m_depthImage.allocate(m_kinect.width, m_kinect.height, OF_IMAGE_GRAYSCALE);
    // m_depthOriginal.allocate(m_kinect.width, m_kinect.height, OF_IMAGE_GRAYSCALE);
    // m_grayThreshNear.allocate(m_kinect.width, m_kinect.height, OF_IMAGE_GRAYSCALE);
    // m_grayThreshFar.allocate(m_kinect.width, m_kinect.height, OF_IMAGE_GRAYSCALE);
    // irImage.allocate(m_kinect.width, m_kinect.height, OF_IMAGE_GRAYSCALE);
    // m_irOriginal.allocate(m_kinect.width, m_kinect.height, OF_IMAGE_GRAYSCALE);

    // ALLOCATE CROPPING MASKS
    // depthCroppingMask = Mat::ones(m_kinect.height, m_kinect.width, CV_8UC1);
    // irCroppingMask = Mat::ones(m_kinect.height, m_kinect.width, CV_8UC1);

    // depthLeftMask = irLeftMask = 0;
    // depthRightMask = irRightMask = m_kinect.width;
    // depthTopMask = irTopMask = 0;
    // depthBottomMask = irBottomMask  = m_kinect.height;

    // // FILTER PARAMETERS DEPTH IMAGE
    // m_depthNumDilates = 4;
    // m_depthNumErodes = 2;
    // m_depthBlurValue = 7;

    // // FILTER PARAMETERS IR IMAGE
    // m_irNumDilates = 3;
    // m_irNumErodes = 1;
    // m_irBlurValue = 21;

    // // KINECT PARAMETERS
    // m_flipKinect = false;

    // m_nearClipping = 500.f;
    // m_farClipping = 4000.f;

    // m_nearThreshold = 255.f;
    // m_farThreshold = 165.f;
    // m_minContourSize = 20.f;
    // m_maxContourSize = 250.0f;

    // m_irThreshold = 70.f;
    // m_minMarkerSize = 5.0;
    // m_maxMarkerSize = 80.0f;
    // m_irMarkerFinder.setMinAreaRadius(m_minMarkerSize);
    // m_irMarkerFinder.setMaxAreaRadius(m_maxMarkerSize);

    // m_trackerPersistence = 200;
    // m_trackerMaxDistance = 300;
    // m_tracker.setPersistence(m_trackerPersistence);     // wait for 'm_trackerPersistence' frames before forgetting something
    // m_tracker.setMaximumDistance(m_trackerMaxDistance); // an object can move up to 'm_trackerMaxDistance' pixels per frame

    // MARKER PARTICLES
    //    m_emitterParticles = ParticleSystem();
    //    m_emitterParticles.setup(EMITTER, m_kinect.width, m_kinect.height);
    //    m_emitterParticles.setup(EMITTER, m_windowWidth, m_windowHeight);

    // GRID PARTICLES
    m_gridParticles = ParticleSystem();
    //    m_gridParticles.setup(GRID, m_kinect.width, m_kinect.height);
    m_gridParticles.setup(GRID, ofGetWidth(), ofGetHeight());

    // // BOIDS PARTICLES
    // m_boidsParticles = new ParticleSystem();
    // m_boidsParticles.setup(BOIDS, m_kinect.width, m_kinect.height);

    // // ANIMATIONS PARTICLES
    // m_animationsParticles = new ParticleSystem();
    // m_animationsParticles.animation = RAIN;
    // m_animationsParticles.setup(ANIMATIONS, m_kinect.width, m_kinect.height);

    // VECTOR OF PARTICLE SYSTEMS
    m_particleSystems.resize(1);
    //    m_particleSystems[0] = m_emitterParticles;
    m_particleSystems[0] = m_gridParticles;
    // m_particleSystems[2] = m_boidsParticles;
    // m_particleSystems[3] = m_animationsParticles;
    m_currentParticleSystem = 0;

    // SCALE FACTOR TO DO FLOW AND FLUID COMPUTATIONS
    // float scaleFactor = 4.0;

    // SILHOUETTE CONTOUR
    // m_contour.setup(m_kinect.width, m_kinect.height, scaleFactor);
    // m_contour.setMinAreaRadius(m_minContourSize);
    // m_contour.setMaxAreaRadius(m_maxContourSize);

    // // FLUID
    // fluid.setup(m_kinect.width, m_kinect.height, scaleFactor);

    // // SEQUENCE
    // sequence.setup(numMarkers);
    // sequence.load("sequences/sequenceT2.xml");
    // m_drawSequence = false;
    // m_drawSequenceSegments = false;

    // MARKERS
    // drawMarkers = false;
    // drawMarkersPath = false;

    // SONG
    // song.load("songs/ASuitableEnsemble.mp3", true);

    // SETUP GUI
    // setupGUI();

    // ALLOCATE FBO AND FILL WITH BG COLOR
    // m_fbo.allocate(m_kinect.width, m_kinect.height, GL_RGBA32F_ARB);
    // m_fbo.begin();
    // ofClear(255, 255, 255, 0);
    // m_fbo.end();

    // fadeAmount = 80;
    // m_useFBO = false;

    // Create directories in /data if they dont exist
    // string directory[2] = {"sequences", "settings"};
    // for (int i = 0; i < 2; ++i)
    // {
    //     if (!ofDirectory::doesDirectoryExist(directory[i])) // relative to /data folder
    //     {
    //         ofDirectory::createDirectory(directory[i]);
    //     }
    // }
}

//--------------------------------------------------------------
void ofApp::update()
{
    // Compute dt
    float time = ofGetElapsedTimef();
    float dt = ofClamp(time - m_time0, 0.f, 0.1f);
    m_time0 = time;

    //    m_windowWidth = ofGetWindowWidth();
    //    m_windowHeight = ofGetWindowHeight();

    // Compute rescale value to scale kinect image
    // reScale = (float)m_windowHeight / (float)m_kinect.height;
    //    reScale = ofVec2f((float)ofGetWidth()/(float)m_kinect.width, (float)ofGetHeight()/(float)m_kinect.height); // deforms the image a little bit
    //    reScale = ofVec2f((float)ofGetHeight()/(float)m_kinect.height, (float)ofGetHeight()/(float)m_kinect.height);

    // Update the sound playing system
    // ofSoundUpdate();

    // Interpolate GUI widget values
    // if (interpolatingWidgets)
    // {
    // 	interpolateWidgetValues();
    // }

    // // Update sequence playhead to draw gesture
    // if (m_drawSequence)
    // {
    // 	sequence.update();
    // }

    // // Load a saved image for playback
    // #ifndef KINECT_CONNECTED

    //    #ifdef KINECT_SEQUENCE
    //    kinectSequence.update();
    //    #endif // KINECT_SEQUENCE

    //    // Get the number of images on the image sequence
    //    int n = m_savedDepthImages.size();

    //    // Calculate sequence duration assuming 30 fps
    //    float duration = n / 30.0f;

    //    // Calculate playing percent in sequence
    //    float percent = time / duration;

    //    // Convert percent in the frame number
    //    if (percent < 0.0f || percent > 1.0f)
    //    {
    //    	percent -= floor(percent);
    //    }
    //    int i = MIN((int)(percent*n), n-1);

    //    ofImage *depthImg = m_savedDepthImages.at(i);
    //    m_depthOriginal.setFromPixels(depthImg->getPixels(), depthImg->getWidth(), depthImg->getHeight(), OF_IMAGE_GRAYSCALE);
    //    if (m_flipKinect)
    //    {
    //    	m_depthOriginal.mirror(false, true);
    //    }
    //    ofImage *irImg = m_savedIrImages.at(i);
    //    m_irOriginal.setFromPixels(irImg->getPixels(), irImg->getWidth(), irImg->getHeight(), OF_IMAGE_GRAYSCALE);
    //    if (m_flipKinect)
    //    {
    //    	m_irOriginal.mirror(false, true);
    //    }
    // #endif // KINECT_CONNECTED

    //    // Nothing will happen here if the kinect is unplugged
    //    m_kinect.update();
    //    if (m_kinect.isFrameNew())
    //    {
    //        m_depthOriginal.setFromPixels(m_kinect.getDepthPixels());
    //        m_irOriginal.setFromPixels(m_kinect.getPixels());
    //        if (m_flipKinect)
    //        {
    //        	m_depthOriginal.mirror(false, true);
    //        	m_irOriginal.mirror(false, true);
    //        }
    //    }

    //    copy(m_irOriginal, m_irImage);
    //    copy(m_depthOriginal, m_depthImage);
    //    copy(m_depthOriginal, m_grayThreshNear);
    //    copy(m_depthOriginal, m_grayThreshFar);

    //    // Filter and then threshold the IR image
    //    for (int i = 0; i < m_irNumErodes; ++i)
    //    {
    //        erode(m_irImage); // delete small white dots
    //    }
    //    for (int i = 0; i < m_irNumDilates; ++i)
    //    {
    //        dilate(m_irImage);
    //    }
    //    blur(m_irImage, m_irBlurValue);
    //    threshold(m_irImage, m_irThreshold);

    //    // Treshold and filter depth image
    //    threshold(m_grayThreshNear, m_nearThreshold, true);
    //    threshold(m_grayThreshFar, m_farThreshold);
    //    bitwise_and(m_grayThreshNear, m_grayThreshFar, m_depthImage);

    //    m_grayThreshNear.update();
    //    m_grayThreshFar.update();

    //    for (int i = 0; i < m_depthNumErodes; ++i)
    //    {
    //        erode(m_depthImage);
    //    }
    //    for (int i = 0; i < m_depthNumDilates; ++i)
    //    {
    //        dilate(m_depthImage);
    //    }
    //    blur(m_depthImage, m_depthBlurValue);

    // // Crop depth image
    // Mat depthMat = toCv(m_depthImage);
    // Mat depthCropped = Mat::zeros(kinect.height, kinect.width, CV_8UC1);
    // depthCropped = depthMat.mul(depthCroppingMask);
    // copy(depthCropped, m_depthImage);

    // // Crop IR image
    // Mat irMat = toCv(m_irImage);
    // Mat irCropped = Mat::zeros(kinect.height, kinect.width, CV_8UC1);
    // irCropped = irMat.mul(irCroppingMask);
    // copy(irCropped, m_irImage);

    // // Update images
    // m_irImage.update();
    // m_depthImage.update();

    // // Contour Finder + marker tracker in the IR Image
    // m_irMarkerFinder.findContours(m_irImage);
    // m_tracker.track(m_irMarkerFinder.getBoundingRects());

    // // Track markers
    // vector<irMarker>& markers = m_tracker.getFollowers();   // TODO: assign dead labels to new labels and have a MAX number of markers
    // vector<unsigned int> deadLabels = m_tracker.getDeadLabels();
    // vector<unsigned int> currentLabels = m_tracker.getCurrentLabels();
    // // vector<unsigned int> newLabels      = m_tracker.getNewLabels();

    // // Update markers if we loose track of them
    // for (unsigned int i = 0; i < markers.size(); ++i)
    // {
    //     markers[i].updateLabels(deadLabels, currentLabels);
    // }

    // Record sequence when recording button is true
    // if (recordingSequence->getValue() == true)
    // {
    // 	sequence.record(markers);
    // }

    // Update contour
    // m_contour.update(dt, depthImage);

    // Update fluid
    // fluid.update(dt, markers, contour, mouseX, mouseY);

    // Update particles
    //    m_emitterParticles.update(dt);
    //    m_emitterParticles.update(dt, markers, contour, fluid);
    // m_gridParticles.update(dt, markers, contour, fluid);
    // m_boidsParticles.update(dt, markers, contour, fluid);
    // m_animationsParticles.update(dt, markers, contour, fluid);
}

//--------------------------------------------------------------
void ofApp::draw()
{  
    // #ifdef SECOND_WINDOW
    // secondWindow.begin();
    // #endif

    // ofPushMatrix();
    // ofPushStyle();

    ofColor contourBg(m_red, m_green, m_blue);
    ofColor centerBg(m_red, m_green, m_blue);
    if (m_bgGradient)
    {
        if (centerBg.getBrightness() > 0)
        {
            contourBg.setBrightness(ofMap(centerBg.getBrightness(), 0.0f, 255.0, 20.0f, 130.0f));
        }
        ofBackgroundGradient(centerBg, contourBg);
    }
    else
    {
        ofBackground(centerBg);
    }


    // ofRectangle canvasRect(0, 0, m_windowWidth, m_windowHeight);
    // ofRectangle kinectRect(0, 0, m_kinect.width, m_kinect.height);
    // m_kinectRect.scaleTo(canvasRect, OF_SCALEMODE_FIT);
    // ofTranslate(kinectRect.x, kinectRect.y);
    // ofScale(reScale, reScale);

    // if (m_useFBO)
    // {
    //     m_fbo.begin();

    //     // Draw semi-transparent white rectangle to slightly clear buffer (depends on the history value)
    //     ofFill();
    //     ofSetColor(red, green, blue, ofMap(fadeAmount, 0, 100, 250, 0));
    //     ofDrawRectangle(0, 0, m_kinect.width, m_kinect.height);

    //     // Graphics
    //     ofNoFill();
    //     ofSetColor(255);
    //     m_contour.draw();
    //     m_emitterParticles.draw();
    //     m_gridParticles.draw();
    //     m_boidsParticles.draw();
    //     m_animationsParticles.draw();

    //     m_fbo.end();

    //     // Draw buffer (graphics) on the screen
    //     ofSetColor(255);
    //     m_fbo.draw(0, 0);
    // }
    // else
    // {
    // Draw Graphics
    // m_contour.draw();
    // fluid.draw();
    //    m_emitterParticles.draw();
    m_gridParticles.draw();
    // m_boidsParticles.draw();
    // m_animationsParticles.draw();
    // }

    // if (drawMarkers || drawMarkersPath)
    // {
    //     vector<irMarker>& markers = m_tracker.getFollowers();
    //     // Draw identified IR markers
    //     for (int i = 0; i < markers.size(); ++i)
    //     {
    //         if (drawMarkers) markers[i].draw();
    //         if (drawMarkersPath) markers[i].drawPath();
    //     }
    // }

    // #ifdef KINECT_SEQUENCE
    // kinectSequence.draw();
    // #endif // KINECT_SEQUENCE

    // if (m_drawSequence)
    // {
    // 	sequence.draw();
    // }
    // if (m_drawSequenceSegments)
    // {
    // 	sequence.drawSegments();
    // }
    // if (m_drawSequencePatterns)
    // {
    // 	sequence.drawPatterns(gestureUpdate);
    // }

    // ofPopStyle();
    // ofPopMatrix();

    // #ifdef SECOND_WINDOW
    // secondWindow.end();
    // secondWindow.show();
    // ofBackground(0);
    // #endif

    // if (m_drawSequencePatternsSeparate)
    // {
    // 	sequence.drawPatternsSeparate(gestureUpdate);
    // }

    ofSetColor(255, 200);
    ofDrawRectangle(0, 0, 250, 90);
    ofSetColor(0);
    string info = "FPS "+ofToString(ofGetFrameRate(), 0) + "\n";
    ofDrawBitmapString(info, 20, 20);
}

//--------------------------------------------------------------
void ofApp::exit()
{
    // kinect.close();
    // kinect.clear();

    // if (!interpolatingWidgets && cueList.size())
    // {
    // 	saveGUISettings(cueList[currentCueIndex], false);
    // }
    // saveGUISettings("settings/lastSettings.xml", false);

    // // Delete cue sliders map
    // cueSliders.clear();

    // // Cleanup any loaded images
    // for (int i = 0; i < m_savedDepthImages.size(); ++i)
    // {
    //     ofImage *img = m_savedDepthImages[i];
    //     delete img;
    // }
    // m_savedDepthImages.clear();

    // for (int i = 0; i < m_savedIrImages.size(); ++i)
    // {
    //     ofImage *img = m_savedIrImages[i];
    //     delete img;
    // }
    // m_savedIrImages.clear();
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key)
{
    // if (!cueName->isFocused())
    // {
    //     if (key == 'f')
    //     {
    //         ofToggleFullscreen();
    //     }
    //     else if (key == 'h')
    //     {
    //         for (vector<ofxUICanvas *>::iterator it = guis.begin(); it != guis.end(); ++it)
    //         {
    //             (*it)->setVisible(false);
    //         }
    //     }
    //     else if (key == '1')
    //     {
    //         int idx = 0;
    //         for (vector<ofxUICanvas *>::iterator it = guis.begin(); it != guis.end(); ++it)
    //         {
    //             if (idx == 0 || idx == 1 || idx == 2 || idx == 3) (*it)->setVisible(true);
    //             else (*it)->setVisible(false);
    //             idx++;
    //         }
    //         for (int i = 0; i < labelTabs.size(); ++i)
    //         {
    //             if (i == 0) labelTabs[i]->setColorFill(ofColor(255, 255, 255));
    //             else labelTabs[i]->setColorFill(ofColor(150, 150, 150));
    //         }
    //     }
    //     else if (key == '2')
    //     {
    //         int idx = 0;
    //         for (vector<ofxUICanvas *>::iterator it = guis.begin(); it != guis.end(); ++it)
    //         {
    //             if (idx == 0 || idx == 4 || idx == 5 || idx == 6) (*it)->setVisible(true);
    //             else (*it)->setVisible(false);
    //             idx++;
    //         }
    //         for (int i = 0; i < labelTabs.size(); ++i)
    //         {
    //             if (i == 1) labelTabs[i]->setColorFill(ofColor(255, 255, 255));
    //             else labelTabs[i]->setColorFill(ofColor(150, 150, 150));
    //         }
    //     }
    //     else if (key == '3')
    //     {
    //         int idx = 0;
    //         for (vector<ofxUICanvas *>::iterator it = guis.begin(); it != guis.end(); ++it)
    //         {
    //             if (idx == 0 || idx == 7 || idx == 8 || idx == 9) (*it)->setVisible(true);
    //             else (*it)->setVisible(false);
    //             idx++;
    //         }
    //         for (int i = 0; i < labelTabs.size(); ++i)
    //         {
    //             if (i == 2) labelTabs[i]->setColorFill(ofColor(255, 255, 255));
    //             else labelTabs[i]->setColorFill(ofColor(150, 150, 150));
    //         }
    //     }
    //     else if (key == '4')
    //     {
    //         int idx = 0;
    //         for (vector<ofxUICanvas *>::iterator it = guis.begin(); it != guis.end(); ++it)
    //         {
    //             if (idx == 0 || idx == 10 || idx == 11 || idx == 12) (*it)->setVisible(true);
    //             else (*it)->setVisible(false);
    //             idx++;
    //         }
    //         for (int i = 0; i < labelTabs.size(); ++i)
    //         {
    //             if (i == 3) labelTabs[i]->setColorFill(ofColor(255, 255, 255));
    //             else labelTabs[i]->setColorFill(ofColor(150, 150, 150));
    //         }
    //     }
    //     else if (key == '5')
    //     {
    //         currentParticleSystem = 0;
    //         int idx = 0;
    //         for (vector<ofxUICanvas *>::iterator it = guis.begin(); it != guis.end(); ++it)
    //         {
    //             if (idx == 0 || idx == 13 || idx == 14) (*it)->setVisible(true);
    //             else (*it)->setVisible(false);
    //             idx++;
    //         }
    //         for (int i = 0; i < labelTabs.size(); ++i)
    //         {
    //             if (i == 4) labelTabs[i]->setColorFill(ofColor(255, 255, 255));
    //             else labelTabs[i]->setColorFill(ofColor(150, 150, 150));
    //         }
    //     }
    //     else if (key == '6')
    //     {
    //         currentParticleSystem = 1;
    //         int idx = 0;
    //         for (vector<ofxUICanvas *>::iterator it = guis.begin(); it != guis.end(); ++it)
    //         {
    //             if (idx == 0 || idx == 15 || idx == 16) (*it)->setVisible(true);
    //             else (*it)->setVisible(false);
    //             idx++;
    //         }
    //         for (int i = 0; i < labelTabs.size(); ++i)
    //         {
    //             if (i == 5) labelTabs[i]->setColorFill(ofColor(255, 255, 255));
    //             else labelTabs[i]->setColorFill(ofColor(150, 150, 150));
    //         }
    //     }
    //     else if (key == '7')
    //     {
    //         currentParticleSystem = 2;
    //         int idx = 0;
    //         for (vector<ofxUICanvas *>::iterator it = guis.begin(); it != guis.end(); ++it)
    //         {
    //             if (idx == 0 || idx == 17 || idx == 18) (*it)->setVisible(true);
    //             else (*it)->setVisible(false);
    //             idx++;
    //         }
    //         for (int i = 0; i < labelTabs.size(); ++i)
    //         {
    //             if (i == 6) labelTabs[i]->setColorFill(ofColor(255, 255, 255));
    //             else labelTabs[i]->setColorFill(ofColor(150, 150, 150));
    //         }
    //     }
    //     else if (key == '8')
    //     {
    //         currentParticleSystem = 3;
    //         int idx = 0;
    //         for (vector<ofxUICanvas *>::iterator it = guis.begin(); it != guis.end(); ++it)
    //         {
    //             if (idx == 0 || idx == 19 || idx == 20) (*it)->setVisible(true);
    //             else (*it)->setVisible(false);
    //             idx++;
    //         }
    //         for (int i = 0; i < labelTabs.size(); ++i)
    //         {
    //             if (i == 7) labelTabs[i]->setColorFill(ofColor(255, 255, 255));
    //             else labelTabs[i]->setColorFill(ofColor(150, 150, 150));
    //         }
    //     }
    //     else if (key == ' ')
    //     {
    //         vector<ofxUICanvas *>::iterator it = guis.begin();
    //         ofxUICanvas *guiCueList = *(it+6);
    //         ofxUILabelButton *button = (ofxUILabelButton *) guiCueList->getWidget("GO");
    //         button->setValue(true);
    //         guiCueList->triggerEvent(button);
    //         button->setValue(false);
    //     }
    //     else if (key == OF_KEY_UP)
    //     {
    //         angle++;
    //         if (angle>30) angle=30;
    //         kinect.setCameraTiltAngle(angle);
    //     }
    //     else if (key == OF_KEY_DOWN)
    //     {
    //         angle--;
    //         if (angle<-30) angle=-30;
    //         kinect.setCameraTiltAngle(angle);
    //     }
    //     else if (key == OF_KEY_RIGHT)
    //     {
    //         vector<ofxUICanvas *>::iterator it = guis.begin();
    //         ofxUICanvas *guiCueList = *(it+6);
    //         ofxUIImageButton *button = (ofxUIImageButton *) guiCueList->getWidget("Next Cue");
    //         button->setValue(true);
    //         guiCueList->triggerEvent(button);
    //         button->setValue(false);
    //     }
    //     else if (key == OF_KEY_LEFT)
    //     {
    //         vector<ofxUICanvas *>::iterator it = guis.begin();
    //         ofxUICanvas *guiCueList = *(it+6);
    //         ofxUIImageButton *button = (ofxUIImageButton *) guiCueList->getWidget("Previous Cue");
    //         button->setValue(true);
    //         guiCueList->triggerEvent(button);
    //         button->setValue(false);
    //     }
    // }
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key)
{
}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y )
{
}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button)
{
}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button)
{
}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button)
{
}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h)
{
}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg)
{
}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo)
{
}
