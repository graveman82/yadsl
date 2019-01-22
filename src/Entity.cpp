#include "Entity.h"
#include <algorithm>
#ifdef YADSL_USE_WXDEBUG
#include <wx/wx.h>
#endif
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

#ifdef YADSL_USE_IDVALUEVECTOR_IN_ENTITY
bool Entity::FindComponent(ComponentMap::ElementVec::iterator& it, const ComponentItem** ppItem, uint componentId, uint componentIndex) {
#else
bool Entity::FindComponent(ComponentMap::iterator& it, const ComponentItem** ppItem, uint componentId, uint componentIndex) {
#endif
#ifdef YADSL_USE_IDVALUEVECTOR_IN_ENTITY
    ComponentMap::ElementVec::iterator start;
    ComponentMap::ElementVec::iterator end;
    it = componentMap_.EndIterator();
    bool fFound = componentMap_.Find(componentId, start, end);
    if (componentIndex == kNoIndex) {
        return fFound;
    }
    fFound = false;
    for (; start != end; ++start) {
        const ComponentItem& startItem = start->GetValue();
        if (startItem.index_ == componentIndex) {
            it = start;
#ifdef YADSL_USE_WXDEBUG
            wxASSERT(ppItem != 0);
#endif
            *ppItem = &startItem;
            fFound = true;
            break;
        }
    }
#else
    ComponentMap::iterator start = componentMap_.lower_bound(componentId);
    ComponentMap::iterator end = componentMap_.upper_bound(componentId);
    it = componentMap_.end();
    if (componentIndex == kNoIndex) {
        return start != end;
    }
    bool fFound = false;
    for (; start != end; ++start) {
        ComponentItem& startItem = start->second;
        if (startItem.index_ == componentIndex) {
            it = start;
#ifdef YADSL_USE_WXDEBUG
            wxASSERT(ppItem != 0);
#endif
            *ppItem = &startItem;
            fFound = true;
            break;
        }
    }
#endif
    return fFound;
}

bool Entity::GetComponent(uint componentId, uint componentIndex, PVoid* ppMem, PVoid* ppOwnerPos) {
    const ComponentItem* item = 0;
// Часть, зависящая от реализации
#ifdef YADSL_USE_IDVALUEVECTOR_IN_ENTITY
    typename ComponentMap::ElementVec::iterator it;
#else
    ComponentMap::iterator it;
#endif // Конец части, зависящей от реализации

    bool fFound = FindComponent(it, &item, componentId, componentIndex);
    if (componentIndex == kNoIndex) return fFound;
    if (fFound) {
#ifdef YADSL_USE_WXDEBUG
        wxASSERT(item != 0);
#endif
        if (ppMem != 0) *ppMem = item->mem_;
        if (ppOwnerPos != 0) *ppOwnerPos = item->ownerPos_;
    }
    return fFound;
}

bool Entity::SetOrInsertComponent(uint componentId, uint componentIndex, void* componentMem) {
#ifdef YADSL_USE_IDVALUEVECTOR_IN_ENTITY
    ComponentMap::ElementVec::iterator start;
    bool fFound = FindComponent(start, 0, componentId, componentIndex);
    if (!fFound) {
        componentMap_.Insert(componentId, ComponentItem (componentIndex, componentMem));
    }
    else {
#ifdef YADSL_USE_WXDEBUG
        wxASSERT (start != componentMap_.EndIterator());
#endif
        start->SetValue(ComponentItem (componentIndex, componentMem));
    }
#else
    ComponentMap::iterator start;
    bool fFound = FindComponent(start, 0, componentId, componentIndex);
    if (!fFound) {
        componentMap_.insert(std::make_pair(componentId, ComponentItem (componentIndex, componentMem)));
    }
    else {
#ifdef YADSL_USE_WXDEBUG
        wxASSERT (start != componentMap_.end());
#endif
        start->second.mem_ = componentMem;
    }
#endif
    return fFound;
}

bool Entity::SetComponentOwnerPos(uint componentId, uint componentIndex, void* ownerPos) {
#ifdef YADSL_USE_IDVALUEVECTOR_IN_ENTITY
    ComponentMap::ElementVec::iterator start;
#else
    ComponentMap::iterator start;
#endif

    bool fFound = FindComponent(start, 0, componentId, componentIndex);
    if (fFound) {
#ifdef YADSL_USE_IDVALUEVECTOR_IN_ENTITY
#ifdef YADSL_USE_WXDEBUG
        wxASSERT (start != componentMap_.EndIterator());
#endif
        start->GetValue().ownerPos_ = ownerPos;
#else
#ifdef YADSL_USE_WXDEBUG
        wxASSERT (start != componentMap_.end());
#endif
        start->second.ownerPos_ = ownerPos;
#endif
    }
    return fFound;
}

void Entity::EraseComponent(uint componentId, uint componentIndex) {
#ifdef YADSL_USE_IDVALUEVECTOR_IN_ENTITY
    ComponentMap::ElementVec::iterator start;
#else
    ComponentMap::iterator start;
#endif

    bool fFound = FindComponent(start, 0, componentId, componentIndex);
    if (fFound) {
#ifdef YADSL_USE_IDVALUEVECTOR_IN_ENTITY
#ifdef YADSL_USE_WXDEBUG
        wxASSERT (start != componentMap_.EndIterator());
#endif
        componentMap_.Erase(start);
#else
#ifdef YADSL_USE_WXDEBUG
        wxASSERT (start != componentMap_.end());
#endif
        componentMap_.erase(start);
#endif
    }
}

bool Entity::Empty() const {
#ifdef YADSL_USE_IDVALUEVECTOR_IN_ENTITY
    return componentMap_.Size() == 0;
#else
    return componentMap_.empty();
#endif
}

} // end of yadsl
