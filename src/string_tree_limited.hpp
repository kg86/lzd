// -*- coding: utf-8 -*-
#pragma once

#include <string>
#include <vector>
#include <climits>
#include <cassert>
#include <iostream>
#include <iomanip>

#include "string_tree.hpp"
#include "common_utils.hpp"
#include "lzd.hpp"

class STreeLimit : public STree::Tree{
protected:
  unsigned int num_total_nodes; // include number of deleted nodes
  unsigned int num_total_fnodes; // include number of deleted factor nodes
public:
  const static unsigned int  MAX_FNODE = UINT_MAX;
  unsigned int max_fnode;
  std::vector<unsigned int> deleted_id; // deleted node id
  std::vector<unsigned int> deleted_fid; // deleted factor node id

  unsigned long long mask;
  unsigned long long out_bit; // a buffer to write to ostream
  unsigned int out_bit_size;
  unsigned int codePairSize;

  const unsigned int NUM_BITS;
  unsigned int remain_bitsize;
  unsigned long long in_bit; // a buffer

  unsigned int count_factors;

  STreeLimit(std::istream & is, std::ostream & os,
             unsigned int codeSize) :
    Tree(is, os, codeSize),
    max_fnode(1 << this->codeSize),
    mask((1 << codeSize)-1),
    out_bit(0),
    out_bit_size(8 * sizeof(this->out_bit)),
    codePairSize(2 * this->codeSize),
    NUM_BITS(sizeof(in_bit) * 8),
    remain_bitsize(NUM_BITS),
    in_bit(0),
    count_factors(0){

    if (this->stat){
      this->stat->pos_prev_used.assign(max_fnode, UINT_MAX);
    }
    this->num_total_nodes = this->num_nodes;
    this->num_total_fnodes = this->num_fnodes;
    this->fnodes.resize(this->max_fnode);
    this->nodes.resize(2*this->max_fnode);

    for (unsigned int i = max_fnode - 1; i >0 ; i--){
      this->deleted_fid.push_back(i);
    }

    for (unsigned int i = 2*max_fnode - 1; i > 0; i--){
      this->deleted_id.push_back(i);
    }
  }
  ~STreeLimit(){};

  inline void write(unsigned int first, unsigned int second){
    this->out_factor_size++;
    if (UTIL::DEBUG_LEVEL > 2){
      std::cout << "[" << ++count_factors << "]=["
              << first << "]["
              << second << "]" << std::endl;
    }
    assert(first <= this->mask);
    assert(second <= this->mask || first == 0); // root
    unsigned long long bit = this->mask & first;
    bit = bit << this->codeSize;
    bit |= this->mask & second;

    if (this->remain_bitsize >= this->codePairSize){
      this->out_bit |= bit << (this->remain_bitsize-this->codePairSize);
      this->remain_bitsize -= this->codePairSize;
    }else {
      this->out_bit |= bit >> (this->codePairSize-this->remain_bitsize);
      if (UTIL::DEBUG_LEVEL > 2){
        std::cerr << std::hex;
        std::cerr << "write out=" << this->out_bit << std::endl;
        std::cerr << std::dec;
      }
      this->os.write(reinterpret_cast<const char *> (&out_bit), sizeof(out_bit));
      out_bit = 0;
      remain_bitsize = (out_bit_size - (codePairSize - remain_bitsize));
      out_bit = bit << remain_bitsize;
    }
  }
  void compress(LZFF::FileInfo & info){
    std::string remain_string;
    this->write(0, 0);
    while (is || !remain_string.empty()){
      STree::Node * cur = this->findLastFNode(is, remain_string);
      if(cur == this->root) break;

      assert(cur != NULL);
      assert(0 < cur->fid && cur->fid < this->max_fnode);

      if (firstFactor == NULL){
        firstFactor = cur;
        remain_string.erase(0, cur->depth);
      }else{
        const STree::Node * new_node =
          this->insertNewNode(firstFactor, cur, remain_string);
        this->in_str_size += new_node->depth;
        remain_string.erase(0, cur->depth);

        if(UTIL::DEBUG_LEVEL > 1){
          std::string left;
          std::string right;
          this->toString(firstFactor, left);
          this->toString(cur, right);
          std::cerr << "[" << new_node->fid << "]=["
                    << firstFactor->fid << "]["
                    << cur->fid << "]=["
                    << left << "]["
                    << right << "]"
                    << std::endl;
        }

        this->write(firstFactor->fid, cur->fid);
        this->delIFfull();

        firstFactor = NULL;
      }
    }
    if (firstFactor != NULL) {
      std::string left;
      this->toString(firstFactor, left);
      count_factors++;
      this->write(firstFactor->fid, this->root->fid);
      this->in_str_size += firstFactor->depth;
      info.last_right_factor = firstFactor->fid;
    }

    // output $out_bit since there may be unwritten bits
    os.write(reinterpret_cast<const char *> (&(this->out_bit)), sizeof(this->out_bit));
    info.fileSize = this->in_str_size;
    info.seqSize = this->out_factor_size;
  }

