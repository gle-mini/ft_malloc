# Compiler and flags
CC      := cc
CFLAGS  := -Wall -Wextra -Werror -g3 -fPIC -I./srcs -MMD -MP
TEST_CFLAGS := $(CFLAGS) -Wno-unused-variable


# Directories
SRC_DIR    := srcs/
TEST_DIR   := tests/
OBJ_DIR    := objs/

# Define the host type and set the shared library name accordingly.
HOST    := $(shell uname -m)
LIBNAME := libft_malloc_$(HOST).so

# Source files (for the shared library)
SRCS     := $(addsuffix .c, $(addprefix $(SRC_DIR), ft_malloc ft_free show_alloc_mem))
OBJS     := $(patsubst $(SRC_DIR)%.c, $(OBJ_DIR)%.o, $(SRCS))
DEPS     := $(OBJS:.o=.d)

# Test source files and object files
TEST_SRCS := $(addprefix $(TEST_DIR), test_free.c test_malloc.c test_threads.c)
# Object files for tests (placed in objs; note that test source names are unique)
TEST_OBJS := $(patsubst $(TEST_DIR)%.c, $(OBJ_DIR)%.o, $(TEST_SRCS))
# Test executables names
TEST_EXES := test_free test_malloc test_threads

# Rule to create the object directory if needed
$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

# Build the shared library.
$(LIBNAME): $(OBJS)
	$(CC) -shared -fPIC -o $@ $(OBJS)

# Compile source files to objects (for the shared library).
$(OBJ_DIR)%.o: $(SRC_DIR)%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@ -MF $(@:.o=.d)

# Compile test source files to objects.
 $(OBJ_DIR)%.o: $(TEST_DIR)%.c | $(OBJ_DIR)
	$(CC) $(TEST_CFLAGS) -c $< -o $@ -MF $(@:.o=.d)

# Link test_free executable.
test_free: $(OBJ_DIR)test_free.o $(LIBNAME)
	$(CC) $(CFLAGS) -o $@ $(OBJ_DIR)test_free.o -L. -lft_malloc_$(HOST) -Wl,-rpath,.

# Link test_malloc executable.
test_malloc: $(OBJ_DIR)test_malloc.o $(LIBNAME)
	$(CC) $(CFLAGS) -o $@ $(OBJ_DIR)test_malloc.o -L. -lft_malloc_$(HOST) -Wl,-rpath,.

test_threads: $(OBJ_DIR)test_threads.o $(LIBNAME)
	$(CC) $(CFLAGS) -o $@ $(OBJ_DIR)test_threads.o -L. -lft_malloc_$(HOST) -Wl,-rpath,.

# Test rule: launch test_free and test_malloc executables.
test: test_free test_malloc test_threads
	./test_free
	./test_malloc

# Valgrind target: run both test executables under Valgrind.
vg: test_free test_malloc test_threads
	valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./test_free
	valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./test_malloc
	valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./test_threads

# Hellgrind target: run both test executables under Helgrind.
helgrind: test_free test_malloc test_threads
	valgrind --tool=helgrind ./test_free
	valgrind --tool=helgrind ./test_malloc
	valgrind --tool=helgrind ./test_threads

drd: test_free test_malloc test_threads
	valgrind --tool=drd ./test_free
	valgrind --tool=drd ./test_malloc
	valgrind --tool=drd ./test_threads


.PHONY: all clean fclean re vg hellgrind test

# Include dependency files.
-include $(DEPS)
-include $(TEST_OBJS:.o=.d)

.PHONY: all clean fclean re vg

all: $(LIBNAME)

clean:
	rm -rf $(OBJ_DIR)
	rm -f $(TEST_EXES)
	rm -f *.log

fclean: clean
	rm -f $(LIBNAME)

re: fclean all
