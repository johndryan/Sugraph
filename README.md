# Súgraph
An interactive learning toy for my daughter's magnet wall. It uses OpenCV and a trainable [convnet](http://ml4a.github.io/ml4a/convnets/) classifier to identify magnetic letters on a surface and turn them into interactive games… (well, eventually when I finish!)

<p align="center">
  <img src="https://raw.githubusercontent.com/johndryan/Sugraph/master/documentation/photos/180521_early_prototype.jpg" title="Early Prototype" /><br />
  <img src="https://raw.githubusercontent.com/johndryan/Sugraph/master/documentation/screengrabs/180521_screengrab_debug_view.jpg" title="Screengrab of Training Interface" />
</p>

Built with [OpenFrameworks](https://openframeworks.cc) (v0.10.0)

## Installation

1. Use the `ProjectGenerator` (provided with OpenFrameworks) to update the base path to OpenFramework and install add-ons ([as per the ml4a guide](https://github.com/ml4a/ml4a-ofx#setup-your-project)).
2. Run `sh setup.sh` to download the necessary data.

## Usage Notes

1. *Calibration feature to follow…*
2. In `Adding Samples` state, click thumbnails and enter character on key to label. When several are selected, hit `add samples`. (For false positives, i.e. non-letters, enter `*` to train the system to ignore these.)
3. Repeat previous step until enough samples have been collected to train the model. Hint: Pausing the video feed makes this easier!
4. When ready, click `Train New Model`.
5. When trained, the system will automatically enter `Classifying` state and begin classifying.

### Webcam Fallback
If you don't have a webcam, or want to work on the code without a test setup, uncomment `line 20` of `ofApp.h`: `#define USE_VIDEO_FILE_INSTEAD_OF_WEBCAM`. Drop a 640x480 video ([like this one](https://www.dropbox.com/s/6f1q7ae8y06wi19/demofeed.mp4?dl=0)) into `bin/data/movies`. This will be used instead of the webcam stream.

### Add-ons

This application requires the following non-core addons:

- [ofxCv](https://github.com/kylemcdonald/ofxCv) 
- [ofxCcv](https://github.com/kylemcdonald/ofxCcv) 
- [ofxGrt](https://github.com/nickgillian/ofxGrt)
- [ofxFailSafeVideoGrabber](https://github.com/armadillu/ofxFailSafeVideoGrabber)

- - -

## Acknowledgments

This project began from a fork of [DoodleClassifier](http://ml4a.github.io/guides/DoodleClassifier/) ([github](https://github.com/ml4a/ml4a-ofx/tree/master/apps/DoodleClassifier)) by [Andreas Refsgaard](https://andreasrefsgaard.dk/) and [Gene Kogan](https://www.genekogan.com/).

- - - 

## **Sú… What Now?**

_Súgradh_ is the Irish for play, and _graph_ is… well… about drawing and showing the relationship between things and graphics and from the Greek for writing. So it's relevant…