// in c+-, integral promotion and fptosi are the same priority

void f(char c);
void f(double d);

void g() {
    int i = 42;
    f(i);   // error: ambiguous call
}
