
int main() {
	int arr[5];
	if(sizeof(arr) == 5 * sizeof(*arr))
		return 42;
}
