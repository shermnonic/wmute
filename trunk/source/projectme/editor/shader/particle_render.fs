#version 120

uniform bool doSprite;
uniform sampler2D sprite;

void main(void)
{  
#if 1
	gl_FragColor = gl_Color; // texture2D( sprite, gl_PointCoord );
#else
	if( doSprite )
		gl_FragColor = texture2D( sprite, gl_PointCoord );
	else	
		gl_FragColor = gl_Color;
#endif
}
