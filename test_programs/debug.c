#include <stdio.h>
int x = 12;
int y = 17;
int main(){
    int x = 123;
    while (x > 1){
        if (x & 1){
            debug();
            x = x * 3 + 1;
        }else {
            x = x / 2;
        }
            printf("%d\n", x);
    }
    return 12;
}
