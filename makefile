BIN=./src/scm

.PHONY: clean test t repl

all: repl

# clean up
clean:
	$(MAKE) clean -C src

repl: $(BIN)
	@echo "Welcome to scm!"
	@$(BIN)

src/scm:
	$(MAKE) -C src

t: test

# build and run tests
test: $(BIN) 
	@find ./test -type f \
		| grep -v out$  \
		| xargs -n1 -I{} echo "echo {}; cat {} | $(BIN) | diff -u {}.out - || exit 1" 2> /dev/null \
		| bash && echo "Done!"

