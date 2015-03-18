#include "ofMain.h"
#include "ofApp.h"

//========================================================================
int main(){
	ofSetupOpenGL(1024,768,OF_WINDOW);			// <-------- setup the GL context

	// this kicks off the running of my app
	// can be OF_WINDOW or OF_FULLSCREEN
	// pass in width and height too:
	ofRunApp(new ofApp());
}

//#include "ofMain.h"
//#include "ofApp.h"
//#include "ofAppGLFWWindow.h"
//
////========================================================================
//int main( ){
//	ofAppGLFWWindow window;
//	window.
//    window.setMultiDisplayFullscreen(true);
//    ofSetupOpenGL(&window,1024,768,OF_FULLSCREEN); 			// <-------- setup the GL context
//	ofRunApp(new ofApp());
//}

