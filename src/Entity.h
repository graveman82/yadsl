#ifndef YADSL_ENTITY_H_
#define YADSL_ENTITY_H_

/** @file Entity.h.

Назначение: доступ к данным и управление сущностью.
*/

#ifdef YADSL_USE_IDVALUEVECTOR_IN_ENTITY
#include "IdValueVector.h"
#else
#include <map>
#include "BaseTypes.h"
#endif



namespace yadsl
{

/** @brief Сущность.

Сущность - это набор компонент. Каждая компонента определяет какой-то функционал и
размещение сущности в связанных с функционалом компоненты контейнерах.

EC - Entity Component

###Сборка###
Макрос YADSL_USE_IDVALUEVECTOR_IN_ENTITY отвечает за реализацию на основе IdValueMultiVector. Без установки
этого макроса класс будет собран на основе std::map.

*/
class Entity {
public:
    static const uint kComponentIdNotAssigned_; // недопустимое значение идентификатора компоненты сущности - число, обозначающее, что идентификатор не назначен

    typedef void* PVoid;


    struct ComponentItem {
        uint index_;        // индекс компоненты
        PVoid mem_;         // указатель на память, где хранится экземпляр компоненты
        PVoid ownerPos_;    // позиция в контейнере владеющей компонентой сущности

        ComponentItem() :
            index_(kNoIndex), mem_(0), ownerPos_(0) {}
        ComponentItem(uint index, PVoid mem = 0, PVoid ownerPos = 0) : index_(index), mem_(mem), ownerPos_(ownerPos) {}
    };

#ifdef YADSL_USE_IDVALUEVECTOR_IN_ENTITY
    typedef IdValueMultiVector<ComponentItem> ComponentMap;
#else
    typedef std::multimap<uint, ComponentItem> ComponentMap;
#endif


private:
    int id_; // идентификатор сущности
    ComponentMap componentMap_;     // словарь доступа к составляющим частям (компонентам)

    /** @brief Поиск компоненты среди данных сущности.
    @param [out] it - переменная, куда сохраняется итератор элемента вектора найденной компоненты.
    @param [out] item - переменная, куда сохраняется адрес пункта компоненты.
    @param componentId - идентификатор компоненты.
    @param componentIndex - индекс компоненты среди компонент такого же типа. Передать kNoIndex, если индекс не важен, а важен только факт наличия компоненты вообще.
    (@see kNoIndex)
    @return Возвращает true, если компонента найдена и false в обратном случае.
    */
#ifdef YADSL_USE_IDVALUEVECTOR_IN_ENTITY
    bool FindComponent(ComponentMap::ElementVec::iterator& it,  const ComponentItem** ppItem, uint componentId, uint componentIndex = kNoIndex);
#else
    bool FindComponent(ComponentMap::iterator& it,              const ComponentItem** ppItem, uint componentId, uint componentIndex = kNoIndex);
#endif
public:

    /** @brief Сгенерировать уникальный идентификатор для компоненты сущности.

    @note используется менеджером компоненты!
    @return целое больше нуля. Нуль обозначает, что идентификатор не назначен.
    */
    static uint GenerateComponentId();

    Entity();
    ~Entity();

    /// Получить идентификатор экземпляра этой сущности
    int GetId() const { return id_; }

    /** @brief Проверка наличия компоненты среди данных сущности.
    @param componentId - идентификатор компоненты.
    @param componentIndex - индекс компоненты среди компонент такого же типа. Передать kNoIndex, если индекс не важен, а важен только факт наличия компоненты вообще.
    (@see kNoIndex)
    @param [out] ppMem - адрес переменной, куда сохраняется указатель на память экземпляра типа компоненты с данным индексом. Может быть равен нулю.
    @param [out] ppOwnerPos - адрес переменной, куда сохраняется указатель на позицию сущности в контейнере внутри менеджера компонент. Может быть равен нулю.
    @return Возвращает true, если компонента найдена и false в обратном случае.
    */
    bool GetComponent(uint componentId, uint componentIndex = kNoIndex, PVoid* ppMem = 0, PVoid* ppOwnerPos = 0);

    /** @brief Установить память компоненты или вставить компоненту, если ее еще нет среди данных сущности.
    @param componentId - идентификатор компоненты.
    @param componentIndex - индекс компоненты среди компонент такого же типа.
    @param componentMem - указатель на память экземпляра типа компоненты, может быть равна 0.
    @return false, если компонента была вставлена в сущность, true, если было
    изменено значение указателя на память экземпляра ее типа.
    */
    bool SetOrInsertComponent(uint componentId, uint componentIndex, void* componentMem = 0);

    /** @brief Записать в структуре данных хранения компоненты позицию сущности в контейнере менеджера компоненты.
    @param componentId - идентификатор компоненты.
    @param componentIndex - индекс компоненты среди компонент такого же типа.
    @param ownerPos - устанавливаемая позиция.
    @return false при ошибке.
    */
    bool SetComponentOwnerPos(uint componentId, uint componentIndex, void* ownerPos);

    /** @brief Удалить компоненту из сущности.
    @param componentId - идентификатор компоненты.
    @param componentIndex - индекс компоненты среди компонент такого же типа.
    */
    void EraseComponent(uint componentId, uint componentIndex);

    bool Empty() const;

private:
    Entity(const Entity&);
    Entity& operator=(const Entity&);
};

} // end of yadsl

#endif // YADSL_ENTITY_H_

