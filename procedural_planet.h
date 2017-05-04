#ifndef PROCEDURAL_PLANET_H
#define PROCEDURAL_PLANET_H
#include "glla.h"
//#include "dynamic_terrain_tree.h"
#include "quadtree.h"
#include "triangular_terrain_tile.h"
#include "terrain_constants.h"
#include "math/bpos.h"

/*
MEGA TODO:
Make a table of minerals, with different color properties. Noise functions and things like asteroid impacts,
diffusion-limited aggregation, continental plate drift, star system composition, etc. can affect the deposition
of these minerals on the surface of the planet. In turn, these minerals can affect the color of the surface,
the kind of surface features we'll see, what kind of plants we may see, and even the position of cities (which
could, say, pop up near an ancient asteroid impact that left behind a valuable resource to mine). Do minerals
affect where continent plates form, or do continent plates form first and push minerals around?

Ideas for continent creation:
Place points on sphere, find voronoi cells, fractelize edges, or distort using a noise function.
Start at a random point, random walk until it intersects with itself, make the closed area a continent, repeat.

A simple way to make mountains from that is to add medium-frequency bumpiness at continent borders, with a falloff.
That bumpiness can be done using diffusion-limited aggregation (or some approximation), to get a "mountain range"
looking effect, with some apparent erosion. Ideally I'd have some data structure so that any point on the sphere
can quickly look up its continent, nearby mountain range, local surface features, etc. Noise can then be saved for
the highest-frequency features, for efficiency. Could also use it to distort the vertexes of my "mountain graph",
like with domain warping (could look good, could look bad).
*/

typedef struct procedural_planet {
	vec3 up;
	vec3 right;
	float radius;
	float noise_radius;
	float amplitude;
	float edge_len;
	//TODO: Consolidate these.
	vec3 colors[3];
	vec3 color_family;
	quadtree_node *tiles[NUM_ICOSPHERE_FACES];
	height_map_func height;
} proc_planet;

struct planet_terrain_context {
	int splits_left;
	vec3 cam_pos;
	bpos_origin cam_sec;
	proc_planet *planet;
	int visited;
	tri_tile **tiles;
	int num_tiles;
	int max_tiles;
};

proc_planet * proc_planet_new(float radius, height_map_func height, vec3 color_family);
void proc_planet_free(proc_planet *p);
int proc_planet_drawlist(proc_planet *p, tri_tile **tiles, int max_tiles, vec3 cam_pos, bpos_origin cam_sec);
float proc_planet_height(vec3 pos, vec3 *variety);

//Raycast towards the planet center and find the altitude on the deepest terrain tile. O(log(n)) complexity in the number of planet tiles.
float proc_planet_altitude(proc_planet *p, vec3 pos, bpos_origin sec, vec3 *intersection, bpos_origin *intersection_sector);

#endif
