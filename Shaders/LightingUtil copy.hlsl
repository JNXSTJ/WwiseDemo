//***************************************************************************************
// LightingUtil.hlsl by Frank Luna (C) 2015 All Rights Reserved.
//
// Contains API for shader lighting.
//***************************************************************************************

in float3 worldPosition;
in float3 worldNormal;
in float2 texcoord;

struct AreaLight
{
    float intensity;
    float3 color;
    float3 points[4];
    bool twoSided;
};

struct Material
{
    sampler2D diffuse;
    vec4 albedoRoughness; // (x,y,z) = color, w = roughness
};
Material material;

float3 viewPosition;
Texture2D LTC1; // for inverse M
Texture2D LTC2; // GGX norm, fresnel, 0(unused), sphere

const float LUT_SIZE  = 64.0; // ltc_texture size
const float LUT_SCALE = (LUT_SIZE - 1.0)/LUT_SIZE;
const float LUT_BIAS  = 0.5/LUT_SIZE;


// Vector form without project to the plane (dot with the normal)
// Use for proxy sphere clipping
float3 IntegrateEdgeVec(float3 v1, float3 v2)
{
    // Using built-in acos() function will result flaws
    // Using fitting result for calculating acos()
    float x = dot(v1, v2);
    float y = abs(x);

    float a = 0.8543985 + (0.4965155 + 0.0145206*y)*y;
    float b = 3.4175940 + (4.1616724 + y)*y;
    float v = a / b;

    float theta_sintheta = (x > 0.0) ? v : 0.5*inversesqrt(max(1.0 - x*x, 1e-7)) - v;

    return cross(v1, v2)*theta_sintheta;
}

float IntegrateEdge(float3 v1, float3 v2)
{
    return IntegrateEdgeVec(v1, v2).z;
}

// P is fragPos in world space (LTC distribution)
float3 LTC_Evaluate(float3 N, float3 V, float3 P, float3x3 Minv, float3 points[4], bool twoSided)
{
    // construct orthonormal basis around N
    float3 T1, T2;
    T1 = normalize(V - N * dot(V, N));
    T2 = cross(N, T1);

    // rotate area light in (T1, T2, N) basis
    Minv = Minv * transpose(float3x3(T1, T2, N));

    // polygon (allocate 4 vertices for clipping)
    float3 L[4];
    // transform polygon from LTC back to origin Do (cosine weighted)
    L[0] = Minv * (points[0] - P);
    L[1] = Minv * (points[1] - P);
    L[2] = Minv * (points[2] - P);
    L[3] = Minv * (points[3] - P);

    // use tabulated horizon-clipped sphere
    // check if the shading point is behind the light
    float3 dir = points[0] - P; // LTC space
    float3 lightNormal = cross(points[1] - points[0], points[3] - points[0]);
    bool behind = (dot(dir, lightNormal) < 0.0);

    // cos weighted space
    L[0] = normalize(L[0]);
    L[1] = normalize(L[1]);
    L[2] = normalize(L[2]);
    L[3] = normalize(L[3]);

    // integrate
    float3 vsum = float3(0.0);
    vsum += IntegrateEdgeVec(L[0], L[1]);
    vsum += IntegrateEdgeVec(L[1], L[2]);
    vsum += IntegrateEdgeVec(L[2], L[3]);
    vsum += IntegrateEdgeVec(L[3], L[0]);

    // form factor of the polygon in direction vsum
    float len = length(vsum);

    float z = vsum.z/len;
    if (behind)
        z = -z;

    vec2 uv = vec2(z*0.5f + 0.5f, len); // range [0, 1]
    uv = uv*LUT_SCALE + LUT_BIAS;

    // Fetch the form factor for horizon clipping
    float scale = texture(LTC2, uv).w;

    float sum = len*scale;
    if (!behind && !twoSided)
        sum = 0.0;

    // Outgoing radiance (solid angle) for the entire polygon
    float3 Lo_i = float3(sum, sum, sum);
    return Lo_i;
}

// PBR-maps for roughness (and metallic) are usually stored in non-linear
// color space (sRGB), so we use these functions to convert into linear RGB.
float3 Powfloat3(float3 v, float p)
{
    return float3(pow(v.x, p), pow(v.y, p), pow(v.z, p));
}

const float gamma = 2.2;
float3 ToLinear(float3 v) { return Powfloat3(v, gamma); }
float3 ToSRGB(float3 v)   { return Powfloat3(v, 1.0/gamma); }


void main()
{
    // gamma correction
    float3 mDiffuse = texture(material.diffuse, texcoord).xyz;// * float3(0.7f, 0.8f, 0.96f);
    float3 mSpecular = ToLinear(float3(0.23f, 0.23f, 0.23f)); // mDiffuse

    float3 result = float3(0.0f);

    float3 N = normalize(worldNormal);
    float3 V = normalize(viewPosition - worldPosition);
    float3 P = worldPosition;
    float dotNV = clamp(dot(N, V), 0.0f, 1.0f);

    // use roughness and sqrt(1-cos_theta) to sample M_texture
    vec2 uv = vec2(material.albedoRoughness.w, sqrt(1.0f - dotNV));
    uv = uv*LUT_SCALE + LUT_BIAS;

    // get 4 parameters for inverse_M
    vec4 t1 = texture(LTC1, uv);

    // Get 2 parameters for Fresnel calculation
    vec4 t2 = texture(LTC2, uv);

    float3x3 Minv = float3x3(
        float3(t1.x, 0, t1.y),
        float3(  0,  1,    0),
        float3(t1.z, 0, t1.w)
    );

    // translate light source for testing
    float3 translatedPoints[4];
    translatedPoints[0] = areaLight.points[0] + areaLightTranslate;
    translatedPoints[1] = areaLight.points[1] + areaLightTranslate;
    translatedPoints[2] = areaLight.points[2] + areaLightTranslate;
    translatedPoints[3] = areaLight.points[3] + areaLightTranslate;

    // Evaluate LTC shading
    float3 diffuse = LTC_Evaluate(N, V, P, float3x3(1), translatedPoints, areaLight.twoSided);
    float3 specular = LTC_Evaluate(N, V, P, Minv, translatedPoints, areaLight.twoSided);

    // GGX BRDF shadowing and Fresnel
    // t2.x: shadowedF90 (F90 normally it should be 1.0)
    // t2.y: Smith function for Geometric Attenuation Term, it is dot(V or L, H).
    specular *= mSpecular*t2.x + (1.0f - mSpecular) * t2.y;

    result = areaLight.color * areaLight.intensity * (specular + mDiffuse * diffuse);

    fragColor = vec4(ToSRGB(result), 1.0f);
}
