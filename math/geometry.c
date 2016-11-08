#include <stdbool.h>
#include "geometry.h"
#include <glalgebra.h>

//Ported from http://geometrictools.com/Documentation/IntersectionSphereCone.pdf
bool sphere_cone_intersect(struct sphere s, struct cone c)
{
	vec3 u = vec3_sub(c.vertex, vec3_scale(c.axis, (s.radius*c.sin_rec)));
	vec3 d = vec3_sub(s.center, u);
	float d_sq = vec3_dot(d, d);
	float e = vec3_dot(c.axis, d);
	if (e > 0 && e*e >= d_sq * c.cos_sq) {
		d = vec3_sub(s.center, c.vertex);
		d_sq = vec3_dot(d, d);
		e = -vec3_dot(c.axis, d);
		if (e > 0 && e*e >= d_sq*c.sin_sq)
			return d_sq <= s.radius_sq;
		else
			return true;
	}
	return false;
}