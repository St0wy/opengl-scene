//
// Created by stowy on 30/06/2023.
//
#include "scene_graph.hpp"

stw::SceneGraph::SceneGraph(std::vector<SceneGraphElement> elements, std::vector<SceneGraphNode> nodes)
	: m_Elements(std::move(elements)), m_Nodes(std::move(nodes))
{}

void stw::SceneGraph::AddElement(std::size_t meshId, std::size_t materialId, const glm::mat4& transformMatrix)
{
	m_Elements.emplace_back(meshId, materialId, transformMatrix, glm::mat4{});
	std::size_t elementIndex = m_Elements.size() - 1;
	m_Nodes.emplace_back(elementIndex, std::nullopt, std::nullopt, std::nullopt);
}
std::span<const stw::SceneGraphElement> stw::SceneGraph::GetElements() const { return m_Elements; }
std::span<const stw::SceneGraphNode> stw::SceneGraph::GetNodes() const { return m_Nodes; }
