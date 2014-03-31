#version 120

uniform sampler1D lookup;
uniform bool mapScalars;

uniform float scalarShift;
uniform float scalarScale;

varying vec3 vNormal;
varying vec4 vColor;

varying vec3 vViewPos;
//varying vec3 vLightDir;
varying vec3 vLightPos;

flat varying float vScalar;
varying float vSelection;

// Classical Phong shading
vec3 phong( vec3 N, vec3 E, vec3 L, 
            vec3 ambient, vec3 diffuse, vec3 specular, 
			float shininess )
{	
	// Perfect reflection	
	vec3 R = normalize(2.0*N*dot(N,L)-L); // same as: normalize(-reflect(N,L))
		
	// Phong model
	vec3 Iamb  = ambient.rgb;
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
	vec3 normal   = normalize(vNormal);
	vec3 viewDir  = normalize(-vViewPos);
#if 0
	// Static light (i.e. not moving with camera)
	vec3 lightDir = normalize( gl_LightSource[0].position.xyz + viewDir );
#else
	// Moving light (i.e. specified in world coordinates)
	vec3 lightDir = normalize(vLightPos - vViewPos);
		// normalize(gl_LightSource[0].position.xyz - vViewPos);
#endif
	
	// Compatibilitiy fixed pipeline material
#if 0
	// Material explicitily specified
	vec4 mat_ambient  = gl_FrontMaterial.ambient; //vec4(0.2, 0.2, 0.2, 1.0);
	vec4 mat_diffuse  = gl_FrontMaterial.diffuse; //vec4(0.8, 0.8, 0.8, 1.0);
	vec4 mat_specular = gl_FrontMaterial.specular;//vec4(0.0, 0.0, 0.0, 1.0);	
	vec4 ambient = mat_ambient  * gl_LightSource[0].ambient;
	vec4 diffuse = mat_diffuse  * gl_LightSource[0].diffuse;	
	vec4 specular= mat_specular * gl_LightSource[0].specular;
	float shininess = gl_FrontMaterial.shininess;
#else
	// Use pre-multiplied material and light coefficients from OpenGL
	vec4 ambient = gl_FrontLightProduct[0].ambient;
	vec4 diffuse = gl_FrontLightProduct[0].diffuse;	
	vec4 specular= gl_FrontLightProduct[0].specular;
	float shininess = gl_FrontMaterial.shininess;
#endif
	// Considering scene color this way comes closer to fixed pipeline result ?!
	//ambient = ambient + gl_FrontLightModelProduct.sceneColor;

	// Custom color-coding
#if 1
	diffuse.rgb = vColor.rgb; //gl_FrontLightProduct[0].diffuse.rgb;	
	
	// Apply transfer function to scalar value
	if( mapScalars )
		diffuse.rgb = texture1D( lookup, scalarScale*(vScalar+scalarShift) ).rgb;
	
	// Custom selection shading based on vertex color
	if( vSelection > .5 )
		diffuse.rgb = vec3(0.0,1.0,0.0);
#endif

	// Phong shading
	vec3 shading = 
		phong( normal, viewDir, lightDir,
				ambient.rgb, diffuse.rgb, specular.rgb, shininess );

	gl_FragColor = vec4( shading, 1.0 );
}
