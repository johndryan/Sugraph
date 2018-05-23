//
//  letter.hpp
//  Sugraph
//
//  Created by John Ryan on 5/22/18.
//

#ifndef letter_hpp
#define letter_hpp

#include "ofMain.h"
#include "ofxCv.h"

using namespace ofxCv;
using namespace cv;

class Letter : public ofxCv::RectFollower {
protected:
    char character;
    ofColor color;
    ofVec2f position, smooth;
public:
    Letter(){}
    void setup(const cv::Rect& track);
    void update(const cv::Rect& track);
    void kill();
    void draw();
};


#endif /* letter_hpp */
