//
//  letter.cpp
//  Sugraph
//
//  Created by John Ryan on 5/22/18.
//

#include "letter.h"

const string Letter::getStateTitle(classificationState state) {
    switch(state) {
        case NO_IMAGE: return "NO_IMAGE";
        case HAS_IMAGE: return "HAS_IMAGE";
        case READY_TO_CLASSIFY: return "READY_TO_CLASSIFY";
        case CLASSIFIED: return "CLASSIFIED";
    }
}

void Letter::setup(const cv::Rect& track) {
    color.setHsb(ofRandom(0, 255), 255, 255);
    position = toOf(track).getCenter();
    rect = track;
    smooth = position;
    characterLabel.set("Unknown");
    state = NO_IMAGE;
}

void Letter::setImage(const ofImage & _img) {
    if (rect.x < _img.getWidth()
        && rect.y < _img.getHeight()
        && rect.width < _img.getWidth()
        && rect.height < _img.getHeight()) {
        // Copy and crop image
        img.setFromPixels(_img.getPixels());
        img.crop(rect.x, rect.y, rect.width, rect.height);
        img.resize(224, 224);
        state = READY_TO_CLASSIFY;
    } else {
        string message = "Image failed to be set for Letter " + ofToString(label) + ": crop rect did not fall within bounds of passed image.";
        ofLog(OF_LOG_NOTICE, message);
    }
}

ofImage Letter::getImage() {
    return img;
}

void Letter::classify(string _classificationLabel, bool _isPrediction) {
    if (state == READY_TO_CLASSIFY) {
        characterLabel.set(_classificationLabel);
        state = CLASSIFIED;
        // If no classification, remove this letter
        if (!_isPrediction) {
            kill();
        }
    }
}

void Letter::update(const cv::Rect& track) {
    position = toOf(track).getCenter();
    smooth.interpolate(position, .5);
    // If life is long enough, then classify. This same classifying blobs that don't last for that long.
    if (state == HAS_IMAGE && getAge() > ageBeforeClassifiying) {
        state = READY_TO_CLASSIFY;
    }
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
    string labelToDraw = ofToString(label);// + ": " + characterLabel;
    ofDrawBitmapStringHighlight(labelToDraw, position, ofColor(0,150));
    ofPopStyle();
}

void Letter::drawThumb(int size) {
    img.draw(0, 0, size, size);
    string labelStr = "no class";
    labelStr = ofToString(getLabel())+":"+ofToString(getStateTitle(state))+":"+characterLabel;
    ofDrawBitmapStringHighlight(labelStr, 4, 4);
    ofDrawBitmapStringHighlight("{"+ofToString(rect.x)+","+ofToString(rect.y)+","+ofToString(rect.width)+","+ofToString(rect.height)+"}", 4, 21);
}

bool Letter::isItSquareEnough(float squareness) {
    if (rect.width > squareness*rect.height || rect.height > squareness*rect.width) {
        kill();
        return false;
    } else {
        return true;
    }
}

cv::Rect Letter::getRect() {
    return rect;
}

int Letter::getLabel() {
    return label;
}

bool Letter::readyToClassify() {
    if (state == READY_TO_CLASSIFY) {
        return true;
    } else {
        return false;
    }
}
