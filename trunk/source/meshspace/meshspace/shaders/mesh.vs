#version 120

#extension GL_EXT_gpu_shader4 : require

attribute float scalar;
attribute float selection;

varying vec3 vNormal;
varying vec4 vColor;
varying vec4 vVertex; // for special moves in the fragment shader

varying vec3 vViewPos;
//varying vec3 vLightDir;
varying vec3 vLightPos;

varying float vScalar;
/*flat*/ varying float vScalarFlat;
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
	vScalarFlat = scalar;
	
	// Interpolated variables
	
	vViewPos = viewpos;
	//vViewDir = -normalize(viewpos);
	vNormal = normalize(normal_matrix * normal);
	
	vLightPos = vec3(modelview * gl_LightSource[0].position);
	//~ vLightDir = normalize(viewpos - vLightPos);
	
	vColor = color;
	
	vVertex = position;
	
	// Pipeline output
	gl_FrontColor = color;
	gl_Position   = mviewproj * gl_Vertex;
}
