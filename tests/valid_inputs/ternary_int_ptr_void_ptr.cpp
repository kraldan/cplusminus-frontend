// check that implicit conversion nullptr_t -> int* works with ternary operator

int main() {
	int *p;
	bool b;
	b ? p : nullptr;
}
