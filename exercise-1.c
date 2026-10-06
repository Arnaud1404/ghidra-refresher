#include <stdlib.h>
#include <stdio.h>

int fibonacci(int n) {
  if (n <= 0) return 0;
  if (n == 1) return 1;
  return fibonacci(n - 1) + fibonacci(n - 2);
}

void display_fibonacci(int n) {
  printf("Fibonacci sequence up to %d: ", n);
  for (int i = 1; i <= n; i++)
    printf("%d ", fibonacci(i));
  printf("\n");
}

int main() {
  char buffer[16];
  printf("Enter your name: ");
  fgets(buffer, sizeof(buffer), stdin);
  printf("Hello, %s!\n", buffer);

  int num;
  puts("Enter a number: ");
  scanf("%d", &num);
  if (num < 0) {
    printf("Negative numbers are not allowed.\n");
    return EXIT_FAILURE;
  }
  display_fibonacci(num);

  return EXIT_SUCCESS;
}