  inline std::pair<unsigned int, unsigned int>read() {
    // reads from $in_bits if remained
    if (remain_bitsize >= codePairSize){
      assert(remain_bitsize <= NUM_BITS);
      const std::pair<unsigned int, unsigned int> res = std::make_pair(
        mask & (in_bit >> (remain_bitsize-codeSize)),
        mask&(in_bit >> (remain_bitsize-codePairSize)));
      remain_bitsize -= codePairSize;
      return res;
    }
    assert(remain_bitsize <= NUM_BITS);
    // reallocate the remain bits to left
    unsigned long long bit = ((1<<codePairSize)-1) & (in_bit << (codePairSize-remain_bitsize));
    remain_bitsize = NUM_BITS - (codePairSize - remain_bitsize); // remain bit size after reading $codePairSize bits

    is.read(reinterpret_cast<char *> (&in_bit), sizeof(in_bit));

    bit |= (in_bit >> remain_bitsize);
    return std::make_pair(mask & (bit >> codeSize), mask & bit);
  }

  inline void writeTreeStr(unsigned int node_id, std::string & str){
    str.clear();
    this->toString(this->getFNode(node_id), str);
    os << str;
  }

  void decompress(LZFF::FileInfo & info) {
    std::string tmp_str;
    std::pair<unsigned int, unsigned int> v;
    this->read();

    for (unsigned long long i = 1; i < info.seqSize - 1; i++){
      v = this->read();
      if (v.first == this->root->fid) {
        this->insertCharNodeNotWrite((unsigned char) v.second);
      }else {
        writeTreeStr(v.first, tmp_str);
        if(UTIL::DEBUG_LEVEL > 1){
          std::cerr << "left=[" << tmp_str << "]" << std::endl;
        }
        writeTreeStr(v.second, tmp_str);
        if(UTIL::DEBUG_LEVEL > 1){
          std::cerr << "right=[" << tmp_str << "]" << std::endl;
        }
        this->insertNewNode(this->getFNode(v.first), this->getFNode(v.second), tmp_str);
        this->delIFfull();
      }
    }
    v = this->read(); // it must not be (0, x)
    writeTreeStr(v.first, tmp_str);
    if (v.second != this->root->fid){
      writeTreeStr(v.second, tmp_str);
    }
    os.flush();
  }

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
    assert(!deleted_fid.empty());
    const unsigned int ret = deleted_fid.back();
    assert(ret < this->max_fnode);
    deleted_fid.pop_back();
    return ret;
  }

  virtual STree::Node * insertCharNodeNotWrite(char c) = 0;
  virtual STree::Node * insertCharNode(char c) {
      this->write(this->root->fid, (unsigned char)c);
      STree::Node * node = this->insertCharNodeNotWrite(c);
      return node;
  }
  virtual STree::Node * insertNewNode(STree::Node * node1, const STree::Node * node2, std::string & remain_string)=0;

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
      // Important: vfocct may delete multiple nodes in random order,
      // then a parent node may be deleted before the child node would be deleted.
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
      child->label = node->label + child->label;
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
          parchild->label = par->label + parchild->label;
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

  /** 
   * updates the list of least recently used nodes.
   * 
   * @param node 
   */
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

