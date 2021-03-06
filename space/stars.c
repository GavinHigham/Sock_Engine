#include "glla.h"
#include "macros.h"
#include "effects.h"
#include "math/utility.h"
#include "math/bpos.h"
#include "graphics.h"
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <inttypes.h>

//Handy externs.
extern bpos_origin eye_sector;
extern float log_depth_intermediate_factor;

const int   STARS_NUM           = 40000;  //Total number of stars to generate.
const float STARS_SECTOR_RADIUS = 100000; //We want to make stars in a sphere, what is its radius in sectors?

static struct stars_globals {
	GLuint vbo;
	GLuint vao;
	bpos_origin all[STARS_NUM];   //All the stars
	int32_t nearby[3 * STARS_NUM]; //All the nearby stars, cast to int32_t to pass to the shader.
	int nearby_num;                //Number of stars that could be expressed with int32_t, number to draw.
} stars = {
	.nearby_num = 0
};

void stars_init()
{
	srand(101);

	//Make a cube that encompasses our sphere of stars.
	//Create random points in the cube, discard those not also in sphere.
	//Cube volume: 8*radius^3, Sphere volume: (4/3)*pi*r^3
	//Point is outside of sphere volume (1-((4/3)*pi)/8) or 47.6% of the time.
	//Corners of the volume within which sectors will be chosen.
	vec3 c1 = {-STARS_SECTOR_RADIUS, -STARS_SECTOR_RADIUS, -STARS_SECTOR_RADIUS};
	vec3 c2 = {STARS_SECTOR_RADIUS, STARS_SECTOR_RADIUS, STARS_SECTOR_RADIUS};
	for (int i = 0; i < STARS_NUM; i++) {
		//Choose a random sector to put the star in.
		vec3 s = rand_box_vec3(c1, c2);
		//If the star is in the sphere, keep it.
		if (vec3_mag(s) < STARS_SECTOR_RADIUS * STARS_SECTOR_RADIUS) {
			stars.all[i] = (bpos_origin){s.x, s.y, s.z};
		}
		else {
			i--;
		}
	}

	stars.nearby_num = 0;
	for (int i = 0; i < STARS_NUM; i++) {
		bpos_origin s = stars.all[i];
		int32_t x = s.x;
		int32_t y = s.y;
		int32_t z = s.z;
		//Check that an int32_t can hold the value safely
		if (x == s.x && y == s.y && z == s.z) {
			stars.nearby[i*3]     = x;
			stars.nearby[i*3 + 1] = y;
			stars.nearby[i*3 + 2] = z;
			stars.nearby_num++;
		}
	}

	glUseProgram(effects.stars.handle);
	glGenVertexArrays(1, &stars.vao);
	glBindVertexArray(stars.vao);
	glGenBuffers(1, &stars.vbo);
	glBindBuffer(GL_ARRAY_BUFFER, stars.vbo);
	glBufferData(GL_ARRAY_BUFFER, stars.nearby_num*3*sizeof(int32_t), stars.nearby, GL_STATIC_DRAW);
	glEnableVertexAttribArray(effects.stars.sector_coords);
	glVertexAttribIPointer(effects.stars.sector_coords, 3, GL_INT, 0, NULL);
	glUniform1f(effects.stars.sector_size, BPOS_CELL_SIZE);
	glUniform1f(effects.stars.log_depth_intermediate_factor, log_depth_intermediate_factor);
	glBindVertexArray(0);
}

void stars_deinit()
{
	glDeleteVertexArrays(1, &stars.vao);
	glDeleteBuffers(1, &stars.vbo);
}

void stars_draw(amat4 eye_frame, float proj_view_mat[16])
{
	glEnable(GL_BLEND); //We're going to blend the contribution from each individual star.
	glBlendEquation(GL_FUNC_ADD); //The light contributions get blended additively.
	glBlendFunc(GL_ONE, GL_ONE); //Just straight addition.
	glBindVertexArray(stars.vao);
	glUseProgram(effects.stars.handle);
	glUniform3f(effects.stars.eye_pos, eye_frame.t.x, eye_frame.t.y, eye_frame.t.z);
	glUniform3i(effects.stars.eye_sector_coords, eye_sector.x, eye_sector.y, eye_sector.z);
	glUniformMatrix4fv(effects.stars.model_view_projection_matrix, 1, GL_TRUE, proj_view_mat);
	glDrawArrays(GL_POINTS, 0, stars.nearby_num);
	glDisable(GL_BLEND);
	glBindVertexArray(0);
}