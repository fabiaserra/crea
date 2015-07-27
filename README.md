CREA
====
Interactive system that generates computer graphics using the motion data from the Kinect camera. Written for openFrameworks 0.8.4 (http://openframeworks.cc/). 

<a href="https://www.youtube.com/watch?v=D07h5T6-29I" target="_blank"><img src="http://img.youtube.com/vi/D07h5T6-29I/0.jpg" 
alt="CREA VIDEO" width="480" height="360" border="5" /></a>

Video tutorial explaining the interface: https://www.youtube.com/watch?v=i2qrqvZc22Y

##Requirements
To use _CREA_ you will need:
* A Windows, Mac OS X or Linux computer with modern graphics.
*  Xbox Kinect Sensor, preferably model 1414, with USB adapter and Power Supply.
*  Projector or LCD screen.
* _CREA_ software.
* openFrameworks 0.8.4.

##Installation

1. Download openFrameworks in your computer (http://openframeworks.cc/download/). Extract the folder you downloaded and put it somewhere convenient.

2. Download or clone the repository of _CREA_ (https://github.com/fabiaserra/crea) and place it inside the openFrameworks folder, in _apps/myApps_. You should have a folder named _crea_ with two folders inside, _src_ and _bin_, and a few more files. The _src_ folder contains C++ source codes for your project. The _bin_ folder contains the _data_ subfolder where all the content files needed for _CREA_ are located: 
  * __cues__: Cue XML files.
  * __fonts__: Font files.
  * __kinect__: Sequence of images to use _CREA_ without plugging a Kinect.
  * __sequences__: IR Markers sequence XML files.
  * __settings__: Settings XML files of the control interface.
  * __songs__: Audio files to play with the basic audio player.

3. The _bin_ folder contains also an executable file of _CREA_ after you compile it. If this file has the '\_debug' suffix means that the project was compiled in the Debug mode of compilation, which lets you debug the project by using breakpoints and other debugging tools. Projects compiled in the Debug mode can work very slowly so always compile it in the Release mode if you use _CREA_ in a live performance.

4. Download the list of required addons and place them inside the _addons_ folder, only ofxCv and ofxSecondWindow (I have included the rest to _src_ so the updates of the addons do not affect the project):
  * __ofxKinect__ (comes with OFX): https://github.com/ofTheo/ofxKinect
  * __ofxCv__: https://github.com/kylemcdonald/ofxCv
  * __ofxFlowTools__ (included in src): https://github.com/moostrik/ofxFlowTools
  * __ofxUI__ (included in src): https://github.com/rezaali/ofxUI
  * __ofxSecondWindow__: https://github.com/genekogan/ofxSecondWindow

5. Create a new openFrameworks project using the [project generator](http://www.openframeworks.cc/tutorials/introduction/002\_projectGenerator.html) tool that is inside the openFrameworks root folder. Name it _crea_ (same name as repository folder you downloaded before) and make sure all these addons are enabled in the list of addons shown: ofxKinect, ofxCv, ofxOpenCV, ofxXmlSettings and ofxSecondWindow.

6. Go to _apps/myApps_ and inside the folder _crea_ you should see a new file. This file is the project file and depending on your development environment it has extension .sln (Visual Studio), .xcodeproj (Xcode), or .workspace (Code::Blocks).

7. Open the file with your development environment.

8. Inside ofApp.h there are a set of macros to change some basic features that are set by default. 
  
  If we want to use a separate window for the control interface this line of code has to be uncommented:
  ```c
  // Use a separate window for control interface
  #define SECOND_WINDOW
  ```
  
  With the following lines of code we set the resolution of the projector:
  ```c
  #define PROJECTOR_RESOLUTION_X 1680
  #define PROJECTOR_RESOLUTION_Y 1050
  ```

  In order to use the Kinect live input the following line has to be uncommented. Otherwise we will use the sequence of images saved in the folder `bin/data/kinect' as input:
  ```c
  // Use the Kinect live input stream
  #define KINECT_CONNECTED
  ```
  
  In order to be able to use the Gesture Follower feature this line has to be uncommented:
  ```c
  // include gesture follower files (you need to have vmo.cpp, vmo.h, helper.cpp and helper.h in src)
  #define GESTURE_FOLLOWER
  ```
  
  In order to use an offline sequence of IR Markers as input to the Gesture Follower this line has to be uncommented:
  ```c
  // Use an xml IR Markers sequence file as input to Gesture Follower
  #define KINECT_SEQUENCE
  ```
9. Compile the project and enjoy _CREA_.
