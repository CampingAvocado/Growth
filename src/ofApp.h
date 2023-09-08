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

extern parameters params; // run-time parameters and objects

/*
 * The basic living Entity on Grid [0, girdsizex] x [0, gridsizey].
 * Can look at neighboring potential, influence potential in a radius up to cellattractrad and celldeterrad,
 * and multiply (i.e. spawn a Cell) in one of the neighboring Pixels on the Grid.
 * Nothing outside of the provided Ctor is intended behaviour (no copy, assign etc.)
 */
class Cell {

	// Nasty types
	using cell_shrptr = std::shared_ptr<Cell>;

	public:
		// Ctor
		Cell(int i, int j) : potentialmap_(params.potentialmap),
							 i_(i),
							 j_(j),
							 neighborpot_(4) {
			if (i_ >= gridsizex_ || j_ >= gridsizey_ || i_ < 0 || j_ < 0) {
				throw std::runtime_error("Spawned a Cell out of bounds!");
			}
			// Set the potential to zero (no Cell can overlap another)
			potentialmap_(i_, j_) = 0.;
			// get neighboring potential info and attract direct neighbors (increase potenital)
			factor_(cellattractfac_);
			// Attract nearby neighbors beyond direct ones (optional, off if cellattractrad_ < 2)
			attractfartherneighbors_(cellattractfac_, cellattractrad_);
			// Draw itself
			draw();
		}

		Cell(ofVec2f pos) : Cell(pos.x, pos.y) {}

		~Cell() = default; // pot. todo: remove pixel upon destruct.
		
		// Get sum of potential around cell
		float getsumpot() {
			return sumpot_;
		}

		// Computes the sum of potentials
		// and thereby deternmines if it can still multiply (non-zero potential)
		// Should be called before multiply()!
		// Also takes care of potential-manipulation (deter) and ages cell
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

		// Multiplies Cell, returning a shard_ptr to the new one. Decides location based on probability dist.
		// relative to neighboring potentials
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

		// weight function for probability dist., optional
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

		// Gather neighboring potential
		inline void survey_() {
			neighborpot_[0] = potentialmapreadonly_(i_+1, j_); // right
			neighborpot_[1] = potentialmapreadonly_(i_, j_-1); // above
			neighborpot_[2] = potentialmapreadonly_(i_-1, j_); // left
			neighborpot_[3] = potentialmapreadonly_(i_, j_+1); // below
		}

		// factor neighboring potential by f (for direct attraction)
		// Note that f should be larger than 1 to work
		inline void factor_(float f) {
			// (note that out of bounds is allowed by neighborpot_)
			potentialmap_(i_+1, j_) *= f; // right
			potentialmap_(i_, j_-1) *= f; // above
			potentialmap_(i_-1, j_) *= f; // left
			potentialmap_(i_, j_+1) *= f; // below
		}

		// lower potential farther than direct neighbor Pixels by a
		// Linearly increasing (with radius) factor f in [0, 1]
		// note that rad < 2 does nothing and f should be in [0, 1] !
		inline void deterfartherneighbors_(float f, int rad) {
			// lower potential further around the cell in a circle
			for (int i = 0; i <= rad - 2; i++) {
				for (const auto& ij : cind_[i]) {
					potentialmap_(ij.first + i_, ij.second + j_) *= f * (i + 2.) / rad;
				}
			}
		}

		// increase potential farther than direct neighbor Pixels by a
		// Linearly increasing (with radius) factor f in [1, inf]
		// note that rad < 2 does nothing and f should be in [1, inf] !
		// note that rad < 2 does nothing
		inline void attractfartherneighbors_(float f, int rad) {
			// lower potential further around the cell in a circle
			for (int i = 0; i <= rad - 2; i++) {
				for (const auto& ij : cind_[i]) {
					potentialmap_(ij.first + i_, ij.second + j_) *= f * (rad - i) / rad;
				}
			}
		}

		inline void draw() {
			ofDrawRectangle(i_*pixelsize_, j_*pixelsize_,
							pixelsize_, pixelsize_
			); // pixelsize mult. translates from standard Cell grid [0, gridsizex] x [0, gridsizey] to
			   // what OF uses: [0, windowwidth] x [0, windowheight] 
		}

		// grab parameters form params.h
		const int pixelsize_ = params.pixelsize;
		const int gridsizex_ = params.gridsizex;
		const int gridsizey_ = params.gridsizey;
		const int celldeterage_ = params.celldeterage;
		const unsigned celldeterrad_ = params.celldeterrad;
		const unsigned cellattractrad_ = params.cellattractrad;
		const float celldeterfac_ = params.celldeterfactor;
		const float cellattractfac_ = params.cellattractfactor;
		edgebufArr<float>& potentialmap_; // reference to potential map in params.h
		const edgebufArr<float>& potentialmapreadonly_
			= params.potentialmap; // const reference for readonly access (safety)
		const std::vector<
			std::vector<std::pair<int, int>>
		>& cind_ = params.cind; // vector array of indices for a pixelated circle

		// own member vars
		int age_ = 0;						// Cell age determines at what point it deters farther neighbors (rad > 1)
		const int i_;						// Cell has position [i, j] on
		const int j_;						// Grid [0, gridsizex] x [0, gridsizey]
		std::vector<float> neighborpot_;	// Cell stores neighbor potential vals here
		float sumpot_;						// sum over nerighborpot_
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
		std::vector<cell_shrptr> activecells_;  // vector of shared_ptrs to cells being managed.
												// Cells with 0 sumpot_ get deleted off this vector
		ofFbo fbo_;								// buffer, see doc

		// grab parameters form params.h
		const int windowwidth_ = params.windowwidth;
		const int windowheight_ = params.windowheight;
		const int numinitcells_ = params.numinitcells;
		std::vector<ofVec2f>& initcellcoords_ = params.initcellcoords;
		edgebufArr<float>& potentialmap_
			= params.potentialmap; // reference to potential map in params.h
		const int stride_ = params.stride;
		const int pixelsize_ = params.pixelsize;
};