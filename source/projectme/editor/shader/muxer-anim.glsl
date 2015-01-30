/*	Texture muxer (animation)
	www.386dx25.de 
*/
uniform vec3 iResolution;
uniform float iGlobalTime;

uniform sampler2D iChannel0;
uniform sampler2D iChannel1;
uniform sampler2D iChannel2;
uniform sampler2D iChannel3;

float animSpeed=0.5; //###
float alpha=iGlobalTime; //0.0;

void main(void)
{
	alpha = fmod(animSpeed*alpha,4.0);
	float a = alpha - floor(alpha);
	
	vec4 b = vec4(0.0);
	if( alpha < 1.0 ) b = vec4((1.0-a),a,0.0,0.0); else
	if( alpha < 2.0 ) b = vec4(0.0,(1.0-a),a,0.0); else
	if( alpha < 3.0 ) b = vec4(0.0,0.0,(1.0-a),a); else
	if( alpha < 4.0 ) b = vec4(a,0.0,0.0,(1.0-a));

	// Domain [0,1]x[0,1]
	vec2 uv = gl_FragCoord.xy / iResolution.y;

	gl_FragColor = //vec4(1.0,0.5,0.1,1.0);
	  b.x*texture2D(iChannel0,uv) 
	+ b.y*texture2D(iChannel1,uv) 
	+ b.z*texture2D(iChannel2,uv) 
	+ b.w*texture2D(iChannel3,uv);
}
