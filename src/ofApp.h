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

    vector<string> classNames =
    {
        "E",
        "M",
        "B",
        "R"
    };

    void setup();
    void update();
    void draw();
    void exit();
    
    void setTrainingLabel(int & label_);
    void addSamplesToTrainingSet();
    void gatherFoundSquares();
    void trainClassifier();
    void classifyCurrentSamples();
    
    void addSamplesToTrainingSetNext();
    void classifyNext();
    
    void save();
    void load();
    
    int width, height;
    
    #ifdef USE_VIDEO_FILE_INSTEAD_OF_WEBCAM
    #else
    #endif
    ofxFailSafeVideoGrabber cam;
    
    ContourFinder contourFinder, contourFinder2;
    ofFbo fbo;
    ofxCvGrayscaleImage grayImage;
    ofxCvColorImage colorImage;
    
    ofxPanel gui;
    ofxToggle bRunning, bPause;
    ofxButton bAdd, bTrain, bClassify, bSave, bLoad;
    ofParameter<float> minArea, maxArea, threshold, trackerPersist, trackerDist;
    ofParameter<int> nDilate;
    ofParameter<int> trainingLabel;
    
    ofxCv::RectTrackerFollower<Letter> letterTracker;
    
    ClassificationData trainingData;
    GestureRecognitionPipeline pipeline;
    ofxCcv ccv;
    bool isTrained, toAddSamples, toClassify;
    
    string allFoundChars;
};
