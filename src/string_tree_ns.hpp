// -*- coding: utf-8 -*-
#pragma once

#include <string>
#include <vector>
#include <climits>
#include <iostream>
#include <cassert>

namespace NOT_STREAM{
namespace STree{
  const static unsigned int NNODE_FID = UINT_MAX; // default fid of normal node
  const static unsigned int CHAR_SIZE = 256; // default fid of normal node
  class Stat{
    public:
    std::vector<unsigned int> len_factors;
    std::vector<unsigned int> len_lfactors;
    std::vector<unsigned int> len_rfactors;
    std::vector<unsigned int> len_deleted_factors;
    std::vector<unsigned int> alive_time;
    std::vector<unsigned int> pos_prev_used;
    Stat(unsigned int max_fnode = 0) : len_factors(0), len_lfactors(0), len_rfactors(0),
             len_deleted_factors(0), alive_time(0),
             pos_prev_used(max_fnode, 0) {};
    void delNode(unsigned int fid, unsigned int depth);
    ~Stat();
  };

  class Node{
  public:
    unsigned int in_edge_begin; // the beginning position of the in-edge label in the input string
    unsigned int in_edge_len; // the length of the in-edge label
    unsigned int depth; // the depth of root node is 0.
    unsigned int id; // unique id.
    unsigned int fid; // unique id for a factor node, set UINT if the node is not a factor node.

    Node * parent;
    std::vector<Node *> children;

    Node(unsigned int in_edge_begin, unsigned int in_edge_len, unsigned int depth,
         unsigned int id, unsigned int fid);
    ~Node(){};

    inline unsigned int edgeLen() const{return in_edge_len;};
    inline bool isFactor() const{return this->fid != UINT_MAX;}
    void info() const; // for debug
  };

  class Tree{
  protected:
    unsigned int num_nodes;
    unsigned int num_fnodes;
    const std::string & str; // input string
    Stat * stat;
  public:
    Node * root;
    std::vector<Node *> nodes;
    std::vector<Node *> fnodes;


    Tree(const std::string & str);
    virtual ~Tree();


    inline char getCharAt(size_t i) const{
      assert(i < this->str.size());
      return this->str.at(i);
    }
    inline std::string getEdgeLabel(const Node * node) const {
      return this->str.substr(node->in_edge_begin, node->in_edge_len);
    }
    inline size_t getStrSize() const{return this->str.size();};

    /** 
     * creates a node whose depth is $depth, and also that in-edge label is $str[$in_edge_begin..$in_edge_begin+$in_edge_len - 1]
     * 
     * @param isFactor 
     * @param in_edge_begin 
     * @param in_edge_len 
     * @param depth 
     * 
     * @return 
     */
    Node * createNode(bool isFactor, unsigned int in_edge_begin, unsigned int in_edge_len, unsigned int depth);

    inline virtual void registerFNode(Node * node){
      this->fnodes.push_back(node);
      this->num_fnodes++;
    };
    inline virtual void registerNode(Node * node){
      this->nodes.push_back(node);
      this->num_nodes++;
    };
    /** 
     * changes the node state from a normal node to a factor node
     * 
     * @param node 
     */
    inline void change2FNode(Node * node){
      node->fid = this->getNewFId();
      registerFNode(node);
    }

    inline virtual unsigned int getNewId(){
      return getNumNodes();
    };
    inline virtual unsigned int getNewFId(){
      return getNumFNodes();
    };

    inline unsigned int getNumNodes(){return num_nodes;};
    inline unsigned int getNumFNodes(){return num_fnodes;};
    inline Node * getNode(unsigned int node_id) const {
      assert(this->nodes[node_id] != NULL);
      return this->nodes[node_id];
    };
    inline Node * getFNode(unsigned int node_fid) const {return this->fnodes[node_fid];};

    /** 
     * gets the child node whose in-edge label begins with  $c
     * 
     * @param node 
     * @param c 
     * 
     * @return 
     */
    Node * getChildBeginWith(const Node * node, char c) const;

    /**
     * gets the child whose in-edge label matches with $str[$strbegin..]
     * 
     * @param node 
     * @param strbegin 
     * 
     * @return 
     */
    Node * getChild(const Node * node, unsigned int strbegin) const;

    /** 
     * checks whether the in-edge label matches with $str[$strbegin..]
     * 
     * @param node 
     * @param strbegin 
     * 
     * @return 
     */
    bool canReachTo(const Node * node, unsigned int strbegin) const;

