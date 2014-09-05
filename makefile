.PHONY: tests

tests: test.c naivesearch.o
	$(CC) $^ -o $@
	./tests
