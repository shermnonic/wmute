///////////////////////////////////////////////////////////////////////////////
// raycast - fragment shader
//
// Straight forward single-pass raycasting. Shader is initialized with
// traversal start and end positions in "fronttex" and "backtex". 
// In the easiest case you just render the front- respectively back-faces 
// of the volume bounding box to the front- and back-textures with canonical
// RGB coloring from (0,0,0) to (1,1,1).
//
// We use here custom preprocessor strings "__opt__XXX__".
// (Could think about default value definition in-code which would
//  be automatically set if preprocessor define is not set by host.
//  Could use something like "__opt__XXX__[default_value_here]__".)
//
// TODO:
// - allow optional "fronttex_depth" input sampler which can be used to
//   calculate the real depth value of the rendered isosurface voxel
//   to write it out to gl_FragDepth.
// 
// Max Hermann, March 22, 2010
///////////////////////////////////////////////////////////////////////////////

// Debug modes:
// 1 = debug ray termination (red=left volume, green=opacity>=threshold)
// 2 = render ray length
// 3 = render texture coordinate
//#define DEBUG

// replace MIP rendering by Groeller's MIDA algorithm
// - performance loss because early-ray termination not applicable
// - most notable differences to DVR when LIGHTING turned on
// TODO: * check correctness of implementation 
//       * add gamma parameter to interpolate between DVR, MIDA and MIP
//#define MIDA_TEST

// Lighting for direct volume rendering (works, but pretty slow)
//#define LIGHTING

// Texture filtering (assuming texture mode is GL_LINEAR)
//   0 - simulate nearest neighbour
//   1 - hw linear filtering
#define FILTER 1

// Color mode (if warps are used)
// 0 - plain white
// 1 - color warp strength in Red channel
// 2 - custom warp strength coloring (see code below)
// 3 - fancy coloring (add displacement to phong color)
#define COLORMODE 3

// Non-polygonal isosurface rendering with Phong shading
// and intersection refinement
#define ISOSURFACE <__opt_ISOSURFACE__>

// Maximum intensity projection (DVR, lighting supported but useless in general)
#define MIP        <__opt_MIP__>

// Simple silhouette shader (comparing abs(dot(n,view)) < eps)
#define SILHOUETTE <__opt_SILHOUETTE__>

// Numbner of channels per element (1=scalar field, 3=vector field)
#define CHANNELS   <__opt_CHANNELS__>

// Warp volume by using a second offset texture for displacement
#define WARP       <__opt_WARP__>

// Support for several warp volumes (new samplers and uniforms!), requires WARP
#define MULTIWARP  <__opt_MULTIWARP__>

// A mean warp volume can be added to warp offset
#define MEANWARP   <__opt_MEANWARP__>

// Depth buffer texture for front geometry supplied?
#define DEPTHTEX   <__opt_DEPTHTEX__>


// input
uniform sampler3D voltex;
uniform sampler2D fronttex;
uniform sampler2D backtex;
varying vec4 color;
varying vec2 tc;
#if WARP==1
	#if MULTIWARP==1
	// hardcoded support for 5 warp modes
	uniform sampler3D warpmode0;
	uniform sampler3D warpmode1;
	uniform sampler3D warpmode2;
	uniform sampler3D warpmode3;
	uniform sampler3D warpmode4;
	uniform float lambda0;
	uniform float lambda1;
	uniform float lambda2;
	uniform float lambda3;
	uniform float lambda4;
	#else
	uniform sampler3D warpmode0; // offset texture
	uniform float     lambda0;
	uniform vec3      warp_ofs; // custom offset (yet only used for single warptex)	
	#endif
	
	#if MEANWARP==1
	uniform sampler3D meanwarptex; // additional offset texture
	#endif	
#endif
#if DEPTHTEX==1
uniform sampler2D depthtex; // depth buffer of front geometry
#endif

uniform float isovalue;     // non-polygonal isosurface
uniform float alpha_scale;  // arbitrary alpha scaling for DVR

// size of a voxel (for finite differences)
// used for:
// - normal calculation
// - displacement vector scaling 
const vec3 voxelsize = vec3( 1.0/<__opt_WIDTH__>.0, 
                             1.0/<__opt_HEIGHT__>.0, 
							 1.0/<__opt_DEPTH__>.0 );
const vec3 voxelsizei = vec3( <__opt_WIDTH__>, 
                              <__opt_HEIGHT__>, 
							  <__opt_DEPTH__> );							 

