#version 120
uniform sampler2D iPos;
uniform sampler2D iVel;
varying vec2 vTexCoord;
varying vec4 vColor;
varying vec4 vPos;

void main(void)
{
	vTexCoord = gl_MultiTexCoord0.xy;
	gl_TexCoord[0] = gl_MultiTexCoord0;
	//vTexCoord = gl_Vertex.xy;
	vColor = gl_Color;
	gl_Position = gl_Vertex;
	vPos = gl_Vertex;
}
