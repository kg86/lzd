// -*- coding: utf-8 -*-
#pragma once

#include <vector>
#include <climits>
#include <cassert>
#include <iostream>

#include "common.hpp"

class LRUList{

  std::vector<unsigned int> Prevs; // indicates previous elements
  std::vector<unsigned int> Nexts; // indicates next elements
public:
  static const unsigned int NONE;
  const unsigned int SIZE;
  const unsigned int VFRONT; // virtual element which is in front of the first element;
  const unsigned int VBACK; // virtual element which is behind the last element;

  LRUList();

  /**
   *  initializes like that VFRONT <-> 1 <-> 2 <-> ... <-> $size-1 <-> VBACK
   *
   *  @param size
   *
   */
  LRUList(unsigned int size);
  inline unsigned int front() const {return Nexts[VFRONT];};
  inline unsigned int back() const {return Prevs[VBACK];};

  /**
   *  deletes the element of $idx.
   *  unlinks from the predecessor and the successor.
   *
   *  @param idx
   */
  inline void del(int idx){
    if (UTIL::DEBUG_LEVEL > 2){
      std::cerr << "LRUList DEL idx=" << idx << " size=" << Nexts.size()
                << " Nexts[idx]=" << Nexts[idx]
                << " Prevs[idx]=" << Prevs[idx]
                << std::endl;
    }
    assert(0 <= idx && (unsigned int) idx < Prevs.size());

    // not changes Prevs[idx] and Nexts[idx]
    Prevs[Nexts[idx]] = Prevs[idx];
    Nexts[Prevs[idx]] = Nexts[idx];
  }
  inline void del_unused(int idx){
    this->del(idx);
    this->insertNext(idx, VBACK);
  }

  /** 
   * swap  $idx1 and $idx2
   * 
   * @param idx1 
   * @param idx2 
   */
  inline void swap(unsigned int idx1, unsigned int idx2){
    unsigned int tmp_prev = Prevs[idx1];
    unsigned int tmp_next = Nexts[idx1];
    Prevs[idx1] = Prevs[idx2];
    Nexts[idx1] = Nexts[idx2];

    Prevs[idx2] = tmp_prev;
    Nexts[idx2] = tmp_next;
  }

  /** 
   * inserts $new_element next of $left
   * 
   * @param new_element 
   * @param left 
   */
  inline void insertNext(unsigned int new_element, unsigned int left){
    Nexts[new_element] = Nexts[left];
    Prevs[new_element] = left;

    Nexts[left] = new_element;
    Prevs[Nexts[new_element]] = new_element;
  }

  /** 
   * uses $idx and updates the lru list
   * 
   * @param idx 
   */
  void use(unsigned int idx);

  /**
   *  displays all elements in the list
   */
  void dump();
};
