#pragma once

#include "ofMain.h"
#include "params.h"
#include <memory>
#include <algorithm> // std::for_each
#include <numeric> // std::accumulate
#include <iostream> // todo: remove once all works

extern parameters params; // global parameters

class Cell {
	public:
		Cell(int i, int j) : potentialmap_(params.potentialmap),
							 i_(i),
							 j_(j),
							 pos_(potentialmap_(i, j).first),
							 neighborpot_(4) {
			if (i_ >= gridsizex_ || j_ >= gridsizey_ || i_ < 0 || j_ < 0) {
				throw std::runtime_error("Spawned a Cell out of bounds!");
			}
			// Set the potential to zero (no Cell can overlap another)
			potentialmap_(i, j).second = 0.;
			ofDrawRectangle(pos_, pixelsize_, pixelsize_);
		}

		Cell(ofVec2f pos) : Cell(pos.x/params.pixelsize, pos.y/params.pixelsize) {}

		~Cell() = default; // pot. todo: remove pixel upon destruct.

		ofVec2f getpos() {
			return pos_;
		}
		
		float getsumpot() {
			return sumpot_;
		}

		bool canmultiply() {
			// running_ = ;
			if (age_++ > 5) { // discourage  other cells to spawn next to an old cell
				potentialmap_(i_+1, j_).second *= 0.001; // right
				potentialmap_(i_, j_-1).second *= 0.001; // above
				potentialmap_(i_-1, j_).second *= 0.001; // left
				potentialmap_(i_, j_+1).second *= 0.001; // below
			}
			// survey surrounding potentials (note that out of bounds is allowed by neighborpot_)
			// and increase potential around alive cell (does not affect itself)
			neighborpot_[0] = (potentialmap_(i_+1, j_).second *= 1.2); // right
			neighborpot_[1] = (potentialmap_(i_, j_-1).second *= 1.2); // above
			neighborpot_[2] = (potentialmap_(i_-1, j_).second *= 1.2); // left
			neighborpot_[3] = (potentialmap_(i_, j_+1).second *= 1.2); // below

			sumpot_ = std::accumulate(neighborpot_.begin(), neighborpot_.end(), 0.,
									   		[](float sum, float f) {
												return sum + f*f*f;
											}
					   );

			running_ = sumpot_ > 0.;
			return running_; 
		}

		std::shared_ptr<Cell> multiply() {
			if (!running_) {
				throw std::runtime_error("tried to multiply a cell that supposedly has no free neighbor pixels");
			}

			// normalize to get probabilities
			std::for_each(neighborpot_.begin(), neighborpot_.end(),
							[this](float& f) {
									f = f*f*f;
									f /= sumpot_;
						  }
			);
			std::cerr << "normalized ? " << std::accumulate(neighborpot_.begin(), neighborpot_.end(), 0.) << " sum pot: " << sumpot_ << "\n";

			// choose neighbor pixel with resp. probabilities
			float p = ofRandom(0., 1.);
			int chosen_idx = 0;
			while ((p -= neighborpot_[chosen_idx]) > 0.) { // the larger the probability in chosen.second,
				chosen_idx++;							  		   // the more likely the end condition is met in that
			}								  					   // iteration

			int i_n, j_n; // Detemine grid coordinates of neighbor
			switch (chosen_idx) {
				case 0:
					i_n = i_ + 1;
					j_n = j_;
					break;
				case 1:
					i_n = i_;
					j_n = j_ - 1;
					break;
				case 2:
					i_n = i_ - 1;
					j_n = j_;
					break;
				case 3:
					i_n = i_;
					j_n = j_ + 1;
					break;
				default:
					throw std::runtime_error("Cell couldn't determine neighbor (chosen_idx > 3)");
			}
			return std::shared_ptr<Cell>(new Cell(i_n, j_n));
		}
	
	private:
		// grab parameters form params.h
		const int gridsizex_ = params.gridsizex;
		const int gridsizey_ = params.gridsizey;
		const int& pixelsize_ = params.pixelsize;
	 	// reference to potential map in params.h
		edgebufArr<std::pair<ofVec2f, float>>& potentialmap_;

		// own member vars
		int age_ = 0;
		bool running_ = true;
		const int i_;
		const int j_;
		const ofVec2f pos_;
		std::vector<float> neighborpot_;
		float sumpot_;
};

class ofApp : public ofBaseApp{

	public:
		void setup();
		void update();
		void draw();

		void keyPressed(int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y );
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void mouseEntered(int x, int y);
		void mouseExited(int x, int y);
		void windowResized(int w, int h);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);

		std::vector<std::shared_ptr<Cell>> cellvec_;
		ofFbo fbo_;
		// grab parameters form params.h
		const int windowwidth_ = params.windowwidth;
		const int windowheight_ = params.windowheight;
		const int numinitcells_ = params.numinitcells;
		std::vector<ofVec2f>& initcellcoords_ = params.initcellcoords;
		edgebufArr<std::pair<ofVec2f, float>>& potentialmap_
			= params.potentialmap; // reference to potential map in params.h
		const int stride_ = params.stride;
};