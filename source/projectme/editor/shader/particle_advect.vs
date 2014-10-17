#version 120
//varying vec2 vTexCoord;

void main(void)
{
	gl_TexCoord[0] = gl_MultiTexCoord0;
	//vTexCoord = gl_MultiTexCoord0.xy;
	gl_Position = gl_Vertex;
}