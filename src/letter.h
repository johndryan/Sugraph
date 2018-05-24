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

using namespace ofxCv;
using namespace cv;

class Letter : public ofxCv::RectFollower {
protected:
    string characterLabel;
    
    ofImage img;
    bool isPrediction = false;
    cv::Rect rect;
    
    ofColor color;
    ofVec2f position, smooth;
public:
    Letter(){}
    void setup(const cv::Rect& track);
    void setImage(const ofxCvColorImage * camImage);
    void classify();
    void update(const cv::Rect& track);
    void kill();
    void draw();
    void drawThumb();
    cv::Rect getRect();
};


#endif /* letter_h */
