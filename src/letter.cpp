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
    characterLabel.set("Unknown");
    state = NO_IMAGE;
    selected = true;
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

void Letter::assignLabel(string _assignedLabel) {
    characterLabel.set(_assignedLabel);
    state = LABEL_ASSIGNED;
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
    if (selected) {
        ofPushStyle();
        ofNoFill();
        ofSetColor(ofColor::white);
        ofSetLineWidth(4);
        ofDrawRectangle(1, 1, size-4, size-4);
        ofPopStyle();
    }
    string labelStr = "no class";
    ofColor foreground = (selected?ofColor::black:ofColor::white);
    ofColor background = (selected?ofColor::white:ofColor::black);
    labelStr = (selected?"☑︎ ":"☒ ")+ofToString(getLabel())+":"+ofToString(getStateTitle(state))+":"+characterLabel;
    ofDrawBitmapStringHighlight(labelStr, 4, 4, background, foreground);
    ofDrawBitmapStringHighlight("{"+ofToString(rect.x)+","+ofToString(rect.y)+","+ofToString(rect.width)+","+ofToString(rect.height)+"}", 4, 21, background, foreground);
    // Save image location
    ofMatrix4x4 matrix = ofGetCurrentMatrix(OF_MATRIX_MODELVIEW);
    ofVec3f translation = matrix.getTranslation();
    thumbnailPosition.x = translation.x + ofGetWindowWidth()/2;
    thumbnailPosition.y = translation.y + ofGetWindowHeight()/2;
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

bool Letter::isSelected() {
    return selected;
}

void Letter::setSelection(bool _selection) {
    selected = _selection;
}

const string Letter::getStateTitle(classificationState state) {
    switch(state) {
        case NO_IMAGE: return "NO_IMAGE";
        case HAS_IMAGE: return "HAS_IMAGE";
        case LABEL_ASSIGNED: return "LABEL_ASSIGNED";
        case READY_TO_CLASSIFY: return "READY_TO_CLASSIFY";
        case CLASSIFIED: return "CLASSIFIED";
    }
}

bool Letter::toggleSelectedIfMouseClickIsInside(int x, int y, int thumbnailSize) {
    if (x > thumbnailPosition.x
        && y > thumbnailPosition.y
        && x < (thumbnailPosition.x + thumbnailSize)
        && y < (thumbnailPosition.y + thumbnailSize)) {
        // Successful hit
        selected = !selected;
        return true;
    } else {
        return false;
    }
}
