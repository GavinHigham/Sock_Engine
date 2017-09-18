#ifndef RENDERER_H
#define RENDERER_H
#include <GL/glew.h>

extern float screen_width;
extern float screen_height;
void update(float dt); //should be moved to its own source file when things get big
void render();
void renderer_init();
void renderer_deinit();
void renderer_queue_reload();
void handle_resize(int screen_width, int screen_height);
void make_projection_matrix(GLfloat fov, GLfloat a, GLfloat n, GLfloat f, GLfloat *buf);

#endif