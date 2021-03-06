#version 330 

in vec3 vPos;

uniform mat4 model_matrix;
uniform mat4 model_view_projection_matrix;

out vec3 gPos;

void main()
{
	gl_Position = model_view_projection_matrix * vec4(vPos, 1);
	gPos = vec3(model_matrix * vec4(vPos, 1));
}