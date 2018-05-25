#include "ofApp.h"

//--------------------------------------------------------------

bool isFoundSquareRightOfOther( Letter &a, Letter &b){
    return a.getRect().x > b.getRect().x;
}

//--------------------------------------------------------------
void ofApp::setup(){
    ofSetWindowShape(1600, 900);
    systemState = ADDING_SAMPLES;
    
    width = 640;
    height = 480;
    
    ccv.setup(ofToDataPath("image-net-2012.sqlite3"));

    cam.start(width, height, 30, true, ofToDataPath("movies/demofeed.mp4"));
    #ifdef USE_VIDEO_FILE_INSTEAD_OF_WEBCAM
    ofLog(OF_LOG_NOTICE, "Using Video File instead of Webcam stream");
    cam.forceMovie();
    #endif
    
    bAdd.addListener(this, &ofApp::addSamplesToTrainingSetNext);
    bTrain.addListener(this, &ofApp::trainClassifier);
    bClassify.addListener(this, &ofApp::classifyNext);
    bSave.addListener(this, &ofApp::save);
    bLoad.addListener(this, &ofApp::load);
    trainingLabel.addListener(this, &ofApp::setTrainingLabel);
    
    gui.setup();
    gui.setName("SugraphClassifier");
    ofParameterGroup gCv, gCvTracker;
    gCv.setName("CV initial");
    gCv.add(minArea.set("Min area", 10, 1, 100));
    gCv.add(maxArea.set("Max area", 200, 1, 500));
    gCv.add(squareness.set("Squareness (W/H)", 1.5, 1, 2));
    gCv.add(threshold.set("Threshold", 128, 0, 255));
    gCv.add(nDilate.set("Dilations", 1, 0, 8));
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
    gui.add(gCv);
    gui.add(gCvTracker);
    gui.setPosition(0, 400);
    gui.loadFromFile("settings_sugraphclassifier_cv.xml");
    
    fbo.allocate(width, height);
    colorImage.allocate(width, height);
    imgTest.allocate(width, height, OF_IMAGE_COLOR);
    grayImage.allocate(width, height);
    isTrained = false;
    toAddSamples = false;
    toClassify = false;
    
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
        imgTest.setFromPixels(colorImage.getPixels());
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
        
        if (toAddSamples) {
            addSamplesToTrainingSet();
            toAddSamples = false;
        }
        else if (isTrained && (bRunning || toClassify)) {
            classifyCurrentSamples();
            toClassify = false;
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
    
    int tileSize = 150; // 224
    
    // draw tiles
    ofPushMatrix();
    ofPushStyle();
    ofTranslate(210, 0.75*height+25);
    int nPerRow = max(8, (int) ceil(letters.size()/3.0));
    // Basic scrolling: is there a better way?
    ofTranslate(-ofMap(ofGetMouseX(), 0, ofGetWidth(), 0, max(0,nPerRow-5)*(tileSize+2)), 0);
    for (int i=0; i<letters.size(); i++) {
        ofPushMatrix();
        ofTranslate((tileSize+2)*(i%nPerRow), (tileSize+16)*floor(i/nPerRow));
        // TODO: Fix letter thumbnail images
        // Temp fix to show something
//        ofImage img;
//        img.setFromPixels(colorImage.getPixels());
//        img.crop(letters[i].getRect().x, letters[i].getRect().y, letters[i].getRect().width, letters[i].getRect().height);
//        img.resize(tileSize, tileSize);
//        img.draw(0, 0);
        // ^ remove above, and handle in class method below v
        letters[i].drawThumb(tileSize);
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
void ofApp::gatherFoundSquares() {    
//    foundSquares.clear();
//    for (int i=0; i<contourFinder2.size(); i++) {
//        FoundSquare fs;
//        fs.rect = contourFinder2.getBoundingRect(i);
//        fs.area = contourFinder2.getContourArea(i);
//        fs.img.setFromPixels(cam.getPixels());
//        fs.img.crop(fs.rect.x, fs.rect.y, fs.rect.width, fs.rect.height);
//        fs.img.resize(224, 224);
//        foundSquares.push_back(fs);
//    }
//    // Sort by X position
//    ofSort(foundSquares,isFoundSquareRightOfOther);
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
void ofApp::classifyCurrentSamples() {
//    ofLog(OF_LOG_NOTICE, "Classifiying on frame "+ofToString(ofGetFrameNum()));
//    gatherFoundSquares();
//    string theseChars = "";
//    for (int i=0; i<foundSquares.size(); i++) {
//        vector<float> encoding = ccv.encode(foundSquares[i].img, ccv.numLayers()-1);
//        VectorFloat inputVector(encoding.size());
//        for (int i=0; i<encoding.size(); i++) inputVector[i] = encoding[i];
//        if (pipeline.predict(inputVector)) {
//            // gt classification
//            int label = pipeline.getPredictedClassLabel();
//            foundSquares[i].isPrediction = true;
//            foundSquares[i].label = classNames[label];
//
//            // Add to allChars
//            theseChars += foundSquares[i].label+" ";
//        }
//    }
//    if (allFoundChars != theseChars) {
//        // New letter/order found
//        ofLog(OF_LOG_NOTICE, "New letters found: "+theseChars);
//        #ifdef USE_SYSTEM_SPEECH
//        string cmd = "say '" + theseChars + "'";
//        system(cmd.c_str());
//        #endif
//        if (theseChars == "E M B E R ") {
//            ofLog(OF_LOG_NOTICE, "EMBER FOUND!");
//            #ifdef USE_SYSTEM_SPEECH
//            system("say 'You spelled Ember'");
//            #endif
//        }
//        allFoundChars = theseChars;
//    }
}

//--------------------------------------------------------------
void ofApp::setTrainingLabel(int & label_) {
    trainingLabel.setName(classNames[label_]);
}

//--------------------------------------------------------------
void ofApp::save() {
    pipeline.save(ofToDataPath("sugraphclassifier_model.grt"));
}

//--------------------------------------------------------------
void ofApp::load() {
    pipeline.load(ofToDataPath("sugraphclassifier_model.grt"));
    isTrained = true;
}

//--------------------------------------------------------------
void ofApp::classifyNext() {
    toClassify = true;
}

//--------------------------------------------------------------
void ofApp::addSamplesToTrainingSetNext() {
    toAddSamples = true;
}
