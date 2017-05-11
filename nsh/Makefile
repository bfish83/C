# Makefile for Project 4 of CS485, Spring 2015.
# This project is to replace linux shell

GOAL = nsh
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

