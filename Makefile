NAME = udp_audio_visualizer

CC = gcc
CFLAGS = -O3 -g3 -Wpedantic -Wextra -Wall --std=c11 -I./kiss_fft130
LDFLAGS = -lpthread -lm -lpulse

SRC = $(wildcard *.c) $(wildcard **/*.c)
OBJ = $(SRC:.c=.o)

.PHONY: tsan asan clean format


$(NAME): $(OBJ)
	$(CC) -o $@ $^ $(LDFLAGS)

debug: clean $(NAME)
debug: CFLAGS+=-g

callgrind: debug
	-timeout 15s valgrind --tool=callgrind ./$(NAME)
	-kcachegrind

tsan: debug
tsan: CFLAGS+=-fsanitize=thread
tsan: LDFLAGS+=-ltsan"

asan: debug
asan: CFLAGS+=-fsanitize=address
asan: LDFLAGS+=-fsanitize=address

clean:
	-rm -f $(OBJ) $(NAME)
	-find . -name '*~' -delete
	-find . -name '*.swp' -delete

format:
	-find -regex ".*\.\(c\|h\)" -exec \
		astyle --indent=spaces=4\
		--indent-labels --pad-oper --unpad-paren --pad-header \
		--keep-one-line-statements --convert-tabs \
		--indent-preprocessor "{}" \;
	-find -regex ".*\(orig\)" -exec rm -v "{}" \;
