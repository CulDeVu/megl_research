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
int make_equals(int a1, int a2)
{
    int ret = allocate_bit();

    clauses.push_back(vector<int> { ret, a1, -a2 });
    clauses.push_back(vector<int> { ret, -a1, a2 });
    clauses.push_back(vector<int> { ret, -a1, -a2 });
    clauses.push_back(vector<int> { ret, a1, a2 });

    return ret;
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

    clauses.push_back(vector<int> { ret, -a1, -a2});
    clauses.push_back(vector<int> { -ret, a1 });
    clauses.push_back(vector<int> { -ret, a2 });

    return ret;
}

//-----------------
// bytes
//-----------------
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
    int ret = make_literal_one();

    for (int i = 0; i < 8; ++i)
    {
        ret = make_and(ret, make_equals(a1.b[i], a2.b[i]));
    }

    return ret;
}
byte make_addition(byte a1, byte a2)
{
    byte ret;
    
    int carry = make_literal_zero();
    for (int i = 0; i < 8; ++i)
    {
        ret.b[i] = make_xor(carry, make_xor(a1.b[i], a2.b[i]));
        carry = make_and(carry, make_and(a1.b[i], a2.b[i]));
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
    /*id_counter = 3;
    int out = make_xor(1, 2);
    clauses.push_back(vector<int> { out });

    for (int i = 0; i < clauses.size(); ++i)
    {
        for (int& c : clauses[i].dis)
        {
            printf("%d ", c);
        }
        printf("\n");
    }

    int e[] = { -1, 1, 1 };
    printf("%d\n", is_sat(vector<int>(e, e + 3)));*/

    byte ans = allocate_byte();
    byte b1 = make_literal_byte(15);
    byte b2 = make_literal_byte(20);
    byte b3 = make_addition(ans, b1);
    int a = make_equals(b2, b3);
    clauses.push_back(vector<int> { a });

    /*byte b1 = make_literal_byte(15);
    //byte b2 = make_literal_byte(15);
    byte b2 = allocate_byte();
    int a = make_equals(b1, b2);*/
    
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