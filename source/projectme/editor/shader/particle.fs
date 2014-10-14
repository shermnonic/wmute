#version 120
uniform sampler2D iPos;
uniform sampler2D iVel;
varying vec2 vTexCoord;
varying vec4 vColor;
varying vec4 vPos;

void main(void)
{
//	vec2 tc = vColor.xy;
	vec2 tc = vTexCoord;
//	vec2 tc = gl_TexCoord[0].xy;
//	vec2 tc = 0.5*(vPos.xy + vec2(1.0,1.0));
//	vec2 tc = gl_FragCoord.xy / vec2(256.0,256.0);
	
   vec4 pos = texture2D( iPos, tc );
//   vec4 vel = texture2D( iVel, tc );
	
	vec4 debugColor = 
		vec4( tc.x, tc.y, 1.0, 1.0 ); 
		//vec4( 0.0, 1.0, 0.0, 0.0 ); 
		//vec4( tc.x, tc.y, 0.0, 1.0 ); 
   gl_FragData[0] = debugColor; //vec4( pos.xyz, 1.0 );
   gl_FragData[1] = debugColor; //vec4( vel.xyz, 1.0 );	
}