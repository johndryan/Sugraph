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

    void setup();
    void update();
    void draw();
    void exit();
    
    void keyPressed(int key);
    void keyReleased(int key);
    void mouseDragged(int x, int y, int button);
    void mousePressed(int x, int y, int button);
    void mouseReleased(int x, int y, int button);
    void mouseScrolled(int x, int y, float scrollX, float scrollY);
    float scrollPos;
    
    void addSamplesToTrainingSet();
    void trainClassifier();
    void classifyLetter(Letter & letter);
    void setImagesForNewLettersFound();
    void addSamplesToTrainingSetNext();
    void checkLettersForMouseClick(int x, int y);
    
    void saveModel();
    void loadModel();
    
    int width, height, thumbnailSize;
    ofVec2f drawThumbnailsLocation;
    
    ofxFailSafeVideoGrabber cam;
    
    ContourFinder contourFinder, contourFinder2;
    ofFbo fbo;
    ofxCvGrayscaleImage grayImage;
    ofxCvColorImage colorImage;
    
    ofxPanel gui;
    ofxToggle bPause;
    ofxButton bAdd, bSave, bLoad;
    ofParameterGroup gSysState, gCv, gCvTracker;
    ofParameter<float> minArea, maxArea, squareness, threshold, trackerPersist, trackerDist;
    ofParameter<int> nDilate;
    ofParameter<bool> inCalibrationState, inAddingSamplesState, inTrainingState, inClassifyingState;
    
    ofxCv::RectTrackerFollower<Letter> letterTracker;
    
    ClassificationData trainingData;
    GestureRecognitionPipeline pipeline;
    ofxCcv ccv;
    bool isTrained, toAddSamples;
    char falsePositiveCharacter = '*';
    
    sugraphSystemStates systemState;
};
