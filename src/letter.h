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
    ofParameter<string> characterLabel;
    
    ofImage img;
    cv::Rect rect;
    
    ofColor color;
    ofVec2f position, smooth;
    
    // Should get this from an parameter in Tracker eventually
    unsigned int ageBeforeClassifiying = 0;
    
    enum classificationState: short{
        NO_IMAGE,
        HAS_IMAGE,
        READY_TO_CLASSIFY,
        CLASSIFIED
    };
    const string getStateTitle(classificationState state);
    classificationState state;
public:
    Letter(){}
    void setup(const cv::Rect& track);
    void setImage(const ofImage & _img);
    ofImage getImage();
    void classify(string _classificationLabel, bool _isPrediction);
    void update(const cv::Rect& track);
    void kill();
    void draw();
    void drawThumb(int size);
    bool isItSquareEnough(float squareness);
    cv::Rect getRect();
    int getLabel();
    bool readyToClassify();
};

#endif /* letter_h */
