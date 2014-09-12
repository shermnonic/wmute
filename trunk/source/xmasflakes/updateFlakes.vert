uniform float Test;

void main()
{
	gl_TexCoord[0] = gl_MultiTexCoord0;
	gl_Position = gl_Vertex;
}
