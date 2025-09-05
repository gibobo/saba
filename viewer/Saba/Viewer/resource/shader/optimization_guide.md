# MMD 著色器優化版本說明

## 三個版本的著色器

### 1. 原始版本 (mmd.vert/mmd.frag)
- OpenGL ES 2.0 相容
- precision lowp float
- 多個條件分支
- 函數調用開銷

### 2. 優化版本 (mmd_optimized.vert/mmd_optimized.frag)
**改進：**
- 提升精度到 mediump float
- 頂點著色器預計算正規化向量
- 內聯紋理混合函數
- 早期 alpha 測試
- 部分條件式混合

**預期效能提升：** 5-15%

### 3. 無分支版本 (mmd_branchless.frag)
**改進：**
- 完全移除所有分支判斷
- 使用 step() 和 mix() 函數
- 所有紋理都會被取樣但有條件地混合
- 最適合現代 GPU

**預期效能提升：** 10-25%
**缺點：** 即使不使用的紋理也會被取樣

## 主要優化技術

### 1. 預計算正規化向量
```glsl
// 在頂點著色器中計算，避免在每個像素重複計算
vs_LightDir = normalize(-u_LightDir);
vs_EyeDir = normalize(worldPos.xyz);
```

### 2. 條件式混合替代分支
```glsl
// 原始: if (u_TexMode != 0) { color *= texColor; }
// 優化: 
float texModeActive = step(0.5, float(u_TexMode));
color = mix(color, color * texColor.rgb, texModeActive);
```

### 3. 內聯函數減少調用開銷
```glsl
// 將 ComputeTexMulFactor 和 ComputeTexAddFactor 合併為單一函數
vec3 ApplyTextureFactor(vec3 texColor, vec4 mulFactor, vec4 addFactor)
```

### 4. 精度提升
```glsl
precision mediump float;  // 替代 lowp，提升視覺品質
```

## 使用建議

### 桌面平台：
- 推薦使用 **mmd_branchless.frag**
- GPU 有足夠的並行處理能力
- 紋理快取效果好

### 移動平台：
- 推薦使用 **mmd_optimized.frag**
- 保留必要的分支以節省紋理頻寬
- 在效能和功耗間取得平衡

### 低端設備：
- 保持使用原始的 **mmd.frag**
- lowp precision 節省記憶體頻寬
- 較好的相容性

## 整合方法

1. 複製優化檔案到 shader 目錄
2. 修改 GLSLShaderUtil 以載入不同版本
3. 添加效能測試模式
4. 根據硬體能力自動選擇版本
