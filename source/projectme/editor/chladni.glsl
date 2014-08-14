/*	2D Chladni patterns, see for instance:
	http://paulbourke.net/geometry/chladni/
	www.386dx25.de 
*/
uniform vec3 iResolution;
uniform float iGlobalTime;

const float pi = 3.1415926535897932384;

// Edge lengths of the rectunglar plate. Note that in reality only 
// for the case a=b degenerate eigenmodes appear, leading to the 
// superimposition as implemented here.
float a;
float b = 1.0;

// Chladni eigenmodes
float chladni( float m, float n, vec2 uv )
{	
	// cos()*cos() for modes of a plate fixed at its center
	// sin()*sin() for modes of a plate fixed at its border (boring)
	return cos(n*pi*uv.x/a)*cos(m*pi*uv.y/b);
}

// Eigenfrequencies (not used)
float omega( float m, float n )
{
	const float rho = 1.0;
	const float eta = 1.0;	
	return pi * sqrt( (rho/eta) * (m/a)*(m/a) + (n/b)*(n/b) );
}

void main(void)
{
	// Domain [0,1]x[0,1]
	vec2 uv = gl_FragCoord.xy / iResolution.y;
	a = iResolution.x / iResolution.y;
	
	// Knot numbers
	vec2 mn = vec2(5.0,2.0);
	
	// Superposition coefficients
	float alpha = iGlobalTime;
	mat2 R = mat2( cos(alpha), sin(alpha), -sin(alpha), cos(alpha) );
	vec2 c = R * vec2(1.0,-1.0);	
	//c = vec2(1.0,-1.0); // Default coefficients
	
	// Superposition of eigenmodes
	float u = c.x*chladni(mn.x,mn.y,uv) + c.y*chladni(mn.y,mn.x,uv);
	
	// Shift-scale from [-1,+1] to [0,1]		
	u = (0.5+u/2.0);
	
	// Visualize knot lines (i.e. zero-crossings)
	//u = step( abs(u-0.3), 0.13 );
	
	// Smooth radial border blend out
	float r = length(2.0*uv - vec2(1.0,1.0));
	float border = 1.0 - smoothstep( 0.8, 1.1, r );
	
	gl_FragColor = vec4(border*u*vec3(0.5),1.0);
}
