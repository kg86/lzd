#pragma once

#include <string>
#include <vector>
#include <climits>
#include <cassert>
#include <iostream>

#include "string_tree_ns.hpp"
#include "string_tree_limited_ns.hpp"
#include "common_utils.hpp"

namespace NOT_STREAM{
class STreeOcc : public STreeLimit{
protected:
  std::vector<unsigned int> vOcc;
  std::vector<std::pair<int, int> > vars;

public:
  STreeOcc(const std::string & str, unsigned int max_fnode = MAX_FNODE):
    STreeLimit(str, max_fnode), vOcc(max_fnode, 0), vars(max_fnode, std::make_pair(0, 0)){};

  ~STreeOcc(){};

  inline unsigned int delIFfull(){
    if(UTIL::DEBUG_LEVEL > 1){
      std::cerr << "DEBUG DEL num_fnodes=" << num_fnodes
                << " max_fnode=" << max_fnode << std::endl;
      if (this->isFull()){
        std::cerr << "DELETE NODES" << std::endl;
      }
    }
    while(this->isFull()){
      for(unsigned int i = 1; i < this->max_fnode ; i++){
        switch(vOcc[i]){
          // vOcc[i] = 0 means that the factor whose fid=i has already deleted
          // Note that we have to reserve 3 empty spaces to insert a new factor.
        case 0:
            break;
        case 1:
            this->delNode(this->getFNode(i));
        default:
            vOcc[i]--;
        }
      }
    }
    return 256;
  }

  inline STree::Node * insertNewNode(unsigned int i, STree::Node * node1, const STree::Node * node2){
    STree::Node * new_node = this->insertFactorNode(node1, i, node2->depth);
    if (UTIL::DEBUG_LEVEL > 1){
      std::cerr << "INSERT NEW NODE" << std::endl;
      std::cerr << "INSERT NEW NODE " << new_node->fid << " = ("
                << node1->fid << ")(" << node2->fid << ")"
                <<std::endl;
    }
    vars[new_node->fid].first = node1->fid;
    vars[new_node->fid].second = node2->fid;
    this->useNode(new_node);
    if(this->stat) {
      assert(this->stat->pos_prev_used[new_node->fid] == UINT_MAX);
      this->stat->pos_prev_used[new_node->fid] = this->stat->len_factors.size()-1;
    }
    return new_node;
  }

  inline STree::Node * getLRUNode(){return NULL;};

  void useNode_rec(size_t i){
    if (i == 0) return;
    this->vOcc[i]++;

    useNode_rec(vars[i].first);
    useNode_rec(vars[i].second);
  }

  inline void useNode(const STree::Node * node){
    assert(node->isFactor());
    assert(node->fid < this->max_fnode);
    if (UTIL::DEBUG_LEVEL > 1){
      std::cerr << "use node_fid_=" << node->fid << std::endl;
    }
    useNode_rec(node->fid);
  }
};
}
