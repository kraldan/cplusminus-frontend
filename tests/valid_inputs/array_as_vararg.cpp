// test that array is decayed to pointer when used as vararg argument, 
// the test passes if it is compiled
void f(char c, ...) {

}

int main() {
	int arr[5];
	f('c', arr);
}
