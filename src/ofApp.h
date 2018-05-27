#pragma once

#include "ofMain.h"
#include "ofxOpenCv.h"
#include "ofxCv.h"
#include "ofxGui.h"
#include "ofxGrt.h"
#include "ofxCcv.h"
#include "ofxOsc.h"
#include "ofxXmlPoco.h"
#include "ofxFailSafeVideoGrabber.h"
#include "letter.h"

using namespace ofxCv;
using namespace cv;

//#define RELEASE

// Video or webcam?
#define USE_VIDEO_FILE_INSTEAD_OF_WEBCAM
// Speak?
// #define USE_SYSTEM_SPEECH

class ofApp : public ofBaseApp
{
public:

    enum sugraphSystemStates: short{
        CALIBRATION,
        ADDING_SAMPLES,
        TRAINING,
        CLASSIFYING
    };
    void enterCalibrationState(bool& val);
    void enterAddingSamplesState(bool& val);
    void enterTrainingState(bool& val);
    void enterClassifyingState(bool& val);
    void setSystemStateTo(sugraphSystemStates _state);
    
    vector<string> classNames =
    {
        "0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "A", "B", "C", "D", "E", "F", "G", "H", "I", "G", "K", "L", "M", "N", "O", "P", "Q", "R", "S", "T", "U", "V", "W", "X", "Y", "Z", "a", "b", "c", "d", "e", "f", "g", "h", "i", "j", "k", "l", "m", "n", "o", "p", "q", "r", "s", "t", "u", "v", "w", "x", "y", "z"
    };

    void setup();
    void update();
    void draw();
    void exit();
    
    void setTrainingLabel(int & label_);
    void addSamplesToTrainingSet();
    void trainClassifier();
    void classifyLetter(Letter & letter);
    
    void addSamplesToTrainingSetNext();
    
    void saveModel();
    void loadModel();
    
    int width, height;
    
    ofxFailSafeVideoGrabber cam;
    
    ContourFinder contourFinder, contourFinder2;
    ofFbo fbo;
    ofxCvGrayscaleImage grayImage;
    ofxCvColorImage colorImage;
    
    ofxPanel gui;
    ofxToggle bRunning, bPause;
    ofxButton bAdd, bTrain, bClassify, bSave, bLoad;
    ofParameterGroup gSysState, gCv, gCvTracker;
    ofParameter<float> minArea, maxArea, squareness, threshold, trackerPersist, trackerDist;
    ofParameter<int> nDilate, trainingLabel;
    ofParameter<bool> inCalibrationState, inAddingSamplesState, inTrainingStateOn, inClassifyingState;
    
    ofxCv::RectTrackerFollower<Letter> letterTracker;
    
    ClassificationData trainingData;
    GestureRecognitionPipeline pipeline;
    ofxCcv ccv;
    bool isTrained, toAddSamples;
    
    sugraphSystemStates systemState;
};
