
int foo(int a);

int foo(int a = foo()) {
	return a;
}

int main() {

}
