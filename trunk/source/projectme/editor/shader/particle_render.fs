#version 120

uniform bool doSprite;
uniform sampler2D sprite;

in vec2 lifetime;

void main(void)
{  	
	vec2 uv = gl_FragCoord.xy / vec2(1024.0); // FIXME: Hardcoded texture size!
	
	// Point / sprite color
	vec4 base = gl_Color * vec4((vec3(.23),1.0));
	vec4 color;	
	if( doSprite )
		color = base * texture2D( sprite, gl_PointCoord );
	else	
		color = gl_Color;
	
	// Blend in/out
  #if 1
	float blendDuration = 0.23; 
	float blendIn  = smoothstep( 0.0, blendDuration, lifetime.x );
	float blendOut = smoothstep( 0.0, blendDuration, lifetime.y );	
	color.a = min(color.a, blendIn*blendOut);
  #endif	

  #if 1
	// Smooth box border blend out
	vec2 dir = abs(2.0*uv - vec2(1.0,1.0));
	//float r = max(dir.x,dir.y); // box blend
	float r = length(2.0*uv - vec2(1.0,1.0)); // radial blend
	float border = 1.0 - smoothstep( 0.7, 1.0, r );
  #else
	float border = 1.0;
  #endif
	color.a = min(color.a, border);
	
	color.a *= 3.0;
	
	gl_FragColor = color;
}
