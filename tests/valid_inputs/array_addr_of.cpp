// test that operator '&' really treats array like an array, and there's no array_to_ptr decay
int main() {
	int arr[5];
	arr[2] = 42;
	int (*ptr_to_arr)[5] = &arr; // this assignment would fail if array to pointer decay took place
	return (*ptr_to_arr)[2];
}
