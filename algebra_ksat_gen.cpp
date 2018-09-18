#include <iostream>
#include <stdio.h>
#include <string>
#include <vector>

using namespace std;

static int id_counter = 1;

struct clause
{
    vector<int> dis;

    clause() {}

    clause(int* arr, int len)
        : dis(arr, arr + len)
    {
    }

    clause(vector<int> a)
        : dis(a)
    {}
};

vector<clause> clauses;

//-----------------------------------------------------------------------------
// bits
//-----------------------------------------------------------------------------
int allocate_bit()
{
    int ret = id_counter;
    ++id_counter;
    return ret;
}
int make_literal_zero()
{
    int ret = allocate_bit();
    clauses.push_back(vector<int> { -ret });
    return ret;
}
int make_literal_one()
{
    int ret = allocate_bit();
    clauses.push_back(vector<int> { ret });
    return ret;
}
// output = (a1 == a2)
int make_equals(int a1, int a2)
{
    int ret = allocate_bit();

    clauses.push_back(vector<int> { -ret, a1, -a2 });
    clauses.push_back(vector<int> { -ret, -a1, a2 });
    clauses.push_back(vector<int> { ret, -a1, -a2 });
    clauses.push_back(vector<int> { ret, a1, a2 });

    return ret;
}
void force_equals(int a1, int a2)
{
    clauses.push_back(vector<int> { a1, -a2 });
    clauses.push_back(vector<int> { -a1, a2 });
}
int make_xor(int a1, int a2)
{
    int ret = allocate_bit();

    clauses.push_back(vector<int> { ret, a1, -a2});
    clauses.push_back(vector<int> { ret, -a1, a2 });
    clauses.push_back(vector<int> { -ret, a1, a2 });
    clauses.push_back(vector<int> { -ret, -a1, -a2 });

    return ret;
}
int make_and(int a1, int a2)
{
    int ret = allocate_bit();

    clauses.push_back(vector<int> { ret, -a1, -a2 });
    clauses.push_back(vector<int> { -ret, a1 });
    clauses.push_back(vector<int> { -ret, a2 });

    return ret;
}
int make_or(int a1, int a2)
{
    int ret = allocate_bit();

    clauses.push_back(vector<int> { -ret, a1, a2 });
    clauses.push_back(vector<int> { ret, -a1 });
    clauses.push_back(vector<int> { ret, -a2 });

    return ret;
}
int make_not(int a)
{
    int ret = allocate_bit();

    clauses.push_back(vector<int> { ret, a });
    clauses.push_back(vector<int> { -ret, -a });
    
    return ret;
}
// output = a ? b : c
int make_if(int a, int b, int c)
{
    int ret = allocate_bit();

    clauses.push_back(vector<int> { -ret, a, c });
    clauses.push_back(vector<int> { -ret, -a, b });
    clauses.push_back(vector<int> { -ret, b, c });
    clauses.push_back(vector<int> { ret, -a, -b });
    clauses.push_back(vector<int> { ret, a, -c });

    return ret;
}

//-----------------------------------------------------------------------------
// bytes
//-----------------------------------------------------------------------------
struct byte
{
    int b[8];
};
byte allocate_byte()
{
    byte ret;
    for (int i = 0; i < 8; ++i)
    {
        ret.b[i] = allocate_bit();
    }
    return ret;
}
byte make_literal_byte(int a)
{
    byte ret;

    for (int i = 0; i < 8; ++i)
    {
        bool b = (a >> i) & 1;
        if (b)
            ret.b[i] = make_literal_one();
        else
            ret.b[i] = make_literal_zero();
    }

    return ret;
}
int make_equals(byte a1, byte a2)
{
    byte mask = allocate_byte();
    for (int i = 0; i < 8; ++i)
    {
        mask.b[i] = make_equals(a1.b[i], a2.b[i]);
    }

    int ret = make_literal_one();
    for (int i = 0; i < 8; ++i)
    {
        ret = make_and(ret, mask.b[i]);
    }

    return ret;
}
void force_equals(byte a1, byte a2)
{
    for (int i = 0; i < 8; ++i)
    {
        force_equals(a1.b[i], a2.b[i]);
    }
}
// output = a ? b : c
byte make_if(int a, byte b, byte c)
{
    byte ret;

    for (int i = 0; i < 8; ++i)
    {
        ret.b[i] = make_if(a, b.b[i], c.b[i]);
    }

    return ret;
}
byte make_lshift(byte a1)
{
    byte ret = allocate_byte();

    clauses.push_back(vector<int> { -ret.b[0] });
    for (int i = 1; i < 8; ++i)
    {
        force_equals(ret.b[i], a1.b[i - 1]);
    }

    return ret;
}
byte make_addition(byte a1, byte a2)
{
    byte ret;
    
    int carry = make_literal_zero();
    for (int i = 0; i < 8; ++i)
    {
        // full adder implementation
        int temp = make_xor(a1.b[i], a2.b[i]);
        ret.b[i] = make_xor(carry, temp);
        carry = make_or(make_and(carry, temp), make_and(a1.b[i], a2.b[i]));
    }

    return ret;
}
byte make_2s_compliment(byte a)
{
    byte ret;
    for (int i = 0; i < 8; ++i)
    {
        ret.b[i] = make_not(a.b[i]);
    }

    byte one = make_literal_byte(1);
    ret = make_addition(ret, one);

    return ret;
}
byte make_multiplication(byte a1, byte a2)
{
    byte zero = make_literal_byte(0);
    byte temp = allocate_byte();
    force_equals(a1, temp);

    byte ret = make_literal_byte(0);

    for (int i = 0; i < 8; ++i)
    {
        ret = make_addition(ret, make_if(a2.b[i], temp, zero));
        temp = make_lshift(temp);
    }

    return ret;
}

bool is_sat(vector<int> vars)
{
    bool ret = true;
    for (int i = 0; i < clauses.size(); ++i)
    {
        bool k = false;
        for (int& c : clauses[i].dis)
        {
            if (c > 0)
                k = k || (vars[c - 1] > 0);
            else
                k = k || (vars[-c - 1] < 0);
        }
        ret = ret && k;
        printf("clause %d: %d\n", i, ret);
    }

    return ret;
}

int main()
{
    // example
    // after plugging into SAGE, this gives that ans = 5

    // usage: g++ -std=c++11 algebra_ksat_gen.cpp
    //        a.exe > a.txt
    // in sage:
    //   solver = SAT()
    //   solver.read("/path/to/a.txt")
    //   solver()
    
    /*byte ans = allocate_byte();
    byte b1 = make_literal_byte(3);
    byte b2 = make_literal_byte(6);
    byte b3 = make_multiplication(ans, b1);
    int a = make_equals(b2, b3);
    clauses.push_back(vector<int> { a });*/

    byte ans = allocate_byte();
    byte x1 = make_literal_byte(3);
    byte y1 = make_literal_byte(1);
    byte x2 = make_literal_byte(1);
    byte y2 = make_literal_byte(4);
    byte determinant = make_addition(make_multiplication(x1, y2), make_2s_compliment(make_multiplication(x2, y1)));
    force_equals(ans, determinant);

    /*byte ans = allocate_byte();
    byte x = make_literal_byte(5);
    byte a = make_2s_compliment(x);
    force_equals(ans, a);*/
    
    printf("p cnf %d %d\n", id_counter - 1, clauses.size());
    for (int i = 0; i < clauses.size(); ++i)
    {
        for (int& c : clauses[i].dis)
        {
            //int c = clauses[i].dis[j];
            printf("%d ", c);
        }
        printf("0\n");
    }
}