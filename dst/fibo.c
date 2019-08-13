#include<stdio.h>
     
int fibo(int var){
    int a0 = 1;
    int a1 = 1;
    int a2;
    switch (var){
      case 1:{
	    printf("1\n");
	    break;
	}
      case 2:{
	   printf("1 1\n");
	   break;
	}
      default:{
	   while (var--){
		printf("%d%c",a0,(var == 0)? '\n':' ');
		a2 = a0 + a1;
		a0 = a1;
		a1 = a2;	
	   }
	}
    }
    
}

int main(){
    int var;
    scanf("%d", &var);
    fibo(var);
    return 0;
}
