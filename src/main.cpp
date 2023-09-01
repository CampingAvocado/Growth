#include "ofMain.h"
#include "ofApp.h"

extern parameters params;
//========================================================================
int main(){
	ofSetupOpenGL(params.windowwidth, params.windowheight, OF_WINDOW);			// <-------- setup the GL context

	// this kicks off the running of my app
	// can be OF_WINDOW or OF_FULLSCREEN
	// pass in width and height too:
	ofRunApp(new ofApp());
}
