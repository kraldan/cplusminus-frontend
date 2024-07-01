// test that the public switch works with class

class C {
public:
	int a;
};

int main() {
	C c;
	return c.a = 42;
}
