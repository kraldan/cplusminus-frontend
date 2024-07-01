/* This program calculates the first n primes.
 */

bool is_prime(const int n, const int*primes, const int primes_found) {
    if(n <= 1 || primes_found == 0)
        return false;
    int i = 0;
    while(i < primes_found && primes[i] * primes[i] <= n) {
        if(n % primes[i] == 0)
            return false;
        i += 1;
    }
    return true;
}

/**
 * Generate first n primes.
 *
 */
int main() {

    int n;
    printf("Enter how many primes you want to see:\n");
    if(scanf("%d", &n) != 1 || n <= 0) {
        printf("Wrong number\n");
        return 1;
    }
    int *primes = (int*) malloc(n * sizeof(*primes));
    if(primes == nullptr)
        return 2;
    int prime_count = 0;
    primes[prime_count++] = 2;
    int i = 3;
    while(prime_count < n) {
        if(is_prime(i, primes, prime_count))
            primes[prime_count++] = i;
        i += 1;
    }
    printf("First %d primes: ", n);
    for(int j = 0; j < n; j++)
        printf("%d ", primes[j]);
    printf("\n");
    free(primes);
}
