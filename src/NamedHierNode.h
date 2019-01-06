#ifndef YADSL_NAMEDHIERNODE_H
#define YADSL_NAMEDHIERNODE_H

#include "HierNode.h"

namespace yadsl {

class NamedHierarchyNode : public yadsl::HierarchyNode{
    // Предикат для поиска по имени
    class FindPred_ByName{
        const char* name_;
    public:
        FindPred_ByName(const char* name) : name_(name) {}
        bool operator()(const NodePtr& nodePtr){
            return ((NamedHierarchyNode*)nodePtr.get())->name_ == name_;
        }
    };
public:
    std::string name_;

    NamedHierarchyNode(const char* name = "");
    virtual ~NamedHierarchyNode();
    NamedHierarchyNode* Child(const char* name);
    void PrintAllNames(int indent = 0);
};

} // end of yadsl

#endif // YADSL_NAMEDHIERNODE_H

