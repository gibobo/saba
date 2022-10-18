#version 100
attribute vec3 in_Pos;
attribute vec3 in_Nor;
attribute vec2 in_UV;

varying vec3 vs_Pos;
varying vec3 vs_Nor;
varying vec2 vs_UV;

uniform mat4 u_WV;
uniform mat4 u_WVP;

void main()
{
    gl_Position = u_WVP * vec4(in_Pos, 1.0);
    vs_Pos = (u_WV * vec4(in_Pos, 1.0)).xyz;
    vs_Nor = mat3(u_WV) * in_Nor;
    vs_UV = in_UV;

}
