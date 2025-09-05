#pragma once

#include <vector>
#include <memory>
#include <string>
#include "AppContext.h"
#include "Material.h"
#include <glm/glm.hpp>

// 包圍盒結構
struct BoundingBox
{
	glm::vec3 min;
	glm::vec3 max;
	
	BoundingBox() : min(FLT_MAX), max(-FLT_MAX) {}
	BoundingBox(const glm::vec3& minPos, const glm::vec3& maxPos) : min(minPos), max(maxPos) {}
	
	// 檢查是否在視錐內
	bool IsInFrustum(const glm::mat4& mvpMatrix) const;
	
	// 更新包圍盒
	void UpdateBounds(const glm::vec3& point);
	void Reset() { min = glm::vec3(FLT_MAX); max = glm::vec3(-FLT_MAX); }
	
	// 獲取中心點和尺寸
	glm::vec3 GetCenter() const { return (min + max) * 0.5f; }
	glm::vec3 GetSize() const { return max - min; }
};

// 遮擋查詢結果
struct OcclusionQuery
{
	GLuint queryId = 0;
	bool isOccluded = false;
	bool queryInProgress = false;
	int framesSinceVisible = 0;
	
	void BeginQuery();
	void EndQuery();
	bool CheckResult(); // 非阻塞檢查
};

struct Model
{
	std::shared_ptr<saba::MMDModel> m_mmdModel;
	std::unique_ptr<saba::VMDAnimation> m_vmdAnim;

	GLuint m_posVBO = 0;
	GLuint m_norVBO = 0;
	GLuint m_uvVBO = 0;
	GLuint m_ibo = 0;
	GLenum m_indexType;

	GLuint m_mmdVAO = 0;
	GLuint m_mmdEdgeVAO = 0;
	GLuint m_mmdGroundShadowVAO = 0;

	std::vector<Material> m_materials;
	
	// 遮擋剔除相關
	BoundingBox m_boundingBox;
	OcclusionQuery m_occlusionQuery;
	bool m_enableOcclusionCulling = true;
	bool m_shouldRender = true;
	
	// LOD包圍盒（如果子網格很多，可以分組）
	std::vector<BoundingBox> m_subMeshBounds;

	bool Setup(AppContext &appContext);
	void Clear();

	void UpdateAnimation(const AppContext &appContext);
	void Update(const AppContext &appContext);
	void Draw(const AppContext &appContext);
	
	// 遮擋剔除相關方法
	void UpdateBoundingBox();
	bool PerformFrustumCulling(const AppContext &appContext);
	void PerformOcclusionCulling(const AppContext &appContext);
	void DrawBoundingBox(const AppContext &appContext); // 包圍盒渲染（用於遮擋查詢）
};
