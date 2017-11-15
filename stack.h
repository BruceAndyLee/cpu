#ifndef CLASS_STACK_INCLUDED
#define CLASS_STACK_INCLUDED

#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <cassert>
#include <cmath>

#define Govnocode
#define MAX_STR_LEN 100

//Poisons
#define PSN_DBL 3.14159265
enum {PSN_INT = 0xDEADFACE,};

//Error codes
enum STKERRS
{
    STKERR_OK,
    STKERR_OVERFLOW,
    STKERR_EMPTY,
    STKERR_NULLDATAPTR,
};

typedef double StkData;

class Stack
{
    public:
        //Initializes fields of the 'Stack' class
        Stack();
        explicit Stack(size_t capacity);
        ~Stack();

		int		 Create (size_t capacity);
        int      Push   (double value   );
        StkData  Pop    (int* errcode   );
        StkData  Peek   (int* errcode   );
        size_t   getSize();
        size_t   getCapacity();
        int      Empty  ();
        int      Print  ();
        int      Add    (int* errcode);
        int      Mul    (int* errcode);
        int      Dump_  (const char* stackName);
        StkData* getDataPtr();
        void     ReadExpression();
        int      Sub    (int* errcode);
        int      Div    (int* errcode);

    private:
		int			 isdequal(double a, double b);
        int          IsOk();
        double       getCheckSum();
        int          Extend();
        unsigned int canary1_;
        double*      data_canary_left_;
        double*      data_canary_right_;
        StkData*     data_;
        size_t       counter_;
        size_t       capacity_;
        size_t       extender_;
        double       check_sum_;
        unsigned int canary2_;
};


#endif
