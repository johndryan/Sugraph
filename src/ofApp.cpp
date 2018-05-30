#include "ofApp.h"

//--------------------------------------------------------------

bool isFoundSquareRightOfOther( Letter &a, Letter &b){
    return a.getRect().x > b.getRect().x;
}

//--------------------------------------------------------------
void ofApp::setup(){
    ofSetWindowShape(1600, 900);
    
    width = 640;
    height = 480;
    scrollPos = 0;
    thumbnailSize = 150;
    drawThumbnailsLocation.set(210, 0.75*height+25);
    
    ccv.setup(ofToDataPath("image-net-2012.sqlite3"));

    cam.start(width, height, 30, true, ofToDataPath("movies/demofeed.mp4"));
    #ifdef USE_VIDEO_FILE_INSTEAD_OF_WEBCAM
    ofLog(OF_LOG_NOTICE, "Using Video File instead of Webcam stream");
    cam.forceMovie();
    #endif
    
    bAdd.addListener(this, &ofApp::addSamplesToTrainingSetNext);
    bTrain.addListener(this, &ofApp::trainClassifier);
    bSave.addListener(this, &ofApp::saveModel);
    bLoad.addListener(this, &ofApp::loadModel);
    trainingLabel.addListener(this, &ofApp::setTrainingLabel);
    
    // There's got to be a better way to toggle an enum in ofxGui?
    systemState = CALIBRATION;
    inCalibrationState.addListener(this, &ofApp::enterCalibrationState);
    inAddingSamplesState.addListener(this, &ofApp::enterAddingSamplesState);
    inTrainingStateOn.addListener(this, &ofApp::enterTrainingState);
    inClassifyingState.addListener(this, &ofApp::enterClassifyingState);
    gSysState.setName("System State");
    gSysState.add(inCalibrationState.set("Calibrating", true));
    gSysState.add(inAddingSamplesState.set("Adding Samples", false));
    gSysState.add(inTrainingStateOn.set("Train New Model", false));
    gSysState.add(inClassifyingState.set("Classifying", false));
    
    gui.setup();
    gui.setName("SugraphClassifier");
    gCv.setName("CV initial");
    gCv.add(minArea.set("Min area", 14, 1, 100));
    gCv.add(maxArea.set("Max area", 26, 1, 500));
    gCv.add(squareness.set("Squareness (W/H)", 1.5, 1, 2));
    gCv.add(threshold.set("Threshold", 105, 0, 255));
    gCv.add(nDilate.set("Dilations", 3, 0, 8));
    gCvTracker.setName("Contour Tracker");
    gCvTracker.add(trackerPersist.set("Persistance (Frames)", 60, 0, 120));
    gCvTracker.add(trackerDist.set("Distance (Pixels)", 32, 10, 100));
    gui.add(trainingLabel.set("Training Label", 0, 0, classNames.size()-1));
    gui.add(bPause.setup("Pause Video", false));
    gui.add(bAdd.setup("Add samples"));
    gui.add(bTrain.setup("Train"));
    gui.add(bRunning.setup("Run", false));
    gui.add(bClassify.setup("Classify"));
    gui.add(bSave.setup("Save"));
    gui.add(bLoad.setup("Load"));
    
    gui.add(gSysState);
    gui.add(gCv);
    gui.add(gCvTracker);
    gui.setPosition(0, 400);
    gui.loadFromFile("settings_sugraphclassifier_cv.xml");
    
    fbo.allocate(width, height);
    colorImage.allocate(width, height);
    grayImage.allocate(width, height);
    isTrained = false;
    toAddSamples = false;
    
    trainingData.setNumDimensions(4096);
    AdaBoost adaboost;
    adaboost.enableNullRejection(false);
    adaboost.setNullRejectionCoeff(3);
    pipeline.setClassifier(adaboost);
    
    // wait before forgetting something
    letterTracker.setPersistence(trackerPersist);
    // an object can move up to X pixels per frame
    letterTracker.setMaximumDistance(trackerDist);
}

