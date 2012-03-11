int (killed)(int);
const int a = 10 + 20 + 30, b, c = 5;
double d, e, f = 10;
namespace justdefineit {
  long unsigned g;
  long unsigned int h;
  long unsigned long k;
}
long double i;
class koalacub {
  int leavesEaten;
  int chutesEaten;
};
class koala: koalacub {
  koalacub *children[10]; // A koala can have at most ten children because of reasons.
};

int *level1_simple[10];
unsigned long const int   (  *  level2_better )  [ 10+5+7+9 / 3 ] ;
int* (*(*level3_confusing)[10][12])[15];
long double (*(&*level4_whatthefuck[10])[2][4])(char, short, int, long, double, long double);
long double (*(&*(***level5_okaycutitout[1^2^4^8])(int illegalnest, char (*but)(int oh, int well))[10])[2][4])(char, short, int, long, double, long double);


int my_implemented_function(int) {
  implementation
}

int my_implemented_function_2(int, double, char) {
  implementation
}
int (my_implemented_function_3)(const long double) {
  implementation
}

/*
int function(int,int) {
  implementation 2
}

template<typename a> int function2(a b) { }

template<typename b> class templateclass { b aidu; }

#if false
#error Failure
#endif
*/

int zzzzz_success;
