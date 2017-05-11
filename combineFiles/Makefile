# Makefile for Project 1 of CS485, Spring 2015.
# This project is to manipulate binary data in C.

GOAL = combineFiles
CFLAGS = -Wall
FILES = $(wildcard *.data)
OUTPUT = result.out

default: $(GOAL)

run: $(GOAL) $(FILES)
	./$(GOAL) $(FILES) > $(OUTPUT)

clean:
	rm -f $(GOAL) $(OUTPUT)

submit: $(GOAL).c Makefile README
	tar -czf $(GOAL).tgz $(GOAL).c Makefile README

