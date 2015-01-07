#version 120

uniform bool doSprite;
uniform sampler2D sprite;

void main(void)
{  
	if( doSprite )
		gl_FragColor = (vec4(.23) + gl_Color) * texture2D( sprite, gl_PointCoord );
	else	
		gl_FragColor = gl_Color;
}
