// test that printf, scanf, malloc, free, sprintf and fprintf are implicitly declared in every programme
// (and they should be linked form libc during ir->assembly translation)

int main() {
	// test printf, scanf, sprintf, sscanf
	char n1, n2;
	char arr[2];
	printf("ahoj\n");
	if(scanf("%c", &n1) != 1)
		return 1;
	// save n1 in arr
	sprintf(arr, "%c", n1);
	// load contents of arr to n2
	sscanf(arr, "%c", &n2);
	if(n1 != n2)
		return 2;
	// test malloc and free
	int *p = (int*) malloc(1);
	free(p);
}
