float avg(int count, int *value) {
  int total = 0;
  int sum;
  sum = 0;
  for (int i = 0; i < count; i++) {
    total += value[i];
  }
  return (float) (total / count);
}



int main() {
  int count, i, sum;
  int mark[4];
  float average;
  count = 4;
  sum = 0;
  for (int  i = 0; i < count; i++) {
    mark[i] = (i * 30);
    sum = sum + mark[i];
    average = avg(i + 1, mark);
    if (average > 40) {
      printf("%f\n", average);
    }
  }
  return 0;
}