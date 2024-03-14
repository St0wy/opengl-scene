/**
 * @file scene_graph.cpp
 * @author Fabian Huber (fabian.hbr@protonmail.ch)
 * @brief Contains the Scene Graph of the project.
 * @version 1.0
 * @date 30/06/2023
 *
 * @copyright SAE (c) 2023
 *
 */

module;

#include "glm/detail/_noise.hpp"

#include <cstddef>
#include <optional>
#include <span>
#include <vector>

#include <absl/container/flat_hash_map.h>
#include <glm/ext/matrix_transform.hpp>
#include <glm/mat4x4.hpp>
#include <spdlog/spdlog.h>

export module scene_graph;

import utils;
import consts;
import number_types;

export namespace stw
{
struct SceneGraphElement
{
	std::size_t meshId = InvalidId;
	std::size_t materialId = InvalidId;
	glm::mat4 localTransformMatrix{ 1.0f };
	glm::mat4 parentTransformMatrix{ 1.0f };
};

// This is a type used for caching when iterating over the scene graph
struct SceneGraphElementIndex
{
	std::size_t meshId = InvalidId;
	std::size_t materialId = InvalidId;

	bool operator==(const SceneGraphElementIndex& other) const;
};
}// namespace stw

export template<>
struct std::hash<stw::SceneGraphElementIndex>
{
	std::size_t operator()(const stw::SceneGraphElementIndex& sceneGraphElementIndex) const noexcept
	{
		const std::size_t h1 = std::hash<std::size_t>{}(sceneGraphElementIndex.meshId);
		const std::size_t h2 = std::hash<std::size_t>{}(sceneGraphElementIndex.materialId);
		return h1 ^ (h2 << 1);
	}
};


export namespace stw
{
struct SceneGraphNode
{
	std::size_t elementId = InvalidId;
	std::optional<std::size_t> parentId{};
	std::optional<std::size_t> childId{};
	std::optional<std::size_t> siblingId{};
};

class SceneGraph
{
public:
	void Init();
	usize AddElementToRoot(std::size_t meshId, std::size_t materialId, const glm::mat4& transformMatrix);
	usize AddChild(usize parentId, std::size_t meshId, std::size_t materialId, const glm::mat4& transformMatrix);
	usize AddSibling(usize siblingId, std::size_t meshId, std::size_t materialId, const glm::mat4& transformMatrix);
	[[maybe_unused]] [[nodiscard]] std::span<const SceneGraphElement> GetElements() const;
	[[maybe_unused]] [[nodiscard]] std::span<const SceneGraphNode> GetNodes() const;

	void TranslateElement(std::size_t nodeIndex, const glm::vec3& translation);
	void RotateElement(std::size_t nodeIndex, f32 angle, const glm::vec3& axis);
	void ScaleElement(std::size_t nodeIndex, const glm::vec3& scale);

	/// Will call `function` for each elements in the scene graph with the correct transform matrix.
	/// \param function Function that will be called on each elements. Has these parameters :
	/// `void(SceneGraphElementIndex elementIndex, std::span<const glm::mat4> transformMatrices)`.
	void ForEach(Consumer<SceneGraphElementIndex, std::span<const glm::mat4>> auto&& function)
	{
		absl::flat_hash_map<SceneGraphElementIndex, std::vector<glm::mat4>> instancingMap{};

		const auto lambda = [&instancingMap](const SceneGraphElement& element) {
			if (element.materialId == InvalidId || element.meshId == InvalidId)
			{
				return;
			}

			const glm::mat4 transform = element.parentTransformMatrix * element.localTransformMatrix;
			const SceneGraphElementIndex index{ element.meshId, element.materialId };

			instancingMap[index].push_back(transform);
		};

		ForEachChildren(m_Nodes[0], lambda);

		for (const auto& [elementIndex, transforms] : instancingMap)
		{
			std::invoke(function, elementIndex, transforms);
		}
	}

