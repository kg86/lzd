#pragma once

#include <string>
#include <vector>
#include <climits>
#include <cassert>
#include <iostream>

#include "string_tree.hpp"
#include "string_tree_limited.hpp"
#include "common_utils.hpp"
#include "lzFF.hpp"

namespace STreeOcc {
  class STreeOcc : public STreeLimit{
  protected:
    std::vector<unsigned int> vOcc;
    std::vector<std::pair<unsigned int, unsigned int> > vars;

  public:
    STreeOcc(std::istream & is, std::ostream & os,
             unsigned int codeSize):
      STreeLimit(is, os, codeSize),
      vOcc(max_fnode, 0), vars(max_fnode, std::make_pair(0, 0)){};

    ~STreeOcc(){};

    inline unsigned int delIFfull(){

      if(UTIL::DEBUG_LEVEL > 1){
        std::cerr << "DEBUG DEL num_fnodes=" << num_fnodes
                  << " max_fnode=" << max_fnode << std::endl;
        if (this->isFull()){
          std::cerr << "DELETE NODES" << std::endl;
        }
      }

      unsigned int num_del = 0;
      while(this->isFull()){
        for(unsigned int i = 1; i < this->max_fnode ; i++){

          if (UTIL::DEBUG_LEVEL > 2){
            std::cerr << "i=" << i
                      << " vOcc[i]=" << vOcc[i] << std::endl;
          }

          switch(vOcc[i]){
          case 0:
            break;
          case 1:
            num_del++;
            vars[i].first = 0;
            vars[i].second = 0;
            this->delNode(this->getFNode(i));
          default:
            vOcc[i]--;
          }
        }
      }
      return num_del;
    }
  inline STree::Node * insertCharNodeNotWrite(char c){
    STree::Node * new_node = this->addChild(this->root, true, std::string(1, c));
    this->useNode(new_node);
    vars[new_node->fid].first = this->root->fid;
    vars[new_node->fid].second = c;

    if(this->stat) {
      assert(this->stat->pos_prev_used[new_node->fid] == UINT_MAX);
      this->stat->len_factors.push_back(1);
      this->stat->pos_prev_used[new_node->fid] = this->stat->len_factors.size()-1;
    }
    return new_node;
  }

    inline STree::Node * insertNewNode(STree::Node * node1, const STree::Node * node2, std::string & remain_string){
      STree::Node * new_node = this->insertFactorNode(node1, remain_string, 0, node2->depth);

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

    void useNode_rec(unsigned int i){
      if (i == 0) return;
      this->vOcc[i]++;

      if (vars[i].first != this->root->fid) {
        useNode_rec(vars[i].first);
        useNode_rec(vars[i].second);
      }
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


  void STreeOccCompress(std::string & in_fname, std::string & out_fname,
                        unsigned int codeSize){
    std::ifstream ifs(in_fname.c_str());
    std::ofstream ofs(out_fname.c_str(), std::ios::binary);
    LZFF::FileInfo info(0, 0, codeSize, 0);
    LZFF::writeHeader(ofs, info);
    STreeOcc tree(ifs, ofs, codeSize);
    tree.remain_bitsize = tree.NUM_BITS;
    tree.compress(info);
    ifs.close();
    ofs.close();

    std::fstream fs(out_fname.c_str(), std::ios::in | std::ios::out | std::ios::binary);
    writeHeader(fs, info);
    fs.close();
  }

  void STreeOccCompress(std::istream & is, std::ostream & os,
                        unsigned int codeSize){
    LZFF::FileInfo info(0, 0, codeSize, 0);
    LZFF::writeHeader(os, info);
    STreeOcc tree(is, os, codeSize);
    tree.remain_bitsize = tree.NUM_BITS;
    tree.compress(info);
  }

  void STreeOccDecompress(std::istream & is, std::ostream & os){
    LZFF::FileInfo info(0, 0, 0, 0);
    LZFF::readHeader(is, info);
    STreeOcc tree(is, os, info.codeSize);
    tree.remain_bitsize = 0;
    tree.decompress(info);
  }
}
