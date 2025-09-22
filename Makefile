ifeq ($(HOSTTYPE),)
HOSTTYPE := $(shell uname -m)_$(shell uname -s)
endif

CC      := clang
CFLAGS  := -Wall -Wextra -Werror -g3 -fPIC -I./srcs -MMD -MP
TEST_CFLAGS := $(CFLAGS) -fno-builtin-free -fno-builtin-malloc \
               -fno-builtin-realloc -Wno-unused-variable

SRC_DIR    := srcs/
TEST_DIR   := tests/
OBJ_DIR    := objs/

LIBNAME := libft_malloc_$(HOSTTYPE).so
SONAME  := libft_malloc.so

SRCS     := malloc.c free.c show_alloc_mem.c show_alloc_mem_hex.c
SRCS     := $(addprefix $(SRC_DIR),$(SRCS))
OBJS     := $(patsubst $(SRC_DIR)%.c,$(OBJ_DIR)%.o,$(SRCS))
DEPS     := $(OBJS:.o=.d)

TEST_SRCS := test_free.c test_malloc.c test_threads.c
TEST_SRCS := $(addprefix $(TEST_DIR),$(TEST_SRCS))
TEST_OBJS := $(patsubst $(TEST_DIR)%.c,$(OBJ_DIR)%.o,$(TEST_SRCS))
TEST_DEPS := $(TEST_OBJS:.o=.d)
TEST_EXES := test_free test_malloc test_threads

.PHONY: all clean fclean re test vg helgrind drd

all: $(LIBNAME) $(SONAME)

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

$(OBJ_DIR)%.o: $(SRC_DIR)%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@ -MF $(@:.o=.d)

$(OBJ_DIR)%.o: $(TEST_DIR)%.c | $(OBJ_DIR)
	$(CC) $(TEST_CFLAGS) -c $< -o $@ -MF $(@:.o=.d)

$(LIBNAME): $(OBJS)
	$(CC) -shared -fPIC -o $@ $(OBJS)
	@echo "Built $@"

$(SONAME): $(LIBNAME)
	ln -sf $(LIBNAME) $(SONAME)

test: $(TEST_EXES)
	./test_free
	./test_malloc
	./test_threads

test_free: $(OBJ_DIR)test_free.o $(LIBNAME)
	$(CC) $(CFLAGS) -o $@ $< -L. -lft_malloc_$(HOSTTYPE) -Wl,-rpath,.

test_malloc: $(OBJ_DIR)test_malloc.o $(LIBNAME)
	$(CC) $(CFLAGS) -o $@ $< -L. -lft_malloc_$(HOSTTYPE) -Wl,-rpath,.

test_threads: $(OBJ_DIR)test_threads.o $(LIBNAME)
	$(CC) $(CFLAGS) -o $@ $< -L. -lft_malloc_$(HOSTTYPE) -Wl,-rpath,.

vg: test
	valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./test_free
	valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes --errors-for-leak-kinds=definite
	valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./test_malloc
	valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./test_threads

helgrind: test
	valgrind --tool=helgrind ./test_free
	valgrind --tool=helgrind ./test_malloc
	valgrind --tool=helgrind ./test_threads

drd: test
	valgrind --tool=drd ./test_free
	valgrind --tool=drd ./test_malloc
	valgrind --tool=drd ./test_threads

clean:
	rm -rf $(OBJ_DIR)
	rm -f $(TEST_EXES)
	rm -f *.log

fclean: clean
	rm -f $(LIBNAME) $(SONAME)

re: fclean all

-include $(DEPS)
-include $(TEST_DEPS)