// stepsize for ray traversal                FIXME: should be a user parameter
//const float stepsize = 0.005; //0.0005; // 0.001
uniform float stepsize;
// number of isosurface refinement steps
const int refinement_steps = 5;

// light/material definition                 FIXME: should be adjustable (use GL_LIGHT0?)
const vec3  light_pos = vec3(1,1,1);
//~ const float mat_ka  = 0.04;  // ambient  coeff.
//~ const float mat_ks  = 0.36;  // specular coeff.
//~ const float mat_kd  = 0.6;   // diffuse  coeff.
//~ const float mat_phong_exp = 8.0;

//~ const float mat_ka  = 0.23;  // ambient  coeff.
//~ const float mat_ks  = 0.06;  // specular coeff.
//~ const float mat_kd  = 0.7;   // diffuse  coeff.
//~ const float mat_phong_exp = 5.8;

const float mat_ka  = 0.1;  // ambient  coeff.
const float mat_ks  = 0.94;  // specular coeff.
const float mat_kd  = 0.54;   // diffuse  coeff.
const float mat_phong_exp = 12;

const vec3 col_ka = vec3(.8,.9,.8);
const vec3 col_kd = vec3(.2,.2,.95);
const vec3 col_ks = vec3(1,1,.3);

#if WARP == 1
//------------------------------------------------------------------------------
// Return displacement vector
// TODO: correct normalization/scaling
vec3 get_warp( vec3 x )
{
	const float mode_scale = 200.0;
	const float mode_shift = 0.5;
	
  #if MULTIWARP==1
	// TODO: make number of warps configurable and auto produce shader code
	const float eps = 0.0001;
	const float stdev[5] = { 1,1,1,1,1 };  // stdev already premultiplied
		// { 0.7551, 0.1280, 0.0894, 0.0445, 0.0559 };
		// { 1,1,1,1,1 };  // stdev already premultiplied
		// default:
		//   0.187, 0.187, 0.187, 0.187, 0.187
		// svm2_GerbiMuri_selection2_Vw10_projected_orthogonal_to_w0:
		//   0.6456, 0.3169, 0.2469, 0.1187, 0.1028
		// svm3_Size_nothreshold_Vw10_projected_orthogonal_to_w0:
		//   0.7551, 0.1280, 0.0894, 0.0445, 0.0559
	vec3 disp = vec3(0,0,0);	
	
	// FIXME: Using all 5 modes produces heavy pixel artifacts in colored 
	//        isosurface rendering!
	
	if( abs(lambda0)>eps ) disp += stdev[0] * lambda0 * mode_scale*(texture3D(warpmode0, x).rgb - mode_shift) * voxelsize;
	if( abs(lambda1)>eps ) disp += stdev[1] * lambda1 * mode_scale*(texture3D(warpmode1, x).rgb - mode_shift) * voxelsize;
	if( abs(lambda2)>eps ) disp += stdev[2] * lambda2 * mode_scale*(texture3D(warpmode2, x).rgb - mode_shift) * voxelsize;
	if( abs(lambda3)>eps ) disp += stdev[3] * lambda3 * mode_scale*(texture3D(warpmode3, x).rgb - mode_shift) * voxelsize;
	if( abs(lambda4)>eps ) disp += stdev[4] * lambda4 * mode_scale*(texture3D(warpmode4, x).rgb - mode_shift) * voxelsize;	
  #else	
	// scale with inverse standard deviation = sqrt(eigenvalue)/norm(eigenvector) for principal warps	
	float stdev = 1.0; // stdev already premultiplied	
		// defaults: 0.187     - nothreshold
		//           0.2187    - selection2 ?
		//
		// projector_svm2_GerbiMuri_nothreshold_Vw10:
		// ------------------------------------------
		//		matlab> classified0 = V(:,1:K)*w0 + repmat(b0,30,1)
		//		matlab> sqrt(var(classified))/30
		//
		//           0.0297    - looks plausible for this dataset
		// 0.024
	
	// displacement
	vec3 disp = stdev * lambda0 * mode_scale*(texture3D( warpmode0, x + 0.1*warp_ofs ).rgb - mode_shift) * voxelsize;
  #endif
	
  #if MEANWARP == 1
	const float mean_scale = 1.0;
	disp += texture3D( meanwarptex, x ).rgb * mean_scale;
  #endif
  
	return disp;
}

#endif

