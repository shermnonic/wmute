#version 120

uniform bool doSprite;
uniform sampler2D sprite;

in vec2 lifetime;

void main(void)
{  	
	// Point / sprite color
	vec4 color;	
	if( doSprite )
		color = (vec4(.23) + gl_Color) * texture2D( sprite, gl_PointCoord );
	else	
		color = gl_Color;
	
	// Blend in/out
	float blendDuration = 0.23; 
	float blendIn  = smoothstep( 0.0, blendDuration, lifetime.x );
	float blendOut = smoothstep( 0.0, blendDuration, lifetime.y );	
	color.a *= blendIn*blendOut;
	
	vec2 uv = gl_FragCoord.xy / vec2(1024.0);
  #if 1
	// Smooth box border blend out
	vec2 dir = abs(2.0*uv - vec2(1.0,1.0));
	float r = max(dir.x,dir.y); // for radial blend use: r = length(2.0*uv - vec2(1.0,1.0));
	float border = 1.0 - smoothstep( 0.7, 1.0, r );
  #else
	float border = 1.0;
  #endif
	color.a *= border;
	
	gl_FragColor = color;
}
