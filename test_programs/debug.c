#include <stdio.h>

int main(){
    int x = 12  ;
    while (x > 1){
        if (x & 1){
            x = x * 3 + 1;
        }else {
            x = x / 2;
        }
        printf("%d\n", x);
    }
    return 12.0;
}
