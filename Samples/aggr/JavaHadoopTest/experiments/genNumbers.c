int main() {
	int size = 100;
	int random = 1;
	int i = 0;
	srand((unsigned)(time(0))); 
	for (i =0; i < 100; i++) 
		if (random) 
			printf("%d\n", rand()%1000);
		else
			printf("%d\n", i*10);

}
