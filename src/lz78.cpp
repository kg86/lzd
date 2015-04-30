#include <cassert>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <numeric>

#include "lz78.hpp"
#include "common.hpp"

namespace LZ78{
struct stat_vec {
  double avg;
  unsigned int median;
  unsigned int max;
  unsigned int min;
};
stat_vec statVec(std::vector<unsigned int> & vec){
  stat_vec res;
  res.avg = res.median = res.max = res.min = 0;
  if (vec.size() > 0) {
    std::sort(vec.begin(), vec.end());
    res.avg = (double)std::accumulate(vec.begin(),vec.end(),1.0) / vec.size();
    res.median = vec[vec.size()/2];
    res.min = * std::min_element(vec.begin(), vec.end());
    res.max = * std::max_element(vec.begin(), vec.end());
  }
  return res;
}
void Stat::delNode(unsigned int fid, unsigned int depth){
  this->len_deleted_factors.push_back(depth);
  const unsigned int pos_current = this->len_factors.size();
  assert (this->pos_prev_used[fid] != UINT_MAX);
  const unsigned int pos_prev = this->pos_prev_used[fid];
  assert(pos_prev < pos_current);
  this->alive_time.push_back(pos_current - pos_prev);
  this->pos_prev_used[fid] = UINT_MAX;
}
Stat::~Stat(){
  // computes life spans for factors which remains at the end of factorization.
  for (unsigned int i = 0; i < this->pos_prev_used.size(); i++){
    unsigned int pos_prev = this->pos_prev_used[i];
    if (pos_prev != UINT_MAX) {
      assert(len_factors.size() > pos_prev);
      alive_time.push_back(len_factors.size() - pos_prev);
    }
  }

  unsigned int num_stat = 5;
  std::vector<std::string> stat_names;
  stat_names.push_back("len_factors");
  stat_names.push_back("len_lfactors");
  stat_names.push_back("len_rfactors");
  stat_names.push_back("len_deleted_factors");
  stat_names.push_back("alive_time");
  std::vector<std::vector<unsigned int> *> vecs;
  vecs.push_back(&len_factors);
  vecs.push_back(&len_lfactors);
  vecs.push_back(&len_rfactors);
  vecs.push_back(&len_deleted_factors);
  vecs.push_back(&alive_time);

  std::cerr << "len_factors.size()=" << len_factors.size()
            << " len_lfactors.size()=" << len_lfactors.size()
            << " alive_time.size()=" << alive_time.size()
            << std::endl;
  for (unsigned int i = 0; i < num_stat; i++){
    stat_vec statv = statVec(*vecs[i]);
    std::cerr << stat_names[i] << " : "
              << "avg=" << statv.avg
              << " median=" << statv.median
              << " max=" << statv.max
              << " min=" << statv.min
              << std::endl;
  }
}


Tree::Tree(){
  stat = NULL;
  if (UTIL::DEBUG_LEVEL > 0) stat = new Stat();
  this->root = new Node('+', 0);
  this->nodes.push_back(this->root);
}

Tree::~Tree(){
  for(unsigned int i = 0; i < this->nodes.size(); i++){
    if (this->nodes[i]) delete this->nodes[i];
  }
  if (stat) delete stat;
}

Node * Tree::findLPF(const std::string & in_str, unsigned int pos){
  Node * cur = this->root;
  std::map<unsigned char, Node*>::iterator itr;
  for(;pos < (unsigned int) in_str.size(); pos++){
    itr = cur->children.find(in_str[pos]);
    if (itr != cur->children.end()){
      cur = (*itr).second;
    }else{
      break;
    }
  }
  return cur;
}
void Tree::insert(Node * from_node, unsigned char c){
  Node * new_node = new Node(c, (unsigned int) this->nodes.size());
  this->nodes.push_back(new_node);
  assert(from_node->children.find(c) == from_node->children.end());

  from_node->children[c] = new_node;
  new_node->depth = from_node->depth + 1;
}

LZ78::LZ78() : fileSize(0), seqSize(0), seq(0), ignoreLastChar(false){}
LZ78::LZ78(const std::string & fname){
  std::ifstream f(fname.c_str(), std::ios::in|std::ios::binary);
  f.read(reinterpret_cast<char *> (&this->fileSize), sizeof(this->fileSize));
  f.read(reinterpret_cast<char *> (&this->seqSize), sizeof(this->seqSize));
  f.read(reinterpret_cast<char *> (&this->ignoreLastChar), sizeof(this->ignoreLastChar));

  assert(seqSize > 0);
  this->seq.reserve(seqSize);

  unsigned int factor_id;
  unsigned char new_char = '\0';
  while(this->seq.size() < seqSize){
    if(!f.read(reinterpret_cast<char *> (&factor_id), sizeof(factor_id)) ||
       !f.read(reinterpret_cast<char *> (&new_char), sizeof(new_char))){
      std::cerr << "error" << std::endl;
    }
    this->seq.push_back(std::make_pair(factor_id, new_char));
  }
}

void LZ78::out(const std::string & out_fname){
  std::ofstream os;
  os.open(out_fname.c_str());
  os.write(reinterpret_cast<const char *> (&this->fileSize), sizeof(this->fileSize));
  os.write(reinterpret_cast<const char *> (&this->seqSize), sizeof(this->seqSize));
  os.write(reinterpret_cast<const char *> (&this->ignoreLastChar), sizeof(this->ignoreLastChar));

  assert(this->seqSize == this->seq.size());
  for(unsigned int i = 0; i < this->seqSize; i++){
      os.write(reinterpret_cast<const char *> (&this->seq[i].first), sizeof(this->seq[i].first));
      os.write(reinterpret_cast<const char *> (&this->seq[i].second), sizeof(this->seq[i].second));
  }
  os.close();
}

void compress(const std::string & s, LZ78 & lz78){
  Tree tree;
  unsigned int p = 0;
  unsigned int n = (unsigned int) s.size();
  lz78.fileSize = n;
  while (p < n){
    Node * lpf = tree.findLPF(s, p);
    if (p + lpf->depth < n){
      unsigned char new_char = s[p + lpf->depth];
      lz78.seq.push_back(std::make_pair(lpf->id, new_char));
      tree.insert(lpf, new_char);
      if (tree.stat) {
        tree.stat->len_factors.push_back(lpf->depth + 1);
        tree.stat->len_lfactors.push_back(lpf->depth);
        tree.stat->len_rfactors.push_back(1);
      }
    }else{
      lz78.seq.push_back(std::make_pair(lpf->id, 0));
      lz78.ignoreLastChar = true;
    }
    p += lpf->depth + 1;
  }
  lz78.seqSize = (unsigned int) lz78.seq.size();
}
void decompress(std::string & s, const LZ78 & lz78){
  assert(lz78.seqSize > 0);
  assert(lz78.fileSize > 0);
  assert(lz78.seq.size() == lz78.seqSize);
  s.reserve(lz78.fileSize);
  std::vector<std::pair<unsigned int, unsigned int> > factor_pos; // (pos, len)
  unsigned int p = 0;
  factor_pos.push_back(std::make_pair(0, 0));
  for(unsigned int i = 0; i < lz78.seqSize - lz78.ignoreLastChar?1:0; i++){
    unsigned int factor_id = lz78.seq[i].first;
    unsigned char new_char = lz78.seq[i].second;
    assert(factor_id < factor_pos.size());
    factor_pos.push_back(std::make_pair(p, factor_pos[factor_id].second + 1));
    if (factor_id > 0){
      s += s.substr(factor_pos[factor_id].first, factor_pos[factor_id].second);
    }
    s += new_char;
    p += factor_pos[factor_id].second + 1;
  }
  if (lz78.ignoreLastChar){
    unsigned int factor_id = lz78.seq.back().first;
    s += s.substr(factor_pos[factor_id].first, factor_pos[factor_id].second);
  }
}


void seq2vars(
  const std::vector<std::pair<unsigned int, unsigned char> > & in_seq,
  std::vector<std::pair<unsigned int, unsigned int> > & vars){
    std::vector<unsigned int> * seq = new std::vector<unsigned int>();
    std::map<std::pair<unsigned int, unsigned int>, unsigned int>::iterator itr;
    unsigned int CHAR_SIZE = 256;
    std::vector<unsigned int> s2v(in_seq.size());
    assert(s2v.size() == in_seq.size());
    vars.clear();
    unsigned int cur = CHAR_SIZE;
    unsigned int seqsize = 0;
    vars.reserve(in_seq.size() * 2);
    seq->reserve(in_seq.size());
    assert(seq->size() == 0);

    // stores dummy values for SLPs
    for(size_t i = 0; i < CHAR_SIZE; i++){
      vars.push_back(std::make_pair(-1, -1));
    }

    // renumbers variable ids for SLPs
    for (unsigned int i = 0; i < in_seq.size(); i++){
      if (in_seq[i].first == 0) {
        // the factor represents a character
        assert(0 <= in_seq[i].second && in_seq[i].second <= 255);
        s2v[i+1] = (unsigned int)in_seq[i].second;
        seq->push_back((unsigned int)in_seq[i].second);
      } else if ((i + 1) == in_seq.size() && in_seq[i].second == 0){
        // ignores the last character
        seq->push_back(s2v[in_seq[i].first]);
      } else{
        s2v[i+1] = cur;
        vars.push_back(std::make_pair(s2v[in_seq[i].first], (unsigned int)in_seq[i].second));
        seq->push_back(cur);
        cur++;
      }
    }
    seqsize = seq->size();
    while(seqsize > 1){
      unsigned int new_seqsize = 0;
      for(size_t i = 1; i < seqsize; i+=2){
        vars.push_back(std::make_pair(seq->at(i-1), seq->at(i)));
        seq->at(new_seqsize) = cur;
        cur++;
        new_seqsize++;
      }
      if (seqsize % 2 == 1){
        seq->at(new_seqsize) = seq->at(seqsize-1);
        new_seqsize++;
      }
      seqsize = new_seqsize;
    }
    delete seq;
  }
}
