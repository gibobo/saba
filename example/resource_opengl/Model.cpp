#include "Model.h"
#include "GL_ShaderProgram.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <Saba/Base/Time.h>
#include <Saba/Model/MMD/PMDModel.h>
#include <Saba/Model/MMD/PMXModel.h>
#include <Saba/Model/MMD/VMDCameraAnimation.h>
#include <algorithm>
#include <cfloat>

//=============================================================================
// 包圍盒和遮擋剔除實現
//=============================================================================

bool BoundingBox::IsInFrustum(const glm::mat4& mvpMatrix) const
{
    // 檢查包圍盒的8個頂點是否全部在視錐外
    glm::vec3 corners[8] = {
        glm::vec3(min.x, min.y, min.z),
        glm::vec3(max.x, min.y, min.z),
        glm::vec3(min.x, max.y, min.z),
        glm::vec3(max.x, max.y, min.z),
        glm::vec3(min.x, min.y, max.z),
        glm::vec3(max.x, min.y, max.z),
        glm::vec3(min.x, max.y, max.z),
        glm::vec3(max.x, max.y, max.z)
    };
    
    // 檢查每個裁剪平面
    for (int plane = 0; plane < 6; ++plane) {
        int outsideCount = 0;
        
        for (int i = 0; i < 8; ++i) {
            glm::vec4 clipPos = mvpMatrix * glm::vec4(corners[i], 1.0f);
            
            // 檢查該頂點是否在當前裁剪平面外
            bool outside = false;
            switch (plane) {
                case 0: outside = clipPos.x < -clipPos.w; break; // 左
                case 1: outside = clipPos.x > clipPos.w; break;  // 右
                case 2: outside = clipPos.y < -clipPos.w; break; // 下
                case 3: outside = clipPos.y > clipPos.w; break;  // 上
                case 4: outside = clipPos.z < -clipPos.w; break; // 近
                case 5: outside = clipPos.z > clipPos.w; break;  // 遠
            }
            
            if (outside) outsideCount++;
        }
        
        // 如果所有8個頂點都在某個平面外，則整個包圍盒都被剔除
        if (outsideCount == 8) {
            return false;
        }
    }
    
    return true; // 包圍盒至少部分在視錐內
}

void BoundingBox::UpdateBounds(const glm::vec3& point)
{
    min.x = std::min(min.x, point.x);
    min.y = std::min(min.y, point.y);
    min.z = std::min(min.z, point.z);
    
    max.x = std::max(max.x, point.x);
    max.y = std::max(max.y, point.y);
    max.z = std::max(max.z, point.z);
}

void OcclusionQuery::BeginQuery()
{
    if (queryId == 0) {
        glGenQueries(1, &queryId);
    }
    
    glBeginQuery(GL_ANY_SAMPLES_PASSED, queryId);
    queryInProgress = true;
}

void OcclusionQuery::EndQuery()
{
    if (queryInProgress) {
        glEndQuery(GL_ANY_SAMPLES_PASSED);
        queryInProgress = false;
    }
}

bool OcclusionQuery::CheckResult()
{
    if (queryId == 0 || queryInProgress) {
        return false; // 查詢尚未完成
    }
    
    GLint available = 0;
    glGetQueryObjectiv(queryId, GL_QUERY_RESULT_AVAILABLE, &available);
    
    if (available) {
        GLint result = 0;
        glGetQueryObjectiv(queryId, GL_QUERY_RESULT, &result);
        isOccluded = (result == 0);
        
        if (isOccluded) {
            framesSinceVisible++;
        } else {
            framesSinceVisible = 0;
        }
        
        return true; // 結果可用
    }
    
    return false; // 結果尚未準備好
}

