#version 120
uniform sampler2D iPos;
uniform sampler2D iVel;
uniform sampler2D iForce;
//varying vec2 vTexCoord;
const float dt = 0.001;

vec4 getForce( vec4 pos )
{
	vec2 tc = 0.5*(pos.xy+vec2(1.0,1.0));
	return texture2D( iForce, tc );
}

void main(void)
{
	vec2 tc = gl_TexCoord[0].xy;
//	vec2 tc = vTexCoord;
//	vec2 tc = gl_FragCoord.xy / vec2(256.0,256.0);
	
	vec4 pos   = texture2D( iPos,   tc );
	vec4 vel   = texture2D( iVel,   tc );
	
	vec4 force = getForce( pos );
	
	vel.xyz = force.xyz;
	
	pos.xyz -= dt * vel.xyz; //vec3( -1.0, -1.0, 0.0 );
	pos.x = clamp( pos.x, -1.0, 1.0 );
	pos.y = clamp( pos.y, -1.0, 1.0 );
	pos.z = clamp( pos.z, -1.0, 1.0 );
   
	// Pass through
	gl_FragData[0] = vec4( pos.xyz, 1.0 );
	gl_FragData[1] = vec4( vel.xyz, 1.0 );	
}
