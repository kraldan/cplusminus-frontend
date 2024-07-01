// test that the short-circuit evaluation can handle chained logical operators

int main() {
	int i = 42, *p = nullptr;
	bool cond = (p && *p) || i;	// if the eval doesn't shortcircuit, this should segfault
	if(cond)
		return 42;
}
