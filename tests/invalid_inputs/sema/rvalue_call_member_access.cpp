struct S {
    int a;
    S() {
        this->a = 42;
    }
};

void f() {
    S().a;  // c+- error: cannot access member of rvalue; 
            // is ok in c++
}