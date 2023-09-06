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
		edgebufArr(int sizex, int sizey, int bufsize, T buf) : sizex_(sizex+2*bufsize),
									   	   		  			   sizey_(sizey+2*bufsize),
															   bufsize_(bufsize) {
			arr_ = new T[sizex_ * sizey_](); // "()" for allocating default value of type T

			// set buffer values to buf - inefficient but only run once.
			for (int i = 0; i < sizex_; i++) {
				for (int j = 0; j < bufsize_; j++) {
					arr_[i*sizey_ + j] = buf;
					arr_[i*sizey_ + sizey_-1-j] = buf;
				}
			}
			for (int j = 0; j < sizey_; j++) {
				for (int i = 0; i < bufsize_; i++){
					arr_[i*sizey_ + j] = buf;
					arr_[(sizex_-1-i)*sizey_ + j] = buf;
				}
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

		// Write-Access
		T& operator()(int i, int j) {
			return arr_[(i+bufsize_)*sizey_ + (j+bufsize_)];
		}

		// Only Read-Access: use operator() on a const ref of class
		// to call this function instead (for safety of not overwriting buffer)
		T& operator()(int i, int j) const {
			return arr_[(i+bufsize_)*sizey_ + (j+bufsize_)];
		}

		int sizex() {
			return sizex_ - 2 * bufsize_;
		}

		int sizey() {
			return sizey_ - 2 * bufsize_;
		}
	
		int size() {
			return sizex() * sizey();
		}

	private:
		T* arr_;
		const int sizex_;
		const int sizey_;
		const int bufsize_;
};

class circleindices {
	public:
		// Generate indices of concentric pixelated circles around (0,0) with radius r
		// from r0 onwards
		circleindices(unsigned r0, unsigned r) {
			if (r0 > r) {
				throw std::runtime_error("circleindices got invalid radii args");
			}
			indices.resize(r - r0 + 1);
			edgebufArr<bool> logger(r, r, 1, true);

			std::vector<std::pair<int, int>> tmp;
			if (r0 > 0) midpcircledraw_(r0 - 1, logger); // log one more circle inwards for holefill
			for (unsigned i = 0; i <= r - r0 ; i++) {
				tmp = midpcircledraw_(i + r0, logger);
				
				for (unsigned j = 0; j < tmp.size(); j++) {
					auto xy = tmp[j];
					int x = xy.first, y = xy.second;

					// indices (Anti-)Symmetric across quadrants
					indices[i].push_back({x, y});
					indices[i].push_back({-x, y});
					indices[i].push_back({-x, -y});
					indices[i].push_back({x, -y});

					if (!logger(x, y - 1)) {
						indices[i].push_back({x, y - 1});  // fill hole between this and
						indices[i].push_back({-x, y - 1}); // last circle
						indices[i].push_back({-x, -y + 1});
						indices[i].push_back({x, -y + 1});
					}
				}
			}
		}

		std::vector<
			std::vector<std::pair<int, int>>
		> indices;

	private:

		// Mid-Point Circle Drawing Algorithm (Bresenham Alg.)
		// Only gives coordinates of bottom right quadrant!
		// Mostly pulled from here: https://www.geeksforgeeks.org/bresenhams-circle-drawing-algorithm/
		std::vector<std::pair<int, int>> 
			midpcircledraw_(int r, edgebufArr<bool>& logger) {
			std::vector<std::pair<int, int>> coords;

			int x = r, y = 0;
			
			// Printing the initial point on the axes
			// after translation
			coords.push_back({x, y});
			logger(x, y) = true;
			
			// When radius is zero only a single
			// point will be printed
			if (r > 0) {
				coords.push_back({0, x});
				logger(0, x) = true;
			}
			
			// Initialising the value of P
			int P = 1. - r;
			while (x > y) {
				y++;
				
				// Mid-point is inside or on the perimeter
				if (P <= 0.) {
					P = P + 2*y + 1;
				}
				// Mid-point is outside the perimeter
				else {
					x--;
					P = P + 2.*y - 2.*x + 1.;
				}
				
				// All the perimeter points have already been printed
				if (x < y) break;
				
				// Printing the generated point and its reflection
				// in the other octants after translation
				coords.push_back({x, y});
				logger(x, y) = true;
				
				// If the generated point is on the line x = y then
				// the perimeter points have already been printed
				if (x != y) {
					coords.push_back({y, x});
					logger(y, x) = true;
				}
			}
			return coords;
		}
};

class parameters {
	public:
		parameters() : initcellcoords(numinitcells),
					   potentialmap(gridsizex, gridsizey, celldeterrad,
					  		std::make_pair(ofVec2f(0., 0.), 0.f)),
					   cind(circleindices(2, celldeterrad).indices) {
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
			return 0.5*(y + 1.);
		}

		const int windowwidth = 800; // in pixels
		const int windowheight = 800; // in pixels
		const int pixelsize = 4; // size of  cell
		const int numinitcells = 1; // inital number of cells
		const int stride = 1; // how many multiplications per frame, adjusts speed
		const int celldeterrad = 10; // radius in which a cell diminishes potential >= 2 if it should exist
		const int cellattractrad = 2; // radius in which a cell increases potential >= 2 if it should exist
		const int celldeterage = 3;
		const float celldeterfactor = 0.9; // the smaller, the more a cell of age celldeterage
											 // will try and keep cells of distance [2, celldeterrad]
											 // away, IN [0, 1]
		const float cellattractfactor = 10.;
		std::vector<ofVec2f> initcellcoords; // Set Contents in Ctor
		// *****************

		// Set automatically
		const int gridsizex = windowwidth/pixelsize;
		const int gridsizey = windowheight/pixelsize;
		const std::vector<
			std::vector<std::pair<int, int>>
		> cind;
		edgebufArr<std::pair<ofVec2f, float>> potentialmap;
};