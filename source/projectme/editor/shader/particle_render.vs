#version 130
uniform sampler2D iPos;      // w counts lifetime backwards from max to zero
uniform sampler2D iBirthPos; // w contains max lifetime
uniform float pointSize;     // global point size
uniform vec3 iSize;    // size of particle textures (width*height = #particles)

out vec2 lifetime;
out float vertexid;

// https://stackoverflow.com/questions/4200224/random-noise-functions-for-glsl#4275343
float rand( vec2 co )
{
    return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
}

float computeLifetime( vec2 tc )
{
    return abs(rand(vec2(tc.x*tc.y,tc.x)))+0.3;
}

void main(void)
{
	// Get 2D texture coordinate from vertex index
	float id = float(gl_VertexID);
	vec2 tc;
#if 0
	tc = vec2(0.0,0.0);
#else
	tc.y = floor( id / iSize.x );
	tc.x = id - iSize.x*tc.y;
	tc /= iSize.xy;
#endif
	
	// Vertex position
	vec4 data = texture2D( iPos, tc );
    //vec4 data = texelFetch( iPos, ivec2(tc*iSize.xy), 0 );
	vec4 pos = vec4( vec3(data.xy, 0.5), 1.0 );	
	mat4 MVP = mat4(1.0); // Identity (was: gl_ModelViewProjectionMatrix)
	gl_Position = MVP * pos;
	
	// Lifetime
	float timeMax = computeLifetime( tc ); //texture2D( iBirthPos, tc ).w;
	float timeLeft = data.w;
	lifetime.x = timeMax - timeLeft; // current age
	lifetime.y = timeLeft; // time left
	
	// Color (pass through)
	gl_FrontColor = gl_Color * tc.x;
  #if 0
	if( data.z < 0.01 ) // If size scale = 0 ...
		gl_PointSize = 0.0; // ... do not draw this particle
	else
  #endif
	gl_PointSize = pointSize*(0.5+2.0*data.z); // Enforce minimum size

	//gl_PointSize = pointSize;
    
    vertexid = id;
#if 0 // DEBUG
    // It should hold id == data.w
	//tc.y = floor( id / iSize.x );	tc.x = id - iSize.x*tc.y;	
    float mid = data.w;    tc.y = floor( mid / iSize.x );	tc.x = mid - iSize.x*tc.y;	
	tc /= iSize.xy;
    lifetime = tc.xy;
#endif
}
