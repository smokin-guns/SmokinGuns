uniform sampler2D u_DiffuseMap;
uniform int       u_AlphaTest;

varying vec2      var_Tex1;
varying vec4      var_Color;


void main()
{
	vec4 color = texture2D(u_DiffuseMap, var_Tex1);
	float alpha = color.a * var_Color.a;
	if ((u_AlphaTest == 1 && alpha == 0.0) ||
	    (u_AlphaTest == 2 && alpha >= 0.5) ||
	    (u_AlphaTest == 3 && alpha < 0.5))
		discard;

	gl_FragColor = vec4(color.rgb * var_Color.rgb, alpha);
}
