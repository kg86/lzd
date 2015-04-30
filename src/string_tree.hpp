
// -*- coding: utf-8 -*-
#pragma once

#include <string>
#include <vector>
#include <climits>
#include <iostream>
#include <cassert>
#include <fstream>
#include <queue>
#include <istream>
#include <ostream>

#include "common.hpp"

namespace STree{
  const static unsigned int NNODE_FID = UINT_MAX; // default fid for normal nodes
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
    unsigned int depth; // the depth of the root node is 0.
    unsigned int id; // unique id for nodes.
    unsigned int fid; // unique id for factor nodes, set UINT if a node is not a factor node.
    std::string label;

    Node * parent;
    std::vector<Node *> children;

    Node(unsigned int depth, unsigned int id, unsigned int fid,
               std::string & label):
      depth(depth),
      id(id),
      fid(fid),
      label(label),
      parent(NULL){
    };

    Node(unsigned int depth, unsigned int id, unsigned int fid,
         std::string & label, unsigned int beg, unsigned int len):
      depth(depth),
      id(id),
      fid(fid),
      label(label, beg, len),
      parent(NULL){
    };

    ~Node(){};

    inline unsigned int edgeLen() const{return label.size();};
    inline bool isFactor() const{return this->fid != UINT_MAX;}
    void info() const; // for debug
  };

  class Tree{
  protected:
    unsigned int num_nodes;
    unsigned int num_fnodes;
    Stat * stat;
    std::istream & is;
    std::ostream & os;
    unsigned int codeSize;
    unsigned int in_str_size;
    unsigned int out_factor_size;
  public:
    Node * root;
    std::vector<Node *> nodes;
    std::vector<Node *> fnodes;
    STree::Node * cur;
    unsigned int len_matched;
    Node * lastFNode;
    Node * firstFactor;
    Tree(std::istream & is, std::ostream & os,
         unsigned int codeSize = 0);
    void Init();

    inline bool atNode() const{
      return len_matched == cur->label.size();
    }
    inline bool atNode(const Node * node, unsigned int len_matched) const{
      return len_matched == node->label.size();
    }

    inline void write(unsigned int first, unsigned int second){
      this->out_factor_size++;
      os.write(reinterpret_cast<const char *> (&first), sizeof(first));
      os.write(reinterpret_cast<const char *> (&second), sizeof(second));
    }
    void compress();
    void decompress();
    virtual ~Tree();

    /** 
     * creates a node whose depth is $depth, and also that in-edge label is $str[$in_edge_begin..$in_edge_begin+$in_edge_len - 1]
     * 
     * @param isFactor 
     * @param in_edge_begin 
     * @param in_edge_end 
     * @param depth 
     * 
     * @return 
     */
    inline Node * createNode(bool isFactor,  unsigned int depth, std::string label,
                             unsigned int label_begin, unsigned int label_len){
      Node * node;
      if (isFactor){
        node = new Node (depth, this->getNewId(), this->getNewFId(), label,
          label_begin, label_len);
        registerFNode(node);
      }else{
        node = new Node (depth, this->getNewId(), UINT_MAX, label,
                         label_begin, label_len);
      }
      registerNode(node);
      return node;
    }
    inline Node * createNode(bool isFactor,  unsigned int depth, std::string label){
      return this->createNode(isFactor, depth, label, 0, label.size());
    }

    inline virtual void registerFNode(Node * node){
      this->fnodes.push_back(node);
      this->num_fnodes++;
    };
    inline virtual void registerNode(Node * node){
      this->nodes.push_back(node);
      this->num_nodes++;
    };
    /** 
     * changes the node to a factor node.
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
    Node * getChild(const Node * node, std::string & label,
                    unsigned int label_begin, unsigned int label_len) const;

    /** 
     * checks whether the concatenated string of in-edge labeles
     * from $from to $to matches with $str[$strbegin..]
     *
     * @param node 
     * @param strbegin 
     * 
     * @return 
     */
    inline bool canReachTo(const Node * node, std::string & label,
                           unsigned int label_begin, unsigned int label_len) const {
      assert(node != NULL);
      assert(label_begin + label_len <= label.size());
      return node->label.size() == LCPToNode(node, label, label_begin, label_len);
    };

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
    inline Node * addChild(Node * parent, bool isFactor, std::string label,
      unsigned int label_begin, unsigned int label_len){
      assert(parent != NULL);
      unsigned int depth = parent->depth + label_len;
      Node * child = this->createNode(isFactor, depth, label, label_begin, label_len);
      this->addChild(parent, child);
      return child;
    }
    inline Node * addChild(Node * parent, bool isFactor, std::string label){
      return this->addChild(parent, isFactor, label, 0, label.size());
    }

    /**
     * inserts a new factor node that can be reached from $node with
     * $str[$strbegin..$strbegin + $len-1]
     * 
     * @param node 
     * @param strbegin 
     * @param len
     */
    Node * insertFactorNode(Node * node, std::string & label,
                            unsigned int beg, unsigned int len);
    inline Node * insertFactorNode(Node * node, std::string & label){
      return this->insertFactorNode(node, label, 0, label.size());
    };

    /** 
     * inserts a new factor node that corresonds a single character
     * 
     * @param c 
     * 
     * @return
     */
    inline virtual Node * insertCharNode(char c){
      this->write(this->root->fid, (unsigned char)c);
      if (this->stat) {
        this->stat->len_factors.push_back(1);
      }
      return this->addChild(this->root, true, std::string(1, c));
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
    unsigned int findLastNodeIdFrom(const Node * node, bool findFNode, std::string & label,
                                      unsigned int label_begin, unsigned int label_len) const;
    inline Node * findLastNodeFrom(
      bool findFNode, const Node * node, std::string & label,
      unsigned int label_begin, unsigned int label_len) const{
      return this->getNode(this->findLastNodeIdFrom(node, findFNode, label, label_begin, label_len));
    }
    inline Node * findLastNNodeFrom(Node * node, std::string & label,
      unsigned int label_begin, unsigned int label_len){
      return findLastNodeFrom(false, node, label, label_begin, label_len);
    }

    /** 
     * 
     * sets the state at the root node
     * 
     * @return 
     */
    void inline initState() {
      this->cur = this->root;
      this->lastFNode = this->root;
      this->len_matched = 0;
    }

    inline Node * moveWith(Node * node, unsigned int len_matched, const char c) const {
      if (this->atNode(node, len_matched)) {
        return this->getChildBeginWith(node, c);
      } else if (node->label[len_matched] == c) {
        return node;
      }else {
        return NULL;
      }
    }
    inline bool moveIfPossible(const char c){
      Node * res = moveWith(this->cur, this->len_matched, c);
      if (res != NULL) {
        if (this->cur == res) {
          this->len_matched++;
        }else {
          this->cur = res;
          this->len_matched = 1;
          // if (res->isFactor()) this->lastFNode = res;
        }
        return true;
      }
      this->cur = this->root;
      this->len_matched = 0;
      return false;
    }

    /** 
     * return NULL if we can move with $new_char,
     * otherwise return the last factor node.
     * 
     * @param new_char 
     * 
     * @return 
     */
    Node * findLastFNodeWithChar(const unsigned char new_char){
      if (!moveIfPossible(new_char)){
        // we couldn't move with $new_char
        if (this->lastFNode == this->root){
          if (UTIL::DEBUG_LEVEL > 1){
            std::cerr << "insert new char [" << new_char << "]"
                      // << this->getNewFId() << ")"
                      << std::endl;
          }
          return this->insertCharNode(new_char);
        }else{
          Node * tmp = this->lastFNode;
          this->lastFNode = this->root;
          return tmp;
        }
      }else if (this->atNode(this->cur, this->len_matched) &&
                this->cur->isFactor()){
        // we could move with $new_char, and be at a factor node.
        this->lastFNode = this->cur;
      }
      // we could move
      return NULL;
    }

    /** 
     * finds the last factor node with $remain_string and $is.
     * Note that $remain_string contains the decompressed string of the obtained node.
     * 
     * @param is 
     * @param remain_string 
     * 
     * @return 
     */
    Node * findLastFNode(std::istream & is, std::string & remain_string){
      assert(is || !remain_string.empty());
      char new_char;
      // reads $remain_string first.
      for (unsigned int i = 0; i < remain_string.size(); i++){
        Node * res = findLastFNodeWithChar(remain_string[i]);
        if (res != NULL){
          // found the last factor node
          this->initState();
          return res;
        }
      }
      // reads $is
      for(; is && is.get(new_char);) {
        remain_string.push_back(new_char);
        Node * res = findLastFNodeWithChar(new_char);
        if (res != NULL){
          this->initState();
          return res;
        }
      }
      Node * res = this->lastFNode;
      this->initState();
      return res;
    }

    /**
     * computes the longest common prefix between $str[$strbegin..$strbegin + $len - 1] and the label strings on the paths from $node
     * NOTE if @len = UINT_MAX, we recognize @len is the length of  str.
     *
     * @param node
     * @param strbegin
     * @param len
     *
     * @return
     */
    unsigned int LCPToNode(const Node * node, std::string & label,
                           unsigned int label_begin, unsigned int label_len) const;
    inline unsigned int LCPToNode(const Node * node, std::string & label, unsigned int label_begin) const {
      return this->LCPToNode(node, label, label_begin, label.size() - label_begin);
    };

    void toString(const Node * node, std::string & os) const;
    inline void info();
    inline void verify(const Node * node) const{
      assert(this->getNode(node->id) == node);
      assert(this->getFNode(node->fid) == node);
    }

    void showAllNode() const;
    void showAllNode(const Node * node, std::string & os) const;
    bool isValid() const;
    bool isValid(const Node * node) const;
  };
}
