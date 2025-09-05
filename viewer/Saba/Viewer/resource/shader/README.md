# MMD 著色器版本說明

## 🎯 可用的著色器版本

### ✅ **推薦使用**

#### `mmd.frag` + `mmd.vert` (當前使用)
- **效能**: 最佳 (~39 fps)
- **類型**: 優化版本，平衡效能與相容性
- **特點**: 
  - `precision mediump float` 提升視覺品質
  - 合併紋理處理函數減少開銷
  - 智慧分支保留，避免不必要運算
  - 早期 alpha 測試優化

### 🔄 **替代版本**

#### `mmd_original.frag` 
- **效能**: 基準版本
- **類型**: 原始版本，最高相容性
- **特點**:
  - `precision lowp float` 
  - 傳統分支結構
  - 分離的紋理處理函數
  - 適合低端硬體

#### `mmd_branchless.frag`
- **效能**: 良好 (~36 fps)  
- **類型**: 無分支版本，適合特定場景
- **特點**:
  - 完全移除條件分支
  - 使用 `step()` 和 `mix()` 替代
  - 適合高端 GPU 或 VR/AR 應用

#### `mmd_optimized.frag`
- **效能**: 最佳 (~39 fps)
- **類型**: 備份的最佳效能版本
- **特點**: 與當前 `mmd.frag` 相同內容

## 🚀 使用方式

### 切換到不同版本
```bash
# 使用原始版本
cp mmd_original.frag mmd.frag

# 使用無分支版本  
cp mmd_branchless.frag mmd.frag

# 恢復最佳效能版本
cp mmd_optimized.frag mmd.frag
```

### 重新編譯
```bash
cd /path/to/saba/build
cmake --build . --config Debug --target simple_mmd_viewer
```

## 📊 效能比較

| 版本 | FPS | 特點 | 適用場景 |
|------|-----|------|----------|
| **優化版本** | ~39 | 最佳平衡 | **推薦使用** |
| 無分支版本 | ~36 | 一致時間 | 高端GPU/VR |
| 原始版本 | 基準 | 最高相容性 | 低端硬體 |

## 🎉 優化成果

相比原始版本，優化版本實現了：
- ✅ 更好的效能表現
- ✅ 提升的視覺品質 (mediump精度)
- ✅ 現代化的著色器代碼
- ✅ 完全的向下相容性
