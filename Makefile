NAME = udp_audio_visualizer

CC = gcc
CFLAGS = -O3 -g3 -Wpedantic -Wextra -Wall -D_GNU_SOURCE --std=gnu11 -I./kiss_fft130
CFLAGS += -DFIXED_POINT=16 #Set KISS_FFT to use int16_t
LDFLAGS = -lpthread -lm -lpulse

SRC = $(wildcard *.c) $(wildcard **/*.c)
OBJ = $(SRC:.c=.o)

.PHONY: tsan asan clean format


$(NAME): $(OBJ)
	$(CC) -o $@ $^ $(LDFLAGS)

debug: clean $(NAME)
debug: CFLAGS += -g

callgrind: debug
	-timeout 15s valgrind --tool=callgrind ./$(NAME)
	-kcachegrind

valgrind: debug
	valgrind --leak-check=full ./$(NAME)

tsan: debug
tsan: CFLAGS  += -fsanitize=thread
tsan: LDFLAGS += -ltsan

asan: debug
asan: CFLAGS  += -fsanitize=address
asan: LDFLAGS += -fsanitize=address

clean:
	-rm -f $(OBJ) $(NAME)
	-find . -name '*~'      -exec rm -v "{}" \;
	-find . -name '*.swp'   -exec rm -v "{}" \;
	-find . -name '*.out.*' -exec rm -v "{}" \;
	-find . -name "*.orig"  -exec rm -v "{}" \;

format:
	-find -regex ".*\.\(c\|h\)" -exec \
		astyle --indent=spaces=4\
		--indent-labels --pad-oper --unpad-paren --pad-header \
		--keep-one-line-statements --convert-tabs \
		--indent-preprocessor "{}" \;
