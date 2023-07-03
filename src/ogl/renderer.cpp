#include "ogl/renderer.hpp"

#include <GL/glew.h>
#include <glm/gtc/type_ptr.hpp>
#include <queue>
#include <spdlog/spdlog.h>
#include <unordered_set>

#include "model.hpp"
#include "timer.hpp"
#include "utils.hpp"

stw::Renderer::~Renderer()
{
	if (m_IsInitialized)
	{
		spdlog::error("Destructor called on renderer that is still initialized");
	}
}

void stw::Renderer::Init()
{
	m_IsInitialized = true;
	m_MatricesUniformBuffer.Init(0);
	m_MatricesUniformBuffer.Bind();
	constexpr GLsizeiptr matricesSize = 2 * sizeof(glm::mat4);
	m_MatricesUniformBuffer.Allocate(matricesSize);
	m_SceneGraph.Init();
}

void stw::Renderer::SetEnableMultisample(const bool enableMultisample)
{
	SetOpenGlCapability(enableMultisample, GL_MULTISAMPLE, m_EnableMultisample);
}

void stw::Renderer::SetEnableDepthTest(const bool enableDepthTest)
{
	SetOpenGlCapability(enableDepthTest, GL_DEPTH_TEST, m_EnableDepthTest);
}

void stw::Renderer::SetDepthFunc(const GLenum depthFunction)
{
	m_DepthFunction = depthFunction;
	GLCALL(glDepthFunc(depthFunction));
}

void stw::Renderer::SetEnableCullFace(const bool enableCullFace)
{
	SetOpenGlCapability(enableCullFace, GL_CULL_FACE, m_EnableCullFace);
}

[[maybe_unused]] void stw::Renderer::SetCullFace(const GLenum cullFace)
{
	m_CullFace = cullFace;
	GLCALL(glCullFace(cullFace));
}

[[maybe_unused]] void stw::Renderer::SetFrontFace(const GLenum frontFace)
{
	m_FrontFace = frontFace;
	GLCALL(glFrontFace(m_FrontFace));
}

[[maybe_unused]] void stw::Renderer::SetClearColor(const glm::vec4& clearColor)
{
	m_ClearColor = clearColor;
	GLCALL(glClearColor(clearColor.r, clearColor.g, clearColor.b, clearColor.a));
}

void stw::Renderer::SetProjectionMatrix(const glm::mat4& projection) const
{
	assert(m_IsInitialized);
	m_MatricesUniformBuffer.Bind();
	m_MatricesUniformBuffer.SetSubData(0, sizeof(glm::mat4), value_ptr(projection));
	m_MatricesUniformBuffer.UnBind();
}

void stw::Renderer::SetViewMatrix(const glm::mat4& view) const
{
	assert(m_IsInitialized);
	m_MatricesUniformBuffer.Bind();
	m_MatricesUniformBuffer.SetSubData(sizeof(glm::mat4), sizeof(glm::mat4), value_ptr(view));
	m_MatricesUniformBuffer.UnBind();
}

void stw::Renderer::SetViewport(const glm::ivec2 pos, const glm::uvec2 size) const
{
	GLCALL(glViewport(pos.x, pos.y, static_cast<GLsizei>(size.x), static_cast<GLsizei>(size.y)));
}

void stw::Renderer::Clear(const GLbitfield mask) { GLCALL(glClear(mask)); }

void stw::Renderer::DrawScene()
{
	m_SceneGraph.ForEach([&](std::size_t meshId, std::size_t materialId, const glm::mat4& transformMatrix) {
		m_MatricesUniformBuffer.Bind();
		auto& material = m_MaterialManager[materialId];

		BindMaterial(material, m_TextureManager);
		auto& mesh = m_Meshes[meshId];
		mesh.Bind({ &transformMatrix, 1 });

		const auto size = static_cast<GLsizei>(mesh.GetIndicesSize());
		GLCALL(glDrawElementsInstanced(GL_TRIANGLES, size, GL_UNSIGNED_INT, nullptr, 1));

		mesh.UnBind();
		m_MatricesUniformBuffer.UnBind();
	});
}

void stw::Renderer::Delete()
{
	m_IsInitialized = false;
	m_MatricesUniformBuffer.Delete();
	m_TextureManager.Delete();

	for (auto& mesh : m_Meshes)
	{
		mesh.Delete();
	}
}

