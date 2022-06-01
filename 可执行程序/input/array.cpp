/*# include <iostream>
using namespace std;*/
int program(int a, int b, int c)
{
    int i;
    int j;
    i = 0;
    if (a > (b + c))
    {
        j = a + (b * c + 1);
    }
    else
    {
        j = a;
    }
    while (i <= 100)
    {
        j = j * 2;
        i = j;
    }
    return i;
}

int demo(int a)
{
    a = a + 2;
    return a * 2;
}

int main()
{
    int a[2][2];
    a[0][0] = 3;
    a[0][1] = a[0][0] + 1;
    a[1][0] = a[0][1] + 1;
    a[1][1] = program(a[0][0], a[0][1], demo(a[1][0]));
    // cout<<a[1][1]<<endl;
    return a[1][1];
}