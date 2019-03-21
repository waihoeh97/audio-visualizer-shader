precision mediump float;
varying vec4 fColor;
varying vec2 fTexCoord;

uniform sampler2D sampler2d;
uniform float Wave1;
uniform float Wave2;
uniform float Wave3;
uniform float Wave4;
uniform float Wave5;
uniform float Wave6;
uniform float Wave7;

float invLerp(float v0, float v1, float v)
{
	return (v - v0) / (v1 - v0);
}

void main()
{
	vec3 color;
	vec2 resolution = vec2 (800.0, 800.0);
	
	vec2 position = (gl_FragCoord.xy / resolution.xy );
	float colorR;
	float colorG;
	float colorB;
	float alpha;
	
	vec2 p = vec2(floor(gl_FragCoord.x), floor(gl_FragCoord.y));
	
	if ((mod(p.y, 2.0) == 0.0) && position.y <= 0.9) 
	{
		colorR = position.y * 0.9;
		colorG = (position.x / position.y)-(position.x * position.y) + sin(Wave1 * 10.0) * 0.5;
		colorB = position.x * 0.9;
		alpha = 1.0;
    } 
	else 
	{
		colorR = 0.5;
		colorG = 0.1;
		colorB = 0.8;
		alpha = 0.0;
	}
	
	
	// Flower 1 & 2
	vec2 st = gl_FragCoord.xy/150.0;
    vec2 pos = vec2(0.5, 3.5)-st;
	vec2 pos1 = vec2(4.9, 3.5)-st;

    float r = length(pos)*2.0;
    float a = atan(pos.y,pos.x);
	
	float r1 = length(pos1)*2.0;
    float a1 = atan(pos1.y,pos1.x);

    float f = cos(a*3.0);
    f = abs(cos(a*2.5+Wave3))*0.5+0.3;
	
	float f1 = cos(a1*3.0);
    f1 = abs(cos(a1*2.5+Wave5))*0.5+0.3;

    color = vec3(1.0-smoothstep(f,f+0.02 + Wave1,r)) + vec3(1.0-smoothstep(f1,f1+0.02 + Wave1,r1));
	
	
	
	// Bottom Wave
	float offsetX = Wave2 * 0.1;
	float offsetY = 50.0 + Wave1 * 100.0;
	float frequency = 60.0;
	float amplitude = 30.0 + Wave1 * 3.0;
	float width = 10.0;
	float speed = 10.0;
	
	float wave = amplitude * sin ((gl_FragCoord.x / frequency) - (offsetX * speed)) + offsetY + width;
	
	// Circle
	float rInner0 = 90.0 + Wave1 * 20.0 - 5.0 * sin(Wave2 * 2.0);
	float rOuter0 = 95.0 + Wave1 * 25.0 - 5.0 * sin(Wave2 * 2.0);
	float circleCoord0X = 400.0 - 1.0 * 10.0;
	float circleCoord0Y = 350.0 + 14.5 + 1.0 * 5.0;
	
	float ax0 = pow((gl_FragCoord.x - circleCoord0X), 2.0);
	float ay0 = pow((gl_FragCoord.y - circleCoord0Y), 2.0);
	
	float a0 = ax0 + ay0;
	float b0 = rInner0 * rInner0;
	float c0 = rOuter0 * rOuter0;
	
	
	if(gl_FragCoord.y >= 0.0 && gl_FragCoord.y <= wave)
	{
		color.r = 0.0;
		color.g = 0.5;
		color.b = 0.5;
	}	
	if(a0 >= b0 && a0 <= c0)
	{
		color.r = 1.0 - Wave1;
		color.g = 1.0 - Wave1;
		color.b = 0.0;
	}
	
	gl_FragColor = vec4(vec3(color), 1.0) + vec4(colorR,colorG,colorB,alpha) + texture2D(sampler2d, fTexCoord);
}







