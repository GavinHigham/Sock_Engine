-- noise.GL33 --

//
// Description : Array and textureless GLSL 2D/3D/4D simplex 
//               noise functions.
//      Author : Ian McEwan, Ashima Arts.
//  Maintainer : stegu
//     Lastmod : 20110822 (ijm)
//     License : Copyright (C) 2011 Ashima Arts. All rights reserved.
//               Distributed under the MIT License. See LICENSE file.
//               https://github.com/ashima/webgl-noise
//               https://github.com/stegu/webgl-noise
// 

vec3 mod289(vec3 x) {
	return x - floor(x * (1.0 / 289.0)) * 289.0;
}

vec4 mod289(vec4 x) {
	return x - floor(x * (1.0 / 289.0)) * 289.0;
}

vec4 permute(vec4 x) {
		 return mod289(((x*34.0)+1.0)*x);
}

vec4 taylorInvSqrt(vec4 r)
{
	return 1.79284291400159 - 0.85373472095314 * r;
}

float snoise(vec3 v)
{ 
	const vec2 C = vec2(1.0/6.0, 1.0/3.0);
	const vec4 D = vec4(0.0, 0.5, 1.0, 2.0);

// First corner
	vec3 i  = floor(v + dot(v, C.yyy));
	vec3 x0 =   v - i + dot(i, C.xxx);

// Other corners
	vec3 g = step(x0.yzx, x0.xyz);
	vec3 l = 1.0 - g;
	vec3 i1 = min( g.xyz, l.zxy);
	vec3 i2 = max( g.xyz, l.zxy);

	//   x0 = x0 - 0.0 + 0.0 * C.xxx;
	//   x1 = x0 - i1  + 1.0 * C.xxx;
	//   x2 = x0 - i2  + 2.0 * C.xxx;
	//   x3 = x0 - 1.0 + 3.0 * C.xxx;
	vec3 x1 = x0 - i1 + C.xxx;
	vec3 x2 = x0 - i2 + C.yyy; // 2.0*C.x = 1/3 = C.y
	vec3 x3 = x0 - D.yyy;      // -1.0+3.0*C.x = -0.5 = -D.y

// Permutations
	i = mod289(i); 
	vec4 p = permute(permute(permute( 
		i.z + vec4(0.0, i1.z, i2.z, 1.0)) +
		i.y + vec4(0.0, i1.y, i2.y, 1.0)) +
		i.x + vec4(0.0, i1.x, i2.x, 1.0));

// Gradients: 7x7 points over a square, mapped onto an octahedron.
// The ring size 17*17 = 289 is close to a multiple of 49 (49*6 = 294)
	float n_ = 0.142857142857; // 1.0/7.0
	vec3  ns = n_ * D.wyz - D.xzx;

	vec4 j = p - 49.0 * floor(p * ns.z * ns.z);  //  mod(p,7*7)

	vec4 x_ = floor(j * ns.z);
	vec4 y_ = floor(j - 7.0 * x_);    // mod(j,N)

	vec4 x = x_ *ns.x + ns.yyyy;
	vec4 y = y_ *ns.x + ns.yyyy;
	vec4 h = 1.0 - abs(x) - abs(y);

	vec4 b0 = vec4( x.xy, y.xy);
	vec4 b1 = vec4( x.zw, y.zw);

	//vec4 s0 = vec4(lessThan(b0,0.0))*2.0 - 1.0;
	//vec4 s1 = vec4(lessThan(b1,0.0))*2.0 - 1.0;
	vec4 s0 = floor(b0)*2.0 + 1.0;
	vec4 s1 = floor(b1)*2.0 + 1.0;
	vec4 sh = -step(h, vec4(0.0));

	vec4 a0 = b0.xzyw + s0.xzyw*sh.xxyy;
	vec4 a1 = b1.xzyw + s1.xzyw*sh.zzww;

	vec3 p0 = vec3(a0.xy,h.x);
	vec3 p1 = vec3(a0.zw,h.y);
	vec3 p2 = vec3(a1.xy,h.z);
	vec3 p3 = vec3(a1.zw,h.w);

//Normalise gradients
	vec4 norm = taylorInvSqrt(vec4(dot(p0,p0), dot(p1,p1), dot(p2, p2), dot(p3,p3)));
	p0 *= norm.x;
	p1 *= norm.y;
	p2 *= norm.z;
	p3 *= norm.w;

// Mix final noise value
	vec4 m = max(0.6 - vec4(dot(x0,x0), dot(x1,x1), dot(x2,x2), dot(x3,x3)), 0.0);
	m = m * m;
	return 42.0 * dot(m*m, vec4(dot(p0,x0), dot(p1,x1), dot(p2,x2), dot(p3,x3)));
}

