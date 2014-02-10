varying vec3 N;
varying vec3 v;
varying vec3 lightPos;

void main(void)
{		
	vec3 L = normalize(lightPos - v); // was: gl_LightSource[0].position.xyz
	vec3 E = normalize(-v);
	vec3 R = normalize(-reflect(L,N));
	
	vec4 Iamb = gl_FrontLightProduct[0].ambient;
		
	vec4 Idiff = gl_FrontLightProduct[0].diffuse * max(dot(N,L), 0.0);     
	Idiff = clamp(Idiff, 0.0, 1.0);
	
	vec4 Ispec = gl_FrontLightProduct[0].specular
	  * pow( max(dot(R,E),0.0), gl_FrontMaterial.shininess ); // was: 0.3 * ;
	Ispec = clamp(Ispec, 0.0, 1.0);

	gl_FragColor = gl_FrontLightModelProduct.sceneColor
				 + Iamb + Idiff + Ispec;
}	