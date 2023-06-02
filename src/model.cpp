#include "model.hpp"

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <spdlog/spdlog.h>

#include "utils.hpp"

void stw::Model::AddMesh(Mesh mesh)
{
	m_Meshes.push_back(std::move(mesh));
}

void stw::Model::Draw(const Pipeline& pipeline) const
{
	for (const auto& mesh : m_Meshes)
	{
		mesh.Draw(pipeline);
	}
}

void stw::Model::DrawNoSpecular(const Pipeline& pipeline) const
{
	for (const auto& mesh : m_Meshes)
	{
		mesh.DrawNoSpecular(pipeline);
	}
}

void stw::Model::DrawMeshOnly(const Pipeline& pipeline) const
{
	for (const auto& mesh : m_Meshes)
	{
		mesh.DrawMeshOnly(pipeline);
	}
}

std::expected<stw::Model, std::string> stw::Model::LoadFromPath(const std::filesystem::path& path)
{
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(path.string().c_str(), aiProcess_Triangulate | aiProcess_FlipUVs);

	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
	{
		return std::unexpected(importer.GetErrorString());
	}

	Model model{};
	model.m_Directory = path.parent_path();
	model.m_Meshes.reserve(scene->mNumMeshes);
	model.ProcessNode(scene->mRootNode, scene);

	return {std::move(model)};
}

void stw::Model::ProcessNode(const aiNode* node, const aiScene* scene)
{
	for (std::size_t i = 0; i < node->mNumMeshes; ++i)
	{
		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
		m_Meshes.push_back(ProcessMesh(mesh, scene));
	}

	for (std::size_t i = 0; i < node->mNumChildren; ++i)
	{
		ProcessNode(node->mChildren[i], scene);
	}
}

stw::Mesh stw::Model::ProcessMesh(aiMesh* mesh, const aiScene* scene) const
{
	std::vector<Vertex> vertices{};
	vertices.reserve(mesh->mNumVertices);
	for (std::size_t i = 0; i < mesh->mNumVertices; ++i)
	{
		const auto meshVertex = mesh->mVertices[i];
		const auto meshNormal = mesh->mNormals[i];

		glm::vec2 textureCoords(0.0);
		if (mesh->mTextureCoords[0])
		{
			const auto meshTextureCoords = mesh->mTextureCoords[0][i];
			textureCoords.x = meshTextureCoords.x;
			textureCoords.y = meshTextureCoords.y;
		}

		Vertex vertex{
			{meshVertex.x, meshVertex.y, meshVertex.z,},
			{meshNormal.x, meshNormal.y, meshNormal.z,},
			textureCoords
		};
		vertices.push_back(vertex);
	}

	std::vector<u32> indices{};
	indices.reserve(static_cast<std::size_t>(mesh->mNumFaces) * 3);
	for (std::size_t i = 0; i < mesh->mNumFaces; ++i)
	{
		const aiFace& face = mesh->mFaces[i];
		for (std::size_t j = 0; j < face.mNumIndices; ++j)
		{
			indices.push_back(face.mIndices[j]);
		}
	}

	std::vector<Texture> textures{};
	if (scene->mNumMaterials > 0)
	{
		aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
		std::vector<Texture> diffuseMaps = LoadMaterialTextures(material, TextureType::Diffuse);

		textures.insert_range(textures.end(), diffuseMaps);

		std::vector<Texture> specularMaps = LoadMaterialTextures(material, TextureType::Specular);
		textures.insert_range(textures.end(), specularMaps);
	}
	return {vertices, indices, textures};
}

std::vector<stw::Texture> stw::Model::LoadMaterialTextures(const aiMaterial* material,
	const TextureType textureType) const
{
	const aiTextureType assimpType = ToAssimpTextureType(textureType);
	const auto textureCount = material->GetTextureCount(assimpType);
	std::vector<Texture> textures{};

	for (std::size_t i = 0; i < textureCount; i++)
	{
		aiString str;
		material->GetTexture(assimpType, static_cast<u32>(i), &str);

		std::filesystem::path texturePath = m_Directory / str.C_Str();
		if (s_LoadedTextures.contains(texturePath))
		{
			continue;
		}

		spdlog::debug("Loading texture : {}", texturePath.string());

		auto loadResult = Texture::LoadFromPath(texturePath, textureType);
		if (!loadResult.has_value())
		{
			spdlog::error(loadResult.error());
			assert(false);
		}

		s_LoadedTextures.insert(texturePath);
		textures.push_back(loadResult.value());
	}

	return textures;
}
