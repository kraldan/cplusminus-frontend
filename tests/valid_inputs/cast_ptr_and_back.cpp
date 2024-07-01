
int main() {
	int arr[1]; arr[0] = 42;
	void *vp = (void *) arr;
	int *p = (int*)vp;
	return p[0];
}
