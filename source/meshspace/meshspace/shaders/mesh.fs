#version 120

uniform sampler1D lookup;
uniform bool mapScalars;

uniform float scalarShift;
uniform float scalarScale;

varying vec3 vNormal;
varying vec4 vColor;

varying vec3 vViewDir;
varying vec3 vLightDir;
varying vec3 vLightPos;

varying float vScalar;
varying float vSelection;

// Classical Phong shading
vec3 phong( vec3 N, vec3 E, vec3 L, 
            vec3 ambient, vec3 diffuse, vec3 specular, 
			float shininess )
{	
	// Perfect reflection	
	vec3 R = normalize(-reflect(L,N));
	
	// Phong model
	vec3 Iamb  = vec3(0.,0.,0.); // ambient.rgb;	
#if 0
	// Two-sided shading:
	vec3 Idiff = diffuse.rgb * max(abs(dot(N,L)), 0.0);
	vec3 Ispec = specular.rgb * pow( max(abs(dot(R,E)),0.0), shininess );
#else
	// One-sided shading:
	vec3 Idiff = diffuse.rgb * max(dot(N,L), 0.0);
	vec3 Ispec = specular.rgb * pow( max(dot(R,E),0.0), shininess );	
#endif
	
	Idiff = clamp(Idiff, 0.0, 1.0);		
	Ispec = clamp(Ispec, 0.0, 1.0);
	
	return Iamb + Idiff + Ispec;
}

void main(void)
{
	vec3 lightDir = vec3(0.,-2.,1.); // = vLightDir
	
	vec3 diffuse = vColor.rgb; //gl_FrontLightProduct[0].diffuse.rgb;	
	
	// Apply transfer function to scalar value
	if( mapScalars )
		diffuse = texture1D( lookup, scalarScale*(vScalar+scalarShift) ).rgb;
	
	// Custom selection shading based on vertex color
	if( vSelection > .5 )
		diffuse = vec3(0.0,1.0,0.0);	
	
	// Phong shading
	vec3 shading = 
		phong( vNormal, normalize( vViewDir ), normalize( lightDir ),
				// Compatibilitiy fixe pipeline material
				gl_FrontLightProduct[0].ambient.rgb + gl_FrontLightModelProduct.sceneColor.rgb,
				diffuse,
				gl_FrontLightProduct[0].specular.rgb,
				gl_FrontMaterial.shininess );	

	gl_FragColor = vec4( shading, 1.0 );
}