bool Model::Setup(AppContext &appContext)
{
	if (m_mmdModel == nullptr)
	{
		return false;
	}

	// Setup vertices
	size_t vtxCount = m_mmdModel->GetVertexCount();
	glGenBuffers(1, &m_posVBO);
	glBindBuffer(GL_ARRAY_BUFFER, m_posVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * vtxCount, nullptr, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glGenBuffers(1, &m_norVBO);
	glBindBuffer(GL_ARRAY_BUFFER, m_norVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * vtxCount, nullptr, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glGenBuffers(1, &m_uvVBO);
	glBindBuffer(GL_ARRAY_BUFFER, m_uvVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec2) * vtxCount, nullptr, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	size_t idxSize = m_mmdModel->GetIndexElementSize();
	size_t idxCount = m_mmdModel->GetIndexCount();
	glGenBuffers(1, &m_ibo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, idxSize * idxCount, m_mmdModel->GetIndices(), GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	if (idxSize == 1)
	{
		m_indexType = GL_UNSIGNED_BYTE;
	}
	else if (idxSize == 2)
	{
		m_indexType = GL_UNSIGNED_SHORT;
	}
	else if (idxSize == 4)
	{
		m_indexType = GL_UNSIGNED_INT;
	}
	else
	{
		return false;
	}

	// Setup MMD VAO
	glGenVertexArrays(1, &m_mmdVAO);
	glBindVertexArray(m_mmdVAO);

	const auto &mmdShader = appContext.m_mmdShader;
	glBindBuffer(GL_ARRAY_BUFFER, m_posVBO);
	glVertexAttribPointer(mmdShader->m_inPos, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (const void *)0);
	glEnableVertexAttribArray(mmdShader->m_inPos);

	glBindBuffer(GL_ARRAY_BUFFER, m_norVBO);
	glVertexAttribPointer(mmdShader->m_inNor, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (const void *)0);
	glEnableVertexAttribArray(mmdShader->m_inNor);

	glBindBuffer(GL_ARRAY_BUFFER, m_uvVBO);
	glVertexAttribPointer(mmdShader->m_inUV, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), (const void *)0);
	glEnableVertexAttribArray(mmdShader->m_inUV);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ibo);

	glBindVertexArray(0);

	// Setup MMD Edge VAO
	glGenVertexArrays(1, &m_mmdEdgeVAO);
	glBindVertexArray(m_mmdEdgeVAO);

	const auto &mmdEdgeShader = appContext.m_mmdEdgeShader;
	glBindBuffer(GL_ARRAY_BUFFER, m_posVBO);
	glVertexAttribPointer(mmdEdgeShader->m_inPos, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (const void *)0);
	glEnableVertexAttribArray(mmdEdgeShader->m_inPos);

	glBindBuffer(GL_ARRAY_BUFFER, m_norVBO);
	glVertexAttribPointer(mmdEdgeShader->m_inNor, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (const void *)0);
	glEnableVertexAttribArray(mmdEdgeShader->m_inNor);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ibo);

	glBindVertexArray(0);

	// Setup MMD Ground Shadow VAO
	glGenVertexArrays(1, &m_mmdGroundShadowVAO);
	glBindVertexArray(m_mmdGroundShadowVAO);

	const auto &mmdGroundShadowShader = appContext.m_mmdGroundShadowShader;
	glBindBuffer(GL_ARRAY_BUFFER, m_posVBO);
	glVertexAttribPointer(mmdGroundShadowShader->m_inPos, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (const void *)0);
	glEnableVertexAttribArray(mmdGroundShadowShader->m_inPos);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ibo);

	glBindVertexArray(0);

	// Setup materials
	for (size_t i = 0; i < m_mmdModel->GetMaterialCount(); i++)
	{
		const auto &mmdMat = m_mmdModel->GetMaterials()[i];
		Material mat(mmdMat);
		if (!mmdMat.m_texture.empty())
		{
			auto tex = appContext.GetTexture(mmdMat.m_texture);
			mat.m_texture = tex.m_texture;
			mat.m_textureHasAlpha = tex.m_hasAlpha;
		}
		if (!mmdMat.m_spTexture.empty())
		{
			auto tex = appContext.GetTexture(mmdMat.m_spTexture);
			mat.m_spTexture = tex.m_texture;
		}
		if (!mmdMat.m_toonTexture.empty())
		{
			auto tex = appContext.GetTexture(mmdMat.m_toonTexture);
			mat.m_toonTexture = tex.m_texture;
		}
		m_materials.emplace_back(std::move(mat));
	}
	
	// 計算初始包圍盒
	UpdateBoundingBox();
	
	// 初始化遮擋查詢
	m_shouldRender = true; // 預設為可見

	return true;
}

