#ifndef YADSL_UNIQINTGEN_H
#define YADSL_UNIQINTGEN_H

/** @file UniqIntGen.h. */
#define YADSL_TRACE_UNIQINTGENERATOR 1 ///< флаг включения трассировки для класса генерации уникальных чисел
#define YADSL_TEST_UNIQINTGENERATOR 1 ///< флаг включения теста для класса генерации уникальных чисел
#include <vector>
#include "BaseTypes.h"

namespace yadsl
{

/** @brief Генератор уникальных целочисленных идентификаторов.
###Использование###
Определить переменную генератора - все экземпляры независят друг от друга. То есть в списке выдаваемых
чисел двух разных экземпляров генератора могут быть одинаковые числа.
Использовать метод Get() для получения уникальных или неиспользуемых числовых идентификаторов и
метод Put() для возврата неиспользуемых идентификаторов обратно в генератор.
Пример:

@code
UniqIntGenerator uig;
uint n = uig.Get();
//...
uig.Put(n);
@endcode
*/
class UniqIntGenerator {
private:
    std::vector<uint> generated_;
    uint num_;

    // Доступ к контейнеру с идентификаторами
    const uint* Data() const { return &generated_[0]; }

    // генерация нового числа
    void Generate();

public:

    UniqIntGenerator() : num_(0) {}
    ~UniqIntGenerator();

    /// Получить новый уникальный или неиспользуемый идентификатор
    uint Get();
    /// Положить неиспользуемый идентификатор в буфер генератора
    bool Put(uint n);

    /// Число неиспользуемых в данное время ранее сгенерированных идентификаторов
    uint Unused() const { return generated_.size(); }
    /// Число сгенерированных идентификаторов (как используемых в настоящий момент, так и неиспользуемых)
    uint Num() const { return num_; }



#if YADSL_TRACE_UNIQINTGENERATOR
    /// Вывести неиспользуемые числа и максимальное число сгенерированных чисел на консоль
    void TraceToConsole(bool fShowMemAddressOfInstance = true);
#endif

#if YADSL_TEST_UNIQINTGENERATOR
    /// Проверка состояния, в котором должен пребывать генератор после того как в него будут возвращены все сгенерированные числа
    bool IsValidWhenAllPutBack() const;
#endif

};

} // end of yadsl

#endif // YADSL_UNIQINTGEN_H

