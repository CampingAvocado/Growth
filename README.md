# Growth
a first generative art project

## General Idea
Given a canvas of resolution $G = [0, \text{windowwidth}] \subset [0, \text{windowheight}] \in \mathbb{N}_0^2$ (<- I call this the "grid") and a "coordinate system" $C = [-1, 1]^2 \subset \mathbb{R}^2$, with an invertible linear map $\sigma : S \subset C \to G$ that "translates" from the grid $S$ to the canvas $G$, define a potential field $\phi : S \to \mathbb{R}^+$ that gives each grid point ("Pixel") a positive value.
Start with an initial set of **Cells** that "live in" $G$ where one Cell occupies one Pixel of $G$.
The Cells should then spawn neighbouring Cells according to some **Multiplication Algorithm** where a larger potential should be more likely to be filled.
Finally, define some termination condition (nothing implemented yet).

## Note
This Project works with the openFrameworks library.
The repo needs to sit in a working installation of openFrameworks: `openFrameworks/apps/<FolderName>/Growth` to be compiled.
I try to always work with openFramework's most recent version.
Comments in code are sparse and maybe even outdated and there is currently no usage guide until I get a first properly working version.

# Version History
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
Add parameter `int cellinfluencerad`  that determines in what radius a Cell should diminish its surrounding potential (whenever it does). Adjust `params.h:edgebufArray` to hold a custom buffer size of  `cellinfluencerad` (so cells on the boundary of the Grid can just keep writing to `edgebufArr potentialmap` without dealing with out of range issues.
#### Multiplication Algorithm:
Store Cells on a Max-Heap (or priority list, same thing?) rather than a vector that sorts them by surrounding sum of potentials. All Cells stay alive so long as the surrounding potential isn't 0 (e.g. because the Cell is surrounded). Then the top X% of Cells are multiplied.
This should make it less (im-?) possible for the growth to terminate early as it can regrow from any free cell.
#### Cell Behaviour
- When a Cell spawns, immediately increase potential of directly neighbouring Pixels to encourage growth in a collective direction. At the next iteration, decrease the potential in a (rough) radius to discourage the joining of different branch growths. Do this only once!
- Find a nice weight function that increases influence of potential, maybe dependent on `windowheight` and`windowwidth` such that it works the same regardless of resolution changes.
