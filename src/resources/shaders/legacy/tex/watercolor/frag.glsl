// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2019 The MMapper Authors

//uniform sampler2D uTexture;
//uniform vec4 uColor;

uniform float fTime;
//uniform vec4 fResolution;

varying vec2 vTexCoord;

vec2 psuedoRand(vec2 p) {
	vec3 a = fract(p.xyy * vec3(123.34, 234.45, 345.56));
    a += dot(a, a + 67.78);
    return fract(vec2(a.x * a.y, a.y * a.z));
}

void main()
{
    // Normalized pixel coordinates (from -1 to 1)
    vec2 uv = (2.0 * gl_FragCoord.xy - vec2(1000, 1000))/vec2(1000, 1000);
    float circlePoints = 0.0;
    float minDist = 100.0;
    
    for(float i = 1.0; i < 1500.0; ++i){
    	vec2 randNum = psuedoRand(vec2(i));
        vec2 position = sin(randNum * (fTime + 10.0) * 0.4);
        float dist = length(uv - position);
        circlePoints += 1.0 - smoothstep(0.0015, 0.14, dist);
    }
    
	gl_FragColor = vec4(0.9 / vec3(circlePoints + 55.0, circlePoints + 2.5, 1.0 + vTexCoord.x * 0.0000001), 1.0);
}