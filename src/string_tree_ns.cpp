// -*- coding: utf-8 -*-
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <sys/stat.h>
#include <cassert>
#include <algorithm>
#include <numeric>

#include "string_tree_ns.hpp"
#include "common.hpp"

namespace NOT_STREAM{
namespace STree{
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
  // computes alive time for each of the rest factors
  for (unsigned int i = 0; i < pos_prev_used.size(); i++){
    unsigned int pos_prev = pos_prev_used[i];
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
Node::Node(unsigned int in_edge_begin, unsigned int in_edge_len, unsigned int depth,
           unsigned int id, unsigned int fid):
  in_edge_begin(in_edge_begin),
  in_edge_len(in_edge_len),
  depth(depth),
  id(id),
  fid(fid),
  parent(NULL){
  }

void Node::info() const{
  std::cerr << " in_edge_begin=" << in_edge_begin
            << " in_edge_len=" << in_edge_len
            << " depth=" << depth
            << " id=" << id
            << " fid=" << fid
            << " children.size()=" << children.size();
  if (parent){
    std::cerr << " par_id=" << parent->id;
    std::cerr << " par_fid=" << parent->fid;
  }
  std::cerr << std::endl;
}

Tree::Tree(const std::string & str) : str(str) {
  if (UTIL::DEBUG_LEVEL > 0) stat = new Stat();
  else stat = NULL;
  // initially adds root node
  this->num_nodes = this->num_fnodes = 0;
  this->root = this->createNode(true, UINT_MAX, 0, 0); // root id and fid are set to 0 and 0 respectively.
  if (UTIL::DEBUG_LEVEL > 0) {
    stat = new Stat();
  }
}

Tree::~Tree(){
  for(unsigned int i = 0; i < this->nodes.size(); i++){
    if (this->nodes[i]) delete this->nodes[i];
  }
  if (this->stat) delete this->stat;
}

Node * Tree::createNode(bool isFactor, unsigned int in_edge_begin,
                        unsigned int in_edge_len, unsigned int depth){
  Node * node;
  if (isFactor){
    node = new Node (in_edge_begin, in_edge_len, depth, this->getNewId(), this->getNewFId());
    registerFNode(node);
  }else{
    node = new Node (in_edge_begin, in_edge_len, depth, this->getNewId(), UINT_MAX);
  }
  registerNode(node);
  return node;
}

Node * Tree::getChildBeginWith(const Node * node, char c) const{
  assert(node != NULL);
  std::vector<Node *>::const_iterator itr;
  for(itr = node->children.begin(); itr != node->children.end(); itr++){
    if (getInFirstChar(*itr) == c){
      return *itr;
    }
  }
  return NULL;
}

Node * Tree::getChild(const Node * node, unsigned int strbegin) const{
  assert(node != NULL);
  assert(strbegin < this->getStrSize());
  Node * candidate_child = this->getChildBeginWith(node, this->getCharAt(strbegin));
  if (candidate_child != NULL && canReachTo(candidate_child, strbegin)) return candidate_child;
  else return NULL;
}

bool Tree::canReachTo(const Node * node, unsigned int strbegin) const{
  assert(node != NULL);
  assert(strbegin < this->getStrSize());
  if (node->depth == 1){
    if (getInFirstChar(node) == (char)this->getCharAt(strbegin)) return true;
    else return false;
  }else{
    return node->in_edge_len == LCPToNode(node, strbegin);
  }
}

bool Tree::canReachFromTo(Node * from, Node * to, unsigned int strbegin){
  if (from == to) return true;
  Node * child = this->getChild(from, strbegin);
  if (child == NULL) return false;
  return canReachFromTo(child, to, strbegin + child->edgeLen());
}

char Tree::getInFirstChar(const Node * node) const{
  assert(node != NULL);
  if (node->depth == 1){
    return (char) node->in_edge_begin;
  }else{
    return str.at(node->in_edge_begin);
  }
}

Node * Tree::addChild(Node * parent, bool isFactor, unsigned int in_edge_begin, unsigned int in_edge_len){
  assert(parent != NULL);
  unsigned int depth = parent->depth + in_edge_len;
  Node * child = this->createNode(isFactor, in_edge_begin, in_edge_len, depth);
  this->addChild(parent, child);
  return child;
}

Node * Tree::insertFactorNode(Node * node, unsigned int strbegin, unsigned int len){
  if (this->stat) {
    this->stat->len_lfactors.push_back(node->depth);
    this->stat->len_rfactors.push_back(len);
    this->stat->len_factors.push_back(node->depth + len);
  }

  assert(len > 0);
  // Note, $par may be a given node.
  Node * last_node        = findLastNNodeFrom(node, strbegin, len);
  assert(last_node != NULL);
  assert(last_node->depth >= node->depth);
  unsigned int length_to_last_node = last_node->depth - node->depth; // the path length between a givne node and par.
  unsigned int strpar_begin = strbegin + length_to_last_node;
  // if $child is NULL, $par must be a leaf.
  Node * child;
  if (strpar_begin < this->getStrSize()) child = this->getChildBeginWith(last_node, this->getCharAt(strpar_begin));
  else child = NULL;

  if(length_to_last_node == len){
    // We have no remain string.
    // $par must be a branch node, and which is not a factor node.
    // we just change the node to a factor node;
    assert(!last_node->isFactor());
    this->change2FNode(last_node);
    return last_node;
  }else if (child == NULL){
    // There is no child which we can move with the remain string.
    // We only have to insert a new node from $par
    assert(len > (last_node->depth - node->depth));
    assert(!(last_node == this->root && len == 1));
    return this->addChild(last_node, true, strpar_begin, len - (strpar_begin - strbegin));
  }else{
    // We can move to the middle of an edge with the remain string.
    // We have to create a new branch node between $par and $child.
    // If the remain string is completely included in $str[$strbegin:$strend],
    // the new node is a factor node,
    // and other wise we have to create new branch node and insert a leaf.
    unsigned int lcp = this->LCPToNode(child, strpar_begin, len - length_to_last_node);
    bool isFactor = false;
    if (strpar_begin + lcp == (strbegin + len)) isFactor = true;

    Node * branchNode = this->splitEdge(last_node, child, lcp);

    if (!isFactor){
      return this->addChild(branchNode, true, strpar_begin + lcp, len - (strpar_begin + lcp - strbegin));
    }else{
      this->change2FNode(branchNode);
      return branchNode;
    }
  }
}

bool Tree::unlinkChild(Node * par, Node * child, Node * replace_node){
  std::vector<Node *>::iterator itr;
  for(itr = par->children.begin(); itr != par->children.end(); itr++){
    if (*itr == child){
      if (replace_node){
        (*itr) = replace_node;
        replace_node->parent = par;
      }else{
        par->children.erase(itr);
      }
      child->parent = NULL;
      return true;
    }
  }
  return false;
}


Node * Tree::splitEdge(Node * par, Node * child, unsigned int len){
  assert(child->parent == par);
  assert(child->in_edge_len > len);
  // deletes link from $par to $child
  this->unlinkChild(par, child);

  // inserts a branch node
  Node * middleNode = this->addChild(par, false, child->in_edge_begin, len);
  // child->parent = middleNode;
  this->addChild(middleNode, child);

  // updates the child's edge
  child->in_edge_begin += len;
  child->in_edge_len -= len;
  return middleNode;
}

unsigned int Tree::findLastNodeIdFrom(const Node * cur, unsigned long long strbegin, bool findFNode, unsigned int len) const{
  if (len == UINT_MAX) len = (unsigned int) this->getStrSize();
  assert(cur != NULL);
  const Node * lastFNode = NULL;
  if (findFNode && cur->isFactor()) lastFNode = cur;
  unsigned long long i = strbegin;
  while(cur && i < this->getStrSize()){

    Node * child = getChild(cur, i);
    // not founds the child which matches $str[i..]
    if (child == NULL || (i + child->edgeLen()) > (strbegin + len)) break;
    cur = child;
    i += child->edgeLen();
    if (cur->isFactor()) lastFNode = cur;
  }
  assert(cur != NULL);
  if (findFNode) return lastFNode->id;
  else return cur->id;
}

unsigned int Tree::LCPFromNode(Node * node, unsigned int strbegin, unsigned int len){
  if (len == UINT_MAX) len = (unsigned int) this->getStrSize();
  if (len == 0) return 0;
  if (node == NULL || strbegin >= this->getStrSize()) return 0;
  Node * child = this->getChildBeginWith(node, this->getCharAt(strbegin));
  if (child == NULL) return 0;
  if (this->canReachTo(child, strbegin)){
    return child->edgeLen() + LCPFromNode(child, strbegin + child->edgeLen(), len-child->edgeLen());
  }else{
    return LCPToNode(child, strbegin, len);
  }
}

unsigned int Tree::LCPToNode(const Node * node, unsigned int strbegin, unsigned int len) const{
  if (len == UINT_MAX) len = (unsigned int) this->getStrSize();
  if (len == 0) return 0;
  assert(node != NULL);
  unsigned int lcp = 0;
  
  if (node->depth == 1){
    if (getInFirstChar(node) == str[strbegin]) return 1;
    else return 0;
  }


  for(; (lcp < node->in_edge_len) && (strbegin+lcp < this->getStrSize()) && lcp < len; lcp++){
    if (str[node->in_edge_begin + lcp] != str[strbegin+lcp]) return lcp;
  }
  return lcp;
}

void Tree::toString(const Node * node, std::string & os) const{
  if (node == NULL) return;
  if (node == this->root) return;
  this->toString(node->parent, os);

  if (node->depth == 1) {
    os.push_back((char) (node->in_edge_begin));
  }else{
    os.append(this->str, node->in_edge_begin, node->in_edge_len);
  }
}

inline void Tree::info(){
  std::cerr << "** TREE INFO:"
            << " num_nodes=" << this->num_nodes
            << " num_fnodes=" << this->num_fnodes
            << std::endl;
}
void Tree::showAllNode() const{
  std::string tmp = "";
  showAllNode(this->root, tmp);
}
void Tree::showAllNode(const Node * node, std::string & os) const{
  assert(node != this->getParent(node));
  this->verify(node);
  std::string new_os = os;
  if (node != this->root){
    new_os += this->getEdgeLabel(node);
  }
  std::cerr << "[" + new_os+ "] ";
  node->info();
  for (unsigned int i = 0; i < node->children.size(); i++){
    this->showAllNode(node->children[i], new_os);
  }
}

bool Tree::isValid() const{
  return isValid(this->root);
}

bool Tree::isValid(const Node * node) const{
  assert(node->parent != node);
  bool res = true;
  for (unsigned int i = 0; i < node->children.size(); i++){
    res &= this->isValid(node->children[i]);
  }
  return res;
}
}
}
