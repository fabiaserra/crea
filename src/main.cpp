#include "ofMain.h"
#include "ofApp.h"
#include "ofGLProgrammableRenderer.h"

//========================================================================
int main(){
	ofSetupOpenGL(1024,768,OF_WINDOW);			// <-------- setup the GL context
//    ofSetCurrentRenderer(ofGLProgrammableRenderer::TYPE);
	// this kicks off the running of my app
	// can be OF_WINDOW or OF_FULLSCREEN
	// pass in width and height too:
	ofRunApp(new ofApp());
}

//#include "ofMain.h"
//#include "ofApp.h"
//#include "ofAppGLFWWindow.h"

////========================================================================
//int main( ){
//    ofGLFWWindowSettings settings;
//    settings.width = 1024;
//    settings.height = 768;
//    settings.setPosition(ofVec2f(300,0));
//    settings.resizable = true;
//    shared_ptr<ofAppBaseWindow> mainWindow = ofCreateWindow(settings);
//    
//    settings.width = 1024;
//    settings.height = 768;
//    settings.setPosition(ofVec2f(0,0));
//    settings.resizable = false;
//    shared_ptr<ofAppBaseWindow> guiWindow = ofCreateWindow(settings);
//    guiWindow->setVerticalSync(false);
//    
//    shared_ptr<ofApp> mainApp(new ofApp);
////    mainApp->setupGUI();
////    ofAddListener(guiWindow->events().draw,mainApp.get(),&ofApp::drawGui);
//    
//    ofRunApp(mainWindow, mainApp);
//    ofRunMainLoop();
//}

//#include "ofMain.h"
//#include "ofApp.h"
//#include "GuiApp.h"
//#include "ofAppGLFWWindow.h"
//
////========================================================================
//int main( ){
//    ofGLFWWindowSettings settings;
//    
//    settings.width = 600;
//    settings.height = 600;
//    settings.setPosition(ofVec2f(300,0));
//    settings.resizable = true;
//    shared_ptr<ofAppBaseWindow> mainWindow = ofCreateWindow(settings);
//    
//    settings.width = 300;
//    settings.height = 300;
//    settings.setPosition(ofVec2f(0,0));
//    settings.resizable = false;
//    shared_ptr<ofAppBaseWindow> guiWindow = ofCreateWindow(settings);
//    
//    shared_ptr<ofApp> mainApp(new ofApp);
//    shared_ptr<GuiApp> guiApp(new GuiApp);
//    mainApp->gui = guiApp;
//    
//    ofRunApp(guiWindow, guiApp);
//    ofRunApp(mainWindow, mainApp);
//    ofRunMainLoop();
//    
//}