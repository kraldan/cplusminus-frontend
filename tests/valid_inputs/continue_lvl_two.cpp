int main() {
    int i = 0, res = 0;

    while(i < 5) {
        res += 10;
        i += 1;

        while(true) {
            continue 2;
        }
    }
    return res;
}
