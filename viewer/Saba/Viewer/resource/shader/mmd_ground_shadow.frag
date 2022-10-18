#version 100
precision lowp 	float;

uniform vec4	u_ShadowColor;

void main()
{
	gl_FragColor = u_ShadowColor;
}
