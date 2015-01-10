#version 130
uniform sampler2D iPos;      // w counts lifetime backwards from max to zero
uniform sampler2D iBirthPos; // w contains max lifetime
out vec2 lifetime;

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
	vec4 data = texture2D( iPos, tc );
	vec4 pos = vec4( data.xyz, 1.0 );	
	gl_Position = gl_ModelViewProjectionMatrix * pos;
	
	// Lifetime
	float timeMax = texture2D( iBirthPos, tc ).w;
	float timeLeft = data.w;
	lifetime.x = timeMax - timeLeft; // current age
	lifetime.y = timeLeft; // time left
	
	// Color (pass through)
	gl_FrontColor = gl_Color * tc.x;
}
