// test that operands of ternary conditional can be of void type

void f() {

}
void g() {

}
int main() {
	true ? f() : g();
}
