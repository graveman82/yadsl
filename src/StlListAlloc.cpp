#include <stdint.h>
#include <vector>
#include "StlListAlloc.h"

namespace yadsl_private{



struct StlListNodeMemBlock{
    StlListNodeMemBlock(){}
    unsigned char buf_[yadsl::kMAX_SIZE_OF_LIST_ELEMENT];
};

// кэш
static std::vector<StlListNodeMemBlock*> s_StlListNodeMemBlockVec;
void* StlListAllocateNode(){

    if (s_StlListNodeMemBlockVec.empty()) {
        s_StlListNodeMemBlockVec.push_back(new StlListNodeMemBlock());
    }
    StlListNodeMemBlock* mem = s_StlListNodeMemBlockVec.back();
    s_StlListNodeMemBlockVec.pop_back();
    return static_cast<void*>(&mem->buf_[0]);
}

void StlListDeallocateNode(void* p){

    StlListNodeMemBlock* mem = static_cast<StlListNodeMemBlock*>(p);
    s_StlListNodeMemBlockVec.push_back(mem);
}


void ClearStlListCache(){
#ifdef YADSL_LISTALLOCATOR_DEBUG
    int cElem = 0;
#endif
    while (!s_StlListNodeMemBlockVec.empty()){
        StlListNodeMemBlock* mem = s_StlListNodeMemBlockVec.back();
        s_StlListNodeMemBlockVec.pop_back();
        delete mem;
#ifdef YADSL_LISTALLOCATOR_DEBUG
        cElem++;
#endif
    }
#ifdef YADSL_LISTALLOCATOR_DEBUG
    printf("yadsl Stl cache size was: %d elements\n", cElem);
#endif
}
} // end of yadsl_private