//Gradient in xyz, value in w
vec4 snoise_grad(vec3 v)
{
	const vec2 C = vec2(1.0/6.0, 1.0/3.0);
	const vec4 D = vec4(0.0, 0.5, 1.0, 2.0);

// First corner
	vec3 i  = floor(v + dot(v, C.yyy));
	vec3 x0 =   v - i + dot(i, C.xxx);

// Other corners
	vec3 g = step(x0.yzx, x0.xyz);
	vec3 l = 1.0 - g;
	vec3 i1 = min( g.xyz, l.zxy);
	vec3 i2 = max( g.xyz, l.zxy);

	//   x0 = x0 - 0.0 + 0.0 * C.xxx;
	//   x1 = x0 - i1  + 1.0 * C.xxx;
	//   x2 = x0 - i2  + 2.0 * C.xxx;
	//   x3 = x0 - 1.0 + 3.0 * C.xxx;
	vec3 x1 = x0 - i1 + C.xxx;
	vec3 x2 = x0 - i2 + C.yyy; // 2.0*C.x = 1/3 = C.y
	vec3 x3 = x0 - D.yyy;      // -1.0+3.0*C.x = -0.5 = -D.y

// Permutations
	i = mod289(i); 
	vec4 p = permute( permute( permute( 
		i.z + vec4(0.0, i1.z, i2.z, 1.0)) + 
		i.y + vec4(0.0, i1.y, i2.y, 1.0)) +
		i.x + vec4(0.0, i1.x, i2.x, 1.0));

// Gradients: 7x7 points over a square, mapped onto an octahedron.
// The ring size 17*17 = 289 is close to a multiple of 49 (49*6 = 294)
	float n_ = 0.142857142857; // 1.0/7.0
	vec3  ns = n_ * D.wyz - D.xzx;

	vec4 j = p - 49.0 * floor(p * ns.z * ns.z);  //  mod(p,7*7)

	vec4 x_ = floor(j * ns.z);
	vec4 y_ = floor(j - 7.0 * x_);    // mod(j,N)

	vec4 x = x_ *ns.x + ns.yyyy;
	vec4 y = y_ *ns.x + ns.yyyy;
	vec4 h = 1.0 - abs(x) - abs(y);

	vec4 b0 = vec4( x.xy, y.xy);
	vec4 b1 = vec4( x.zw, y.zw);

	//vec4 s0 = vec4(lessThan(b0,0.0))*2.0 - 1.0;
	//vec4 s1 = vec4(lessThan(b1,0.0))*2.0 - 1.0;
	vec4 s0 = floor(b0)*2.0 + 1.0;
	vec4 s1 = floor(b1)*2.0 + 1.0;
	vec4 sh = -step(h, vec4(0.0));

	vec4 a0 = b0.xzyw + s0.xzyw*sh.xxyy;
	vec4 a1 = b1.xzyw + s1.xzyw*sh.zzww;

	vec3 p0 = vec3(a0.xy,h.x);
	vec3 p1 = vec3(a0.zw,h.y);
	vec3 p2 = vec3(a1.xy,h.z);
	vec3 p3 = vec3(a1.zw,h.w);

//Normalise gradients
	vec4 norm = taylorInvSqrt(vec4(dot(p0,p0), dot(p1,p1), dot(p2, p2), dot(p3,p3)));
	p0 *= norm.x;
	p1 *= norm.y;
	p2 *= norm.z;
	p3 *= norm.w;

// Mix final noise value
	vec4 m = max(0.6 - vec4(dot(x0,x0), dot(x1,x1), dot(x2,x2), dot(x3,x3)), 0.0);
	vec4 m2 = m * m;
	vec4 m4 = m2 * m2;
	vec4 pdotx = vec4(dot(p0,x0), dot(p1,x1), dot(p2,x2), dot(p3,x3));

// Determine noise gradient
	vec4 temp = m2 * m * pdotx;
	vec4 gradient;
	gradient.xyz = -8.0 * (temp.x * x0 + temp.y * x1 + temp.z * x2 + temp.w * x3);
	gradient.xyz += m4.x * p0 + m4.y * p1 + m4.z * p2 + m4.w * p3;
	gradient.xyz *= 42.0;
	gradient.w = 42.0 * dot(m4, pdotx);

	return gradient;
}

