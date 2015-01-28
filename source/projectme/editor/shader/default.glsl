uniform vec3      iResolution;
uniform float     iGlobalTime;

float scale = 1.0; //###

#define MODE 0 //###{uvt,grayt}

void main(void)
{
	vec2 uv = gl_FragCoord.xy / iResolution.xy;
#if MODE==0
	vec3 color = vec3(uv,0.5+0.5*sin(iGlobalTime));
#else
	vec3 color = vec3(0.5+0.5*sin(iGlobalTime));
#endif
	gl_FragColor = vec4(scale*color,1.0);
}