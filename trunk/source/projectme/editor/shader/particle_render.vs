#version 130
uniform sampler2D iPos;      // w counts lifetime backwards from max to zero
uniform sampler2D iBirthPos; // w contains max lifetime
uniform float pointSize;    // globale point size

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
	vec4 pos = vec4( vec3(data.xy, 0.5), 1.0 );	
	mat4 MVP = mat4(1.0); // Identity (was: gl_ModelViewProjectionMatrix)
	gl_Position = MVP * pos;
	
	// Lifetime
	float timeMax = texture2D( iBirthPos, tc ).w;
	float timeLeft = data.w;
	lifetime.x = timeMax - timeLeft; // current age
	lifetime.y = timeLeft; // time left
	
	// Color (pass through)
	gl_FrontColor = gl_Color * tc.x;
	if( data.z < 0.01 ) // If size scale = 0 ...
		gl_PointSize = 0.0; // ... do not draw this particle
	else
		gl_PointSize = pointSize*(0.5+2.0*data.z); // Enforce minimum size
}
