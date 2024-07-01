// test that all the shortcircuit evals together produce functioning IR and work correctly

int main() {
	int i = 42, *p = nullptr;
	bool cond = (p && *p || i) ? (p || i || *p) : (i && p && true);
	return cond ? 42 : 7;
}
