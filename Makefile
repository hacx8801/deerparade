# Variables
COMPILER = g++
OPTIONS = -Wall -g -fsanitize=address,undefined
OPTIONS_RELEASE = -Wall -s -Ofast -march=native
PROGRAM = main.out
CFILES = main.cpp
# HPPFILES = list.h
LD_FLAGS = -lraylib -lGL -lm -lpthread -ldl -lrt -lX11

debug: $(CFILES) $(HFILES)
	$(COMPILER) $(OPTIONS) $(CFILES) -o $(PROGRAM) $(LD_FLAGS) 

release: $(CFILES) $(HFILES)
	$(COMPILER) $(OPTIONS_RELEASE) $(CFILES) -o $(PROGRAM) $(LD_FLAGS)

run: $(PROGRAM)
	./$(PROGRAM)

clean:
	rm -f *.out
