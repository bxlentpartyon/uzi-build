%.o: %.c
	$(CC) $(CFLAGS) -c -g -o $@ $<

fsutil_all.o: fsutil.o ls.o
	gcc -g -o $@ $^
