#include "HierNode.h"
#include <algorithm>

namespace yadsl
{

HierarchyNode::HierarchyNode() : parent_(0) {}

void HierarchyNode::LinkChild(HierarchyNode* node) {
#ifdef YADSL_USE_WXDEBUG
    wxASSERT(node != 0);
    wxASSERT_MSG(node->GetParent() == 0, "you must unlink given node before");
#endif
    node->SetParent(this);
    childList_.push_back(NodePtr(node));
}

void HierarchyNode::LinkChild(NodePtr nodePtr) {
#ifdef YADSL_USE_WXDEBUG
    wxASSERT(nodePtr.get() != 0);
    wxASSERT_MSG(nodePtr->GetParent() == 0, "you must unlink given node before");
#endif
    nodePtr->SetParent(this);
    childList_.push_back(nodePtr);
}

HierarchyNode::NodePtr HierarchyNode::Unlink() {
    if (parent_ == 0) return NodePtr();
    ChildList::iterator it = std::find_if(parent_->childList_.begin(),
                                          parent_->childList_.end(),
                                          FindPred(this));
#ifdef YADSL_USE_WXDEBUG
    wxASSERT_MSG (it != parent_->childList_.end(), wxT("node with parent must be in child list of parent"));
#endif
    NodePtr nodePtr (*it);

    parent_->childList_.erase(it);
    nodePtr->SetParent(0);
    return nodePtr;
}

} // end of yadsl
