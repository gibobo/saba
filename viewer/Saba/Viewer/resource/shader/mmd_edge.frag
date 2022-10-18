#version 100

precision lowp 	float;

uniform vec4 u_EdgeColor;

void main()
{
	gl_FragColor = u_EdgeColor;
}
