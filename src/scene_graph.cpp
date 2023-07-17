//
// Created by stowy on 30/06/2023.
//

#include "scene_graph.hpp"

#include <absl/container/flat_hash_map.h>
#include <glm/ext/matrix_transform.hpp>
#include <glm/mat4x4.hpp>
#include <queue>
#include <spdlog/spdlog.h>
#include <unordered_map>

#include "number_types.hpp"

usize stw::SceneGraph::AddElementToRoot(std::size_t meshId, std::size_t materialId, const glm::mat4& transformMatrix)
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
		std::optional<std::size_t> nextNodeIndex = rootNode.childId.value();
		std::size_t currentNodeIndex = 0;
		while (nextNodeIndex.has_value())
		{
			currentNodeIndex = nextNodeIndex.value();
			auto& node = m_Nodes[currentNodeIndex];
			nextNodeIndex = node.siblingId;
		}

		m_Nodes[currentNodeIndex].siblingId = newNodeIndex;
	}

	return m_Nodes.size() - 1;
}
[[maybe_unused]] std::span<const stw::SceneGraphElement> stw::SceneGraph::GetElements() const { return m_Elements; }
[[maybe_unused]] std::span<const stw::SceneGraphNode> stw::SceneGraph::GetNodes() const { return m_Nodes; }

void stw::SceneGraph::ForEach(const std::function<void(SceneGraphElementIndex, std::span<const glm::mat4>)>& function)
{
	absl::flat_hash_map<SceneGraphElementIndex, std::vector<glm::mat4>> instancingMap{};

	ForEachChildren(m_Nodes[0], [&instancingMap](SceneGraphElement& element) {
		if (element.materialId == InvalidId || element.meshId == InvalidId)
		{
			return;
		}

		const glm::mat4 transform = element.parentTransformMatrix * element.localTransformMatrix;
		const SceneGraphElementIndex index{ element.meshId, element.materialId };

		instancingMap[index].push_back(transform);
	});

	for (const auto& [elementIndex, transforms] : instancingMap)
	{
		function(elementIndex, transforms);
	}
}

void stw::SceneGraph::Init()
{
	m_Elements.emplace_back(InvalidId, InvalidId, glm::mat4{ 1.0f }, glm::mat4{ 1.0f });
	m_Nodes.emplace_back(m_Elements.size() - 1, std::nullopt, std::nullopt, std::nullopt);
}

[[maybe_unused]] void stw::SceneGraph::ForEachNoInstancing(
	const std::function<void(SceneGraphElementIndex, const glm::mat4&)>& function)
{
	ForEachChildren(m_Nodes[0], [&function](SceneGraphElement& element) {
		if (element.materialId == InvalidId || element.meshId == InvalidId)
		{
			return;
		}

		const glm::mat4 transform = element.parentTransformMatrix * element.localTransformMatrix;
		function({ element.meshId, element.materialId }, transform);
	});
}

void stw::SceneGraph::TranslateElement(std::size_t nodeIndex, glm::vec3 translation)
{
	auto& node = m_Nodes[nodeIndex];
	auto& element = m_Elements[node.elementId];
	element.localTransformMatrix = glm::translate(element.localTransformMatrix, translation);

	DispatchTransforms(node);
}

void stw::SceneGraph::RotateElement(std::size_t nodeIndex, f32 angle, glm::vec3 axis)
{
	auto& node = m_Nodes[nodeIndex];
	auto& element = m_Elements[node.elementId];
	element.localTransformMatrix = glm::rotate(element.localTransformMatrix, angle, axis);

	DispatchTransforms(node);
}

void stw::SceneGraph::ScaleElement(std::size_t nodeIndex, glm::vec3 scale)
{
	auto& node = m_Nodes[nodeIndex];
	auto& element = m_Elements[node.elementId];
	element.localTransformMatrix = glm::scale(element.localTransformMatrix, scale);

	DispatchTransforms(node);
}

void stw::SceneGraph::ForEachChildren(
	const stw::SceneGraphNode& startNode, const std::function<void(SceneGraphElement&)>& function)
{
	if (!startNode.childId)
	{
		return;
	}

	std::queue<std::size_t> nodes;
	nodes.push(startNode.childId.value());
	while (!nodes.empty())
	{
		const auto currentNode = nodes.front();
		nodes.pop();
		auto& node = m_Nodes[currentNode];
		auto& element = m_Elements[node.elementId];
		function(element);

		if (node.childId)
		{
			nodes.push(node.childId.value());
		}

		if (node.siblingId)
		{
			nodes.push(node.siblingId.value());
		}
	}
}

void stw::SceneGraph::DispatchTransforms(stw::SceneGraphNode& currentNode)
{
	if (!currentNode.childId)
	{
		return;
	}

	auto& currentElement = m_Elements[currentNode.elementId];

	std::vector<glm::mat4> parentMatricesStack;
	parentMatricesStack.push_back(currentElement.parentTransformMatrix * currentElement.localTransformMatrix);

	// The boolean is to check if the node is one that should pop the parentMatricesStack stack
	std::vector<std::pair<bool, usize>> nodes;
	nodes.emplace_back(true, currentNode.childId.value());
	while (!nodes.empty())
	{
		auto top = nodes.back();
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
usize stw::SceneGraph::AddChild(
	usize parentId, std::size_t meshId, std::size_t materialId, const glm::mat4& transformMatrix)
{
	auto& parentNode = m_Nodes[parentId];
	assert(!parentNode.childId.has_value());
	auto& parentElement = m_Elements[parentNode.elementId];

	const glm::mat4 parentTransform = parentElement.parentTransformMatrix * parentElement.localTransformMatrix;
	const SceneGraphElement elem{ meshId, materialId, transformMatrix, parentTransform };
	m_Elements.push_back(elem);
	const SceneGraphNode node{ m_Elements.size() - 1, parentId, std::nullopt, std::nullopt };
	m_Nodes.push_back(node);
	m_Nodes[parentId].childId = m_Nodes.size() - 1;

	return m_Nodes.size() - 1;
}
usize stw::SceneGraph::AddSibling(
	usize siblingId, std::size_t meshId, std::size_t materialId, const glm::mat4& transformMatrix)
{
	auto& siblingNode = m_Nodes[siblingId];
	assert(!siblingNode.siblingId.has_value());

	auto& siblingElement = m_Elements[siblingNode.elementId];

	const glm::mat4 siblingTransform = siblingElement.parentTransformMatrix * siblingElement.localTransformMatrix;
	const SceneGraphElement elem{ meshId, materialId, transformMatrix, siblingTransform };
	m_Elements.push_back(elem);
	const SceneGraphNode node{ m_Elements.size() - 1, std::nullopt, std::nullopt, std::nullopt };
	m_Nodes.push_back(node);
	m_Nodes[siblingId].siblingId = m_Nodes.size() - 1;

	return m_Nodes.size() - 1;
}

bool stw::SceneGraphElementIndex::operator==(const stw::SceneGraphElementIndex& other) const
{
	return meshId == other.meshId && materialId == other.materialId;
}