	/// Will call `function` for each elements in the scene graph with the correct transform matrix.
	/// \param function Function that will be called on each elements. Has these parameters :
	/// `void(SceneGraphElementIndex elementIndex, const glm::mat4& transformMatrix)`.
	[[maybe_unused]] void ForEachNoInstancing(Consumer<SceneGraphElementIndex, glm::mat4> auto&& function);

private:
	std::vector<SceneGraphElement> m_Elements{};
	std::vector<SceneGraphNode> m_Nodes{};

	void ForEachChildren(const SceneGraphNode& startNode, Consumer<const SceneGraphElement&> auto&& function);
	void DispatchTransforms(SceneGraphNode& currentNode);
};

void SceneGraph::ForEachNoInstancing(Consumer<SceneGraphElementIndex, glm::mat4> auto&& function)
{
	const auto lambda = [&function](SceneGraphElement& element) {
		if (element.materialId == InvalidId || element.meshId == InvalidId)
		{
			return;
		}

		const glm::mat4 transform = element.parentTransformMatrix * element.localTransformMatrix;
		std::invoke(function, { element.meshId, element.materialId }, transform);
	};
	ForEachChildren(m_Nodes[0], lambda);
}

void SceneGraph::ForEachChildren(const SceneGraphNode& startNode, Consumer<const SceneGraphElement&> auto&& function)
{
	if (!startNode.childId)
	{
		return;
	}

	std::vector<std::size_t> nodes;
	nodes.reserve(m_Nodes.size());
	nodes.push_back(startNode.childId.value());
	while (!nodes.empty())
	{
		const auto currentNode = nodes.back();
		nodes.pop_back();
		auto& node = m_Nodes[currentNode];
		auto& element = m_Elements[node.elementId];

		std::invoke(function, element);

		if (node.childId)
		{
			nodes.push_back(node.childId.value());
		}

		if (node.siblingId)
		{
			nodes.push_back(node.siblingId.value());
		}
	}
}

usize SceneGraph::AddElementToRoot(std::size_t meshId, std::size_t materialId, const glm::mat4& transformMatrix)
{
	m_Elements.emplace_back(meshId, materialId, transformMatrix, m_Elements[0].localTransformMatrix);
	const std::size_t elementIndex = m_Elements.size() - 1;
	m_Nodes.emplace_back(elementIndex, 0, std::nullopt, std::nullopt);
	const std::size_t newNodeIndex = m_Nodes.size() - 1;
	auto& rootNode = m_Nodes[0];

	// Find closest sibling
	if (!rootNode.childId.has_value())
	{
		rootNode.childId.emplace(newNodeIndex);
	}
	else
	{
		std::optional<std::size_t> nextNodeIndex = rootNode.childId;
		std::size_t currentNodeIndex = 0;
		while (nextNodeIndex.has_value())
		{
			currentNodeIndex = nextNodeIndex.value();
			const auto& node = m_Nodes[currentNodeIndex];
			nextNodeIndex = node.siblingId;
		}

		m_Nodes[currentNodeIndex].siblingId = newNodeIndex;
	}

	return m_Nodes.size() - 1;
}

[[maybe_unused]] std::span<const SceneGraphElement> SceneGraph::GetElements() const { return m_Elements; }

[[maybe_unused]] std::span<const SceneGraphNode> SceneGraph::GetNodes() const { return m_Nodes; }

void SceneGraph::Init()
{
	m_Elements.emplace_back(InvalidId, InvalidId, glm::mat4{ 1.0f }, glm::mat4{ 1.0f });
	m_Nodes.emplace_back(m_Elements.size() - 1, std::nullopt, std::nullopt, std::nullopt);
}

void SceneGraph::TranslateElement(const std::size_t nodeIndex, const glm::vec3& translation)
{
	auto& node = m_Nodes[nodeIndex];
	auto& element = m_Elements[node.elementId];
	element.localTransformMatrix = translate(element.localTransformMatrix, translation);

	DispatchTransforms(node);
}

void SceneGraph::RotateElement(const std::size_t nodeIndex, const f32 angle, const glm::vec3& axis)
{
	auto& node = m_Nodes[nodeIndex];
	auto& element = m_Elements[node.elementId];
	element.localTransformMatrix = rotate(element.localTransformMatrix, angle, axis);

	DispatchTransforms(node);
}

void SceneGraph::ScaleElement(const std::size_t nodeIndex, const glm::vec3& scale)
{
	auto& node = m_Nodes[nodeIndex];
	auto& element = m_Elements[node.elementId];
	element.localTransformMatrix = glm::scale(element.localTransformMatrix, scale);

	DispatchTransforms(node);
}

void SceneGraph::DispatchTransforms(SceneGraphNode& currentNode)
{
	if (!currentNode.childId)
	{
		return;
	}

	const auto& currentElement = m_Elements[currentNode.elementId];

	std::vector<glm::mat4> parentMatricesStack;
	parentMatricesStack.push_back(currentElement.parentTransformMatrix * currentElement.localTransformMatrix);

	// The boolean is to check if the node is one that should pop the parentMatricesStack stack
	std::vector<std::pair<bool, usize>> nodes;
	nodes.emplace_back(true, currentNode.childId.value());
	while (!nodes.empty())
	{
		const auto top = nodes.back();
		auto& node = m_Nodes[top.second];
		nodes.pop_back();
		auto& element = m_Elements[node.elementId];

		element.parentTransformMatrix = parentMatricesStack.back();

		if (top.first)
		{
			parentMatricesStack.pop_back();
		}

		if (node.siblingId)
		{
			const usize sibling = node.siblingId.value();
			if (m_Nodes[sibling].siblingId)
			{
				nodes.emplace_back(false, sibling);
			}
			else
			{
				nodes.emplace_back(true, sibling);
			}
		}

		if (m_Nodes[top.second].childId)
		{
			parentMatricesStack.push_back(element.parentTransformMatrix * element.localTransformMatrix);
			nodes.emplace_back(false, m_Nodes[top.second].childId.value());
		}
	}
}

usize SceneGraph::AddChild(
	usize parentId, const std::size_t meshId, const std::size_t materialId, const glm::mat4& transformMatrix)
{
	const auto& parentNode = m_Nodes[parentId];
	assert(!parentNode.childId.has_value());
	const auto& parentElement = m_Elements[parentNode.elementId];

	const glm::mat4 parentTransform = parentElement.parentTransformMatrix * parentElement.localTransformMatrix;
	const SceneGraphElement elem{ meshId, materialId, transformMatrix, parentTransform };
	m_Elements.push_back(elem);
	const SceneGraphNode node{ m_Elements.size() - 1, parentId, std::nullopt, std::nullopt };
	m_Nodes.push_back(node);
	m_Nodes[parentId].childId = m_Nodes.size() - 1;

	return m_Nodes.size() - 1;
}

usize SceneGraph::AddSibling(
	const usize siblingId, const std::size_t meshId, const std::size_t materialId, const glm::mat4& transformMatrix)
{
	const auto& siblingNode = m_Nodes[siblingId];
	assert(!siblingNode.siblingId.has_value());

	const auto& siblingElement = m_Elements[siblingNode.elementId];

	const glm::mat4 siblingTransform = siblingElement.parentTransformMatrix * siblingElement.localTransformMatrix;
	const SceneGraphElement elem{ meshId, materialId, transformMatrix, siblingTransform };
	m_Elements.push_back(elem);
	const SceneGraphNode node{ m_Elements.size() - 1, std::nullopt, std::nullopt, std::nullopt };
	m_Nodes.push_back(node);
	m_Nodes[siblingId].siblingId = m_Nodes.size() - 1;

	return m_Nodes.size() - 1;
}

bool SceneGraphElementIndex::operator==(const SceneGraphElementIndex& other) const
{
	return meshId == other.meshId && materialId == other.materialId;
}
}// namespace stw
