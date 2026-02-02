#include "DungeonGraph/CustomGraph.h"
#include "DungeonGraph/CustomNode.h" // Include the definition for UCustomNode

UCustomGraph::UCustomGraph()
{
    // Default Constructor
}

void UCustomGraph::AddNode(UCustomNode* Node)
{
    if (Node && !Nodes.Contains(Node)) // Ensure the node is not null or already in the list
    {
        Nodes.Add(Node);
    }
}

void UCustomGraph::RemoveNode(UCustomNode* Node)
{
    if (Node)
    {
        // Remove any edges related to this node
        Edges.RemoveAll([Node](const FCustomEdge& Edge) {
            return Edge.StartNode == Node || Edge.EndNode == Node;
        });

        // Remove the node from the graph
        Nodes.Remove(Node);
    }
}

void UCustomGraph::AddEdge(UCustomNode* StartNode, UCustomNode* EndNode)
{
    if (StartNode && EndNode && StartNode != EndNode) // Ensure valid nodes and no self-connections
    {
        // Check if the edge already exists (prevent duplicates)
        bool EdgeExists = Edges.ContainsByPredicate([StartNode, EndNode](const FCustomEdge& Edge) {
            return (Edge.StartNode == StartNode && Edge.EndNode == EndNode) ||
                   (Edge.StartNode == EndNode && Edge.EndNode == StartNode); // Allow bidirectional checks
        });

        if (!EdgeExists)
        {
            FCustomEdge NewEdge;
            NewEdge.StartNode = StartNode;
            NewEdge.EndNode = EndNode;
            Edges.Add(NewEdge);
        }
    }
}