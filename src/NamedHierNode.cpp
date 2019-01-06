// Test HierarchyNode
#include <conio.h> // getch()
#include "NamedHierNode.h"
#include <algorithm>

namespace yadsl {

NamedHierarchyNode::NamedHierarchyNode(const char* name) : name_(name) {
    printf("NamedHierarchyNode (%x, \"%s\") ctor\n",(int)this, name_.c_str());
}

NamedHierarchyNode::~NamedHierarchyNode() {
    printf("NamedHierarchyNode (%x, \"%s\") dtor\n", (int)this, name_.c_str());
}

NamedHierarchyNode* NamedHierarchyNode::Child(const char* name) {
    ChildList::iterator it = std::find_if(GetChildList().begin(),
                                      GetChildList().end(),
                                      FindPred_ByName(name));
    if (it != GetChildList().end()){
        return (NamedHierarchyNode*)it->get();
    }
    return 0;
}

void NamedHierarchyNode::PrintAllNames(int indent){
    for (int i = 0; i < indent; ++i) printf(" ");
    printf ("node: %s\n", name_.c_str());//getch();
    for (ChildList::iterator it = GetChildList().begin(); it != GetChildList().end(); ++it){
        ((NamedHierarchyNode*)it->get())->PrintAllNames(indent+2);
    }
}

} // end of yadsl

void TestHierarchyNode(){
    using yadsl::NamedHierarchyNode;
    NamedHierarchyNode root("Root");
    root.LinkChild(new NamedHierarchyNode("upper arm(r)"));
    root.Child("upper arm(r)")->LinkChild(new NamedHierarchyNode("lower arm(r)"));
    root.Child("upper arm(r)")->Child("lower arm(r)")->LinkChild(new NamedHierarchyNode("hand(r)"));
    root.Child("upper arm(r)")->Child("lower arm(r)")->Child("hand(r)")->LinkChild(new NamedHierarchyNode("finger1(r)"));
    root.Child("upper arm(r)")->Child("lower arm(r)")->Child("hand(r)")->LinkChild(new NamedHierarchyNode("finger2(r)"));
    root.Child("upper arm(r)")->Child("lower arm(r)")->Child("hand(r)")->LinkChild(new NamedHierarchyNode("finger3(r)"));
    root.LinkChild(new NamedHierarchyNode("upper arm(l)"));
    root.Child("upper arm(l)")->LinkChild(new NamedHierarchyNode("lower arm(l)"));
    root.Child("upper arm(l)")->Child("lower arm(l)")->LinkChild(new NamedHierarchyNode("hand(l)"));
    root.Child("upper arm(l)")->Child("lower arm(l)")->Child("hand(l)")->LinkChild(new NamedHierarchyNode("finger1(l)"));
    root.Child("upper arm(l)")->Child("lower arm(l)")->Child("hand(l)")->LinkChild(new NamedHierarchyNode("finger2(l)"));
    root.Child("upper arm(l)")->Child("lower arm(l)")->Child("hand(l)")->LinkChild(new NamedHierarchyNode("finger3(l)"));

    root.PrintAllNames();
    printf("Rip right lower arm and link it to left\n");
    root.Child("upper arm(l)")->LinkChild(root.Child("upper arm(r)")->Child("lower arm(r)")->Unlink());
    root.PrintAllNames();
    printf("Rip right finger2\n");
    root.Child("upper arm(l)")->Child("lower arm(r)")->Child("hand(r)")->Child("finger2(r)")->Unlink();
    root.PrintAllNames();
    printf("Rip left hand\n");
    root.Child("upper arm(l)")->Child("lower arm(l)")->Child("hand(l)")->Unlink();
    root.PrintAllNames();
}