-- utility --

float rand(vec2 co){
		return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
}

-- lighting --

#define M_PI 3.1415926535897932384626433832795

void point_light_phong(in vec3 l, in vec3 v, in vec3 normal, out float specular, out float diffuse)
{
	vec3 h = normalize(l + v); //Halfway vector.

	float base = 1 - dot(v, h);
	float exponential = pow(base, 5.0);
	float F0 = 0.5;
	float fresnel = exponential + F0 * (1.0 - exponential);

	specular = fresnel * pow(max(0.0, dot(h, normal)), 32.0);
	diffuse = max(0.0, dot(l, normal));
}

// set important material values
// float roughnessValue = 0.8; // 0 : smooth, 1: rough
// float F0 = 0.2; // fresnel reflectance at normal incidence
// float k = 0.2; // fraction of diffuse reflection (specular reflection = 1 - k)

//TODO(Gavin): Figure out what this sort of shading is also called.
void point_light_fresnel(in vec3 l, in vec3 v, in vec3 normal, out float specular, out float diffuse)
{
	vec3 h = normalize(l + v); //Halfway vector.
	
	// do the lighting calculation for each fragment.
	float NdotL = max(dot(normal, l), 0.0);
	specular = 0.0;
	if(NdotL > 0.0)
	{
		// calculate intermediary values
		float NdotH = max(dot(normal, h), 0.0); 
		float NdotV = max(dot(normal, v), 0.0); // note: this could also be NdotL, which is the same value
		float VdotH = max(dot(v, h), 0.0);
		float mSquared = roughnessValue * roughnessValue;
		
		// geometric attenuation
		float NH2 = 2.0 * NdotH;
		float g1 = (NH2 * NdotV) / VdotH;
		float g2 = (NH2 * NdotL) / VdotH;
		float geoAtt = min(1.0, min(g1, g2));
	 
		// roughness (or: microfacet distribution function)
		// beckmann distribution function
		float r1 = 1.0 / ( 4.0 * mSquared * pow(NdotH, 4.0));
		float r2 = (NdotH * NdotH - 1.0) / (mSquared * NdotH * NdotH);
		float roughness = r1 * exp(r2);
		
		// fresnel
		// Schlick approximation
		float fresnel = pow(1.0 - VdotH, 5.0);
		fresnel *= (1.0 - F0);
		fresnel += F0;
		
		specular = (fresnel * geoAtt * roughness) / (NdotV * NdotL * M_PI);
	}
	
	specular = max(0.0, NdotL * (k + specular * (1.0 - k)));
	diffuse = max(0.0, dot(l, normal));
}

--