all: jitnaive
.PHONY: tests clean

tests: test.c naivesearch.o
	$(CC) $^ -o $@
	./tests

jitnaive: dynasm-driver.c jitnaive.h
	$(CC) -O2 -o $@ $< -DJIT=\"jitnaive.h\"

jitnaive.h: jitnaive.dasc
	luajit dynasm/dynasm.lua $^ > $@ || rm $@

clean:
	rm -f jitnaive tests jitnaive.h *.o
