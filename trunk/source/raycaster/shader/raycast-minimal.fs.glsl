// Raycaster for direct volume rendering using the under overaptor
// Max Hermann, March 2014

// inputs
uniform sampler3D voltex;    // 3D image volume with scalar densities
uniform sampler2D fronttex;  // texture with ray start positions
uniform sampler2D backtex;   // texture with ray end positions
uniform sampler1D luttex;    // 1D RGBA lookup table 

varying vec2 tc;             // texture coordinate

// Step sizes
const float s0       = 0.002;
const float stepsize = 0.001;// stepsize for ray traversal

// Sampling rates (used in opacity correction calculation)
const float SR0 = 1.0/s0;
const float SR  = 1.0/stepsize;


// 1 - online opacity correction  & pre-multiplied alpha (like GPU Gems)
// 2 - offline opacity correction & pre-multiplied alpha (like GPU Gems)
// 3 - offline opacity correction (VTK style, allows usage of Paraview XML presets)
#define OPACITY_CORRECTION 3


// Apply transfer function
vec4 transfer( float scalar )
{
	vec4 src = texture1D( luttex, scalar );
	
  #if   OPACITY_CORRECTION == 1
	// Opacity correction
	src.a = (1.0 - pow(1.0 - src.a, SR0/SR));
	
	// Opacity weighted color [Wittenbrink1998]
	src.rgb *= src.a;
	
  #elif OPACITY_CORRECTION == 2
	// Nothing to be done here, everything pre-computed
	
  #elif OPACITY_CORRECTION == 3	
	// VTK custom adaption (opacities somehow corrected, but no pre-mult.alpha)
	src.a *= 128.0;
	src.a = clamp(src.a,0.0,1.0);
	src.rgb *= src.a;	
  #endif
	
	return src;
}


// Undo transfer function mapping (e.g. for display of the original color map)
vec4 untransfer( float scalar )
{
	vec4 lut = texture1D( luttex, scalar );
	
  #if   OPACITY_CORRECTION == 1
	// Linear transfer function (sample rate corrected on-line above)
	float opacity = lut.a;
	vec3 color = lut.rgb;
	
  #elif OPACITY_CORRECTION == 2
	// Undo pre-computed opacity correction
	float opacity = 1.0 - pow(1.0 - lut.a,SR/SR0); // GPUGems
	// Undo pre-multiplied alpha
	vec3 color = lut.rgb / lut.a;
	
  #elif OPACITY_CORRECTION == 3
	// Undo pre-computed opacity correction
	float opacity = 1.0 - pow(1.0 - lut.a,1.0/stepsize); // VTK
	// No pre-multiplied alpha
	vec3 color = lut.rgb;	
  #endif	

	return vec4( color, opacity );
}


void main(void)
{
	// ray start & end position
	vec3 ray_in  = texture2D( fronttex, tc ).xyz;
	vec3 ray_out = texture2D( backtex , tc ).xyz;	
	
	float ray_len = length(ray_out - ray_in);
	vec3  ray_dir = normalize(ray_out - ray_in);
	
	// initial accumulated color and opacity
	vec4 dst = vec4(0.0);
	
	//////////////////////////////////////////////////////////////////////////
	// Accumulative ray-casting using the under operator.
	//
	// 1. Perform equidistant sampling of a ray between ray_in and ray_out in
	//    a for() or a while() loop using the given stepsize. Terminate the
	//    loop when leaving the unit cube or when alpha is saturated, i.e. 
	//    dst.a >= 0.99.
	//
	// 2. Evaluate the scalar volume image at the sampling position, e.g.:
	//
	//         float scalar = texture3D( voltex, sampling_position ).w;
	//
	// 3. Apply the transfer function via a texture lookup
	//
	//         texture1D( luttex, scalar )
	//
	//    where the LUT value is already the opacity computed via 
	//    exponentiation of the density value with the given stepsize.
	//
	// 4. Perform front-to-back compositing using the under operator, e.g.:
	//
	//         dst = dst + (1 - dst.a) * src
	//
	//////////////////////////////////////////////////////////////////////////
	
	const int numsteps = int(SR * 1.4142135); // *sqrt(2)
	for( int i=0; i < numsteps; ++i )
	{
		// Position along ray
		vec3 step = float(i) * stepsize * ray_dir;
		vec3 d = ray_in + step;
		
		// Ray termination 
		if( length(step) > ray_len || dst.a >= (1.0-0.0039) ) // 1/255=0.0039
			break;		
		
		// Scalar intensity
		float scalar = texture3D( voltex, d ).w;
		
		// Map through transfer function
		vec4 src = transfer( scalar );		
		
		if( src.a > 0.0 )
		{
			// Compositing
			dst = dst + (1.0 - dst.a) * src;
		}
	}

	//////////////////////////////////////////////////////////////////////////
	// Final volume rendering

	// Heuristic blending with a background color
	//dst += vec4(vec3(0.3),1.0) * (1.0-smoothstep(0.0,0.2,dst.a));
	
	gl_FragColor = dst;

	//////////////////////////////////////////////////////////////////////////
	// Display transfer function
	
	// Uncomment the following lines to show transfer function for debugging
	vec4 lut = untransfer( tc.x );
	const float opacityScaling = 1.0;
	if( tc.y > 0.9  && tc.y < 0.945 )   gl_FragColor = vec4( vec3(opacityScaling*lut.a), 1.0 );
	if( tc.y > 0.95 && tc.y < 0.995 )	gl_FragColor = vec4( lut.rgb, 1.0 );
	
	// Exponentially pre-filtered transfer function
	//if( tc.y > 0.9 )	gl_FragColor = vec4( lut )  / (5.0*stepsize);
	//if( tc.y > 0.95 )	gl_FragColor = vec4( - log(-lut.a + 1.0) / stepsize );
}
