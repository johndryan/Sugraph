//
//  letter.h
//  Sugraph
//
//  Created by John Ryan on 5/22/18.
//

#ifndef letter_h
#define letter_h

#include "ofMain.h"
#include "ofxOpenCv.h"
#include "ofxCv.h"
#include "ofxGui.h"

using namespace ofxCv;
using namespace cv;

class Letter : public ofxCv::RectFollower {
protected:
    ofParameter<string> characterLabel;
    
    ofImage img;
    bool isPrediction = false;
    cv::Rect rect;
    
    ofColor color;
    ofVec2f position, smooth;
    
    // Maybe replace with a more lightweight approach than a full GUI for each? Only needed for training
    ofxPanel gui;
    ofxButton buttonDelete;
public:
    Letter(){}
    void setup(const cv::Rect& track);
    void setImage(const ofImage & _img);
    void classify();
    void update(const cv::Rect& track);
    void kill();
    void draw();
    void drawThumb(int size);
    bool isItSquareEnough(float squareness);
    cv::Rect getRect();
    int getLabel();
    
    enum classificationState: short{
        NO_IMAGE,
        UNCLASSIFIED_IMAGE,
        CLASSIFIED_IMAGE
    };
    classificationState state;
};


#endif /* letter_h */
