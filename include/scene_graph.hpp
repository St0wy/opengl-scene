//
// Created by stowy on 30/06/2023.
//

#pragma once

#include <cstddef>
#include <functional>
#include <glm/mat4x4.hpp>
#include <optional>
#include <span>
#include <vector>

namespace stw
{
struct SceneGraphElement
{
	std::size_t meshId{};
	std::size_t materialId{};
	glm::mat4 localTransformMatrix{ 1.0f };
	glm::mat4 parentTransformMatrix{ 1.0f };
	// TODO : dirty flag ?
};

// This is a type used for caching when iterating over the scene graph
struct SceneGraphElementIndex
{
	std::size_t meshId{};
	std::size_t materialId{};

	bool operator==(const SceneGraphElementIndex& other) const;
};

struct SceneGraphNode
{
	std::size_t elementId{};
	std::optional<std::size_t> parentId{};
	std::optional<std::size_t> childId{};
	std::optional<std::size_t> siblingId{};
};

class SceneGraph
{
public:
	SceneGraph() = default;

	void Init();
	void AddElementToRoot(std::size_t meshId, std::size_t materialId, const glm::mat4& transformMatrix);
	[[maybe_unused]] [[nodiscard]] std::span<const SceneGraphElement> GetElements() const;
	[[maybe_unused]] [[nodiscard]] std::span<const SceneGraphNode> GetNodes() const;
	[[maybe_unused]] std::vector<SceneGraphElement>& GetElements();
	[[maybe_unused]] std::vector<SceneGraphNode>& GetNodes();

	/// Will call `function` for each elements in the scene graph with the correct transform matrix.
	/// \param function Function that will be called on each elements. Has these parameters :
	/// `void(SceneGraphElementIndex elementIndex, std::span<const glm::mat4> transformMatrices)`.
	void ForEach(const std::function<void(SceneGraphElementIndex, std::span<const glm::mat4>)>& function);

	/// Will call `function` for each elements in the scene graph with the correct transform matrix.
	/// \param function Function that will be called on each elements. Has these parameters :
	/// `void(SceneGraphElementIndex elementIndex, const glm::mat4& transformMatrix)`.
	[[maybe_unused]] void ForEachNoInstancing(
		const std::function<void(SceneGraphElementIndex, const glm::mat4&)>& function);

private:
	std::vector<SceneGraphElement> m_Elements{};
	std::vector<SceneGraphNode> m_Nodes{};
};
}// namespace stw

template<>
struct std::hash<stw::SceneGraphElementIndex>
{
	std::size_t operator()(const stw::SceneGraphElementIndex& sceneGraphElementIndex) const noexcept
	{
		const std::size_t h1 = std::hash<std::size_t>{}(sceneGraphElementIndex.meshId);
		const std::size_t h2 = std::hash<std::size_t>{}(sceneGraphElementIndex.materialId);
		return h1 ^ (h2 << 1);
	}
};
