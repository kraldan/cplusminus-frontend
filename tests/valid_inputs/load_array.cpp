
int scanf(const char * fmt, ...);
int printf(const char * fmt, ...);

int main() {
    /* at the time of writing of this test,
     * arrays could only be declared with integer literals */
// 	const int arr_size = 5;
	int arr[5];
	for(int i = 0; i < 5; i = i + 1)
		scanf("%d", &arr[i]);
	for(int i = 4; i >= 0; i = i - 1)
		printf("%d ", arr[i]);
//  printf("\n");
}
