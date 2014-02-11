#version 120

//uniform sampler1D lookup;

attribute float scalar;
attribute float selection;

varying vec3 vNormal;
varying vec4 vColor;

varying vec3 vViewDir;
varying vec3 vLightDir;
varying vec3 vLightPos;

varying float vScalar;
varying float vSelection;

void main(void)
{ 
	// Compatibility fixed pipeline attributes
	vec4 position  = vec4(gl_Vertex.xyz, 1.0);
	vec3 normal    = gl_Normal.xyz;
	vec4 color     = gl_Color;
	mat4 modelview = gl_ModelViewMatrix;
	mat3 normal_matrix = gl_NormalMatrix;
	mat4 mviewproj = gl_ModelViewProjectionMatrix;
	
	// Temporary vertex position in view space
	vec3 viewpos = vec3(modelview * position);
	
	// Special variables
	
	vSelection = selection;
	vScalar = scalar;
	
	// Interpolated variables
	
	vViewDir = -1.0 * viewpos;
	vNormal = normalize(normal_matrix * normal);
	
	vLightPos = vec3(modelview * gl_LightSource[0].position); 
	vLightDir = vLightPos - viewpos;	
	
	vColor = color;
	
	// Pipeline output
	gl_FrontColor = color;	
	gl_Position   = mviewproj * gl_Vertex;
}