void Model::Clear()
{
	if (m_posVBO != 0)
	{
		glDeleteBuffers(1, &m_posVBO);
	}
	if (m_norVBO != 0)
	{
		glDeleteBuffers(1, &m_norVBO);
	}
	if (m_uvVBO != 0)
	{
		glDeleteBuffers(1, &m_uvVBO);
	}
	if (m_ibo != 0)
	{
		glDeleteBuffers(1, &m_ibo);
	}
	m_posVBO = 0;
	m_norVBO = 0;
	m_uvVBO = 0;
	m_ibo = 0;

	if (m_mmdVAO != 0)
	{
		glDeleteVertexArrays(1, &m_mmdVAO);
	}
	if (m_mmdEdgeVAO != 0)
	{
		glDeleteVertexArrays(1, &m_mmdEdgeVAO);
	}
	if (m_mmdGroundShadowVAO != 0)
	{
		glDeleteVertexArrays(1, &m_mmdGroundShadowVAO);
	}
	m_mmdVAO = 0;
	m_mmdEdgeVAO = 0;
	m_mmdGroundShadowVAO = 0;
	
	// 清理遮擋查詢
	if (m_occlusionQuery.queryId != 0) {
		glDeleteQueries(1, &m_occlusionQuery.queryId);
		m_occlusionQuery.queryId = 0;
	}
}

//=============================================================================
// 遮擋剔除實現
//=============================================================================

void Model::UpdateBoundingBox()
{
	if (m_mmdModel == nullptr) return;
	
	m_boundingBox.Reset();
	
	// 計算模型的包圍盒
	size_t vtxCount = m_mmdModel->GetVertexCount();
	const glm::vec3* positions = m_mmdModel->GetUpdatePositions();
	
	if (positions == nullptr || vtxCount == 0) {
		// 如果沒有更新位置，嘗試獲取原始位置
		const glm::vec3* origPositions = m_mmdModel->GetPositions();
		if (origPositions != nullptr && vtxCount > 0) {
			positions = origPositions;
		} else {
			// 如果仍然沒有頂點數據，設置一個默認的大包圍盒
			m_boundingBox.min = glm::vec3(-10.0f);
			m_boundingBox.max = glm::vec3(10.0f);
			return;
		}
	}
	
	// 檢查第一個頂點是否為零，如果所有頂點都是零，使用預設包圍盒
	bool allZero = true;
	for (size_t i = 0; i < std::min(vtxCount, (size_t)100); ++i) {
		if (positions[i].x != 0.0f || positions[i].y != 0.0f || positions[i].z != 0.0f) {
			allZero = false;
			break;
		}
	}
	
	if (allZero) {
		// 使用一個合理的包圍盒（大約人形模型的大小）
		m_boundingBox.min = glm::vec3(-1.0f, 0.0f, -1.0f);
		m_boundingBox.max = glm::vec3(1.0f, 3.0f, 1.0f);
		return;
	}
	
	for (size_t i = 0; i < vtxCount; ++i) {
		m_boundingBox.UpdateBounds(positions[i]);
	}
	
	// 如果需要，也可以為每個子網格計算包圍盒
	if (m_subMeshBounds.size() != m_mmdModel->GetSubMeshCount()) {
		m_subMeshBounds.resize(m_mmdModel->GetSubMeshCount());
	}
	
	size_t subMeshCount = m_mmdModel->GetSubMeshCount();
	for (size_t i = 0; i < subMeshCount; ++i) {
		const auto& subMesh = m_mmdModel->GetSubMeshes()[i];
		m_subMeshBounds[i].Reset();
		
		// 為子網格計算包圍盒
		for (size_t j = subMesh.m_beginIndex; j < subMesh.m_beginIndex + subMesh.m_vertexCount; ++j) {
			if (j < vtxCount) {
				m_subMeshBounds[i].UpdateBounds(positions[j]);
			}
		}
	}
}

