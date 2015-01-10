/*	Texture muxer
	www.386dx25.de 
*/
uniform vec3 iResolution;
uniform float iGlobalTime;

uniform sampler2D iChannel0;
uniform sampler2D iChannel1;
uniform sampler2D iChannel2;
uniform sampler2D iChannel3;

float w0=0.25; //####
float w1=0.25; //####
float w2=0.25; //####
float w3=0.25; //####

void main(void)
{
	// Domain [0,1]x[0,1]
	vec2 uv = gl_FragCoord.xy / iResolution.y;

	gl_FragColor = //vec4(1.0,0.5,0.1,1.0);
	  w0*texture2D(iChannel0,uv) 
	+ w1*texture2D(iChannel1,uv) 
	+ w2*texture2D(iChannel2,uv) 
	+ w3*texture2D(iChannel3,uv);
}
