/*
 * Copyright (C) 2015 Fabia Serra Arrizabalaga
 *
 * This file is part of Crea
 *
 * Crea is free software: you can redistribute it and/or modify it under
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

#pragma once
#include "ofMain.h"
#include "ofxCv.h"

class irMarker : public ofxCv::RectFollower{
    public:
        irMarker();

        void setup(const cv::Rect& track);
        void update(const cv::Rect& track);
        void updateLabels(vector<unsigned int> deadLabels, vector<unsigned int> currentLabels);
        void draw();
        void drawPath();
        void kill();
        void clearPath();
        //--------------------------------------------------------------
        ofPoint currentPos;
        ofPoint previousPos;
        ofPoint smoothPos;
        ofPoint velocity;
        //--------------------------------------------------------------
        ofColor color;
        ofPolyline all;
        //--------------------------------------------------------------
        float startedDying;
        float dyingTime;
        float timeDead;
        //--------------------------------------------------------------
        bool hasDisappeared;
};