//------------------------------------------------------------------------------
// Return scalar value for volume coordinate x
float get_volume_scalar( vec3 x )
{
#if WARP == 1
	// deform volume by offset texture
	x += get_warp( x );
#endif
#if CHANNELS == 3
	// return magnitude of vectorfield entry
	return length( texture3D( voltex, x ).rgb );
#else
  #if FILTER == 1
	return texture3D( voltex, x ).w;
  #elif FILTER == 0	
	// simulate nearest neighbour filtering
	return texture3D( voltex, (floor(x*voxelsizei)+0.0)*voxelsize ).w;
  #endif
#endif
}

//------------------------------------------------------------------------------
// Compute normal by central differences
vec3 get_normal( vec3 x )
{
  #if FILTER == 0
	// simulate nearest neighbour filtering
	x = (round(x*voxelsizei)-0.0)*voxelsize;
  #endif
	vec3 n = vec3( 
	  get_volume_scalar(x - vec3(voxelsize.x,0,0)) - get_volume_scalar(x + vec3(voxelsize.x,0,0)), 
	  get_volume_scalar(x - vec3(0,voxelsize.y,0)) - get_volume_scalar(x + vec3(0,voxelsize.y,0)), 
	  get_volume_scalar(x - vec3(0,0,voxelsize.z)) - get_volume_scalar(x + vec3(0,0,voxelsize.z))
	);
	return normalize(n);
}

//------------------------------------------------------------------------------
// Compute phong illumation (n==normal at x, eye==vector from x to viewer)
// TODO: check computation and constants, just copied from internet ;-)
vec3 phong( vec3 n, vec3 eye )
{
	float I = 1.0;
	vec3 L = normalize( eye ); //light_pos + eye + vec3(-1,-0,-1) );
	vec3 E = normalize( eye );

	vec3 R = reflect( -L, n );
	float diff = max( dot(L,n), 0.23 );
	float spec = 0.0;
	if( diff > 0.0 )
	{
		spec = max( dot(R,E), 0.7 );
		spec = pow( spec, mat_phong_exp );
	}

	return col_ka*mat_ka + col_ks*mat_ks*spec + col_kd*mat_kd*diff;
}

//------------------------------------------------------------------------------
// Convert fragment depth value to eye space z and vice versa
// where 
//   a = zFar / (zFar - zNear)
//   b = zFar*zNear / (zNear - zFar)
float z_to_depth( float z, float a, float b ) {	return a + b/z;	}
float depth_to_z( float d, float a, float b ) { return b * (1.0 / (d - a)); }

