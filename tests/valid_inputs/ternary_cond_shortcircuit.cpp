// test that ternary conditional does shortcircuit evaluation on b,c operands
// assumes that nullptr deref would lead to segfault

int main() {
	int *p = nullptr, i = 21;
	return (i ? i : *p) + (p ? *p : i);
}
