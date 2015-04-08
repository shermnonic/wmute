#version 120
uniform sampler2D iPos;
uniform sampler2D iVel;
uniform sampler2D iForce;
uniform sampler2D iBirthPos;
uniform sampler2D iBirthVel;
//varying vec2 vTexCoord;
const float dt = 0.0055;

vec2 getTexCoord()
{
	return gl_TexCoord[0].xy;
//	vec2 tc = vTexCoord;
//	vec2 tc = gl_FragCoord.xy / vec2(256.0,256.0);
}

vec4 getForce( vec4 pos )
{
	vec2 tc = 0.5*(pos.xy+vec2(1.0,1.0));
	vec4 force = texture2D( iForce, tc );
	
	//float mag = length(force);
	//force = mag*mag*normalize(force);
	
	return -force;
}

void main(void)
{
	// Get particle data
	vec2 tc = getTexCoord();	
	vec4 pos = texture2D( iPos, tc );
	vec4 vel = vec4( 0.0,0.0,0.0,1.0 );
#if 1
	// Update lifetime
	pos.w -= dt;
  #if 1
	if( pos.w < 0.0 )
	{
		// Re-incarnate
		pos = texture2D( iBirthPos, tc );
		vel = vec4(0.0,0.0,0.0,1.0); //texture2D( iBirthVel, tc );
	}
	else
  #endif
	{	
		// Advect
		vel = texture2D( iVel, tc );
		
		// Forces
		vec4 force = getForce( pos );
		vec3 grav = vec3(1.0,0.0,0.0);
		vec3 rot  = vec3( -pos.y, pos.x, 0.0 );
		
		// Euler step
		vel.xyz = dt * ( force.xyz*700.0 + rot*0.0 + grav*150.0 );
		//vel.xyz = force.xyz; // Alternatively define velocity directly from input
		pos.xyz += dt * vel.xyz;
		
		// Handle borders
	  #if 1
		bool borderHit = false;
		if( pos.x > 1.0 || pos.x < -1.0 ) { vel.x *= -1.0; borderHit=true; }
		if( pos.y > 1.0 || pos.y < -1.0 ) { vel.y *= -1.0; borderHit=true; }
		if( pos.z > 1.0 || pos.z < -1.0 ) { vel.z *= -1.0; borderHit=true; }
		if( borderHit )
		{
		  #if 0// Reflect
			pos.xyz += dt * vel.xyz;
	      #else // Die
			pos.w = -1.0;
			pos.xyz = vec3(-2.0,-2.0,0.0);
		  #endif
		}
	  #else // Clamp
		pos.x = clamp( pos.x, -1.0, 1.0 );
		pos.y = clamp( pos.y, -1.0, 1.0 );
		pos.z = vel.z; //clamp( pos.z, -1.0, 1.0 );		
	  #endif
	
		// WORKAROUND
		//vec3 vel0 = texture2D( iVel, tc ).xyz;
		//pos.z = vel0.z;
		//vel.z = vel0.z;		
	}
#endif	
	// Pass through
	gl_FragData[0] = pos;
	gl_FragData[1] = vel;	
}
