#version 100
precision mediump float;  // 提升精度以獲得更好的視覺品質

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

// 優化的紋理混合函數 - 內聯以減少函數調用開銷
vec3 ApplyTextureFactor(vec3 texColor, vec4 mulFactor, vec4 addFactor)
{
    // 合併乘法和加法因子計算
    vec3 mulResult = texColor * mulFactor.rgb;
    vec3 mulFinal = mix(vec3(1.0), mulResult, mulFactor.a);
    
    vec3 addResult = texColor + (texColor - vec3(1.0)) * addFactor.a;
    addResult = clamp(addResult, vec3(0.0), vec3(1.0)) + addFactor.rgb;
    
    return addResult * mulFinal;  // 組合兩個效果
}

void main()
{
    // 正規化向量計算（在片段著色器中保持相容性）
    vec3 eyeDir = normalize(vs_Pos);
    vec3 lightDir = normalize(-u_LightDir);
    vec3 nor = normalize(vs_Nor);
    
    float ln = dot(nor, lightDir);
    ln = clamp(ln + 0.5, 0.0, 1.0);
    
    // 基礎顏色計算
    vec3 color = (u_Diffuse * u_LightColor) + u_Ambient;
    color = clamp(color, 0.0, 1.0);
    float alpha = u_Alpha;
    
    // 優化的紋理處理 - 使用條件式混合減少分支
    float texModeActive = step(0.5, float(u_TexMode));
    if (texModeActive > 0.0)
    {
        vec4 texColor = texture2D(u_Tex, vs_UV);
        texColor.rgb = ApplyTextureFactor(texColor.rgb, u_TexMulFactor, u_TexAddFactor);
        
        color *= texColor.rgb;
        
        // 處理 alpha 模式
        float alphaMode = step(1.5, float(u_TexMode));
        alpha = mix(alpha, alpha * texColor.a, alphaMode);
    }
    
    // 早期 alpha 測試
    if (alpha < 0.001)
    {
        discard;
    }
    
    // 球面紋理處理
    float sphereModeActive = step(0.5, float(u_SphereTexMode));
    if (sphereModeActive > 0.0)
    {
        vec2 spUV = vec2(nor.x * 0.5 + 0.5, 1.0 - (nor.y * 0.5 + 0.5));
        vec3 spColor = texture2D(u_SphereTex, spUV).rgb;
        spColor = ApplyTextureFactor(spColor, u_SphereTexMulFactor, u_SphereTexAddFactor);
        
        // 使用條件式混合替代分支
        float mulMode = step(0.5, float(u_SphereTexMode)) * (1.0 - step(1.5, float(u_SphereTexMode)));
        float addMode = step(1.5, float(u_SphereTexMode));
        
        color = mix(color, color * spColor, mulMode);
        color = mix(color, color + spColor, addMode);
    }
    
    // 卡通紋理處理
    float toonModeActive = step(0.5, float(u_ToonTexMode));
    if (toonModeActive > 0.0)
    {
        vec3 toonColor = texture2D(u_ToonTex, vec2(0.0, ln)).rgb;
        toonColor = ApplyTextureFactor(toonColor, u_ToonTexMulFactor, u_ToonTexAddFactor);
        color *= toonColor;
    }
    
    // 優化的鏡面反射計算
    float specActive = step(0.001, u_SpecularPower);
    if (specActive > 0.0)
    {
        vec3 halfVec = normalize(eyeDir + lightDir);
        vec3 specularColor = u_Specular * u_LightColor;
        float specPower = pow(max(0.0, dot(halfVec, nor)), u_SpecularPower);
        color += specPower * specularColor;
    }
    
    gl_FragColor = vec4(color, alpha);
}
