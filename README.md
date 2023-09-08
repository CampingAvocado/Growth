# Growth
a first generative art project

## General Idea
Given a canvas of resolution $G = [0, \text{windowwidth} / \text{cellsize}] \subset [0, \text{windowheight} / \text{cellsize}] \in \mathbb{N}_0^2$ (<- I call this the "grid") and a "coordinate system" $C = [-1, 1]^2 \subset \mathbb{R}^2$, with an invertible linear map $\sigma : S \subset C \to G$ that "translates" from the grid $S$ to the canvas $G$, define a potential field $\phi : S \to \mathbb{R}^+$ that gives each grid point ("Pixel") a positive value.
Start with an initial set of **Cells** that "live in" $G$ where one Cell occupies one Pixel of $G$.
The Cells should then spawn neighbouring Cells according to some **Multiplication Algorithm** where a larger potential should be more likely to be filled.
Finally, define some termination condition (nothing implemented yet).

## Note
This Project works with the openFrameworks library.
The repo needs to sit in a working installation of openFrameworks: `openFrameworks/apps/<FolderName>/Growth` to be compiled.
I try to always work with openFramework's most recent version.
Comments in code are sparse and maybe even outdated and there is currently no usage guide until I get a first properly working version.

# Version History
## Version 0.2
### Current Capabilities
Can create a growth pattern from square Cells over repeated iterations depending on a variable initial set of Cells. The Canvas is adjustable in size.
Cells can increase the potential around them at birth and inhibit (not directly next to them) it at a variable age. The potential along the canvas can be adjusted with a function parameter. There is still no clear termination criterion though the cells already can provide nice patterns by diminishing potential to 0 everywhere.
### Changes
#### .gitignore
- Added more output files
### params.h
- Added more comments for explanations and cleaned up the structure
##### class edgebufArr
-  now able to hold a custom size of edgebuffer
-  member functions for sizes of individual dimensions
##### new: class circleindices
- generate indices of concentric pixelated circles around (0,0) with a given radius (This is relevant for Cells being able to access Pixels in a radius around them).
##### class parameters
- changed potential map to only hold the potential values (not the coordinates) only those are needed
- new parameters:
	- celldeterrad: radius in which a cell diminishes potential >= 2 if it should exist
	- cellattractrad: radius in which a cell increases potential >= 2 if it should exist
	- celldeterage: age at which cell diminishes farther potential
	- celldeterfactor: the smaller, the more a cell of age celldeterage will try and keep cells of distance $[2, \text{celldeterrad}]$ away, should be in $[0, 1]$
	- Same as celldeterfactor only increases potential instead (at creation) should be larger (or equal if no attraction) than $1$
#### ofApp.h
- Added more comments for explanations
##### class Cell
- created some inlined member functions for behaviour that should easily be changeable (e.g. what shape the cell draws). Some might be used to create more parameters on params.h in the future.
- Cell deterring (i.e. diminishing surrounding potential)
	- Cell only deters once at celldeterage, instead of every iteration after celldeterage$-1$ (provides adequate results and is more efficient)
	- Cell now doesn't diminish the potential in direct neighbor pixels but instead only on Pixels farther away in pixelated circle (of variable size). The amount it diminishes decreases linearly with radius
- Cell Attraction (i.e. increasing surrounding potential)
	- Cell increases potential of direct neighbor Pixels and farther ones (if desired) where from Pixels one farther than direct neighbors the amount it increases the potential decreases linearly with radius