//--------------------------------------------------------------
void ofApp::update(){
    if (!bPause) {
    cam.update();
    }
    if(cam.isFrameNew())
    {
        // get grayscale image and threshold
        colorImage.setFromPixels(cam.getPixels());
        grayImage.setFromColorImage(colorImage);
        for (int i=0; i<nDilate; i++) {
            grayImage.erode_3x3();
        }
        grayImage.threshold(threshold);
        //grayImage.invert();
        
        // find initial contours
        contourFinder.setMinAreaRadius(minArea);
        contourFinder.setMaxAreaRadius(maxArea);
        contourFinder.setThreshold(127);
        contourFinder.findContours(grayImage);
        contourFinder.setFindHoles(true);
        
        // draw all contour bounding boxes to FBO
        fbo.begin();
        ofClear(0, 255);
        ofFill();
        ofSetColor(255);
        for (int i=0; i<contourFinder.size(); i++) {
            //cv::Rect rect = contourFinder.getBoundingRect(i);
            //ofDrawRectangle(rect.x, rect.y, rect.width, rect.height);
            ofBeginShape();
            for (auto p : contourFinder.getContour(i)) {
                ofVertex(p.x, p.y);
            }
            ofEndShape();
        }
        fbo.end();
        ofPixels pixels;
        fbo.readToPixels(pixels);
        
        // find merged contours
        contourFinder2.setMinAreaRadius(minArea);
        contourFinder2.setMaxAreaRadius(maxArea);
        contourFinder2.setThreshold(127);
        contourFinder2.findContours(pixels);
        contourFinder2.setFindHoles(true);
        
        // letterTracker
        // Could all be handled by a single manager Class?
        letterTracker.setPersistence(trackerPersist);
        letterTracker.setMaximumDistance(trackerDist);
        setImagesForNewLettersFound();
        
//        if (toAddSamples) {
//            addSamplesToTrainingSet();
//            toAddSamples = false;
//        }
    }

    // HANDLE VARIOUS SYSTEM STATES
    
    if (systemState == CLASSIFYING) {
        // Classify all letters that are ready
        vector<Letter>& letters = letterTracker.getFollowers();
        for (Letter & letter : letters) {
            if (letter.readyToClassify()) {
                classifyLetter(letter);
            }
        }
    }
}

//--------------------------------------------------------------
void ofApp::draw(){
    ofBackground(70);
    
    ofPushMatrix();
    ofScale(0.75, 0.75);
    
    // original
    ofPushMatrix();
    ofPushStyle();
    ofTranslate(0, 20);
    cam.draw(0, 0);
    ofDrawBitmapStringHighlight("original", 0, 0);
    ofPopMatrix();
    ofPopStyle();
    
    // thresholded
    ofPushMatrix();
    ofPushStyle();
    ofTranslate(width, 20);
    grayImage.draw(0, 0);
    ofSetColor(0, 255, 0);
    contourFinder.draw();
    // Draw tracker labels
    vector<Letter>& letters = letterTracker.getFollowers();
    for(int i = 0; i < letters.size(); i++) {
        letters[i].draw();
    }
    ofDrawBitmapStringHighlight("thresholded", 0, 0);
    ofPopMatrix();
    ofPopStyle();
    
    // merged
    ofPushMatrix();
    ofPushStyle();
    ofTranslate(2*width, 20);
    fbo.draw(0, 0);
    ofSetColor(0, 255, 0);
    //contourFinder2.draw();
    // Draw tracker labels
    for(int i = 0; i < letters.size(); i++) {
        letters[i].draw();
    }
    ofDrawBitmapStringHighlight("merged", 0, 0);
    ofPopMatrix();
    ofPopStyle();
    
    ofPopMatrix();
    
    // draw tiles
    ofPushMatrix();
    ofPushStyle();
    ofTranslate(drawThumbnailsLocation.x, drawThumbnailsLocation.y);
    int nPerRow = max(8, (int) ceil(letters.size()/3.0));
    // Basic scrolling: is there a better way?
    ofTranslate(-ofMap(scrollPos, 0, ofGetWidth(), 0, max(0,nPerRow-5)*(thumbnailSize+2)), 0);
    for (int i=0; i<letters.size(); i++) {
        ofPushMatrix();
        ofTranslate((thumbnailSize+2)*(i%nPerRow), (thumbnailSize+16)*floor(i/nPerRow));
        letters[i].drawThumb(thumbnailSize);
        ofPopMatrix();
    }
    ofPopMatrix();
    ofPopStyle();
    
    gui.draw();
}

//--------------------------------------------------------------
void ofApp::exit() {
    gui.saveToFile(ofToDataPath("settings_sugraphclassifier_cv.xml"));
}

//--------------------------------------------------------------
void ofApp::setImagesForNewLettersFound() {
    vector<unsigned int> letterTrackerLabels = letterTracker.track(contourFinder2.getBoundingRects());
    vector<Letter>& letters = letterTracker.getFollowers();
    const vector<unsigned int>& newLabels = letterTracker.getNewLabels();
    
    // Find the Letter for any new Labels and set their image
    for(int i = 0; i < newLabels.size(); i++) {
        int newLabel = newLabels[i];
        // Find position of Letter with that Label
        vector<unsigned int>::const_iterator index = find(letterTrackerLabels.begin(), letterTrackerLabels.end(), newLabel);
        if (index != letterTrackerLabels.end()) {
            int iteratorPosition = distance<vector<unsigned int>::const_iterator> (letterTrackerLabels.begin(), index);
            // If iteratorPosition is inside letters, then update image
            if (iteratorPosition >= 0 && iteratorPosition < letters.size()) {
                // Kill if not square enough, helpful for my letters
                if (letters[iteratorPosition].isItSquareEnough(squareness)) {
                    // Do I need to copy of an ofImage?
                    ofImage camTempCopy;
                    camTempCopy.setFromPixels(colorImage.getPixels());
                    letters[iteratorPosition].setImage(camTempCopy);
                }
            }
        }
    }
}

