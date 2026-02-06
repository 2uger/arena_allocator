run_tests:
	gcc -DLINUX_MMAP arena_test.c -o arena_test && ./arena_test
	gcc arena_test.c -o arena_test && ./arena_test
	rm arena_test
