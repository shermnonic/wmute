// For very old drivers the following line has to be commented out:
#extension GL_ARB_texture_rectangle : enable

uniform sampler2DRect FlakePositions;
uniform sampler3D WindSpeed;

void main()
{
  // Your Task: Write a shader to update the positions of the snowflakes.
  // Inputs are:
  //   pos  : the current position of the snowflake
  vec3 pos = texture2DRect(FlakePositions, gl_TexCoord[0].xy).xyz;
  //   wind : local wind speed at the current position of the snowflake
  vec3 windCoord = pos;
  windCoord.x += 50.0;
  windCoord.y += 50.0;
  windCoord /= 100.0;
  vec3 wind = texture3D(WindSpeed, windCoord.xyz).xyz;
  
  // additionally the falling velocity is given:
  vec3 fallingVelocity = vec3(0.0,0.0,-0.1);

  // the snowflakes should of course should stay on the z=0 plane

  //  <<<SOLUTION START>>>
  pos += wind + fallingVelocity;
  if( pos.z < 0.0 )
	  pos.z = 0.0;
  //  <<<SOLUTION END>>>
  
  // return the updated position as color (rgb):
  gl_FragColor = vec4(pos,1);	
}

