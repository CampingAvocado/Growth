#pragma once
#include <exception>
#include "ofMain.h"
#include <vector>
#include <fstream>
/*
 * This File defines all runtime parameters/"global" objects,
 * most importantly the coordinate/potential grid (called potentialmap) that is
 * accessed by both ofApp and Cell(s)
 */

// todo: add foreground/background colors as parameters

// Simple Wrapper Class for dynamic float arrays of 2 dimensions
// that has a buffer: accesses which are out of range by 1 return buf
// - Not Copyable
// - Not Assignable
// - Not Movable
// - Not Copy-Assignable
template <typename T>
class edgebufArr {
	public:
		edgebufArr(int sizex, int sizey, T buf) : sizex_(sizex+2),
									   	   		  sizey_(sizey+2) {
			arr_ = new T[sizex_ * sizey_];

			// set buffer values to buf
			for (int i = 0; i < sizex_; i++) {
				arr_[i*sizey_] = buf;
				arr_[i*sizey_ + sizey_-1] = buf;
			}
			for (int j = 0; j < sizey_; j++) {
				arr_[j] = buf;
				arr_[(sizex_-1)*sizey_ + j] = buf;
			}
		}

		~edgebufArr() {
			delete[] arr_;
		}

		// Not allowed:
		edgebufArr(const edgebufArr&) = delete;
		edgebufArr& operator=(const edgebufArr&) = delete;
		edgebufArr(edgebufArr&&) = delete;
		edgebufArr& operator=(edgebufArr&&) = delete;
		// -----------


		T& operator()(int i, int j) {
			return arr_[(i+1)*sizey_ + (j+1)];
		}

		int size() {
			return (sizex_-2) * (sizey_-2);
		}
	
	private:
		T* arr_;
		const int sizex_;
		const int sizey_;
};

class parameters {
	public:
		parameters() : initcellcoords(numinitcells),
					   potentialmap(gridsizex, gridsizey,
					  		std::make_pair(ofVec2f(0., 0.), 0.f)) {
			if ((windowwidth % pixelsize != 0)
				|| (windowheight % pixelsize != 0)) {
				throw std::runtime_error(
					"params.h:pixelsize does not divide window into homogenous grid"
				);
			}

			// Set up potentialmap to carry coordinate grid and potentials of cells
			std::ofstream pfuncvals("../src/pfuncvals.csv"); // to view for debugging purposes
			ofVec2f coord(0., 0.);
			for (int i = 0; i < gridsizex; i++) {
				coord.y = 0.;
				for (int j = 0; j < gridsizey; j++) {
					// Translate the vec as potential func should be defined on [-1, 1]^2
					ofVec2f translated = maptocoordsys(coord);
					float pval = potentialfunc(translated);
					potentialmap(i, j) = std::make_pair(coord, pval);
					pfuncvals << pval << ",";
					coord.y += pixelsize;
				}
				pfuncvals << "\n";
				coord.x += pixelsize;
			}

			// Define set of first cells
			// Note that here the coordinate Grid is the standard one, i.e.
			// [0, windowwidth] x [0, windowheight]
			// hence use maptogrid as needed
			for (int i = 0; i < numinitcells; i++) {
				initcellcoords[i] = {
					windowwidth/2 - (numinitcells-1 + i)*pixelsize,
					windowheight - pixelsize
				}; // replace with some expression as needed
			}
		}

		// Maps vectors from [0, windowwidth] x [0, windowheight] to [-1, 1]^2 
		ofVec2f maptocoordsys(ofVec2f coord) {
			return (coord - ofVec2f(0.5*(windowwidth-1), 0.5*(windowheight-1))) / ofVec2f(0.5*(windowwidth-1), -0.5*(windowheight-1));
		}

		// Maps vectors from [-1, 1]^2 to [0, windowwidth] x [0, windowheight]
		ofVec2f maptogrid(ofVec2f coord) {
			return coord * ofVec2f(0.5*(windowwidth-1), -0.5*(windowheight-1)) + ofVec2f(0.5*(windowwidth-1), 0.5*(windowheight));
		}

		// RUNTIME PARAMETERS
		// ******Set these*******

		// Potential Function for PotentialMap,
		// should be STRICTLY POSITIVE and DEFINED ON [-1, 1]^2
		float potentialfunc(ofVec2f& pos) {
			float x = pos.x, y = pos.y;
			return 10000.*(y + 1.);
		}

		const int windowwidth = 2000; // in pixels
		const int windowheight = 1800; // in pixels
		const int pixelsize = 4; // size of  cell
		const int gridsizex = windowwidth/pixelsize;
		const int gridsizey = windowheight/pixelsize;
		const int numinitcells = 10; // inital number of cells
		const int stride = 1; // how many multiplications per frame, adjusts speed
		std::vector<ofVec2f> initcellcoords; // Set Contents in Ctor
		// *****************

		// Set automatically
		edgebufArr<std::pair<ofVec2f, float>> potentialmap;
};