// -*- coding: utf-8 -*-
#pragma once

#include <string>
#include <vector>
#include <climits>
#include <iostream>
#include <cassert>
#include <map>

namespace LZ78{
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
    unsigned char label;
    unsigned int id;
    unsigned int depth; // for ease to implement
    std::map<unsigned char, Node *> children;
    Node(unsigned char label, unsigned int id) : label(label), id(id), depth(0){};
  };
  class Tree{
    std::vector<Node *> nodes;
    public:
    Stat * stat;
    Node * root;
    Tree();
    ~Tree();
    Node * findLPF(const std::string & in_str, unsigned int pos);
    void insert(Node * from_node, unsigned char c);
  };

  class LZ78{
    public:
    unsigned int fileSize;
    unsigned int seqSize;
    std::vector<std::pair<unsigned int, unsigned char> > seq;
    bool ignoreLastChar; // ignores last factor's character if true
    LZ78();
    LZ78(const std::string & fname);
    void out(const std::string & out_fname);
  };

  void compress(const std::string & s, LZ78 & lz78);
  void decompress(std::string & s, const LZ78 & lz78);

  /**
   *  computes a SLP which represents lz78 sequence
   *
   *  @param seq  lz78 sequence
   *  @param vars SLP
   */
  void seq2vars(const std::vector<std::pair<unsigned int, unsigned char> > & seq, std::vector<std::pair<unsigned int, unsigned int> > & vars);
};
