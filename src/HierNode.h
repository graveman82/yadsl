#ifndef YADSL_HIERNODE_H
#define YADSL_HIERNODE_H

#include <wx/wx.h>
#include <wx/sharedptr.h>
#include "StlListAlloc.h"

namespace yadsl
{

class HierarchyNode {
public:
    typedef wxSharedPtr<HierarchyNode> NodePtr;
    typedef std::list <NodePtr, StlListAllocator<NodePtr > > ChildList;

    // Предикат для поиска по сырому указателю
    class FindPred{
        HierarchyNode* node_;
    public:
        FindPred(HierarchyNode* node) : node_(node) {}
        bool operator()(const NodePtr& nodePtr){
            return node_ == nodePtr.get();
        }
    };

private:
    HierarchyNode* parent_;
    ChildList childList_;

public:
    HierarchyNode();

    virtual ~HierarchyNode() {}

    const HierarchyNode* GetParent() const { return parent_; }

    void SetParent(HierarchyNode* parent) { parent_ = parent; }

    ChildList& GetChildList() { return childList_; }

    const ChildList& GetChildList() const { return childList_; }

    /** @brief Присоединяет другой узел к данному в качестве дочернего. */
    void LinkChild(HierarchyNode* node);

    /** @brief Присоединяет другой узел к данному в качестве дочернего.
    Эту функцию следует использовать, если заданный узел был отсоединен от
    предыдущего родительского узла.
    */
    void LinkChild(NodePtr nodePtr);

    /** @brief Отсоединяет данный узел от родительского.
    Для узла без родителя ничего не делается.
    Для узла, у которого родитель есть, если не принять возвращаемый указатель,
    то узел будет уничтожен.
    @return пустой указатель для узла без родителя или указатель на данный узел.

    */
    NodePtr Unlink();


};

} // end of yadsl

#endif // YADSL_HIERNODE_H

