// test member access through -> and .

struct S {
	int a;
};

int main() {
	S s;
	s.a = 21;
	(&s)->a += 21;
	return s.a;
}
