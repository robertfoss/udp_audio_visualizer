NAME = udp_audio_visualizer

CC = gcc
CFLAGS = -O3 -g3 -Wextra -Wall --std=c11 -I./kiss_fft130
LDFLAGS = -lpthread -lm -lpulse

SRC = $(wildcard *.c) $(wildcard **/*.c)
OBJ = $(SRC:.c=.o)

$(NAME): $(OBJ)
	$(CC) -o $@ $^ $(LDFLAGS)

.PHONY: clean format
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
