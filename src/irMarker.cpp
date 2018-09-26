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

// #include "irMarker.h"

// // using namespace ofxCv;
// // using namespace cv;

// irMarker::irMarker()
// {
//     m_startedDying = 0;
//     m_dyingTime = 3;
//     m_hasDisappeared = true;
// }

// void irMarker::setup(const cv::Rect& track)
// {
//     m_color.setHsb(ofRandom(0, 255), 255, 255);
//     m_currentPos = toOf(track).getCenter();
//     m_smoothPos = currentPos;
//     m_previousPos = currentPos;
//     m_path.curveTo(m_smoothPos); // necessary duplicate first point for control point
//     m_hasDisappeared = false;
// }

// void irMarker::update(const cv::Rect& track)
// {
//     m_currentPos = toOf(track).getCenter();
//     m_smoothPos.interpolate(currentPos, .5f);
//     m_path.curveTo(m_smoothPos);
//     m_velocity = m_smoothPos - m_previousPos;
//     m_previousPos = m_smoothPos;
// }

// void irMarker::updateLabels(vector<unsigned int> deadLabels, vector<unsigned int> currentLabels)
// {
//     // Labels that have disappeared but can appear again
//     if (find(deadLabels.begin(), deadLabels.end(), label) != deadLabels.end())
//     {
//         m_hasDisappeared = true;
//     }

//     // Labels that are currently being tracked
//     if (find(currentLabels.begin(), currentLabels.end(), label) != currentLabels.end())
//     {
//         m_hasDisappeared = false;
//     }
// }

// void irMarker::draw()
// {
//     ofPushStyle();
//     float size = 16;
//     ofFill();
//     ofSetLineWidth(3);
//     if (m_startedDying)
//     {
//         ofNoFill();
//         size = ofMap(ofGetElapsedTimef() - m_startedDying, 0, dyingTime, size, 0, true);
//     }
//     ofSetColor(color);
//     ofDrawCircle(currentPos, size);
//     ofSetColor(color.getInverted());
//     ofDrawBitmapString(ofToString(label), currentPos.x-2, currentPos.y+2);
//     ofPopStyle();
// }

// void irMarker::drawPath()
// {
//     ofPushStyle();
//     ofSetColor(color);
//     ofSetLineWidth(1.5f);
//     m_path.draw();
//     ofPopStyle();
// }

// void irMarker::kill()
// {
//     float currentTime = ofGetElapsedTimef();
//     if (m_startedDying == 0)
//     {
//         m_startedDying = currentTime;
//         m_path.curveTo(m_smoothPos); // necessary duplicate last point for control point
//     }
//     else if ((currentTime - m_startedDying) > dyingTime)
//     {
//         m_dead = true;
//     }
// }

// void irMarker::clearPath()
// {
//     m_path.clear();
// }
