//#include "model.hpp"
//
//#include <assimp/Importer.hpp>
//#include <assimp/postprocess.h>
//#include <spdlog/spdlog.h>
//
//#include "timer.hpp"
//
//void stw::Model::AddMesh(Mesh mesh)
//{
//	m_Meshes.push_back(std::move(mesh));
//}
//
//std::span<const stw::Mesh> stw::Model::GetMeshes() const
//{
//	return m_Meshes;
//}
//
////
////void stw::Model::Draw(Pipeline& pipeline, const glm::mat4& modelMatrix) const
////{
////	for (const auto& mesh : m_Meshes)
////	{
////		mesh.Draw(pipeline, modelMatrix);
////	}
////}
////
////void stw::Model::DrawNoSpecular(Pipeline& pipeline, const glm::mat4& modelMatrix) const
////{
////	for (const auto& mesh : m_Meshes)
////	{
////		mesh.DrawNoSpecular(pipeline, modelMatrix);
////	}
////}
////
////void stw::Model::DrawInstanced(Pipeline& pipeline, const std::span<const glm::mat4> modelMatrices) const
////{
////	for (const auto& mesh : m_Meshes)
////	{
////		mesh.DrawInstanced(pipeline, modelMatrices);
////	}
////}
////
////void stw::Model::DrawNoSpecularInstanced(Pipeline& pipeline, const std::span<const glm::mat4> modelMatrices) const
////{
////	for (const auto& mesh : m_Meshes)
////	{
////		mesh.DrawNoSpecularInstanced(pipeline, modelMatrices);
////	}
////}
//
//void stw::Model::Delete()
//{
//	for (auto& mesh : m_Meshes)
//	{
//		mesh.Delete();
//	}
//}
//
//std::expected<stw::Model, std::string> stw::Model::LoadFromPath(const std::filesystem::path& path)
//{
//	Assimp::Importer importer;
//	constexpr u32 assimpImportFlags = aiProcessPreset_TargetRealtime_Fast;
//
//	const auto pathString = path.string();
//
//	Timer timer;
//	timer.Start();
//
//	const aiScene* scene = importer.ReadFile(pathString.c_str(), assimpImportFlags);
//
//	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
//	{
//		return std::unexpected(importer.GetErrorString());
//	}
//
//	spdlog::info("Imported model {} in {:0.0f} ms", pathString, timer.GetElapsedTime().GetInMilliseconds());
//
//	Model model{};
//	model.m_Directory = path.parent_path();
//	model.m_Meshes.reserve(scene->mNumMeshes);
//	model.ProcessNode(scene->mRootNode, scene);
//
//	spdlog::info("Converted model {} in {:0.0f} ms", pathString, timer.GetElapsedTime().GetInMilliseconds());
//
//	return {std::move(model)};
//}
//
//void stw::Model::ProcessNode(const aiNode* node, const aiScene* scene)
//{
//	for (std::size_t i = 0; i < node->mNumMeshes; ++i)
//	{
//		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
//		m_Meshes.push_back(ProcessMesh(mesh, scene));
//	}
//
//	for (std::size_t i = 0; i < node->mNumChildren; ++i)
//	{
//		ProcessNode(node->mChildren[i], scene);
//	}
//}
//
//stw::Mesh stw::Model::ProcessMesh(aiMesh* assimpMesh, const aiScene* assimpScene) const
//{
//	std::vector<Vertex> vertices{};
//	vertices.reserve(assimpMesh->mNumVertices);
//	for (std::size_t i = 0; i < assimpMesh->mNumVertices; ++i)
//	{
//		const aiVector3D meshVertex = assimpMesh->mVertices[i];
//		const aiVector3D meshNormal = assimpMesh->mNormals[i];
//		const aiVector3D meshTangent = assimpMesh->mTangents[i];
//
//		glm::vec2 textureCoords(0.0f);
//		if (assimpMesh->mTextureCoords[0])
//		{
//			const auto meshTextureCoords = assimpMesh->mTextureCoords[0][i];
//			textureCoords.x = meshTextureCoords.x;
//			textureCoords.y = meshTextureCoords.y;
//		}
//
//		Vertex vertex{
//			{meshVertex.x, meshVertex.y, meshVertex.z,},
//			{meshNormal.x, meshNormal.y, meshNormal.z,},
//			textureCoords,
//			{meshTangent.x, meshTangent.y, meshTangent.z}
//		};
//		vertices.push_back(vertex);
//	}
//
//	std::vector<u32> indices{};
//	indices.reserve(static_cast<std::size_t>(assimpMesh->mNumFaces) * 3);
//	for (std::size_t i = 0; i < assimpMesh->mNumFaces; ++i)
//	{
//		const aiFace& face = assimpMesh->mFaces[i];
//		for (std::size_t j = 0; j < face.mNumIndices; ++j)
//		{
//			indices.push_back(face.mIndices[j]);
//		}
//	}
//
//	std::vector<Texture> textures{};
//	if (assimpScene->mNumMaterials > 0)
//	{
//		aiMaterial* material = assimpScene->mMaterials[assimpMesh->mMaterialIndex];
//		std::vector<Texture> diffuseMaps = LoadMaterialTextures(material, TextureType::Diffuse);
//		textures.insert(textures.end(),
//			std::make_move_iterator(diffuseMaps.begin()),
//			std::make_move_iterator(diffuseMaps.end()));
//
//		std::vector<Texture> specularMaps = LoadMaterialTextures(material, TextureType::Specular);
//		textures.insert(textures.end(),
//			std::make_move_iterator(specularMaps.begin()),
//			std::make_move_iterator(specularMaps.end()));
//
//		std::vector<Texture> normalMaps = LoadMaterialTextures(material, TextureType::Normal);
//		textures.insert(textures.end(),
//			std::make_move_iterator(normalMaps.begin()),
//			std::make_move_iterator(normalMaps.end()));
//	}
//
//	Mesh mesh;
//	mesh.Init(std::move(vertices), std::move(indices), std::move(textures));
//
//	return mesh;
//}
//
//std::vector<stw::Texture> stw::Model::LoadMaterialTextures(const aiMaterial* material,
//	const TextureType textureType) const
//{
//	const aiTextureType assimpType = ToAssimpTextureType(textureType);
//	const auto textureCount = material->GetTextureCount(assimpType);
//	std::vector<Texture> textures{};
//
//	for (std::size_t i = 0; i < textureCount; i++)
//	{
//		Timer timer;
//		timer.Start();
//
//		aiString str;
//		material->GetTexture(assimpType, static_cast<u32>(i), &str);
//
//		std::filesystem::path texturePath = m_Directory / str.C_Str();
//		if (s_LoadedTextures.contains(texturePath))
//		{
//			continue;
//		}
//
//		auto loadResult = Texture::LoadFromPath(texturePath, textureType, TextureSpace::Srgb);
//		if (!loadResult.has_value())
//		{
//			spdlog::error(loadResult.error());
//			continue;
//		}
//
//		s_LoadedTextures.insert(texturePath);
//		textures.push_back(std::move(loadResult.value()));
//
//		spdlog::info("Loaded texture {} in {:0.0f} ms",
//			texturePath.string(),
//			timer.GetElapsedTime().GetInMilliseconds());
//	}
//
//	return textures;
//}
