#pragma once

#include <expected>
#include <filesystem>
#include <unordered_set>
#include <vector>

#include "mesh.hpp"
#include "pipeline.hpp"
#include "texture.hpp"
#include "assimp/scene.h"

namespace stw
{
class Model
{
public:
	void Draw(const Pipeline& pipeline) const;
	void DrawMeshOnly(const Pipeline& pipeline) const;

	static std::expected<Model, std::string> LoadFromPath(const std::filesystem::path& path);

private:
	Model() = default;

	// TODO: Convert to texture manager
	inline static std::unordered_set<std::filesystem::path> s_LoadedTextures{};

	std::vector<Mesh> m_Meshes{};
	std::filesystem::path m_Directory{};

	void ProcessNode(const aiNode* node, const aiScene* scene);
	Mesh ProcessMesh(aiMesh* mesh, const aiScene* scene) const;
	std::vector<Texture> LoadMaterialTextures(const aiMaterial* material, TextureType textureType) const;
};
}
