#ifndef YADSL_LIST_H
#define YADSL_LIST_H

#include <vector>
#include <wx/wx.h>
#include <wx/SharedPtr.h>

#define YADSL_LIST_TEST 1

#if YADSL_LIST_TEST
#include <stdio.h>
#endif

namespace yadsl
{
enum {kPOD_LIST = 1};
template <typename T, int POD = 0>
class List{
public:
    class Node {
        friend class List<T, POD>;
        friend class List<T, POD>::Pool;
    private:
        Node *next_, *prev_;
        uint8_t data_[sizeof(T)];

    public:
        Node() : next_(0), prev_(0) {
#if YADSL_LIST_TEST
            printf("Node (%x) ctor\n", (int)this);
#endif
        }

        const Node* GetNext() const { return next_; }
        Node* GetNext() { return next_; }
        const Node* GetPrev() const { return prev_; }
        Node* GetPrev() { return prev_; }

        T& GetData() { T* t = reinterpret_cast<T*>(&data_[0]); return *t; }
        ~Node() {
#if YADSL_LIST_TEST
            printf("~Node (%x)\n", (int)this);
#endif
        }
    };

private:
    typedef Node* NodePtr;

    class Pool {
        typedef std::vector<NodePtr> NodePtrVec;
        NodePtrVec freeV_;
        size_t cNewCalls_; // счетчик вызовов new

    public:
        Pool() : cNewCalls_(0) {}

        ~Pool() {
#ifdef YADSL_USE_WXDEBUG
            wxASSERT(freeV_.size() == cNewCalls_);
#endif
            for (size_t i = 0; i < freeV_.size(); ++i) {
                delete freeV_[i];
            }
        }

        NodePtr Alloc() {
            if (freeV_.empty()){
                freeV_.push_back(new Node);
                cNewCalls_++;
            }
            NodePtr p = freeV_.back();
            freeV_.pop_back();
            return p;
        }

        void Free(NodePtr p) {
            if (!POD) {
                T& data = p->GetData();
                data.~T();
            }
            freeV_.push_back(p);
        }
    };

    Pool pool_;
    NodePtr headTail_;
public:
    List() : headTail_(0) {
        headTail_ = pool_.Alloc();
    }

    ~List() {
#ifdef YADSL_USE_WXDEBUG
        wxASSERT(empty());
#endif
        pool_.Free(headTail_);
    }

    bool valid() const { return ((headTail_->next_ == 0 && headTail_->prev_ == 0) || (headTail_->next_ != 0 && headTail_->prev_ != 0)); }
    bool empty() const {
#ifdef YADSL_USE_WXDEBUG
        wxASSERT(valid());
#endif
        return headTail_->next_ == 0;
    }

    NodePtr push_front(const T& data) {
        NodePtr node = pool_.Alloc();
        if (!POD) {
            new((void*)&node->GetData()) T(data);
        }
        else {
            node->GetData() = data;
        }

        if (headTail_->next_ == 0) { // список пуст
            headTail_->prev_ = node;
        }
        else {
            headTail_->next_->prev_ = node;
            node->next_ = headTail_->next_;
        }

        headTail_->next_ = node;
        return node;
    }

    NodePtr push_back(const T& data) {
        NodePtr node = pool_.Alloc();
        if (!POD) {
            new((void*)&node->GetData()) T(data);
        }
        else {
            node->GetData() = data;
        }

        if (headTail_->prev_ == 0) { // список пуст
            headTail_->next_ = node;
        }
        else {
            headTail_->prev_->next_ = node;
            node->prev_ = headTail_->prev_;
        }

        headTail_->prev_ = node;
        return node;
    }

    void erase(NodePtr node) {

        if (node->next_ != 0) {
            node->next_->prev_ = node->prev_;
        }
        else {
            headTail_->prev_ = node->prev_;
        }
        if (node->prev_ != 0) {
            node->prev_->next_ = node->next_;
        }
        else {
            headTail_->next_ = node->next_;
        }
        node->next_ = node->prev_ = 0;

        pool_.Free(node);
    }

    void clear() {
        while (!empty()) erase(GetFirst());
    }

    NodePtr GetFirst() { return headTail_->next_; }
    const NodePtr GetFirst() const { return headTail_->next_; }

    NodePtr GetLast() { return headTail_->prev_; }
    const NodePtr GetLast() const { return headTail_->prev_; }

};


//-----------------------------------------------------------------------------

#if 0 //code for test
#include <iostream>
#include <stdio.h> // printf()
#include <wx/wx.h>

#include "List.h"

#include <stdlib.h> // rand()
#include  <map>

int Random(int high) {
    double k = (double)rand() / RAND_MAX;
    return high * k;
}

void Test() {
    yadsl::List<std::string> l;
    yadsl::List<std::string>::Node* nodes[10] = {0};
    nodes[0] = l.push_back("Arthur King");
    nodes[1] = l.push_front("headbanging_man");
    nodes[2] = l.push_back("Armed");
    nodes[3] = l.push_back("World of fools");
    nodes[4] = l.push_front("Chris Boltendal");
    nodes[5] = l.push_front("Shotgun messiah");
    nodes[6] = l.push_back("Round table");
    nodes[7] = l.push_front("The clans are marching");
    nodes[8] = l.push_front("Rebellion");
    l.erase(nodes[1]);
    l.erase(nodes[3]);
    l.erase(nodes[5]);
    l.erase(nodes[8]);
    nodes[1] = l.push_front("headbanging_man2");
    nodes[3] = l.push_back("World of fools2");
    nodes[5] = l.push_front("Shotgun messiah2");
    nodes[8] = l.push_front("Rebellion2");

    printf("List nodes:\n");
    for (yadsl::List<std::string>::Node* node = l.GetFirst(); node != 0; node = node->GetNext()) {
        printf("%s\n", node->GetData().c_str());
    }
    l.clear();


}

void Test2() {
    yadsl::List<int, yadsl::kPOD_LIST> l;
    yadsl::List<int, yadsl::kPOD_LIST>::Node* nodes[10] = {0};
    const int kMax = 100;
    nodes[0] = l.push_back(Random(kMax));
    nodes[1] = l.push_front(Random(kMax));
    nodes[2] = l.push_back(Random(kMax));
    nodes[3] = l.push_back(Random(kMax));
    nodes[4] = l.push_front(Random(kMax));
    nodes[5] = l.push_front(Random(kMax));
    nodes[6] = l.push_back(Random(kMax));
    nodes[7] = l.push_front(Random(kMax));
    nodes[8] = l.push_front(Random(kMax));
    l.erase(nodes[1]);
    l.erase(nodes[3]);
    l.erase(nodes[5]);
    l.erase(nodes[8]);
    nodes[1] = l.push_front(Random(kMax));
    nodes[3] = l.push_back(Random(kMax));
    nodes[5] = l.push_front(Random(kMax));
    nodes[8] = l.push_front(Random(kMax));

    printf("List nodes:\n");
    for (yadsl::List<int, yadsl::kPOD_LIST>::Node* node = l.GetFirst(); node != 0; node = node->GetNext()) {
        printf("%d\n", node->GetData());
    }
    l.clear();


}

int main(){

    Test();
    std::cout << "Press any key to quit..." << std::endl;
    std::cin.get();
    std::cin.get();
}

#endif

} // end of yadsl
#endif // YADSL_LIST_H
