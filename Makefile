# Variables
COMPILER = g++
OPTIONS_RELEASE = -O2 -march=native -Wall -Wformat -Wformat=2 -Wconversion -Wimplicit-fallthrough \
-Werror=format-security \
-U_FORTIFY_SOURCE -D_FORTIFY_SOURCE=3 \
-D_GLIBCXX_ASSERTIONS \
-D_LIBCPP_HARDENING_MODE=_LIBCPP_HARDENING_MODE_FAST \
-fstrict-flex-arrays=3 \
-fstack-clash-protection -fstack-protector-strong \
-Wl,-z,nodlopen -Wl,-z,noexecstack \
-Wl,-z,relro -Wl,-z,now \
-Wl,--as-needed -Wl,--no-copy-dt-needed-entries \
-Wtrampolines \
-fPIE -pie \
-fcf-protection=full -fzero-call-used-regs=used-gpr \
-fno-delete-null-pointer-checks -fno-strict-overflow \
-fno-strict-aliasing -ftrivial-auto-var-init=zero \
-fsanitize=address,undefined
OPTIONS_DEBUG = -Og -Wall -Wextra -Wpedantic -Wcast-align -Wcast-qual \
-Wdisabled-optimization -Wduplicated-branches -Wduplicated-cond \
-Wformat=2 -Wlogical-op -Wmissing-include-dirs -Wnull-dereference \
-Woverloaded-virtual -Wpointer-arith -Wshadow -Wswitch-enum -Wvla \
-fasynchronous-unwind-tables

PROGRAM = main.out
CFILES = main.cpp World.cpp Object.cpp Ground.cpp GroundTiler.cpp
# HPPFILES = list.h
LD_FLAGS = -lraylib -lGL -lm -lpthread -ldl -lrt -lX11 -g

debug: $(CFILES) $(HFILES)
	$(COMPILER) $(OPTIONS_DEBUG) $(CFILES) -o $(PROGRAM) $(LD_FLAGS) 

release: $(CFILES) $(HFILES)
	$(COMPILER) $(OPTIONS_RELEASE) $(CFILES) -o $(PROGRAM) $(LD_FLAGS)

run: $(PROGRAM)
	./$(PROGRAM)

clean:
	rm -f *.out
