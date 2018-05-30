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
    // ' ' is empty, '*' is false positive, other letters are their (char)
    characterLabel = ' ';
    state = NO_IMAGE;
    selected = false;
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

void Letter::assignLabel(char _assignedLabel) {
    characterLabel = _assignedLabel;
    state = LABEL_ASSIGNED;
}
    
void Letter::classify(char _classificationLabel, bool _isPrediction) {
    if (state == READY_TO_CLASSIFY) {
        characterLabel = _classificationLabel;
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

void Letter::drawThumb(int size, bool selectable) {
    // Only display selected states if selectable
    bool _selected = selected;
    if (state == IN_TRAINING_SET) selectable = false;
    if (!selectable) _selected = false;
    string labelStr = "";
    ofColor foreground = (_selected?ofColor::black:ofColor::white);
    ofColor background = (_selected?ofColor::white:ofColor::black);
    if (state == IN_TRAINING_SET) background = ofColor(70,70,70);
    // Draw image and fake stroke
    ofPushStyle();
    ofSetLineWidth(0);
    ofSetColor(background);
    ofDrawRectangle(0, 0, size, size);
    ofPopStyle();
    int outlineWidth = 4;
    img.draw(outlineWidth, outlineWidth, size-outlineWidth*2, size-outlineWidth*2);
    if (state == IN_TRAINING_SET) {
        // Fade image if in training set
        ofPushStyle();
        ofSetColor(70,70,70,127);
        ofDrawRectangle(0, 0, size, size);
        ofPopStyle();
    }
    // Draw Labels & Checkmark
    labelStr = ofToString(getLabel())+": "+characterLabel;
    ofDrawBitmapStringHighlight(labelStr, 4, size-24, background, foreground);
    labelStr = ofToString(getStateTitle(state));
    ofDrawBitmapStringHighlight(labelStr, 4, size-6, background, foreground);
//    ofDrawBitmapStringHighlight("{"+ofToString(rect.x)+","+ofToString(rect.y)+","+ofToString(rect.width)+","+ofToString(rect.height)+"}", 4, 28, background, foreground);
    if (selectable) drawCheckmark(size-24,8,_selected);
    // Save image location
    ofMatrix4x4 matrix = ofGetCurrentMatrix(OF_MATRIX_MODELVIEW);
    ofVec3f translation = matrix.getTranslation();
    thumbnailPosition.x = translation.x + ofGetWindowWidth()/2;
    thumbnailPosition.y = translation.y + ofGetWindowHeight()/2;
}

void Letter::drawCheckmark(int x, int y, bool _selected) {
    ofPushStyle();
    ofPushMatrix();
    ofSetLineWidth(2);
    ofNoFill();
    ofSetColor(_selected?ofColor::white:ofColor::black);
    ofTranslate(x, y);
    ofDrawRectangle(0, 0, 16, 16);
    if (_selected) {
        ofDrawLine(4, 8, 7, 11);
        ofDrawLine(12, 5, 7, 11);
    }
    ofPopMatrix();
    ofPopStyle();
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

char Letter::getCharacterLabel() {
    return characterLabel;
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

bool Letter::isLabelAssigned() {
    return (state == LABEL_ASSIGNED);
}

void Letter::setInTrainingSet() {
    selected = false;
    state = IN_TRAINING_SET;
}

void Letter::setSelection(bool _selection) {
    selected = _selection;
    if (state == IN_TRAINING_SET) selected = false;
}

const string Letter::getStateTitle(classificationState state) {
    switch(state) {
        case NO_IMAGE: return "NO_IMAGE";
        case HAS_IMAGE: return "HAS_IMAGE";
        case LABEL_ASSIGNED: return "LABEL_ASSIGNED";
        case IN_TRAINING_SET: return "IN_TRAINING_SET";
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
