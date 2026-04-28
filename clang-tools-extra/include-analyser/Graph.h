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

  class Node {
  public:
    Node() = default;
    Node(Data const& data) : m_data(data) {}
    Node(Data&& data) : m_data(data) {}

    Data& getData() { return m_data; }
    Data const& getData() const { return m_data; }
    void setData(Data const& data) { m_data = data; }

    using ParentContainer = std::list<NodeIterator>;
    using ParentIterator = typename ParentContainer::iterator;
    using ConstParentIterator = typename ParentContainer::const_iterator;
    
    using ChildContainer = std::list<NodeIterator>;
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

    ParentIterator addParent(NodeIterator node) {
      m_parents.push_back(node);
      return endParents()--;
    }

    void removeParent(ConstParentIterator iter) { m_parents.erase(iter); }

    SizeType removeParent(ConstNodeIterator parent) {
      auto collected = std::list<ParentIterator>();
      for (auto iter = m_parents.begin(); iter != m_parents.end(); ++iter) {
        if (*iter == parent) {
          collected.push_back(iter);
        }
      }
      for (auto& iter : collected) {
        m_parents.erase(iter);
      }
      return collected.size();
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

    ChildIterator addChild(NodeIterator node) {
      m_children.push_back(node);
      return endChildren()--;
    }

    void removeChild(ConstChildIterator iter) { m_children.erase(iter); }

    SizeType removeChild(ConstNodeIterator child) {
      auto collected = std::list<ChildIterator>();
      for (auto iter = m_children.begin(); iter != m_children.end(); ++iter) {
        if (*iter == child) {
          collected.push_back(iter);
        }
      }
      for (auto& iter : collected) {
        m_children.erase(iter);
      }
      return collected.size();
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

  void removeNode(ConstNodeIterator iter) {
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
