
int f(const int * p) {
	return p[0];
}

int main() {
	int arr[1];
	arr[0] = 42;
	return f(arr);
}
