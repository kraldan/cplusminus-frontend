
int res;

bool side_effect(int i, bool b = true) {
	res += i;
	return b;
}

int main() {
	res = 0;
	// check that rhss are not executed
	side_effect(0, false) && side_effect(5);
	side_effect(0, true) || side_effect(10);
	return res;
}
