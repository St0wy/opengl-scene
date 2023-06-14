#pragma once

#include <expected>
#include <filesystem>
#include <span>
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
	Model() = default;

	[[nodiscard]] std::span<Mesh> Meshes();

	void AddMesh(Mesh mesh);

	void Draw(const Pipeline& pipeline) const;
	void DrawNoSpecular(const Pipeline& pipeline) const;
	void DrawMeshOnly(const Pipeline& pipeline) const;
	void DrawInstanced(const Pipeline& pipeline, GLsizei count) const;
	void DrawNoSpecularInstanced(const Pipeline& pipeline, GLsizei count) const;

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
