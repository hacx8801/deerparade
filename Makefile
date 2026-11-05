# Variables
COMPILER = g++
OPTIONS = -O2 -Wall -Wformat -Wformat=2 -Wconversion -Wimplicit-fallthrough \
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
