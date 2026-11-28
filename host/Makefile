CC      ?= gcc
CFLAGS  ?= -std=c99 -Wall -Wextra -Wno-unused-parameter -Wno-unused-function -O1 -g
INC      = -Istubs -I../src -I.
FW       = ../src/main.c
TESTS    = test_input test_tetris test_snake test_gallery test_highscore test_rtos

all: test

test: $(TESTS)
	@set -e; for t in $(TESTS); do echo "== $$t"; ./$$t; done

test_input: test_input.c fake_hw.c $(FW)
	$(CC) $(CFLAGS) $(INC) -Dmain=firmware_main -c $(FW) -o fw_main.o
	$(CC) $(CFLAGS) $(INC) test_input.c fake_hw.c fw_main.o -o $@

# RTX build: real threads on pthreads (rtos_host.c stands in for Keil RTX)
test_rtos: test_rtos.c fake_hw.c rtos_host.c $(FW)
	$(CC) $(CFLAGS) $(INC) -DUSE_RTX -Dmain=firmware_main -c $(FW) -o fw_main_rtx.o
	$(CC) $(CFLAGS) $(INC) -DUSE_RTX -pthread test_rtos.c fake_hw.c rtos_host.c fw_main_rtx.o -o $@

test_%: test_%.c fake_hw.c $(FW)
	$(CC) $(CFLAGS) $(INC) -Dmain=firmware_main -c $(FW) -o fw_main.o
	$(CC) $(CFLAGS) $(INC) $< fake_hw.c fw_main.o -o $@

sim_%: sim_%.c fake_hw.c $(FW)
	$(CC) $(CFLAGS) $(INC) $(SIMFLAGS) -Dmain=firmware_main -c $(FW) -o fw_main.o
	$(CC) $(CFLAGS) $(INC) $(SIMFLAGS) $< fake_hw.c fw_main.o -o $@

clean:
	rm -f $(TESTS) sim_[0-9] *.o *.raw

.PHONY: all test clean

# Renders the real firmware in the simulator and builds docs/img/sim-demo.gif.
# Needs the board's font headers (Keil LCD_Blinky example): make gif FONTS=/path/to/LCD_Blinky
gif:
	@test -n "$(FONTS)" || (echo "usage: make gif FONTS=/path/to/LCD_Blinky" && false)
	$(CC) -std=c99 -O1 -w $(INC) -I$(FONTS) -DKEIL_FONTS -Dmain=firmware_main -c $(FW) -o fw_main.o
	set -e; for n in 1 2 3 4 5; do \
	  $(CC) -std=c99 -O1 -w $(INC) -I$(FONTS) -DKEIL_FONTS -DSIM_APP=$$n sim.c fake_hw.c fw_main.o -o sim_$$n; \
	  ./sim_$$n seg$$n.raw 100; \
	done
	python3 ../tools/make_gif.py ../docs/img/sim-demo.gif 2 seg1.raw seg2.raw:50 seg3.raw:130 seg4.raw:100 seg5.raw:20

.PHONY: gif
