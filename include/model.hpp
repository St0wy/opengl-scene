#pragma once

#include <expected>
#include <filesystem>
#include <span>
#include <unordered_set>
#include <vector>
#include <assimp/scene.h>

#include "mesh.hpp"
#include "texture.hpp"
#include "ogl/pipeline.hpp"

namespace stw
{
class Model
{
public:
	Model() = default;

	void AddMesh(Mesh mesh);

	void Draw(const Pipeline& pipeline, const glm::mat4&) const;
	void DrawNoSpecular(const Pipeline& pipeline, const glm::mat4& modelMatrix) const;
	void DrawInstanced(const Pipeline& pipeline, std::span<const glm::mat4> modelMatrices) const;
	void DrawNoSpecularInstanced(const Pipeline& pipeline, std::span<const glm::mat4> modelMatrices) const;

	void Delete();

	static std::expected<Model, std::string> LoadFromPath(const std::filesystem::path& path);

private:
	// TODO: Convert to texture manager
	inline static std::unordered_set<std::filesystem::path> s_LoadedTextures{};

	std::vector<Mesh> m_Meshes{};
	std::filesystem::path m_Directory{};

	void ProcessNode(const aiNode* node, const aiScene* scene);
	Mesh ProcessMesh(aiMesh* assimpMesh, const aiScene* assimpScene) const;
	std::vector<Texture> LoadMaterialTextures(const aiMaterial* material, TextureType textureType) const;
};
}