bool Model::PerformFrustumCulling(const AppContext& appContext)
{
	if (!appContext.m_enableFrustumCulling) {
		return true; // 視錐剔除被禁用
	}
	
	// 檢查包圍盒是否有效
	if (m_boundingBox.min.x > m_boundingBox.max.x) {
		// 包圍盒無效，預設為可見
		return true;
	}
	
	// 計算MVP矩陣
	glm::mat4 world = glm::mat4(1.0f); // MMD模型通常在世界座標原點
	glm::mat4 mvp = appContext.m_projMat * appContext.m_viewMat * world;
	
	// 檢查主包圍盒是否在視錐內
	bool inFrustum = m_boundingBox.IsInFrustum(mvp);
	
	return inFrustum;
}

void Model::PerformOcclusionCulling(const AppContext& appContext)
{
	if (!appContext.m_enableOcclusionCulling) {
		return; // 不改變 m_shouldRender 狀態
	}
	
	// 在單物體場景中，遮擋剔除的效果有限，因此保持物體可見
	// 這是一個簡化的實現，適用於演示目的
	m_shouldRender = true;
	
	// 可選：仍然進行查詢以測試系統
	if (m_shouldRender) {
		glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
		
		m_occlusionQuery.BeginQuery();
		DrawBoundingBox(appContext);
		m_occlusionQuery.EndQuery();
		
		glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
		
		// 檢查查詢結果但不用於剔除決策
		m_occlusionQuery.CheckResult();
	}
}

void Model::DrawBoundingBox(const AppContext& appContext)
{
	// OpenGL ES 2.0 兼容的包圍盒渲染
	auto& bbox = m_boundingBox;
	glm::vec3 min = bbox.min;
	glm::vec3 max = bbox.max;
	
	// 定義立方體的8個頂點
	GLfloat vertices[] = {
		// 底面4個頂點
		min.x, min.y, min.z,  // 0
		max.x, min.y, min.z,  // 1
		max.x, min.y, max.z,  // 2
		min.x, min.y, max.z,  // 3
		// 頂面4個頂點
		min.x, max.y, min.z,  // 4
		max.x, max.y, min.z,  // 5
		max.x, max.y, max.z,  // 6
		min.x, max.y, max.z   // 7
	};
	
	if (appContext.m_showBoundingBoxes) {
		// 除錯模式：渲染可見的包圍盒線框
		GLushort lineIndices[] = {
			// 底面4條邊
			0, 1,  1, 2,  2, 3,  3, 0,
			// 頂面4條邊
			4, 5,  5, 6,  6, 7,  7, 4,
			// 垂直4條邊
			0, 4,  1, 5,  2, 6,  3, 7
		};
		
		glDisable(GL_DEPTH_TEST);
		glLineWidth(2.0f);
		
		// 使用 vertex attribute 0 (通常是位置)
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, vertices);
		
		glDrawElements(GL_LINES, 24, GL_UNSIGNED_SHORT, lineIndices);
		
		glDisableVertexAttribArray(0);
		glEnable(GL_DEPTH_TEST);
		glLineWidth(1.0f);
	} else {
		// 遮擋測試模式：渲染實體包圍盒
		// 注意：顏色寫入已在調用方關閉，這裡只需要正常渲染立方體
		GLushort boxIndices[] = {
			// 前面 (z = max.z)
			3, 2, 6,  6, 7, 3,
			// 後面 (z = min.z)
			1, 0, 4,  4, 5, 1,
			// 左面 (x = min.x)
			0, 3, 7,  7, 4, 0,
			// 右面 (x = max.x)
			2, 1, 5,  5, 6, 2,
			// 底面 (y = min.y)
			0, 1, 2,  2, 3, 0,
			// 頂面 (y = max.y)
			7, 6, 5,  5, 4, 7
		};
		
		// 使用 vertex attribute 0
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, vertices);
		
		glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_SHORT, boxIndices);
		
		glDisableVertexAttribArray(0);
	}
}