//--------------------------------------------------------------
void ofApp::addSamplesToTrainingSet() {
//    ofLog(OF_LOG_NOTICE, "Adding samples...");
//    gatherFoundSquares();
//    for (int i=0; i<foundSquares.size(); i++) {
//        foundSquares[i].label = classNames[trainingLabel];
//        vector<float> encoding = ccv.encode(foundSquares[i].img, ccv.numLayers()-1);
//        VectorFloat inputVector(encoding.size());
//        for (int i=0; i<encoding.size(); i++) inputVector[i] = encoding[i];
//        trainingData.addSample(trainingLabel, inputVector);
//        ofLog(OF_LOG_NOTICE, " Added sample #"+ofToString(i)+" label="+ofToString(trainingLabel));
//    }
}

//--------------------------------------------------------------
void ofApp::trainClassifier() {
    ofLog(OF_LOG_NOTICE, "Training...");
    if (pipeline.train(trainingData)){
        ofLog(OF_LOG_NOTICE, "getNumClasses: "+ofToString(pipeline.getNumClasses()));
    }
    isTrained = true;
    ofLog(OF_LOG_NOTICE, "Done training...");
}

//--------------------------------------------------------------
void ofApp::classifyLetter(Letter & letter) {
    ofLog(OF_LOG_NOTICE, "Classifiying Letter "+ofToString(letter.getLabel()));
    vector<float> encoding = ccv.encode(letter.getImage(), ccv.numLayers()-1);
    VectorFloat inputVector(encoding.size());
    for (int i=0; i<encoding.size(); i++) inputVector[i] = encoding[i];
    if (pipeline.predict(inputVector)) {
        // gt classification
        int classification = pipeline.getPredictedClassLabel();
        string letterClassLookup = classNames[classification];
        letter.classify(letterClassLookup, true);
    } else {
        // TODO Does this mean prediction was negative or failed?
        letter.classify("Unknown", false);
    }
}

//--------------------------------------------------------------
void ofApp::setTrainingLabel(int & label_) {
    trainingLabel.setName(classNames[label_]);
}

//--------------------------------------------------------------
void ofApp::saveModel() {
    pipeline.save(ofToDataPath("sugraphclassifier_model.grt"));
}

//--------------------------------------------------------------
void ofApp::loadModel() {
    ofLog(OF_LOG_NOTICE, "LOAD CLASSIFIER MODEL");
    pipeline.load(ofToDataPath("sugraphclassifier_model.grt"));
    isTrained = true;
}

//--------------------------------------------------------------
void ofApp::addSamplesToTrainingSetNext() {
    toAddSamples = true;
}

//--------------------------------------------------------------
// Find a better way to handle this toggle…
void ofApp::enterCalibrationState(bool& val) {
    if (val) setSystemStateTo(CALIBRATION);
}
void ofApp::enterAddingSamplesState(bool& val) {
    if (val) setSystemStateTo(ADDING_SAMPLES);
}
void ofApp::enterTrainingState(bool& val) {
    if (val) setSystemStateTo(TRAINING);
}
void ofApp::enterClassifyingState(bool& val) {
    // Only start classifying if trained model present
    if (val) {
        if (isTrained) {
            setSystemStateTo(CLASSIFYING);
        } else {
            inClassifyingState = false;
            ofLog(OF_LOG_NOTICE, "No Classifier Model. Trying loading or creating one?");
        }
    }
}
void ofApp::setSystemStateTo(sugraphSystemStates _state) {
    systemState = _state;
    inCalibrationState = false;
    inAddingSamplesState = false;
    inTrainingStateOn = false;
    inClassifyingState = false;
    string message = "Entering System State: ";
    switch(_state) {
    case CALIBRATION:
        inCalibrationState = true;
            message += "CALIBRATION";
            break;
    case ADDING_SAMPLES:
        inAddingSamplesState = true;
            message += "ADDING_SAMPLES";
            break;
    case TRAINING:
        inTrainingStateOn = true;
            message += "TRAINING";
            break;
    case CLASSIFYING:
        inClassifyingState = true;
            message += "CLASSIFYING";
            break;
    }
    ofLog(OF_LOG_NOTICE, message);
}

//--------------------------------------------------------------
// KEYBOARD & MOUSE
//--------------------------------------------------------------

void ofApp::mouseDragged(int x, int y, int button) {

}
void ofApp::mousePressed(int x, int y, int button){
    if (x > drawThumbnailsLocation.x && y > drawThumbnailsLocation.y) {
        checkLettersForMouseClick(x,y);
    }
}
void ofApp::mouseReleased(int x, int y, int button){

}
void ofApp::mouseScrolled(int x, int y, float scrollX, float scrollY) {
    scrollPos += scrollX * 10;
}

void ofApp::keyPressed  (int key){
    string message = "keyPressed: " + ofToString((char)key);
    ofLog(OF_LOG_NOTICE, message);
}

void ofApp::keyReleased(int key){
    
}

void ofApp::checkLettersForMouseClick(int x, int y) {
    vector<Letter>& letters = letterTracker.getFollowers();
    for (Letter & letter : letters) {
        // Check if any letter was clicked, and stop whenever you discover one
        if (letter.toggleSelectedIfMouseClickIsInside(x, y, thumbnailSize)) break;
    }
}
