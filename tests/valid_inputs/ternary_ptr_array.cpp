// test that array to pointer decay occurs when array is an operand of ternary conditional operator
int main() {
	int *p, arr[5];
	int *p2 = true ? p : arr;
}
