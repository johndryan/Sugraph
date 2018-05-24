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
    rect = track;
    smooth = position;
    characterLabel = "N/A";
    // Height/width Should be inherited from main app somehow?
    // img.allocate(640, 480, OF_IMAGE_COLOR);
    // Should I check width/height ratio here and kill if not square?
}

void Letter::setImage(const ofxCvColorImage * camImage) {
    string message = "setImage to " + ofToString(camImage->getWidth()) + " x " + ofToString(camImage->getHeight()) + " image.";
    ofLog(OF_LOG_NOTICE, message);
// COMMENTED OUT BECAUSE I CAN'T GET IT WORKING YET!
//    img.setFromPixels(camImage->getPixels());
//    img.update();
//    img.crop(rect.x, rect.y, rect.width, rect.height);
//    img.resize(224, 224);
}

void Letter::classify() {
    // if img is Set
    if (img.isAllocated()) {
        
        // Classify image here
        isPrediction = true;
        
        // If no classification, remove this letter
        if (!isPrediction) {
            kill();
        }
    }
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
    string labelToDraw = ofToString(label) + ": " + characterLabel;
    ofDrawBitmapString(labelToDraw, position);
    ofPopStyle();
}

void Letter::drawThumb() {
    img.draw(0, 0);
    string labelStr = "no class";
    labelStr = (isPrediction?"predicted: ":"assigned: ")+characterLabel;
    ofDrawBitmapStringHighlight(labelStr, 4, img.getHeight()-22);
    ofDrawBitmapStringHighlight("{"+ofToString(rect.x)+","+ofToString(rect.y)+","+ofToString(rect.width)+","+ofToString(rect.height)+"}", 4, img.getHeight()-5);
}

cv::Rect Letter::getRect() {
    return rect;
}
