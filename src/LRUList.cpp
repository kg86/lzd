// -*- coding: utf-8 -*-
#include <iostream>

#include "LRUList.hpp"
#include "common_utils.hpp"

const unsigned int LRUList::NONE = UINT_MAX;


LRUList::LRUList(unsigned int size) :
  Prevs(size+2, NONE), Nexts(size+2, NONE),
  SIZE(size), VFRONT(SIZE), VBACK(SIZE+1){
  assert(SIZE >= 2);

  Prevs[VFRONT] = NONE;
  Nexts[VFRONT] = VBACK;
  Prevs[VBACK] = VFRONT;
  Nexts[VBACK] = NONE;

  // stores unused elements from VBACK to VFRONT,
  // and stores used elements from VFRONT to VBACK
  // does not store element id 0.
  // init state of Nexts is VFRONT -> VBACK -> 1 -> 2 -> ... -> SIZE-1 -> VFRONT
  Nexts[VFRONT] = VBACK;
  Prevs[VFRONT] = SIZE-1;

  Prevs[VBACK] = VFRONT;
  Nexts[VBACK] = 1;

  Prevs[1] = VBACK;
  Nexts[1] = 2;
  for(unsigned int i = 2; i < SIZE; i++){
    Prevs[i] = i-1;
    Nexts[i] = i+1;
  }
  Prevs[SIZE-1] = SIZE-2;
  Nexts[SIZE-1] = VFRONT;
}

void LRUList::use(unsigned int idx){
  assert(1 <= idx && idx < this->SIZE);
  this->del(idx);
  this->insertNext(idx, VFRONT);
}

void LRUList::dump(){
  size_t i;
  size_t count = 0;
  std::cerr << "dump LRUList =[";
  for(i = VFRONT; i != VBACK; ){
    count++;
    std::cerr << i << ", ";
    i = Nexts[i];
  }
  std::cerr << "] " << count << " are contain" << std::endl;
}