    /**
     * checks whether the concatenated string of in-edge labeles
     * from $from to $to matches with $str[$strbegin..]
     * 
     * @param from 
     * @param to 
     * @param strbegin 
     * 
     * @return 
     */
    bool canReachFromTo(Node * from, Node * to, unsigned int strbegin);

    /**
     * gets the first character of in-edge label
     * 
     * @param node 
     * 
     * @return 
     */
    char getInFirstChar(const Node * node) const;
    inline Node * getParent(const Node * node) const {return node->parent;};
    inline void addChild(Node * parent, Node * child){
      parent->children.push_back(child);
      child->parent = parent;
    }

    /** 
     * creates and adds a new node whose parent is $parent and
     * in-edge label is $str[$in_edge_begin..$in_edge_begin + $in_edge_len - 1]
     * 
     * @param parent 
     * @param isFactor 
     * @param in_edge_begin 
     * @param in_edge_len 
     * 
     * @return 
     */
    Node * addChild(Node * parent, bool isFactor, unsigned int in_edge_begin, unsigned int in_edge_len);

    /**
     * inserts a new factor node that can be reached from $node with
     * $str[$strbegin..$strbegin + $len-1]
     * 
     * @param node 
     * @param strbegin 
     * @param len
     */
    Node * insertFactorNode(Node * node, unsigned int strbegin, unsigned int len);

    /** 
     * inserts a new factor node that corresonds a single character
     * 
     * @param c 
     * 
     * @return
     */
    inline Node * insertCharNode(char c){
      if (this->stat) {
        this->stat->len_factors.push_back(1);
      }
      return this->addChild(this->root, true, c, 1);
    }

    /**
     * unlinks from $par to $child
     * replaces $child with $replace_node if $replace_node != NULL
     * 
     * @param par 
     * @param child 
     * @param replace_node 
     * 
     * @return 
     */
    bool unlinkChild(Node * par, Node * child, Node * replace_node = NULL);

    /** 
     * splits the edge between $par to $child at $len
     * inserts and returns a new branch node
     * 
     * @param par
     * @param child
     * @param len
     * 
     * @return
     */
    Node * splitEdge(Node * par, Node * child, unsigned int len);

    /**
     * finds the deepest node which can be reached from $node with
     * $str[$strbegin..$strbegin+$len-1]
     * if $findFNode is true, finds a factor node only
     *
     * @param node 
     * @param strbegin 
     * @param findFNode
     * @param len
     * 
     * @return 
     */
    unsigned int findLastNodeIdFrom(const Node * node, unsigned long long strbegin, bool findFNode, unsigned int len = UINT_MAX) const;
    inline Node * findLastNodeFrom(const Node * node, unsigned int strbegin, bool findFNode, unsigned int len = UINT_MAX) const{
      return this->getNode(this->findLastNodeIdFrom(node, strbegin, findFNode, len));
    }
    inline Node * findLastFNodeFrom(Node * node, unsigned int strbegin, unsigned int len = UINT_MAX){
      return findLastNodeFrom(node, strbegin, true, len);
    }
    inline Node * findLastNNodeFrom(Node * node, unsigned int strbegin, unsigned int len = UINT_MAX){
      return findLastNodeFrom(node, strbegin, false, len);
    }

    /** 
     * computes the longest common prefix between $str[$strbegin..$strbegin + $len - 1] and the label strings on the paths from $node
     * 
     * @param node
     * @param strbegin
     * @param len
     * 
     * @return
     */
    unsigned int LCPFromNode(Node * node, unsigned int strbegin, unsigned int len = UINT_MAX);

    /**
     * computes the longest common prefix between $str[$strbegin..$strbegin + $len - 1] and the in-edge label
     *
     * @param node
     * @param strbegin
     * @param len
     *
     * @return
     */
    unsigned int LCPToNode(const Node * node, unsigned int strbegin, unsigned int len = UINT_MAX) const;

    void toString(const Node * node, std::string & os) const;
    inline void info();
    inline void verify(const Node * node) const{
      assert(this->getNode(node->id) == node);
      assert(this->getFNode(node->fid) == node);
    }
    inline std::string substr(unsigned int strbegin, unsigned int len){
      return this->str.substr(strbegin, len);
    }
    void showAllNode() const;
    void showAllNode(const Node * node, std::string & os) const;
    bool isValid() const;
    bool isValid(const Node * node) const;
  };
}
}
