// -*- coding: utf-8 -*-
#pragma once

#include <string>
#include <vector>
#include <climits>
#include <cassert>

#include "string_tree.hpp"
#include "LRUList.hpp"
#include "string_tree_limited.hpp"
#include "common_utils.hpp"
#include "lzd.hpp"

namespace STreePre {
  class STreePre : public STreeLimit{
  public:
    LRUList lru;
    STreePre(std::istream & is, std::ostream & os,
             unsigned int codeSize):
      STreeLimit(is, os, codeSize), lru(1 << codeSize){
    };

    ~STreePre(){};

    inline unsigned int delIFfull(){
      while (this->isFull()){
        STree::Node * node = this->getLRUNode();
#ifdef DEBUG
        if (UTIL::DEBUG_LEVEL > 2){
          this->lru.dump();
        }
#endif
        assert(node->children.size() == 0);
        this->lru.del_unused(node->fid);
        this->delNode(node);
      }
      return 0;
    }

    inline STree::Node * insertCharNodeNotWrite(char c){
      STree::Node * new_node = this->addChild(this->root, true, std::string(1, c));
      this->useNode(new_node);

      if(this->stat) {
        assert(this->stat->pos_prev_used[new_node->fid] == UINT_MAX);
        this->stat->len_factors.push_back(1);
        this->stat->pos_prev_used[new_node->fid] = this->stat->len_factors.size()-1;
      }
      return new_node;
    }

    inline STree::Node * insertNewNode(STree::Node * node1, const STree::Node * node2, std::string & remain_string){
      STree::Node * new_node = this->insertFactorNode(node1, remain_string, 0, node2->depth);
      this->useNode(new_node);
      if(this->stat) {
        assert(this->stat->pos_prev_used[new_node->fid] == UINT_MAX);
        this->stat->pos_prev_used[new_node->fid] = this->stat->len_factors.size()-1;
      }
      return new_node;
    }

    inline STree::Node * getLRUNode(){return this->getFNode(this->lru.back());};

    inline virtual void useNode(const STree::Node * node) {

      if (UTIL::DEBUG_LEVEL > 1){
        std::cerr << "useNode:";
        node->info();
      }

      const STree::Node * cur = node;
      while (cur != this->root){
        assert(cur != cur->parent);
        if (cur->isFactor()){
          this->lru.use(cur->fid);
        }
        cur = this->getParent(cur);
      }
    }
  };

  STreePre * genSTree(std::istream &is, std::ostream &os){
    std::cerr << "genSTree" << std::endl;
    unsigned int codeSize=0;
    is.read(reinterpret_cast<char *>(&codeSize), sizeof(codeSize));
    assert(codeSize < 64);
    std::cerr << "genSTree codeSize=" << codeSize << std::endl;
    return new STreePre(is, os, codeSize);
  }

  void STreePreCompress(std::string & in_fname, std::string & out_fname,
                        unsigned int codeSize){
    std::ifstream ifs(in_fname.c_str());
    std::ofstream ofs(out_fname.c_str(), std::ios::binary);
    LZFF::FileInfo info(0, 0, codeSize, 0);
    LZFF::writeHeader(ofs, info);
    STreePre tree(ifs, ofs, codeSize);
    tree.compress(info);
    ifs.close();
    ofs.close();

    std::fstream fs(out_fname.c_str(), std::ios::in | std::ios::out | std::ios::binary);
    writeHeader(fs, info);
    fs.close();
  }

  void STreePreCompress(std::istream & is, std::ostream & os,
                        unsigned int codeSize){
    LZFF::FileInfo info(0, 0, codeSize, 0);
    LZFF::writeHeader(os, info);
    STreePre tree(is, os, codeSize);
    tree.remain_bitsize = tree.NUM_BITS;
    tree.compress(info);
  }

  void STreePreDecompress(std::istream & is, std::ostream & os){
    LZFF::FileInfo info(0, 0, 0, 0);
    LZFF::readHeader(is, info);
    STreePre tree(is, os, info.codeSize);
    tree.remain_bitsize = 0;
    tree.decompress(info);
  }
}
