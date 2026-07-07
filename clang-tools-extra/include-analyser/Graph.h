#ifndef INCLUDE_ANALYSER_GRAPH_H
#define INCLUDE_ANALYSER_GRAPH_H

#include <list>

template<class Data>
class Graph {
public:
  class Node;

  using NodeContainer = std::list<Node>;
  using NodeIterator = typename NodeContainer::iterator;
  using ConstNodeIterator = typename NodeContainer::const_iterator;
  using SizeType = typename NodeContainer::size_type;

  struct NodeIterLess {
    bool operator()(NodeIterator const& lhs, NodeIterator const& rhs) const {
      return &(*lhs) < &(*rhs);
    }
  };

  class Node {
  public:
    Node() = default;
    Node(Data const& data) : m_data(data) {}
    Node(Data&& data) : m_data(data) {}

    Data& getData() { return m_data; }
    Data const& getData() const { return m_data; }
    void setData(Data const& data) { m_data = data; }

    using ParentContainer = std::set<NodeIterator, NodeIterLess>;
    using ParentIterator = typename ParentContainer::iterator;
    using ConstParentIterator = typename ParentContainer::const_iterator;
    
    using ChildContainer = std::set<NodeIterator, NodeIterLess>;
    using ChildIterator = typename ChildContainer::iterator;
    using ConstChildIterator = typename ChildContainer::const_iterator;

    using SizeType = std::common_type_t<
      typename ParentContainer::size_type, typename ChildContainer::size_type
    >;

    ParentContainer& getParents() { return m_parents; }
    ParentContainer const& getParents() const { return m_parents; }
    
    SizeType getNumParents() const { 
      return m_parents.size();
    }

    ParentIterator beginParents() { return m_parents.begin(); }
    ConstParentIterator beginParents() const { return m_parents.begin(); }
    ConstParentIterator cbeginParents() const { return m_parents.cbegin(); }

    ParentIterator endParents() { return m_parents.end(); }
    ConstParentIterator endParents() const { return m_parents.end(); }
    ConstParentIterator cendParents() const { return m_parents.cend(); }

    std::pair<ParentIterator, bool> addParent(NodeIterator node) {
      return m_parents.insert(node);
    }

    void removeParent(ConstParentIterator iter) { m_parents.erase(iter); }

    SizeType removeParent(NodeIterator iter) {
      return m_parents.erase(iter);
    }

    ChildContainer& getChildren() { return m_children; }
    ChildContainer const& getChildren() const { return m_children; }
    
    SizeType getNumChildren() const { 
      return m_children.size();
    }

    ChildIterator beginChildren() { return m_children.begin(); }
    ConstChildIterator beginChildren() const { return m_children.begin(); }
    ConstChildIterator cbeginChildren() const { return m_children.cbegin(); }

    ChildIterator endChildren() { return m_children.end(); }
    ConstChildIterator endChildren() const { return m_children.end(); }
    ConstChildIterator cendChildren() const { return m_children.cend(); }

    std::pair<ChildIterator, bool> addChild(NodeIterator node) {
      return m_children.insert(node);
    }

    void removeChild(ConstChildIterator iter) { m_children.erase(iter); }

    SizeType removeChild(NodeIterator iter) {
      return m_children.erase(iter);
    }

  private:
    Data m_data;
    ChildContainer m_children;
    ParentContainer m_parents;
  };

  NodeContainer& getNodes() { return m_nodes; }
  NodeContainer const& getNodes() const { return m_nodes; }
  SizeType getNumNodes() const { return m_nodes.size(); }
  NodeIterator beginNodes() { return m_nodes.begin(); }
  ConstNodeIterator beginNodes() const { return m_nodes.begin(); }
  ConstNodeIterator cbeginNodes() const { return m_nodes.begin(); }
  NodeIterator endNodes() { return m_nodes.end(); }
  ConstNodeIterator endNodes() const { return m_nodes.end(); }
  ConstNodeIterator cendNodes() const { return m_nodes.end(); }

  NodeIterator addNode(Data const& data) {
    m_nodes.push_back(Node(data));
    return --endNodes();
  }

  void removeNode(NodeIterator iter) {
    for (auto& child : iter->getChildren()) {
      child->removeParent(iter);
    }
    for (auto& parent : iter->getParents()) {
      parent->removeChild(iter);
    }
    m_nodes.erase(iter);
  }

private:
  NodeContainer m_nodes;
};

#endif // INCLUDE_ANALYSER_GRAPH_H
