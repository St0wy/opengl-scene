//
// Created by stowy on 30/06/2023.
//
#include "scene_graph.hpp"

#include <queue>
#include <spdlog/spdlog.h>

void stw::SceneGraph::AddElementToRoot(std::size_t meshId, std::size_t materialId, const glm::mat4& transformMatrix)
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

		m_Nodes[currentNodeIndex].siblingId.emplace(newNodeIndex);
	}
}
std::span<const stw::SceneGraphElement> stw::SceneGraph::GetElements() const { return m_Elements; }
std::span<const stw::SceneGraphNode> stw::SceneGraph::GetNodes() const { return m_Nodes; }

std::vector<stw::SceneGraphElement>& stw::SceneGraph::GetElements() { return m_Elements; }
std::vector<stw::SceneGraphNode>& stw::SceneGraph::GetNodes() { return m_Nodes; }

void stw::SceneGraph::ForEach(const std::function<void(std::size_t, std::size_t, const glm::mat4&)>& function)
{
	// TODO : Handle instancing
	std::optional<std::size_t> nextNode = m_Nodes[0].childId;
	while (nextNode.has_value())
	{
		auto& node = m_Nodes[nextNode.value()];
		auto& element = m_Elements[node.elementId];
		glm::mat4 transform = element.parentTransformMatrix * element.localTransformMatrix;
		function(element.meshId, element.materialId, transform);

		if (node.childId.has_value())
		{
			nextNode = node.childId;
		}
		else
		{
			nextNode = node.siblingId;
		}
	}
}

void stw::SceneGraph::Init()
{
	m_Elements.emplace_back(0, 0, glm::mat4{ 1.0f }, glm::mat4{ 1.0f });
	m_Nodes.emplace_back(m_Elements.size() - 1, std::nullopt, std::nullopt, std::nullopt);
}
