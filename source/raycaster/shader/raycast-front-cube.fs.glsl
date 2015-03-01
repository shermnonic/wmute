// inputs
uniform sampler3D voltex;
uniform sampler2D fronttex;
uniform sampler2D backtex;

varying vec4 color;
varying vec2 tc;

void main(void)
{
	// ray start & end position
	vec3 ray_in  = texture2D( fronttex, tc ).xyz;
	vec3 ray_out = texture2D( backtex , tc ).xyz;

	gl_FragColor = vec4( ray_in, 1.0 );
}
