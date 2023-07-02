//
// Created by stowy on 30/06/2023.
//

#pragma once

#include <cstddef>
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
	glm::mat4 localTransformMatrix{};
	glm::mat4 parentTransformMatrix{};
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
	SceneGraph(std::vector<SceneGraphElement> elements, std::vector<SceneGraphNode> nodes);

	void AddElement(std::size_t meshId, std::size_t materialId, const glm::mat4& transformMatrix);
	[[nodiscard]] std::span<const SceneGraphElement> GetElements() const;
	[[nodiscard]] std::span<const SceneGraphNode> GetNodes() const;

private:
	std::vector<SceneGraphElement> m_Elements{};
	std::vector<SceneGraphNode> m_Nodes{};
};
}// namespace stw
