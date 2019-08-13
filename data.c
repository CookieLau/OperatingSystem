#include<iostream> 
#include<cstdio> 
#include<cstring>
#define test(x) printf("%d\n",&(x)) 
#include <cstdlib> 
#include<time.h>
using namespace std; 
int num = 0;
int index = 0; 
int key = 8000; int *myArr;
int random[10000]; const int maxn = 105; 
struct Matrix{
int n,m;
int v[maxn][maxn];
Matrix(int n,int m):n(n),m(m){}

