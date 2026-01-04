JAVAC = javac
JAVA = java
SRC_DIR = src
BIN_DIR = bin
MAIN_CLASS = TlipGui

all: compile

compile:
	mkdir -p $(BIN_DIR)
	$(JAVAC) -d $(BIN_DIR) $(SRC_DIR)/$(MAIN_CLASS).java

run: compile
	$(JAVA) -cp $(BIN_DIR) $(MAIN_CLASS)

clean:
	rm -rf $(BIN_DIR)

.PHONY: all compile run clean