//------------------------------------------------------------------------------
void main(void)
{
	// ray start & end position
	vec3 ray_in  = texture2D( fronttex, tc ).xyz;
	vec3 ray_out = texture2D( backtex , tc ).xyz;

	// ray direction and traversal length
	vec3  dir = ray_out - ray_in;
	float len = length(dir)+0.001;
		  dir = dir / len;      // normalize direction

	// traversal step vector
	vec3 step = stepsize * dir;

	// initial ray position
	vec3 ray = vec3(0,0,0); //was: step

	// initial accumulated color and opacity
	vec4 dst = vec4(0,0,0,0);
	
#if MIP == 1
	float mip = 0.0;
	// HACK: alpha_scale not important for MIP?
	alpha_scale = 0.01;
#endif

	// ray traversal
	for( int i=0; i < int(1/stepsize)+200; ++i ) // BUBUG: ray end not correct?
	{
		// ray termination
#ifdef MIDA_TEST
		if( length(ray) >= len )
		{
			break;
		}
#else
	#ifndef DEBUG
		if( length(ray) >= len || dst.a >= 0.99 ) break;
	#else			
		if( length(ray) >= len ) 
		{ 
			dst = vec4(1,0,0,1);  // DEBUG break condition
			break; 
		}
		if( dst.a >= 0.99 ) 
		{ 
			dst.a = 1.0; 
			dst = vec4(0,1,0,1);  // DEBUG break condition
			break; 
		}
	#endif
#endif
		// TODO: transfer funcion
		float intensity = get_volume_scalar( ray_in + ray );

#if ISOSURFACE == 1
		if( intensity > isovalue )
		{
			// intersection refinement
			float searchdir = -1;    // initial step is backwards
			vec3 ministep = step*.5;
			for( int j=0; j < refinement_steps; ++j )
			{
				// binary search
				ray += searchdir*ministep;
				
				intensity = get_volume_scalar( ray_in+ray );

				// move backwards until exact intersection overstepped
				// and vice versa
				if( intensity > isovalue )
					searchdir = -1;
				else
					searchdir = 1;

				// half stepsize
				ministep *= .5;
			}

			vec3 n = get_normal(ray_in+ray);			
		
	#if SILHOUETTE == 1
			if( abs(dot(n,-dir)) < 0.6 )
				dst.rgb = vec3(0,0,0);
			else
				dst.rgb = vec3(1,1,1);
	#else
		#if WARP == 1
			//~ float wlen = clamp( length( get_warp(ray_in+ray) * 25.0 ), 0.0, 1.0 );
			//~ dst.rgb = wlen;
			vec3 disp = get_warp( ray_in+ray );
			float disp_color_scale = 25.0;
		  #if COLORMODE == 1			
			// encode warp strength in R channel
			float li = phong( n, -dir ).x;
			float imp = length( disp*disp_color_scale );
			imp = clamp( imp, 0.0, 0.7 );
			// li = clamp( li, 0.1, 1.0 );
			dst.rgb = vec3(.2+1.6*imp,0.8,1.3-imp) * li;
		  #elif COLORMODE == 2
			// project displacement vector onto surface normal
			// sign indicates if warp deforms deforms surface in- or outwards
			float li = phong( n, -dir ).x;
			float imp = dot( disp*disp_color_scale, n );
			float imp_pos = imp;
			float imp_neg = imp;
			clamp( imp_pos, 0, 1.0 );
			clamp( imp_neg, -1.0, 0 );
			imp_neg = -imp_neg;
			// grey = no change
			// red  = outwards warp
			// blue = inwards warp
			dst.rgb = vec3( .5+.5*imp_pos, .5, .5+.5*imp_neg );
		  #elif COLORMODE == 3	
		    dst.rgb = phong( n, -dir ) + disp;
		  #else // COLORMODE == 0
		    dst.rgb = phong( n, -dir );
		  #endif
		#else
			dst.rgb = phong( n, -dir );
		#endif
	#endif
			dst.a   = 1;
			break;
		}
#else
		vec4 src = intensity; 
			//vec4(texture3D( voltex, ray_in + ray ).rgb, intensity);
		src.a *= alpha_scale;

  #if MIP == 1
	 #ifdef MIDA_TEST
		// MIDA test
		float beta=1;
		if( intensity > mip )
		{
			beta = 1 - (intensity - mip);
			mip = intensity;
		}

		#ifdef LIGHTING
			float li = phong( get_normal(ray_in+ray), -dir ).x;
		#else
			float li = 1.0;
		#endif

		// pre-multiplied alpha
		//dst.rgb = beta*src.rgb + (1 - beta*src.a) * intensity;
		//dst.a   = beta*src.a   + (1 - beta*src.a);
		dst = beta*dst + (1 - beta*dst.a) * src;
		
	 #else
		// MIP
		if(intensity > mip) 
		{
			mip = intensity;
		#ifdef LIGHTING
			float li = phong( get_normal(ray_in+ray), -dir ).x;
		#else
			float li = 1.0;
		#endif
			dst.rgb = intensity*li;
			dst.a   = sqrt(intensity)*(1+alpha_scale);  // hack (see break condition)
		}
	 #endif
  #else
	#ifdef LIGHTING
		float li = phong( get_normal(ray_in+ray), -dir ).x;
		dst.rgb = dst.rgb + (1-dst.a)*src.rgb * li;
		dst.a   = dst.a   + (1-dst.a)*src.a;
	#else
		// front-to-back compositing (w/ pre-multiplied alpha)
		dst = dst + (1 - dst.a) * src;
	#endif // LIGHTING		
  #endif // MIP

#endif
		// advance ray position
		ray += step;
	}

	gl_FragColor = vec4( dst.rgb, dst.a );
//~ #if DEPTHTEX==1	
	//~ gl_FragColor = dst; //gl_FragCoord.z + 0.5*length(ray); //texture2D( depthtex, tc ).w;
//~ #endif
	//gl_FragDepth = FIXME: For depth value we only need depth of ray_start.
	//                      Via R2T its no problem to also render depthbuffer of front
	//                      facing volume bounding geometry to texture and pass it to
	//                      this shader!
	//gl_FragColor = vec4(color.rgb * len * 0.68, 1); // DEBUG ray length (scale by 1/sqrt(3))
	//gl_FragColor = vec4(1,tc.s,tc.t,1);             // DEBUG texture coordinates
}
