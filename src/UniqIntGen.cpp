#include <algorithm>
#include "UniqIntGen.h"
#ifdef YADSL_USE_WXDEBUG
#include <wx/wx.h>
#endif

namespace yadsl
{

int UniqIntGenerator::Get() {
    if (generated_.empty()) {
        return num_++;
    }
    int n = generated_.back();
    generated_.pop_back();
    return n;
}

bool UniqIntGenerator::Put(int n) {
    if (n >= num_) {
#ifdef YADSL_USE_WXDEBUG
        wxFAIL_MSG(wxT("out of range"));
#endif
        return false;
    }

    std::vector<int>::iterator pos = std::lower_bound(generated_.begin(), generated_.end(), n);
    if (pos != generated_.end()) {
        if (n < *pos) {
            generated_.insert(pos, n);
            return true;
        }
        else if (n == *pos) {
#ifdef YADSL_USE_WXDEBUG
            wxFAIL_MSG(wxT("duplicate inserting detected"));
#endif
        }
        else return false;
    }
    else {
        generated_.push_back(n);
        return true;
    }
    return false;
}

} // end of yadsl


//-----------------------------------------------------------------------------

#if 0 // test code

#include <iostream>
#include <stdio.h> // printf()
#ifdef YADSL_USE_WXDEBUG
#include <wx/wx.h>
#endif

#include "UniqIntGen.h"

#include <stdlib.h> // rand()
#include  <algorithm>

int Random(int high) {
    double k = (double)rand() / RAND_MAX;
    return high * k;
}

void PrintUIG(const yadsl::UniqIntGenerator& uig) {
    printf("UIG: ");
    for (int i = 0; i < uig.Size(); i++) {
        printf("%d, ", uig.Data()[i]);
    }
    printf("\n");
}

void PrintBufToPut(const int* a, const std::vector<int>& indexSeq) {
    printf("buf: ");
    for (size_t i = 0; i < indexSeq.size(); i++)  {
        printf("%d, ", a[indexSeq[i]]);
    }
    printf("\n");
}

void PrintBuf(const int* a, int bufSize) {
    printf("buf: ");
    for (size_t i = 0; i < bufSize; i++)  {
        printf("%d, ", a[i]);
    }
    printf("\n");
}

void Test() {
    yadsl::UniqIntGenerator uig;

    const int kBUF_SIZE = 50;
    int a[kBUF_SIZE];

    for (int i = 0 ; i < kBUF_SIZE; i++) {
         a[i] = uig.Get();
    }
    std::vector<int> indexSeq;
    for (int j = 0 ; j < 10; j++) {
        int cToPut = Random(kBUF_SIZE / 3);
        indexSeq.clear();
        while(indexSeq.size() < cToPut) {
            int index = Random(kBUF_SIZE - 1);
            if (std::find(indexSeq.begin(), indexSeq.end(), index) == indexSeq.end()) {
                indexSeq.push_back(index);
            }
        }
        printf("%d pass\n", j);
        PrintBufToPut(a, indexSeq);
        for (size_t i = 0; i < indexSeq.size(); i++) {
            uig.Put(a[indexSeq[i]]);
        }
        PrintUIG(uig);
        printf("\n");
        for (size_t i = 0; i < indexSeq.size(); i++) {
            a[indexSeq[i]] = uig.Get();
        }
    }

    for (int i = 0 ; i < kBUF_SIZE; i++) {
        uig.Put(a[i]);
    }
    PrintBuf(a, kBUF_SIZE);
    PrintUIG(uig);

}

int main(){
    Test();

    std::cout << "Press any key to quit..." << std::endl;
    std::cin.get();
    std::cin.get();
}

#endif

