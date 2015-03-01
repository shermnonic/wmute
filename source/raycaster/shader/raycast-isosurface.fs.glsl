// inputs
uniform sampler3D voltex;    // 3D image volume with scalar densities/opacities
uniform sampler2D fronttex;  // texture with ray start positions
uniform sampler2D backtex;   // texture with ray end positions

varying vec2 tc;             // texture coordinate

const float stepsize = 0.001;// stepsize for ray traversal

uniform float isovalue;      // Isovalue
uniform vec3  voxelsize;     // Size of a single voxel

// Light / material definition 
const vec3  light_pos = vec3(0.0,-1.0,0.0); // light position (in volume coordinates)
const vec3 mat_ka  = 0.1 * vec3(1.0,1.0,1.0);   // ambient  coeff. w/ color
const vec3 mat_ks  = 0.3 * vec3(1.0,1.0,1.0);   // specular coeff. w/ color
const vec3 mat_kd  = 0.6 * vec3(0.7,0.7,1.0);   // diffuse  coeff. w/ color
const float mat_exp = 16.0;  // Blinn-Phong exponent

// Compute normal by central differences
vec3 get_normal( vec3 x )
{		
	//////////////////////////////////////////////////////////////////////////
	// TODO: Implement central differences to compute normal at position x.
	//
	// Note that you can use the uniform "voxelsize" to find the texture 
	// coordinates of the neighbouring voxels to x.
	//
	//////////////////////////////////////////////////////////////////////////
	
	/* ... Insert your implementation here ... */

	return vec3(0,1,0);  // Dummy normal as example
	
	//////////////////////////////////////////////////////////////////////////	
}

// Compute Blinn-Phong illumation 
vec3 blinn_phong( vec3 N, vec3 eye )
{	
	vec3 L = normalize( light_pos + eye );  // Head-light emanating from viewer
	vec3 E = normalize( eye );
	vec3 H = normalize( L + E );            // Half-way vector

	float diff = 0.0;
	float spec = 0.0;
	
	//////////////////////////////////////////////////////////////////////////
	// TODO: Implement Blinn-Phong illumination model.
	//
	// As input this function receives:
	//   N   = the normal at x
	//   eye = vector from x to viewer	
	// 
	// The vectors N, H, E, L are defined as on the slide 56 of chapter 4.
	//
	//////////////////////////////////////////////////////////////////////////

	/* ... Insert your implementation here ... */
	
	//////////////////////////////////////////////////////////////////////////	

	return mat_ka + mat_ks*spec + mat_kd*diff;
}


void main(void)
{
	// ray start & end position
	vec3 ray_in  = texture2D( fronttex, tc ).xyz;
	vec3 ray_out = texture2D( backtex , tc ).xyz;
	
	// initial accumulated color and opacity
	vec4 dst = vec4(0,0,0,0);	
	
	//////////////////////////////////////////////////////////////////////////
	// TODO: Implement isosurface rendering by ray-casting "first hit".
	//
	// 1. Perform ray-casting with a fixed step size and terminate at first
	//    position with scalar value greater than the given isovalue.
	//
	// 2. Compute the normal at the intersection point.
	//
	// 3. Evaluate Blinn-Phong illumination model and return the color.
	//
	//////////////////////////////////////////////////////////////////////////
	
	/* ... Insert your implementation here ... */

	//////////////////////////////////////////////////////////////////////////

	gl_FragColor = dst;
}
