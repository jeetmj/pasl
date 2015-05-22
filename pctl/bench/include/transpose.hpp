
#include "parray.hpp"

#ifndef _PCTL_PBBS_TRANSPOSE_H_
#define _PCTL_PBBS_TRANSPOSE_H_

namespace pasl {
namespace pctl {
    
template <class E, class intT>
class transpose_contr {
public:
  static controller_type contr;
};

template <class E, class intT>
controller_type transpose_contr<E,intT>::contr("transpose");
  
template <class E, class intT>
struct transpose {
  E *A, *B;
  transpose(E *AA, E *BB) : A(AA), B(BB) {}
  
  void transR(intT rStart, intT rCount, intT rLength,
              intT cStart, intT cCount, intT cLength) {
    using controller_type = transpose_contr<E, intT>;
    auto seq = [&] {
      for (intT i=rStart; i < rStart + rCount; i++)
        for (intT j=cStart; j < cStart + cCount; j++)
          B[j*cLength + i] = A[i*rLength + j];
    };
    par::cstmt(controller_type::contr, [&] { return rCount + cCount; }, [&] {
      if (cCount < 2 && rCount < 2) {
        seq();
      } else if (cCount > rCount) {
        intT l1 = cCount/2;
        intT l2 = cCount - cCount/2;
        par::fork2([&] { transR(rStart,rCount,rLength,cStart,l1,cLength); },
                   [&] { transR(rStart,rCount,rLength,cStart + l1,l2,cLength); });
      } else {
        intT l1 = rCount/2;
        intT l2 = rCount - rCount/2;
        par::fork2([&] { transR(rStart,l1,rLength,cStart,cCount,cLength); },
                   [&] { transR(rStart + l1,l2,rLength,cStart,cCount,cLength); });
      }

    }, [&] {
      seq();
    });
  }
  
  void trans(intT rCount, intT cCount) {
    transR(0,rCount,cCount,0,cCount,rCount);
  }
};

template <class E, class intT>
class block_transpose_contr {
public:
  static controller_type contr;
};

template <class E, class intT>
controller_type block_transpose_contr<E,intT>::contr("block_transpose");
  
template <class E, class intT>
struct blockTrans {
  E *A, *B;
  intT *OA, *OB, *L;
  
  blockTrans(E *AA, E *BB, intT *OOA, intT *OOB, intT *LL)
  : A(AA), B(BB), OA(OOA), OB(OOB), L(LL) {}
  
  void transR(intT rStart, intT rCount, intT rLength,
              intT cStart, intT cCount, intT cLength) {
    using controller_type = block_transpose_contr<E, intT>;
    auto seq = [&] {
      for (intT i=rStart; i < rStart + rCount; i++)
        for (intT j=cStart; j < cStart + cCount; j++) {
          E* pa = A+OA[i*rLength + j];
          E* pb = B+OB[j*cLength + i];
          intT l = L[i*rLength + j];
          //cout << "pa,pb,l: " << pa << "," << pb << "," << l << endl;
          for (intT k=0; k < l; k++) *(pb++) = *(pa++);
        }
    };
    par::cstmt(controller_type::contr, [&] { return rCount + cCount; }, [&] {
      if (cCount < 2 && rCount < 2) {
        seq();
      } else if (cCount > rCount) {
        intT l1 = cCount/2;
        intT l2 = cCount - cCount/2;
        par::fork2([&] { transR(rStart,rCount,rLength,cStart,l1,cLength); },
                   [&] { transR(rStart,rCount,rLength,cStart + l1,l2,cLength); });
      } else {
        intT l1 = rCount/2;
        intT l2 = rCount - rCount/2;
        par::fork2([&] { transR(rStart,l1,rLength,cStart,cCount,cLength); },
                   [&] { transR(rStart + l1,l2,rLength,cStart,cCount,cLength); });
      }
    }, [&] {
      seq();
    });
  }
  
  void trans(intT rCount, intT cCount) {
    transR(0,rCount,cCount,0,cCount,rCount);
  }
  
} ;
  
} // end namespace
} // end namespace

#endif /*! _PCTL_PBBS_TRANSPOSE_H_ */