BIN=./src/scm

.PHONY: clean test repl

all: repl

# clean up
clean:
	$(MAKE) clean -C src

repl: $(BIN)
	@echo "Welcome to scm!"
	@$(BIN)

src/scm:
	$(MAKE) -C src

# build and run tests
test: $(BIN) 
	./test/runner.sh $(BIN) ./test/suite