void stw::Renderer::SetOpenGlCapability(const bool enabled, const GLenum capability, bool& field)
{
	field = enabled;
	if (field)
	{
		GLCALL(glEnable(capability));
	}
	else
	{
		GLCALL(glDisable(capability));
	}
}

[[maybe_unused]] stw::TextureManager& stw::Renderer::GetTextureManager() { return m_TextureManager; }

std::optional<std::string> stw::Renderer::LoadModel(const std::filesystem::path& path, Pipeline& pipeline)
{
	Assimp::Importer importer;
	constexpr u32 assimpImportFlags = aiProcessPreset_TargetRealtime_Fast;

	const auto pathString = path.string();

	Timer timer;
	timer.Start();

	const aiScene* assimpScene = importer.ReadFile(pathString.c_str(), assimpImportFlags);

	if (!assimpScene || assimpScene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !assimpScene->mRootNode)
	{
		return { importer.GetErrorString() };
	}

	spdlog::info("Imported model {} in {:0.0f} ms", pathString, timer.GetElapsedTime().GetInMilliseconds());

	auto workingDirectory = path.parent_path();
	const std::size_t materialIndexOffset =
		m_MaterialManager.LoadMaterialsFromAssimpScene(assimpScene, workingDirectory, m_TextureManager, pipeline);

	m_Meshes.reserve(m_Meshes.size() + assimpScene->mNumMeshes);

	std::queue<aiNode*> assimpNodes;
	assimpNodes.push(assimpScene->mRootNode);
	while (!assimpNodes.empty())
	{
		aiNode* currentAssimpNode = assimpNodes.front();
		assimpNodes.pop();

		for (std::size_t i = 0; i < currentAssimpNode->mNumMeshes; ++i)
		{
			aiMesh* assimpMesh = assimpScene->mMeshes[currentAssimpNode->mMeshes[i]];

			if (assimpMesh->mMaterialIndex == 0)
			{
				continue;
			}

			auto [mesh, meshMaterialIndex] = ProcessMesh(assimpMesh, materialIndexOffset);

			m_Meshes.push_back(std::move(mesh));

			// TODO: Take in account parents and children (right now there's only one level of nodes)
			glm::mat4 transformMatrix = ConvertMatAssimpToGlm(currentAssimpNode->mTransformation);
			m_SceneGraph.AddElementToRoot(m_Meshes.size() - 1, meshMaterialIndex, transformMatrix);
		}

		for (std::size_t i = 0; i < currentAssimpNode->mNumChildren; ++i)
		{
			assimpNodes.push(currentAssimpNode->mChildren[i]);
		}
	}

	spdlog::info("Converted model {} in {:0.0f} ms", pathString, timer.GetElapsedTime().GetInMilliseconds());

	return {};
}

stw::ProcessMeshResult stw::Renderer::ProcessMesh(aiMesh* assimpMesh, std::size_t materialIndexOffset)
{
	std::vector<Vertex> vertices{};
	vertices.reserve(assimpMesh->mNumVertices);
	for (std::size_t i = 0; i < assimpMesh->mNumVertices; ++i)
	{
		const aiVector3D meshVertex = assimpMesh->mVertices[i];
		const aiVector3D meshNormal = assimpMesh->mNormals[i];
		const aiVector3D meshTangent = assimpMesh->mTangents[i];

		glm::vec2 textureCoords(0.0f);
		if (assimpMesh->mTextureCoords[0])
		{
			const auto meshTextureCoords = assimpMesh->mTextureCoords[0][i];
			textureCoords.x = meshTextureCoords.x;
			textureCoords.y = meshTextureCoords.y;
		}

		Vertex vertex{ {
						   meshVertex.x,
						   meshVertex.y,
						   meshVertex.z,
					   },
			{
				meshNormal.x,
				meshNormal.y,
				meshNormal.z,
			},
			textureCoords,
			{ meshTangent.x, meshTangent.y, meshTangent.z } };
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

	Mesh mesh;
	std::size_t materialIndex = materialIndexOffset + assimpMesh->mMaterialIndex - 1;
	mesh.Init(std::move(vertices), std::move(indices));

	return { std::move(mesh), materialIndex };
}
