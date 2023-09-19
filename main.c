#include <stdio.h>
#include <stdlib.h>

int x = 60;
int i;



int countdown(int countTime) {
	for (i = 0; ++i; --countTime){
		printf("%i\n", countTime);
		sleep(1);
		
	}
	return 0;
}

int main() {
	printf("Welcome to Pomodoro!!\n");
	countdown(x);
	return 0;
}
