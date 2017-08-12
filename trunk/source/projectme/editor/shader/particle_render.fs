#version 130

uniform bool doSprite;
uniform sampler2D sprite;
uniform vec3 targetSize;

in vec2 lifetime;

float blendDuration = 0.23; //###

void main(void)
{  	
	vec2 uv = gl_FragCoord.xy / max(targetSize.xy,vec2(1.0,1.0));
	
	// Point / sprite color
	vec4 base = gl_Color * vec4((vec3(.23),1.0));
	vec4 color;	
	if( doSprite )
		color = base * texture2D( sprite, gl_PointCoord );
	else	
		color = gl_Color;
	
	// Blend in/out
	if( blendDuration > 0.0 )
	{
		float blendIn  = smoothstep( 0.0, blendDuration, lifetime.x );
		float blendOut = smoothstep( 0.0, blendDuration, lifetime.y );	
		color.a = min(color.a, blendIn*blendOut);
	}

  #if 0
	// Smooth box border blend out
	vec2 dir = abs(2.0*uv - vec2(1.0,1.0));
	float r = max(dir.x,dir.y); // box blend
	//float r = length(2.0*uv - vec2(1.0,1.0)); // radial blend
	float border = 1.0 - smoothstep( 0.7, 1.0, r );
  #else
	float border = 1.0;
  #endif
	color.a = min(color.a, border);
	
	color.a *= 3.0;
	
	gl_FragColor = vec4(0.3)+color;
}
