void main(void)
{
  int i;

  for (i = 0; i < 256; i++) {
      printf("%d\t%d\t%d\t%d\t%d\t%d\n", i, 0, i >> 7, (i & 0x7f) + 1, 0, 0);
  }
}
