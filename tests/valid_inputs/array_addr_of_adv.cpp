// test that step over pointer to array works correctly
int main() {
	int arr[2][5];
	int (*lhs)[5] = (&arr[0]) + 1;
	int (*rhs)[5] = &arr[1];
	return lhs == rhs;
//	return (&arr[0]) + 1 == &arr[1];
}
