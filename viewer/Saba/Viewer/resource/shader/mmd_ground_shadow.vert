#version 100

// Input
attribute vec3	in_Pos;

// Uniform
uniform	mat4	u_WVP;

void main()
{
	gl_Position = u_WVP * vec4(in_Pos, 1.0);
}
