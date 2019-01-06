#include "Entity.h"
#include <algorithm>

#include "UniqIntGen.h"

namespace yadsl
{

const uint Entity::kComponentIdNotAssigned_ = 0;
static UniqIntGenerator s_uig; // генератор уникальных целочисленных идентификаторов

uint Entity::GenerateComponentId() {
    static uint count = kComponentIdNotAssigned_;
    return ++count;
}

Entity::Entity() {
    id_ = s_uig.Get();

}

Entity::~Entity() {
    s_uig.Put(id_);
}

bool Entity::GetComponent(uint componentId, uint componentIndex, PVoid* ppMem, PVoid* ppOwnerPos) {
#ifdef YADSL_USE_IDVALUEVECTOR_IN_ENTITY
    ComponentMap::ElementVec::iterator start;
    ComponentMap::ElementVec::iterator end;
    bool fFound = componentMap_.Find(componentId, start, end);
    if (componentIndex == kNoIndex) {
        return fFound;
    }
    fFound = false;
    for (; start != end; ++start) {
        const ComponentItem& item = start->GetValue();
        if (item.index_ == componentIndex) {
            if (ppMem != 0) *ppMem = item.mem_;
            if (ppOwnerPos != 0) *ppOwnerPos = item.ownerPos_;
            fFound = true;
            break;
        }
    }
#else
    ComponentMap::iterator start = componentMap_.lower_bound(componentId);
    ComponentMap::iterator end = componentMap_.upper_bound(componentId);
    if (componentIndex == kNoIndex) {
        return start != end;
    }
    bool fFound = false;
    for (; start != end; ++start) {
        ComponentItem& item = start->second;
        if (item.index_ == componentIndex) {
            if (ppMem != 0) *ppMem = item.mem_;
            if (ppOwnerPos != 0) *ppOwnerPos = item.ownerPos_;
            fFound = true;
            break;
        }
    }
#endif
    return fFound;
}

bool Entity::SetOrInsertComponent(uint componentId, uint componentIndex, void* componentMem) {
#ifdef YADSL_USE_IDVALUEVECTOR_IN_ENTITY
    ComponentMap::ElementVec::iterator start;
    ComponentMap::ElementVec::iterator end;
    componentMap_.Find(componentId, start, end);
    bool fFound = false;
    for (; start != end; ++start) {
        const ComponentItem& item = start->GetValue();
        if (item.index_ == componentIndex) {
            start->SetValue(ComponentItem (componentIndex, componentMem));
            fFound = true;
            break;
        }
    }
    if (!fFound) {
        componentMap_.Insert(componentId, ComponentItem (componentIndex, componentMem));
    }
#else
    ComponentMap::iterator start = componentMap_.lower_bound(componentId);
    ComponentMap::iterator end = componentMap_.upper_bound(componentId);
    bool fFound = false;
    for (; start != end; ++start) {
        ComponentItem& item = start->second;
        if (item.index_ == componentIndex) {
            item.mem_ = componentMem;
            fFound = true;
            break;
        }
    }
    if (!fFound) {
        componentMap_.insert(std::make_pair(componentId, ComponentItem (componentIndex, componentMem)));
    }
#endif
    return fFound;
}

bool Entity::SetComponentOwnerPos(uint componentId, uint componentIndex, void* ownerPos) {
#ifdef YADSL_USE_IDVALUEVECTOR_IN_ENTITY
    ComponentMap::ElementVec::iterator start;
    ComponentMap::ElementVec::iterator end;
    componentMap_.Find(componentId, start, end);
    bool fFound = false;
    for (; start != end; ++start) {
        const ComponentItem& item = start->GetValue();
        if (item.index_ == componentIndex) {
            start->GetValue().ownerPos_ = ownerPos;
            fFound = true;
            break;
        }
    }
#else
    ComponentMap::iterator start = componentMap_.lower_bound(componentId);
    ComponentMap::iterator end = componentMap_.upper_bound(componentId);
    bool fFound = false;
    for (; start != end; ++start) {
        ComponentItem& item = start->second;
        if (item.index_ == componentIndex) {
            item.ownerPos_ = ownerPos;
            fFound = true;
            break;
        }
    }
#endif
    return fFound;
}

void Entity::EraseComponent(uint componentId, uint componentIndex) {
#ifdef YADSL_USE_IDVALUEVECTOR_IN_ENTITY
    ComponentMap::ElementVec::iterator start;
    ComponentMap::ElementVec::iterator end;
    componentMap_.Find(componentId, start, end);
    bool fFound = false;
    for (; start != end; ++start) {
        const ComponentItem& item = start->GetValue();
        if (item.index_ == componentIndex) {
            fFound = true;
            break;
        }
    }
    if (fFound) {
        componentMap_.Erase(start);
    }
#else
    ComponentMap::iterator start = componentMap_.lower_bound(componentId);
    ComponentMap::iterator end = componentMap_.upper_bound(componentId);
    bool fFound = false;
    for (; start != end; ++start) {
        ComponentItem& item = start->second;
        if (item.index_ == componentIndex) {
            fFound = true;
            break;
        }
    }
    if (fFound) {
        componentMap_.erase(start);
    }
#endif
}

} // end of yadsl
