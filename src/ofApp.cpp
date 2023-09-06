#include "ofApp.h"
#include <algorithm>
#include <iostream> // todo: remove once all works
#include <fstream>

//--------------------------------------------------------------
void ofApp::setup(){

    // Color Setup
    ofSetBackgroundColor(0);
    ofSetColor(255);

    // Animation Setup
    ofSetFrameRate(60);

    // do computation in buffer!
    fbo_.allocate(windowwidth_, windowheight_);
    fbo_.begin();
    ofClear(0);
    // Get initial cells

    for (auto coord : initcellcoords_) {
        cell_shrptr newcell(new Cell(coord));
        activecells_.push_back(newcell);
    }
    fbo_.end();
}

//--------------------------------------------------------------
void ofApp::update(){
    fbo_.begin();
    for (int s = 0; s < stride_; s++) {
        ofLogNotice() << "# of active cells: " << activecells_.size();

        // Output Potential values for debugging purposes
        // if (activecells_.size() == 0) {
        //     std::ofstream os("../src/pfuncvalsend.csv");
        //     for (int j = 0; j < potentialmap_.sizey(); j++) {
		// 		for (int i = 0; i < potentialmap_.sizex(); i++) {
		// 			os << potentialmap_(i, j).second << ",";
		// 		}
		// 		os << "\n";
		// 	}
        //     ofExit();
        // }
        // Erase cells that cannot multiply
        // (i.e. 0 surr. potential, typically because all neighbor pixels occupied)
        unsigned cnt = 0;
        unsigned sizepre = activecells_.size();
        activecells_.erase(std::remove_if(activecells_.begin(),
                                        activecells_.end(),
                                        [&cnt](cell_shrptr c) {
                                                bool b = !c->canmultiply();
                                                if (b) cnt++;
                                                return b;
                                        }
                                        ),
                        activecells_.end()
        );

        if (activecells_.size() == 0) {
            fbo_.end();
            return;
        }

        if (activecells_.size() + cnt != sizepre) {
            throw std::runtime_error("Counted more dead Cells than were deleted!");
        }


        // Multiply Cells with highest potential
        std::vector<cell_shrptr> newcells;
        std::sort(activecells_.begin(), activecells_.end(), cellptrless_);
        int cursizered = std::min((unsigned)activecells_.size(), (unsigned)activecells_.size()/2u + 1);
        for (int i = 0; i < cursizered; i++) {
            ofLogNotice() << "multiplying cell " << i+1;
            cell_shrptr newcell = activecells_[i]->multiply();
            activecells_.push_back(newcell);
        }
    }
    fbo_.end();
}

//--------------------------------------------------------------
void ofApp::draw(){
    fbo_.draw(0, 0, windowwidth_, windowheight_);
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){

}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}
