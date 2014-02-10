varying vec3 vNormal;
varying vec4 vColor;

varying vec3 vViewDir;
varying vec3 vLightDir;
varying vec3 vLightPos;

void main(void)
{
	// Phong variables
	vec3 N = vNormal;
	vec3 E = normalize( vViewDir );	
	vec3 L = normalize( vLightDir );
	vec3 R = normalize(-reflect(L,N));
	
	// Compatibilitiy fixe pipeline material
	vec4 scene   = gl_FrontLightModelProduct.sceneColor; // ?
	vec4 ambient = gl_FrontLightProduct[0].ambient;
	vec4 diffuse = gl_FrontLightProduct[0].diffuse;
	vec4 specular= gl_FrontLightProduct[0].specular;
	float shininess = gl_FrontMaterial.shininess;
	
	// Phong model
	vec4 Iamb  = ambient;
	vec4 Idiff = diffuse * max(dot(N,L), 0.0);
	vec4 Ispec = specular * pow( max(dot(R,E),0.0), shininess );
	
	Idiff = clamp(Idiff, 0.0, 1.0);		
	Ispec = clamp(Ispec, 0.0, 1.0);
	
	// Custom selection shading based on vertex color
	if( vColor.b < .5 )
		Idiff = clamp( vColor * max(dot(N,L), 0.0), 0.0, 1.0 );

	gl_FragColor = scene + Iamb + Idiff + Ispec;
}	