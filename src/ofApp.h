#pragma once

#include "ofMain.h"
#include "params.h"
#include <memory>
#include <algorithm> // std::for_each
#include <numeric> // std::accumulate
#include <iostream> // todo: remove once all works
#include <string> // todo: remove once all works
#include <vector>
#include <functional>  // for sort function

extern parameters params; // global parameters

class Cell {

	// Nasty types
	using cell_shrptr = std::shared_ptr<Cell>;

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
			potentialmap_(i_, j_).second = 0.;
			// get surrounding potential info
			surveyandfactor_(cellattractfac_);
			// Attract nearby neighbors
			attractfartherneighbors_(cellattractfac_, cellattractrad_);
			sumpot_ = computesumpot_();
			// Draw itself
			draw();
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
			age_++;
			if (age_ == celldeterage_) { // discourage  other cells to spawn next to an old cell
				deterfartherneighbors_(celldeterfac_, celldeterrad_);
			}
			survey_();

			sumpot_ = computesumpot_();
			ofLogNotice() << "Computed sumpot = " << sumpot_ << " running = " << (sumpot_ > 0.);
			return sumpot_ > 0.;
		}

		cell_shrptr multiply() {
			if (sumpot_ <= 0.) {
				throw std::runtime_error("tried to multiply a cell that supposedly has no free neighbor pixels, sumpot = " 
				+ std::to_string(sumpot_));
			}

			// normalize to get probabilities
			std::for_each(neighborpot_.begin(), neighborpot_.end(),
							[this](float& f) {
									f = pot_(f);
									f /= sumpot_;
							}
			);

			// choose neighbor pixel with resp. probabilities
			float p = ofRandom(0., 1.);
			int chosen_idx = 0;
			while ((p -= neighborpot_[chosen_idx]) > 0.) { // the larger the probability in chosen.second,
				chosen_idx++;							   // the more likely the end condition is met in that iteration
				if (chosen_idx > 3) {
					throw std::runtime_error("Cell couldn't determine neighbor (chosen_idx > 3)");
				}
			}

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
			return cell_shrptr(new Cell(i_n, j_n));
		}
	
	private:
		inline float pot_(float f) {
			return f;
		}

		inline float computesumpot_() {
			return std::accumulate(neighborpot_.begin(), neighborpot_.end(), 0.,
								   [this](float sum, float f) {
								   	return sum + pot_(f);
								   }
			);
		}

		inline void survey_() {
			neighborpot_[0] = potentialmapreadonly_(i_+1, j_).second; // right
			neighborpot_[1] = potentialmapreadonly_(i_, j_-1).second; // above
			neighborpot_[2] = potentialmapreadonly_(i_-1, j_).second; // left
			neighborpot_[3] = potentialmapreadonly_(i_, j_+1).second; // below
		}

		inline void surveyandfactor_(float f) {
			// survey surrounding potentials (note that out of bounds is allowed by neighborpot_)
			// and adjust potential around cell
			neighborpot_[0] = (potentialmap_(i_+1, j_).second *= f); // right
			neighborpot_[1] = (potentialmap_(i_, j_-1).second *= f); // above
			neighborpot_[2] = (potentialmap_(i_-1, j_).second *= f); // left
			neighborpot_[3] = (potentialmap_(i_, j_+1).second *= f); // below
		}

		// note that rad < 2 does nothing
		inline void deterfartherneighbors_(float f, int rad) {
			// lower potential further around the cell in a circle
			for (int i = 0; i <= rad - 2; i++) {
				for (const auto& ij : cind_[i]) {
					potentialmap_(ij.first + i_, ij.second + j_).second *= f * (i + 2.) / rad;
				}
			}
		}

		// note that rad < 2 does nothing
		inline void attractfartherneighbors_(float f, int rad) {
			// lower potential further around the cell in a circle
			for (int i = 0; i <= rad - 2; i++) {
				for (const auto& ij : cind_[i]) {
					potentialmap_(ij.first + i_, ij.second + j_).second *= f * (rad - i) / rad;
				}
			}
		}

		inline void draw() {
			ofDrawRectangle(pos_, pixelsize_, pixelsize_);
		}

		// grab parameters form params.h
		const int gridsizex_ = params.gridsizex;
		const int gridsizey_ = params.gridsizey;
		const int pixelsize_ = params.pixelsize;
		const unsigned celldeterrad_ = params.celldeterrad;
		const unsigned cellattractrad_ = params.cellattractrad;
		const int celldeterage_ = params.celldeterage;
		const float celldeterfac_ = params.celldeterfactor;
		const float cellattractfac_ = params.cellattractfactor;
		edgebufArr<std::pair<ofVec2f, float>>& potentialmap_; // reference to potential map in params.h
		const edgebufArr<std::pair<ofVec2f, float>>& potentialmapreadonly_
			= params.potentialmap; // const reference for readonly access (safety)
		const std::vector<
			std::vector<std::pair<int, int>>
		>& cind_ = params.cind; // vector array of indices for a pixelated circle

		// own member vars
		int age_ = 0;
		const int i_;
		const int j_;
		const ofVec2f pos_;
		std::vector<float> neighborpot_;
		float sumpot_;
};

class ofApp : public ofBaseApp{

	// Nasty types
	using cell_shrptr = std::shared_ptr<Cell>;
	using cellptr_2arg_functional
		= std::function<bool(cell_shrptr, cell_shrptr)>;

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


		cellptr_2arg_functional cellptrless_ = [](cell_shrptr a, cell_shrptr b) {
			return a->getsumpot() < b->getsumpot();
		};
		std::vector<cell_shrptr> activecells_;
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