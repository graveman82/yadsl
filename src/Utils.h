#ifndef YADSL_UTILS_H
#define YADSL_UTILS_H

#include <vector>
#include <algorithm>

#define YADSL_RELEASE_COM(x) if ((x) != 0) { (x)->Release(); (x) = 0; }

namespace yadsl {

template <typename T>
inline void Swap(T& a, T& b) {
	T tmp = a;
	a = b;
	b = tmp;
}

//A L G O R I T H M S /////////////////////////////////////////

/** @brief Поиск ближайшего элемента в векторе stl.
	@param v вектор, в котором ведется поиск ближайшего элемента.
	@param op функтор сравнения.
	@return итератор на ближайший элемент.
Требования к оператору сравнения:
1) Его объект должен инициализироваться элементом (или указателем на него), поиск ближайшего к которому ведется в контейнере.
Прототип конструктора: ElementLess(const Element& element) : element_(element) {}
2) Должен быть реализован оператор функции, который принимает ссылку на два сравниваемых элемента и возвращает true,
если первый сравниваемый элемент меньше второго.
bool operator ()(const Element& first, const Element& second);

Требования к элементам контейнера:
Класс элемента контйнера должен реализовывать операцию
bool operator==(const Element& other) const;

	@note при работе алгоритма выделяется память под копию входного вектора. Алгоритм не рекомендуется использовать постоянно
	из-за возможной фрагментации кучи.
	Если вектор большой, вызов алгоритма может сказаться на производительности программы при частом вызове.
	Идеальное место использования алгоритма - код инициализации (инициализация подсистем, загрузка уровней).
*/
template <typename T, typename LessOp>
typename std::vector<T>::const_iterator find_closer(const std::vector<T>& v, LessOp op) {
    std::vector<T> sorted_v(v);
    std::sort(sorted_v.begin(), sorted_v.end(), op);
    return std::find(v.begin(), v.end(), sorted_v[0]);

}

} // end of yadsl

#endif // YADSL_UTILS_H
