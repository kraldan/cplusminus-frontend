
class C {
	public:
	private:
	// test that access switching works correctly
	void private_method() {

	}
};

int main() {
	C c;
	c.private_method();
}
