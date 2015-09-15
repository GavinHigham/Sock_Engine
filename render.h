#ifndef RENDER_H
#define RENDER_H
#include <GL/glew.h>

void update(float dt); //should be moved to its own source file when things get big
void render();
void init_render();
void deinit_render();
void make_projection_matrix(GLfloat fov, GLfloat a, GLfloat n, GLfloat f, GLfloat *buf, int buf_len);

#endif