void Model::UpdateAnimation(const AppContext &appContext)
{
	m_mmdModel->BeginAnimation();
	m_mmdModel->UpdateAllAnimation(m_vmdAnim.get(), appContext.m_animTime * 30.0f, appContext.m_elapsed);
	m_mmdModel->EndAnimation();
}

void Model::Update(const AppContext &appContext)
{
	m_mmdModel->Update();

	size_t vtxCount = m_mmdModel->GetVertexCount();
	
	// 先假設模型可見，然後進行剔除測試
	m_shouldRender = true;
	
	// 總是更新包圍盒（在模型更新後）
	UpdateBoundingBox();
	
	// 執行視錐剔除
	bool inFrustum = PerformFrustumCulling(appContext);
	if (!inFrustum) {
		m_shouldRender = false;
		return; // 提早退出，不更新GPU緩衝區
	}
	
	// 執行遮擋剔除（異步）
	PerformOcclusionCulling(appContext);
	
	// 批量更新所有緩衝區以減少GPU狀態切換
	glBindBuffer(GL_ARRAY_BUFFER, m_posVBO);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(glm::vec3) * vtxCount, m_mmdModel->GetUpdatePositions());
	
	glBindBuffer(GL_ARRAY_BUFFER, m_norVBO);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(glm::vec3) * vtxCount, m_mmdModel->GetUpdateNormals());
	
	glBindBuffer(GL_ARRAY_BUFFER, m_uvVBO);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(glm::vec2) * vtxCount, m_mmdModel->GetUpdateUVs());
	
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void Model::Draw(const AppContext &appContext)
{
	// 遮擋剔除檢查：如果物件被剔除，跳過渲染
	if (!m_shouldRender) {
		return;
	}

	const auto &view = appContext.m_viewMat;
	const auto &proj = appContext.m_projMat;

	auto world = glm::mat4(1.0f);
	auto wv = view * world;
	auto wvp = proj * view * world;
	auto wvit = glm::mat3(view * world);
	wvit = glm::inverse(wvit);
	wvit = glm::transpose(wvit);
	
	// 如果啟用了包圍盒顯示，先渲染包圍盒
	if (appContext.m_showBoundingBoxes) {
		DrawBoundingBox(appContext);
	}

	glActiveTexture(GL_TEXTURE0 + 3);
	glBindTexture(GL_TEXTURE_2D, appContext.m_dummyShadowDepthTex);
	glActiveTexture(GL_TEXTURE0 + 4);
	glBindTexture(GL_TEXTURE_2D, appContext.m_dummyShadowDepthTex);
	glActiveTexture(GL_TEXTURE0 + 5);
	glBindTexture(GL_TEXTURE_2D, appContext.m_dummyShadowDepthTex);
	glActiveTexture(GL_TEXTURE0 + 6);
	glBindTexture(GL_TEXTURE_2D, appContext.m_dummyShadowDepthTex);

	glEnable(GL_DEPTH_TEST);

	// Draw model
	size_t subMeshCount = m_mmdModel->GetSubMeshCount();
	for (size_t i = 0; i < subMeshCount; i++)
	{
		const auto &subMesh = m_mmdModel->GetSubMeshes()[i];
		const auto &shader = appContext.m_mmdShader;
		const auto &mat = m_materials[subMesh.m_materialID];
		const auto &mmdMat = mat.m_mmdMat;

		if (mat.m_mmdMat.m_alpha == 0)
		{
			continue;
		}

		glUseProgram(shader->m_prog);
		glBindVertexArray(m_mmdVAO);

		glUniformMatrix4fv(shader->m_uWV, 1, GL_FALSE, &wv[0][0]);
		glUniformMatrix4fv(shader->m_uWVP, 1, GL_FALSE, &wvp[0][0]);

		bool alphaBlend = true;

		glUniform3fv(shader->m_uAmbinet, 1, &mmdMat.m_ambient[0]);
		glUniform3fv(shader->m_uDiffuse, 1, &mmdMat.m_diffuse[0]);
		glUniform3fv(shader->m_uSpecular, 1, &mmdMat.m_specular[0]);
		glUniform1f(shader->m_uSpecularPower, mmdMat.m_specularPower);
		glUniform1f(shader->m_uAlpha, mmdMat.m_alpha);

		glActiveTexture(GL_TEXTURE0 + 0);
		glUniform1i(shader->m_uTex, 0);
		if (mat.m_texture != 0)
		{
			if (!mat.m_textureHasAlpha)
			{
				// Use Material Alpha
				glUniform1i(shader->m_uTexMode, 1);
			}
			else
			{
				// Use Material Alpha * Texture Alpha
				glUniform1i(shader->m_uTexMode, 2);
			}
			glUniform4fv(shader->m_uTexMulFactor, 1, &mmdMat.m_textureMulFactor[0]);
			glUniform4fv(shader->m_uTexAddFactor, 1, &mmdMat.m_textureAddFactor[0]);
			glBindTexture(GL_TEXTURE_2D, mat.m_texture);
		}
		else
		{
			glUniform1i(shader->m_uTexMode, 0);
			glBindTexture(GL_TEXTURE_2D, appContext.m_dummyColorTex);
		}

		glActiveTexture(GL_TEXTURE0 + 1);
		glUniform1i(shader->m_uSphereTex, 1);
		if (mat.m_spTexture != 0)
		{
			if (mmdMat.m_spTextureMode == saba::MMDMaterial::SphereTextureMode::Mul)
			{
				glUniform1i(shader->m_uSphereTexMode, 1);
			}
			else if (mmdMat.m_spTextureMode == saba::MMDMaterial::SphereTextureMode::Add)
			{
				glUniform1i(shader->m_uSphereTexMode, 2);
			}
			glUniform4fv(shader->m_uSphereTexMulFactor, 1, &mmdMat.m_spTextureMulFactor[0]);
			glUniform4fv(shader->m_uSphereTexAddFactor, 1, &mmdMat.m_spTextureAddFactor[0]);
			glBindTexture(GL_TEXTURE_2D, mat.m_spTexture);
		}
		else
		{
			glUniform1i(shader->m_uSphereTexMode, 0);
			glBindTexture(GL_TEXTURE_2D, appContext.m_dummyColorTex);
		}

		glActiveTexture(GL_TEXTURE0 + 2);
		glUniform1i(shader->m_uToonTex, 2);
		if (mat.m_toonTexture != 0)
		{
			glUniform4fv(shader->m_uToonTexMulFactor, 1, &mmdMat.m_toonTextureMulFactor[0]);
			glUniform4fv(shader->m_uToonTexAddFactor, 1, &mmdMat.m_toonTextureAddFactor[0]);
			glUniform1i(shader->m_uToonTexMode, 1);
			glBindTexture(GL_TEXTURE_2D, mat.m_toonTexture);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		}
		else
		{
			glUniform1i(shader->m_uToonTexMode, 0);
			glBindTexture(GL_TEXTURE_2D, appContext.m_dummyColorTex);
		}

		glm::vec3 lightColor = appContext.m_lightColor;
		glm::vec3 lightDir = appContext.m_lightDir;
		glm::mat3 viewMat = glm::mat3(appContext.m_viewMat);
		lightDir = viewMat * lightDir;
		glUniform3fv(shader->m_uLightDir, 1, &lightDir[0]);
		glUniform3fv(shader->m_uLightColor, 1, &lightColor[0]);

		if (mmdMat.m_bothFace)
		{
			glDisable(GL_CULL_FACE);
		}
		else
		{
			glEnable(GL_CULL_FACE);
			glCullFace(GL_BACK);
		}

		if (appContext.m_enableTransparentWindow)
		{
			glEnable(GL_BLEND);
			glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ZERO, GL_ONE_MINUS_SRC_ALPHA);
		}
		else
		{
			if (alphaBlend)
			{
				glEnable(GL_BLEND);
				glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			}
			else
			{
				glDisable(GL_BLEND);
			}
		}

		glUniform1i(shader->m_uShadowMapEnabled, 0);
		glUniform1i(shader->m_uShadowMap0, 3);
		glUniform1i(shader->m_uShadowMap1, 4);
		glUniform1i(shader->m_uShadowMap2, 5);
		glUniform1i(shader->m_uShadowMap3, 6);

		size_t offset = subMesh.m_beginIndex * m_mmdModel->GetIndexElementSize();
		glDrawElements(GL_TRIANGLES, subMesh.m_vertexCount, m_indexType, (GLvoid *)offset);

		glActiveTexture(GL_TEXTURE0 + 2);
		glBindTexture(GL_TEXTURE_2D, 0);
		glActiveTexture(GL_TEXTURE0 + 1);
		glBindTexture(GL_TEXTURE_2D, 0);
		glActiveTexture(GL_TEXTURE0 + 0);
		glBindTexture(GL_TEXTURE_2D, 0);

		glUseProgram(0);
	}

	glActiveTexture(GL_TEXTURE0 + 3);
	glBindTexture(GL_TEXTURE_2D, 0);
	glActiveTexture(GL_TEXTURE0 + 4);
	glBindTexture(GL_TEXTURE_2D, 0);
	glActiveTexture(GL_TEXTURE0 + 5);
	glBindTexture(GL_TEXTURE_2D, 0);
	glActiveTexture(GL_TEXTURE0 + 6);
	glBindTexture(GL_TEXTURE_2D, 0);

	// Draw edge
	glm::vec2 screenSize(appContext.m_screenWidth, appContext.m_screenHeight);
	for (size_t i = 0; i < subMeshCount; i++)
	{
		const auto &subMesh = m_mmdModel->GetSubMeshes()[i];
		int matID = subMesh.m_materialID;
		const auto &shader = appContext.m_mmdEdgeShader;
		const auto &mat = m_materials[subMesh.m_materialID];
		const auto &mmdMat = mat.m_mmdMat;

		if (!mmdMat.m_edgeFlag)
		{
			continue;
		}
		if (mmdMat.m_alpha == 0.0f)
		{
			continue;
		}

		glUseProgram(shader->m_prog);
		glBindVertexArray(m_mmdEdgeVAO);

		glUniformMatrix4fv(shader->m_uWV, 1, GL_FALSE, &wv[0][0]);
		glUniformMatrix4fv(shader->m_uWVP, 1, GL_FALSE, &wvp[0][0]);
		glUniform2fv(shader->m_uScreenSize, 1, &screenSize[0]);
		glUniform1f(shader->m_uEdgeSize, mmdMat.m_edgeSize);
		glUniform4fv(shader->m_uEdgeColor, 1, &mmdMat.m_edgeColor[0]);

		bool alphaBlend = true;

		glEnable(GL_CULL_FACE);
		glCullFace(GL_FRONT);

		if (appContext.m_enableTransparentWindow)
		{
			glEnable(GL_BLEND);
			glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ZERO, GL_ONE_MINUS_SRC_ALPHA);
		}
		else
		{
			if (alphaBlend)
			{
				glEnable(GL_BLEND);
				glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			}
			else
			{
				glDisable(GL_BLEND);
			}
		}

		size_t offset = subMesh.m_beginIndex * m_mmdModel->GetIndexElementSize();
		glDrawElements(GL_TRIANGLES, subMesh.m_vertexCount, m_indexType, (GLvoid *)offset);

		glBindVertexArray(0);
		glUseProgram(0);
	}

	// Draw ground shadow
	glEnable(GL_POLYGON_OFFSET_FILL);
	glPolygonOffset(-1, -1);
	auto plane = glm::vec4(0, 1, 0, 0);
	auto light = -appContext.m_lightDir;
	auto shadow = glm::mat4(1);

	shadow[0][0] = plane.y * light.y + plane.z * light.z;
	shadow[0][1] = -plane.x * light.y;
	shadow[0][2] = -plane.x * light.z;
	shadow[0][3] = 0;

	shadow[1][0] = -plane.y * light.x;
	shadow[1][1] = plane.x * light.x + plane.z * light.z;
	shadow[1][2] = -plane.y * light.z;
	shadow[1][3] = 0;

	shadow[2][0] = -plane.z * light.x;
	shadow[2][1] = -plane.z * light.y;
	shadow[2][2] = plane.x * light.x + plane.y * light.y;
	shadow[2][3] = 0;

	shadow[3][0] = -plane.w * light.x;
	shadow[3][1] = -plane.w * light.y;
	shadow[3][2] = -plane.w * light.z;
	shadow[3][3] = plane.x * light.x + plane.y * light.y + plane.z * light.z;

	auto wsvp = proj * view * shadow * world;

	auto shadowColor = glm::vec4(0.4f, 0.2f, 0.2f, 0.7f);
	if (appContext.m_enableTransparentWindow)
	{
		glEnable(GL_BLEND);
		glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ZERO, GL_ONE_MINUS_SRC_ALPHA);

		glStencilFuncSeparate(GL_FRONT_AND_BACK, GL_NOTEQUAL, 1, 1);
		glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
		glEnable(GL_STENCIL_TEST);
	}
	else
	{
		if (shadowColor.a < 1.0f)
		{
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

			glStencilFuncSeparate(GL_FRONT_AND_BACK, GL_NOTEQUAL, 1, 1);
			glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
			glEnable(GL_STENCIL_TEST);
		}
		else
		{
			glDisable(GL_BLEND);
		}
	}
	glDisable(GL_CULL_FACE);

	for (size_t i = 0; i < subMeshCount; i++)
	{
		const auto &subMesh = m_mmdModel->GetSubMeshes()[i];
		int matID = subMesh.m_materialID;
		const auto &mat = m_materials[subMesh.m_materialID];
		const auto &mmdMat = mat.m_mmdMat;
		const auto &shader = appContext.m_mmdGroundShadowShader;

		if (!mmdMat.m_groundShadow)
		{
			continue;
		}
		if (mmdMat.m_alpha == 0.0f)
		{
			continue;
		}

		glUseProgram(shader->m_prog);
		glBindVertexArray(m_mmdGroundShadowVAO);

		glUniformMatrix4fv(shader->m_uWVP, 1, GL_FALSE, &wsvp[0][0]);
		glUniform4fv(shader->m_uShadowColor, 1, &shadowColor[0]);

		size_t offset = subMesh.m_beginIndex * m_mmdModel->GetIndexElementSize();
		glDrawElements(GL_TRIANGLES, subMesh.m_vertexCount, m_indexType, (GLvoid *)offset);

		glBindVertexArray(0);
		glUseProgram(0);
	}

	glDisable(GL_POLYGON_OFFSET_FILL);
	glDisable(GL_STENCIL_TEST);
	glDisable(GL_BLEND);
}
