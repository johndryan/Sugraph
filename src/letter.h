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
    char characterLabel;
    
    ofImage img;
    cv::Rect rect;
    
    ofColor color;
    ofVec2f position, smooth, thumbnailPosition;
    
    // Should get this from an parameter in Tracker eventually
    unsigned int ageBeforeClassifiying = 0;
    
    enum classificationState: short{
        NO_IMAGE,
        HAS_IMAGE,
        LABEL_ASSIGNED,
        IN_TRAINING_SET,
        READY_TO_CLASSIFY,
        CLASSIFIED
    };
    const string getStateTitle(classificationState state);
    classificationState state;
    
    bool selected;
public:
    Letter(){}
    void setup(const cv::Rect& track);
    void setImage(const ofImage & _img);
    ofImage getImage();
    void assignLabel(char _assignedLabel);
    void classify(char _classificationLabel, bool _isPrediction);
    void update(const cv::Rect& track);
    void kill();
    void draw();
    void drawThumb(int size, bool selectable);
    void drawCheckmark(int x, int y, bool selected);
    bool isItSquareEnough(float squareness);
    cv::Rect getRect();
    // It's a bit confusing that the countourtracker 'label' and the class label both use that term?
    int getLabel();
    char getCharacterLabel();
    bool readyToClassify();
    bool isSelected();
    bool isLabelAssigned();
    void setInTrainingSet();
    void setSelection(bool _selected);
    bool toggleSelectedIfMouseClickIsInside(int x, int y, int thumbnailSize);
};

#endif /* letter_h */
