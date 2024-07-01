// test that 'int' cannot be initialized like a class can
// because our compiler would practically treat this like
// 'int i = int(5)', and function 'int' doesn't exist

int main() {
	int i(5);
}
