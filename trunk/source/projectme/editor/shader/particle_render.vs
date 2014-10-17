#version 130
uniform sampler2D iPos;

const vec2 texSize = vec2(256,256);

void main(void)
{
	// Get 2D texture coordinate from vertex index
	float id = float(gl_VertexID);
	vec2 tc;
	tc.y = floor( id / texSize.y );
	tc.x = id - texSize.y*tc.y;
	
	tc /= texSize;
	
	// Vertex position
	vec4 pos = texture2D( iPos, tc );	
	gl_Position = gl_ModelViewProjectionMatrix * pos;
	
	// Color (pass through)
	gl_FrontColor = gl_Color;
}
