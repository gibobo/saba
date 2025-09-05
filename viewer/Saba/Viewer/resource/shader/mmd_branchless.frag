#version 100
precision mediump float;

varying vec3 vs_Pos;
varying vec3 vs_Nor;
varying vec2 vs_UV;

uniform float u_Alpha;
uniform vec3 u_Diffuse;
uniform vec3 u_Ambient;
uniform vec3 u_Specular;
uniform float u_SpecularPower;
uniform vec3 u_LightColor;
uniform vec3 u_LightDir;

uniform int u_TexMode;
uniform sampler2D u_Tex;
uniform vec4 u_TexMulFactor;
uniform vec4 u_TexAddFactor;

uniform int u_ToonTexMode;
uniform sampler2D u_ToonTex;
uniform vec4 u_ToonTexMulFactor;
uniform vec4 u_ToonTexAddFactor;

uniform int u_SphereTexMode;
uniform sampler2D u_SphereTex;
uniform vec4 u_SphereTexMulFactor;
uniform vec4 u_SphereTexAddFactor;

// 優化的紋理混合函數（無分支版本）
vec3 ApplyTextureFactor(vec3 texColor, vec4 mulFactor, vec4 addFactor)
{
    vec3 mulResult = mix(vec3(1.0), texColor * mulFactor.rgb, mulFactor.a);
    vec3 addResult = clamp(texColor + (texColor - vec3(1.0)) * addFactor.a, vec3(0.0), vec3(1.0)) + addFactor.rgb;
    return addResult * mulResult;
}

void main()
{
    // 在片段著色器中計算正規化向量（與原始版本相容）
    vec3 eyeDir = normalize(vs_Pos);
    vec3 lightDir = normalize(-u_LightDir);
    vec3 nor = normalize(vs_Nor);
    
    float ln = clamp(dot(nor, lightDir) + 0.5, 0.0, 1.0);
    
    // 基礎顏色計算
    vec3 color = clamp((u_Diffuse * u_LightColor) + u_Ambient, 0.0, 1.0);
    float alpha = u_Alpha;
    
    // 主紋理 - 完全無分支處理
    float texModeActive = step(0.5, float(u_TexMode));
    vec4 texColor = texture2D(u_Tex, vs_UV);
    texColor.rgb = ApplyTextureFactor(texColor.rgb, u_TexMulFactor, u_TexAddFactor);
    
    color = mix(color, color * texColor.rgb, texModeActive);
    
    // Alpha 模式處理（紋理模式 2 使用紋理 alpha）
    float alphaMode = step(1.5, float(u_TexMode)) * texModeActive;
    alpha = mix(alpha, alpha * texColor.a, alphaMode);
    
    // 早期 alpha 測試
    if (alpha < 0.001) discard;
    
    // 球面紋理 - 無分支處理
    float sphereModeActive = step(0.5, float(u_SphereTexMode));
    vec2 spUV = vec2(nor.x * 0.5 + 0.5, 1.0 - (nor.y * 0.5 + 0.5));
    vec3 spColor = texture2D(u_SphereTex, spUV).rgb;
    spColor = ApplyTextureFactor(spColor, u_SphereTexMulFactor, u_SphereTexAddFactor);
    
    // 球面紋理模式：1=乘法，2=加法
    float sphereMulMode = step(0.5, float(u_SphereTexMode)) * (1.0 - step(1.5, float(u_SphereTexMode)));
    float sphereAddMode = step(1.5, float(u_SphereTexMode));
    
    color = mix(color, color * spColor, sphereMulMode);
    color = mix(color, color + spColor, sphereAddMode);
    
    // 卡通紋理 - 無分支處理
    float toonModeActive = step(0.5, float(u_ToonTexMode));
    vec3 toonColor = texture2D(u_ToonTex, vec2(0.0, ln)).rgb;
    toonColor = ApplyTextureFactor(toonColor, u_ToonTexMulFactor, u_ToonTexAddFactor);
    color = mix(color, color * toonColor, toonModeActive);
    
    // 鏡面反射 - 無分支處理
    float specActive = step(0.001, u_SpecularPower);
    vec3 halfVec = normalize(eyeDir + lightDir);
    float specPower = pow(max(0.0, dot(halfVec, nor)), max(u_SpecularPower, 1.0));
    vec3 specularContrib = specPower * u_Specular * u_LightColor * specActive;
    color += specularContrib;
    
    gl_FragColor = vec4(color, alpha);
}