- removed running_ member variable as it was only needed for catching errors and that is now done differently (checks `sumpot_ <= 0.` in `multiply()`)
- more checks for errors (at least for development useful despite a little performance loss)
##### class ofApp.h
- now has a `std::function` that compares to summed up potential between cells for sorting them
#### ofApp.cpp
- renamed cellvec_ to activecells_
- added more error checks
- revamped multiply portion of `update()` to sort the activelcells_ vec by sum of potentials of cells and multiplying a top percentage (undecided, currently 50%). This makes it less possible for a growth pattern to corner itself (it can continue growing from any edge with some potential)
### Problems
- Getting a good set of parameters takes time and very different results can be obtained. It would be best to implement some parameter file system such that multiple parameter sets can be easily saved and stored
- Even good results often leave 1 Pixel gaps in branches: This is a result of nearby cells all diminishing the potential in an empty Pixel before a Cell can spawn there. This is unintended behaviour as the diminishing should only encourage Cells not to stay in a clump and branch out. One way to combat this would be by making Cell not factor the surrounding potential at birth but reassigning (and increasing) the original potential function value. Problems are that then a Cell needs to know when the Pixel is empty and the added computation time. Also this might turn the growth pattern into a blob no matter what, continuing on this:
	- 0 potential does not mean that there is a Cell in that Pixel: Cell deterring can factor a potential to a value that is so close to zero that with float precision it becomes 0. This leads to a surprisingly nice termination criterion but it is unintended and makes it harder to recognize where cells actually are. One way of dealing with this would be to assign a negative value to occupied Pixels though that would require checks when summing up potential (and potentially in other stuff). Another way would be to keep a separate edgebufArr that just keeps track of Cell positions and add a tiny potentials to neighboring empty Pixels on Cell creation (this would also deal with the previous problem!). This however needs a new termination criterion that may not solve the problem of branch holes at all.
-  The growth pattern struggles to scale beyond a certain point: branches stay the same size no matter the canvas size and initial pattern. To allow larger scales, one could maybe limit the deter area within the Cell diminishes potential to sit outside the entire attraction area (which can then be scaled).

## v0.1
### Current Capabilities
#### Parameter File (params.h)
- can deploy global parameters and variables on an OOP-friendly (wrapped in class, accessed by reference)
- Notably creates a custom 2D array with an out of range buffer to precompute potential field
#### Cell (ofApp.h:Cell)
- Draw itself upon construction, set potential at own pixel to 0
- decrease surrounding potential after an age threshold is reaches, otherwise increase it every iteration (i.e. call of `Cell::canmultiply()`)
- return its position in $[-1, 1]^2$
- return cumulative potential of direct neighbour pixels (calculated when calling `Cell::canmultiply()`), detect if no neighbour pixels are free ->`running_ = false`
- spawn neighbour Cell, choice of pixel depending on their potential (probabilistic details of choice subject to change, currently weighted relative probability with weight function $w(x) = x^3$)
#### Multiplication Algorithm
- Delete Dead Cells (`Cell::running_ == false`)
- Keep top ~75% Cells with highest surrounding potential
- multiply
- repeat
#### Visual
- Simple black on white with custom pixel size and resolution, adjusting those parameters affects speed and behaviour of growth!
#### Problems:
- final pattern is not self-avoiding enough (joining branches leave blob)
- pattern can terminate early if it cycles into itself
- potential gradient not strong enough to guide growth reliably

### Plans v0.2
#### Params
Add parameter `int cellinfluencerad`  that determines in what radius a Cell should diminish its surrounding potential (whenever it does). Adjust `params.h:edgebufArray` to hold a custom buffer size of  `cellinfluencerad` (so cells on the boundary of the Grid can just keep writing to `edgebufArr potentialmap` without dealing with out of range issues).
#### Multiplication Algorithm:
Store Cells on a Max-Heap (or priority list, same thing?) rather than a vector that sorts them by surrounding sum of potentials. All Cells stay alive so long as the surrounding potential isn't 0 (e.g. because the Cell is surrounded). Then the top X% of Cells are multiplied.
This should make it less (im-?) possible for the growth to terminate early as it can regrow from any free cell.
#### Cell Behaviour
- When a Cell spawns, immediately increase potential of directly neighbouring Pixels to encourage growth in a collective direction. At the next iteration, decrease the potential in a (rough) radius to discourage the joining of different branch growths. Do this only once!
- Find a nice weight function that increases influence of potential, maybe dependent on `windowheight` and`windowwidth` such that it works the same regardless of resolution changes.
