//
// Created by stowy on 30/06/2023.
//

#pragma once

#include <cstddef>
#include <optional>
#include <span>
#include <vector>
#include <absl/container/flat_hash_map.h>
#include <glm/mat4x4.hpp>

#include "consts.hpp"
#include "number_types.hpp"
#include "utils.hpp"

namespace stw
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
	SceneGraph() = default;

	void Init();
	usize AddElementToRoot(std::size_t meshId, std::size_t materialId, const glm::mat4& transformMatrix);
	usize AddChild(usize parentId, std::size_t meshId, std::size_t materialId, const glm::mat4& transformMatrix);
	usize AddSibling(usize siblingId, std::size_t meshId, std::size_t materialId, const glm::mat4& transformMatrix);
	[[maybe_unused]] [[nodiscard]] std::span<const SceneGraphElement> GetElements() const;
	[[maybe_unused]] [[nodiscard]] std::span<const SceneGraphNode> GetNodes() const;

	void TranslateElement(std::size_t nodeIndex, glm::vec3 translation);
	void RotateElement(std::size_t nodeIndex, f32 angle, glm::vec3 axis);
	void ScaleElement(std::size_t nodeIndex, glm::vec3 scale);

	/// Will call `function` for each elements in the scene graph with the correct transform matrix.
	/// \param function Function that will be called on each elements. Has these parameters :
	/// `void(SceneGraphElementIndex elementIndex, std::span<const glm::mat4> transformMatrices)`.
	void ForEach(auto&& function);

	/// Will call `function` for each elements in the scene graph with the correct transform matrix.
	/// \param function Function that will be called on each elements. Has these parameters :
	/// `void(SceneGraphElementIndex elementIndex, const glm::mat4& transformMatrix)`.
	[[maybe_unused]] void ForEachNoInstancing(auto&& function);

private:
	std::vector<SceneGraphElement> m_Elements{};
	std::vector<SceneGraphNode> m_Nodes{};

	void ForEachChildren(const SceneGraphNode& startNode, auto&& function);
	void DispatchTransforms(SceneGraphNode& currentNode);
};

void SceneGraph::ForEach(auto&& function)
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

void SceneGraph::ForEachNoInstancing(auto&& function)
{
	ForEachChildren(m_Nodes[0],
		[&function](SceneGraphElement& element) {
			if (element.materialId == InvalidId || element.meshId == InvalidId)
			{
				return;
			}

			const glm::mat4 transform = element.parentTransformMatrix * element.localTransformMatrix;
			std::invoke(function, { element.meshId, element.materialId }, transform);
		});
}

void SceneGraph::ForEachChildren(const SceneGraphNode& startNode, auto&& function)
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
