//
//  letter.cpp
//  Sugraph
//
//  Created by John Ryan on 5/22/18.
//

#include "letter.h"

void Letter::setup(const cv::Rect& track) {
    color.setHsb(ofRandom(0, 255), 255, 255);
    position = toOf(track).getCenter();
    smooth = position;
    character = 'E';
}

void Letter::update(const cv::Rect& track) {
    position = toOf(track).getCenter();
    smooth.interpolate(position, .5);
}

void Letter::kill() {
    dead = true;
}

void Letter::draw() {
    ofPushStyle();
    float size = 32;
    ofNoFill();
    ofSetColor(color);
    ofDrawCircle(position, size);
    string labelToDraw = ofToString(label) + ": " + character;
    ofDrawBitmapString(labelToDraw, position);
    ofPopStyle();
}
