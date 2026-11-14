uniform sampler2D u_DiffuseMap;
uniform int       u_AlphaTest;

#if defined(USE_LIGHTMAP)
uniform sampler2D u_LightMap;

uniform int       u_Texture1Env;
#endif

varying vec2      var_DiffuseTex;

#if defined(USE_LIGHTMAP)
varying vec2      var_LightTex;
#endif

varying vec4      var_Color;


void main()
{
	vec4 color  = texture2D(u_DiffuseMap, var_DiffuseTex);
#if defined(USE_LIGHTMAP)
	vec4 color2 = texture2D(u_LightMap, var_LightTex);
  #if defined(RGBM_LIGHTMAP)
	color2.rgb *= color2.a;
	color2.a = 1.0;
  #endif

	if (u_Texture1Env == TEXENV_MODULATE)
	{
		color *= color2;
	}
	else if (u_Texture1Env == TEXENV_ADD)
	{
		color += color2;
	}
	else if (u_Texture1Env == TEXENV_REPLACE)
	{
		color = color2;
	}
	
	//color = color * (u_Texture1Env.xxxx + color2 * u_Texture1Env.z) + color2 * u_Texture1Env.y;
#endif

	float alpha = color.a * var_Color.a;
	if ((u_AlphaTest == 1 && alpha == 0.0) ||
	    (u_AlphaTest == 2 && alpha >= 0.5) ||
	    (u_AlphaTest == 3 && alpha < 0.5))
		discard;
	gl_FragColor = vec4(color.rgb * var_Color.rgb, alpha);
}
