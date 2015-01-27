uniform vec3      iResolution;
uniform sampler2D iChannel0;

// https://software.intel.com/en-us/blogs/2014/07/15/an-investigation-of-fast-real-time-gpu-based-image-blur-algorithms
// automatically generated by GenerateGaussFunctionCode in GaussianBlur.h                                                                                            
vec3 GaussianBlur( sampler2D tex0, vec2 centreUV, vec2 pixelOffset )                                                                           
{                                                                                                                                                                    
    vec3 colOut = vec3( 0, 0, 0 );                                                                                                                                   
                                                                                                                                                                     
    ////////////////////////////////////////////////;
    // Kernel width 7 x 7
    //
 	vec2 gWeights = vec2( 0.44908, 0.05092 );
	vec2 gOffsets = vec2( 0.53805, 2.06278 );
    ////////////////////////////////////////////////;

	vec2 texCoordOffset;
	vec3 col;
	
	texCoordOffset = gOffsets.x * pixelOffset;                                                                                                           
	col = texture2D( tex0, centreUV + texCoordOffset ).rgb;
 	col += texture2D( tex0, centreUV - texCoordOffset ).rgb;
	colOut += gWeights.x * col;
	
	texCoordOffset = gOffsets.y * pixelOffset;                                                                                                           
	col = texture2D( tex0, centreUV + texCoordOffset ).rgb;
	col += texture2D( tex0, centreUV - texCoordOffset ).rgb;
	colOut += gWeights.y * col;		

    return colOut;
}

vec3 BoxBlur( sampler2D tex, vec2 uv, vec2 ofs )
{
	return (1.0/25.0)*(
	texture2D(tex,uv+vec2(-2.0,-2.0)*ofs).rgb +
	texture2D(tex,uv+vec2(-1.0,-2.0)*ofs).rgb +
	texture2D(tex,uv+vec2( 0.0,-2.0)*ofs).rgb +
	texture2D(tex,uv+vec2(+1.0,-2.0)*ofs).rgb +
	texture2D(tex,uv+vec2(+2.0,-2.0)*ofs).rgb +

	texture2D(tex,uv+vec2(-2.0,-1.0)*ofs).rgb +
	texture2D(tex,uv+vec2(-1.0,-1.0)*ofs).rgb +
	texture2D(tex,uv+vec2( 0.0,-1.0)*ofs).rgb +
	texture2D(tex,uv+vec2(+1.0,-1.0)*ofs).rgb +
	texture2D(tex,uv+vec2(+2.0,-1.0)*ofs).rgb +

	texture2D(tex,uv+vec2(-2.0, 0.0)*ofs).rgb +
	texture2D(tex,uv+vec2(-1.0, 0.0)*ofs).rgb +
	texture2D(tex,uv+vec2( 0.0, 0.0)*ofs).rgb +
	texture2D(tex,uv+vec2(+1.0, 0.0)*ofs).rgb +
	texture2D(tex,uv+vec2(+2.0, 0.0)*ofs).rgb +

	texture2D(tex,uv+vec2(-2.0, 1.0)*ofs).rgb +
	texture2D(tex,uv+vec2(-1.0, 1.0)*ofs).rgb +
	texture2D(tex,uv+vec2( 0.0, 1.0)*ofs).rgb +
	texture2D(tex,uv+vec2(+1.0, 1.0)*ofs).rgb +
	texture2D(tex,uv+vec2(+2.0, 1.0)*ofs).rgb +

	texture2D(tex,uv+vec2(-2.0,+2.0)*ofs).rgb +
	texture2D(tex,uv+vec2(-1.0,+2.0)*ofs).rgb +
	texture2D(tex,uv+vec2( 0.0,+2.0)*ofs).rgb +
	texture2D(tex,uv+vec2(+1.0,+2.0)*ofs).rgb +
	texture2D(tex,uv+vec2(+2.0,+2.0)*ofs).rgb );
}

void main()
{
	vec2 uv = (gl_FragCoord.xy+vec2(0.5))/iResolution.xy; // Sample at pixel center	
	gl_FragColor = vec4( BoxBlur( iChannel0, uv, vec2(10.0)/iResolution.xy ), 1.0 );
//	gl_FragColor = vec4( GaussianBlur( iChannel0, uv, vec2(7.0)/iResolution.xy ), 1.0 );
}
