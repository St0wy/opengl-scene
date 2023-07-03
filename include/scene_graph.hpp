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
	glm::mat4 localTransformMatrix{1.0f};
	glm::mat4 parentTransformMatrix{1.0f};
	// TODO : dirty flag ?
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
	[[nodiscard]] std::span<const SceneGraphElement> GetElements() const;
	[[nodiscard]] std::span<const SceneGraphNode> GetNodes() const;
	std::vector<SceneGraphElement>& GetElements();
	std::vector<SceneGraphNode>& GetNodes();

	/// Will call `function` for each elements in the scene graph with the correct transform matrix.
	/// \param function Function that will be called on each elements. Has these parameters :
	/// `void(std::size_t meshId, std::size_t materialId, const glm::mat4& transformMatrix)`.
	void ForEach(const std::function<void(std::size_t, std::size_t, const glm::mat4&)>& function);

private:
	std::vector<SceneGraphElement> m_Elements{};
	std::vector<SceneGraphNode> m_Nodes{};
};
}// namespace stw
