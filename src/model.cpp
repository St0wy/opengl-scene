#include "model.hpp"

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <spdlog/spdlog.h>

#include "timer.hpp"
#include "utils.hpp"

std::span<stw::Mesh> stw::Model::Meshes()
{
	return std::span{m_Meshes};
}

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

void stw::Model::DrawMeshOnly() const
{
	for (const auto& mesh : m_Meshes)
	{
		mesh.DrawMeshOnly();
	}
}

void stw::Model::DrawInstanced(const Pipeline& pipeline, const GLsizei count) const
{
	for (const auto& mesh : m_Meshes)
	{
		mesh.DrawInstanced(pipeline, count);
	}
}

void stw::Model::DrawNoSpecularInstanced(const Pipeline& pipeline, const GLsizei count) const
{
	for (const auto& mesh : m_Meshes)
	{
		mesh.DrawNoSpecularInstanced(pipeline, count);
	}
}

void stw::Model::Delete()
{
	for (auto& mesh : m_Meshes)
	{
		mesh.Delete();
	}
}

std::expected<stw::Model, std::string> stw::Model::LoadFromPath(const std::filesystem::path& path)
{
	Assimp::Importer importer;
	constexpr u32 assimpImportFlags = aiProcessPreset_TargetRealtime_Fast;

	const auto pathString = path.string();

	Timer timer;
	timer.Start();

	const aiScene* scene = importer.ReadFile(pathString.c_str(), assimpImportFlags);

	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
	{
		return std::unexpected(importer.GetErrorString());
	}

	spdlog::info("Imported model {} in {:0.0f} ms", pathString, timer.GetElapsedTime().GetInMilliseconds());

	Model model{};
	model.m_Directory = path.parent_path();
	model.m_Meshes.reserve(scene->mNumMeshes);
	model.ProcessNode(scene->mRootNode, scene);

	spdlog::info("Converted model {} in {:0.0f} ms", pathString, timer.GetElapsedTime().GetInMilliseconds());

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

stw::Mesh stw::Model::ProcessMesh(aiMesh* assimpMesh, const aiScene* assimpScene) const
{
	std::vector<Vertex> vertices{};
	vertices.reserve(assimpMesh->mNumVertices);
	for (std::size_t i = 0; i < assimpMesh->mNumVertices; ++i)
	{
		const auto meshVertex = assimpMesh->mVertices[i];
		const auto meshNormal = assimpMesh->mNormals[i];

		glm::vec2 textureCoords(0.0);
		if (assimpMesh->mTextureCoords[0])
		{
			const auto meshTextureCoords = assimpMesh->mTextureCoords[0][i];
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
	indices.reserve(static_cast<std::size_t>(assimpMesh->mNumFaces) * 3);
	for (std::size_t i = 0; i < assimpMesh->mNumFaces; ++i)
	{
		const aiFace& face = assimpMesh->mFaces[i];
		for (std::size_t j = 0; j < face.mNumIndices; ++j)
		{
			indices.push_back(face.mIndices[j]);
		}
	}

	std::vector<Texture> textures{};
	if (assimpScene->mNumMaterials > 0)
	{
		aiMaterial* material = assimpScene->mMaterials[assimpMesh->mMaterialIndex];
		std::vector<Texture> diffuseMaps = LoadMaterialTextures(material, TextureType::Diffuse);

		textures.insert_range(textures.end(), diffuseMaps);

		std::vector<Texture> specularMaps = LoadMaterialTextures(material, TextureType::Specular);
		textures.insert_range(textures.end(), specularMaps);
	}

	Mesh mesh;
	mesh.Init(std::move(vertices), std::move(indices), std::move(textures));

	return mesh;
}

std::vector<stw::Texture> stw::Model::LoadMaterialTextures(const aiMaterial* material,
	const TextureType textureType) const
{
	const aiTextureType assimpType = ToAssimpTextureType(textureType);
	const auto textureCount = material->GetTextureCount(assimpType);
	std::vector<Texture> textures{};

	for (std::size_t i = 0; i < textureCount; i++)
	{
		Timer timer;
		timer.Start();

		aiString str;
		material->GetTexture(assimpType, static_cast<u32>(i), &str);

		std::filesystem::path texturePath = m_Directory / str.C_Str();
		if (s_LoadedTextures.contains(texturePath))
		{
			continue;
		}

		auto loadResult = Texture::LoadFromPath(texturePath, textureType);
		if (!loadResult.has_value())
		{
			spdlog::error(loadResult.error());
			continue;
		}

		s_LoadedTextures.insert(texturePath);
		textures.push_back(loadResult.value());

		spdlog::info("Loaded texture {} in {:0.0f} ms",
			texturePath.string(),
			timer.GetElapsedTime().GetInMilliseconds());
	}

	return textures;
}
