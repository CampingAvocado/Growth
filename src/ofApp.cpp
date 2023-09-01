#include "ofApp.h"
#include <algorithm>
#include <iostream> // todo: remove once all works

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
    cellvec_.reserve(numinitcells_);
    for (int i = 0; i < numinitcells_; i++) {
        cellvec_.push_back(std::shared_ptr<Cell>(
            new Cell(initcellcoords_[i])
        ));
    }
    fbo_.end();
}

//--------------------------------------------------------------
void ofApp::update(){
    // ofLogNotice() << ofGetFrameRate();
    fbo_.begin();
        for (int s = 0; s < stride_; s++) {
        // sort cells by surrounding potential (decreasing order)
        std::sort(cellvec_.begin(), cellvec_.end(),
                    [](std::shared_ptr<Cell> a, std::shared_ptr<Cell> b) {
                        return a->getsumpot() > b->getsumpot();
                    }
        );
        // only keep top todo% Cells with most surr. potential
        cellvec_.erase(cellvec_.end() - cellvec_.size()/4, cellvec_.end());

        // remove cells that have no adjacent empty pixels
        cellvec_.erase(
            std::remove_if(cellvec_.begin(), cellvec_.end(),
                [](std::shared_ptr<Cell> cell) {
                    return !cell->canmultiply();
                }
            ),
            cellvec_.end()
        );

        std::vector<std::shared_ptr<Cell>> newcells;

        std::for_each(cellvec_.begin(), cellvec_.end(),
            [&newcells](std::shared_ptr<Cell> cell) {
                newcells.push_back(cell->multiply());
            }
        );
        cellvec_.insert(cellvec_.end(), newcells.begin(), newcells.end());
        
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
