#pragma once

#include <filesystem>
#include <vector>
#include <expected>

#include "mesh.hpp"
#include "Pipeline.hpp"
#include "texture.hpp"
#include "assimp/scene.h"

namespace stw
{
class Model
{
public:
	void Draw(const Pipeline& pipeline) const;

	static std::expected<Model, std::string> LoadFromPath(std::filesystem::path path);

private:
	Model() = default;

	std::vector<Mesh> m_Meshes;
	std::filesystem::path m_Directory;

	void ProcessNode(const aiNode* node, const aiScene* scene);
	Mesh ProcessMesh(aiMesh* mesh, const aiScene* scene);
	std::vector<Texture> LoadMaterialTextures(const aiMaterial* material, TextureType textureType);
};
}
