uniform vec3      iResolution;
uniform float     iGlobalTime;
uniform sampler2D iChannel0;

// Created by inigo quilez - iq/2013
// License Creative Commons Attribution-NonCommercial-ShareAlike 3.0 Unported License.

void main(void)
{
    // create pixel coordinates
	vec2 uv = gl_FragCoord.xy / iResolution.xy;

	// first texture row is frequency data
	float fft  = texture2D( iChannel0, vec2(uv.x,0.75) ).x; 
	
    // second texture row is the sound wave
	float wave = texture2D( iChannel0, vec2(uv.x,0.25) ).x;
	
	// convert frequency to colors
	vec3 col = vec3( fft, 4.0*fft*(1.0-fft), 1.0-fft ) * fft;

    // add wave form on top	
	col += 1.0 -  smoothstep( 0.0, 0.15, abs(wave - uv.y) );
	
	// output final color
	gl_FragColor = vec4(col,1.0);
}
