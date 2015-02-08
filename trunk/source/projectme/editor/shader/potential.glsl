uniform vec3      iResolution;
uniform float     iGlobalTime;
uniform sampler2D iChannel0;
uniform vec3      iChannelResolution[4];

// Potential is given in channel0 (red component)
float potential( vec2 uv )
{
	return texture2D( iChannel0, uv ).r;
}

// Gradient of scalar potential
vec2 gradient( vec2 uv, vec2 dxy )
{
	vec2 dx = vec2(dxy.x, 0.0);
	vec2 dy = vec2(dxy.y, 0.0);
	vec2 grad;
	grad.x = 0.5*(potential( uv + dx ) - potential( uv - dx ));
	grad.y = 0.5*(potential( uv + dy ) - potential( uv - dy ));
	return grad;
}

void main(void)
{
	vec2 uv = (gl_FragCoord.xy+vec2(0.5))/iResolution.xy; // Sample at pixel center	
	vec2 dxy = vec2(1.0)/iChannelResolution[0].xy ); // Size of pixel in channel0

	gl_FragColor = vec4( gradient(uv,dxy), 0.0, 1.0 );
	
}