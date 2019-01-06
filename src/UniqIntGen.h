#ifndef YADSL_UNIQINTGEN_H
#define YADSL_UNIQINTGEN_H

#include <vector>
namespace yadsl
{

class UniqIntGenerator {
private:
    std::vector<int> generated_;
    int num_;

public:
    enum {kNoId = -1};
    UniqIntGenerator() : num_(0) {}

    int Get();
    bool Put(int n);

    int Size() const { return generated_.size(); }
    int Num() const { return num_; }
    const int* Data() const { return &generated_[0]; }

};

} // end of yadsl

#endif // YADSL_UNIQINTGEN_H

