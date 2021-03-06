#ifndef BUFFER_GROUP_H
#define BUFFER_GROUP_H
#include "graphics.h"
#include "effects.h"

enum {
	BG_BUFFER_NORMALS    = 1,
	BG_BUFFER_COLORS     = 2,
	BG_USING_ADJACENCIES = 4 //These are bitflags, each new flag should be double the last one.
};

//Add more vertex attribute buffer flags as powers-of-two

struct buffer_group {
	GLuint vao, aibo, ibo;
	union {
		struct {
			GLuint sector_coords, star_pos, cbo, nbo, vbo; //Must be in the same order as the attributes in effects.h (until I fix that).
		};
		GLuint buffer_handles[sizeof(attribute_strings)/sizeof(attribute_strings[0])];
	};
	int index_count;
	int flags;
	GLenum primitive_type;
};

struct buffer_group new_buffer_group(int (*buffering_function)(struct buffer_group), EFFECT *effect);
struct buffer_group new_custom_buffer_group(int (*buffering_function)(struct buffer_group), int buffer_flags, GLenum primitive_type);
int buffer_group_attribute_index(const char *attr_name);
void delete_buffer_group(struct buffer_group tmp);
void setup_attrib_for_draw(GLuint attr_handle, GLuint buffer, GLenum attr_type, int attr_size);

#endif