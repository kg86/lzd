// -*- coding: utf-8 -*-
#pragma once

#include <string>
#include <vector>
#include <climits>
#include <cassert>
#include <iostream>

#include "string_tree_ns.hpp"
#include "common_utils.hpp"

namespace NOT_STREAM{
class STreeLimit : public STree::Tree{
protected:
  unsigned int num_total_nodes; // include number of deleted nodes
  unsigned int num_total_fnodes; // include number of deleted factor nodes
public:
  const static unsigned int  MAX_FNODE = UINT_MAX;
  const unsigned int max_fnode;
  std::vector<unsigned int> deleted_id; // deleted/unused node id
  std::vector<unsigned int> deleted_fid; // deleted/unused factor node id

  STreeLimit(const std::string & str, unsigned int max_fnode = MAX_FNODE):
    Tree(str), max_fnode(max_fnode){
    if (this->stat){
      this->stat->pos_prev_used.assign(max_fnode, UINT_MAX);
    }
    this->num_total_nodes = this->num_nodes;
    this->num_total_fnodes = this->num_fnodes;
    this->fnodes.resize(this->max_fnode);
    this->nodes.resize(2*this->max_fnode);

    for (unsigned int i = max_fnode -1; i >0 ; i--){
      this->deleted_fid.push_back(i);
    }

    for (unsigned int i = 2*max_fnode - 1; i > 0; i--){
      this->deleted_id.push_back(i);
    }
  }
  ~STreeLimit(){};

  inline void registerFNode(STree::Node * node){
    if (UTIL::DEBUG_LEVEL > 2){
      std::cerr << "REGISTER NODE : ";
      node->info();
    }
    assert(this->num_fnodes < this->max_fnode);
    assert(node->fid < this->fnodes.size());
    this->fnodes[node->fid] = node;
    this->num_fnodes++;
    this->num_total_fnodes++;
  };
  inline void registerNode(STree::Node * node){
    assert(node->id < this->nodes.size());
    this->nodes[node->id] = node;
    this->num_nodes++;
    this->num_total_nodes++;
  };

  inline void unregisterNode(STree::Node * node){
    assert(node->id < this->nodes.size());
    assert(this->nodes[node->id] == node);
    this->deleted_id.push_back(node->id);
    this->num_nodes--;
    this->nodes[node->id] = NULL;
    if (node->isFactor()){
      unregisterFNode(node);
    }
    delete node;
  }

  inline void unregisterFNode(STree::Node * node){
    assert(this->fnodes[node->fid] == node);
    this->fnodes[node->fid] = NULL;
    this->deleted_fid.push_back(node->fid);
    this->num_fnodes--;
    node->fid = STree::NNODE_FID;
  }

  inline bool isFull() const{
    // at least 3 empty spaces are needed to insert a new factor f1=f2 f3.
    return this->getNumUnusedNode() < 3;
  }

  inline unsigned int getNumUnusedNode() const {
    return this->deleted_fid.size();
  }

  virtual STree::Node * getLRUNode() = 0;

  inline unsigned int getNewId(){
      assert(deleted_id.size() > 0);
      const unsigned int ret = deleted_id.back();
      deleted_id.pop_back();
      return ret;
  }

  inline unsigned int getNewFId() {
    const unsigned int ret = deleted_fid.back();
    assert(ret < this->max_fnode);
    deleted_fid.pop_back();
    return ret;
  }

  inline STree::Node * insertCharNode(char c){

    STree::Node * new_node = this->addChild(this->root, true, c, 1);
    this->useNode(new_node);

    if(this->stat) {
      assert(this->stat->pos_prev_used[new_node->fid] == UINT_MAX);
      this->stat->len_factors.push_back(1);
      this->stat->pos_prev_used[new_node->fid] = this->stat->len_factors.size()-1;
    }
    return new_node;
  }
  virtual STree::Node * insertNewNode(unsigned int i, STree::Node * node1, const STree::Node * node2)=0;

  virtual unsigned int delIFfull() = 0;

  inline void delNode(STree::Node * node){
    if (this->stat){
      this->stat->delNode(node->fid, node->depth);
    }
    // -------------- DEBUG BEGIN ----------------
    if (UTIL::DEBUG_LEVEL > 1){
      std::string tmp;
      this->toString(node, tmp);
      std::cerr << "*** DELNODE *** [" << tmp << "] "
                << node->id << "("<< node->fid << ")" << std::endl;
      assert(node->children.size() == 0);
      if (node->children.size() > 0){
        std::cerr << "Warning!! delete node has children." << std::endl;
        tmp.clear();
        this->toString(node->children[0], tmp);
        std::cerr << "*** the first child of delete node [" << tmp << "] "
                  << "node->chilren_[0]->fid_=" << node->children[0]->fid << std::endl;
        node->info();
        node->children[0]->info();
      }
    }
    // -------------- DEBUG END ----------------

    // checks whether $node has children
    assert(node->isFactor());

    // changes the link from the parent to NULL
    std::vector<STree::Node *>::iterator itr;
    STree::Node *par = node->parent;
    assert(par != NULL);
    if (node->children.size() == 1){
      // deletes the node and connect the parent and the child
      STree::Node * child =  node->children[0];
      assert(child->in_edge_begin >= node->in_edge_len);
      child->in_edge_begin -= node->in_edge_len;
      child->in_edge_len += node->in_edge_len;
      assert(child->in_edge_len == (child->depth - par->depth));
      this->unlinkChild(par, node, child);
      this->unregisterNode(node);
    }else if (node->children.size() > 1){
      // changes the state from factor node to normal node
      this->unregisterFNode(node);
    }else if (par->children.size() == 2 && par->isFactor() == false ){
      // the parent is a node which has two children,
      // one of which just be deleted
      // deletes the parent and connect the parent's parent and parent's another/undeleted child
      STree::Node * parchild = par->children[0] == node ? par->children[1] : par->children[0];
      STree::Node * parpar = par->parent;
      for(itr = parpar->children.begin(); itr != parpar->children.end(); itr++){
        if ((*itr) == par){
          assert(parchild->in_edge_begin >= (*itr)->in_edge_len);
          parchild->in_edge_begin -= (*itr)->in_edge_len;
          parchild->in_edge_len += (*itr)->in_edge_len;
          assert(parchild->in_edge_len == (parchild->depth - parpar->depth));
          (*itr) = parchild;
          parchild->parent = parpar;
          break;
        }
      }
      assert(!(par->isFactor()));
      this->unregisterNode(node);
      this->unregisterNode(par);
    }else{
      this->unlinkChild(par, node);
      this->unregisterNode(node);
    }
  }

  virtual void useNode(const STree::Node * node) = 0;

  inline void info() const{
  std::cerr << "** TREE INFO:"
            << " num_nodes=" << this->num_nodes
            << " num_fnodes=" << this->num_fnodes
            << " num_total_nodes=" << this->num_total_nodes
            << " num_total_fnodes=" << this->num_total_fnodes
            << " max_fnode=" << this->max_fnode
            << std::endl;
  };
};
}
