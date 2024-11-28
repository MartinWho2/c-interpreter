float avg(int count, int *value) {
  int total;
  int sum;
  sum = 0;
  for (int i = 0; i < count; i++) {
    total += value[i];
  }
  return total / count;
}

int main() {
  int studentNumber = 2, count = 3, i, sum;
  int mark[4];
  float average;
  char a = 'c';
  char* ab = "hello";
  studentNumber = count == 3;
  count = 4;
  sum = 0;
  for (int  i = 0; i < count; i++) {
    mark[i] = (i * 30);
    sum = sum + mark[i];
    average = avg(i + 1, mark);
    if (average > 40) {
      printf("hello", average);
    }
  }
  i = 12;
}