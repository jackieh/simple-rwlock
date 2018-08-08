# Uncomment the following line to disable debug functionality.
DEBUG_FLAGS = -DDEBUG -g

.PHONY: default
default: all

TOP_DIR = .
SRC_DIR = $(TOP_DIR)/src
TEST_DIR = $(TOP_DIR)/test
TEST_CLASS_DIR = $(TEST_DIR)/simple_rwlock_test
TESTS_DIR = $(TEST_CLASS_DIR)/tests

CXX = g++ -std=c++17 -Wall -Wextra

# Have all .cpp files built into .o files
# to be linked into a library or executable.
%.o: %.cpp
	$(CXX) $(BUILD_FLAGS) -o $@ -c $<

LIB_OUT = libsimple_rwlock.a
TEST_OUT = simple_rwlock_test

LIB_SRC = $(SRC_DIR)/simple_rwlock_debug_helpers.cpp \
		  $(SRC_DIR)/simple_rwlock.cpp
LIB_OBJ = $(LIB_SRC:.cpp=.o)
$(LIB_OBJ): BUILD_FLAGS := -I $(SRC_DIR) $(DEBUG_FLAGS)
$(LIB_OUT): $(LIB_OBJ)
	rm -f $@
	ar cq $@ $(LIB_OBJ)

TEST_SRC = $(TEST_DIR)/main.cpp \
		   $(TEST_CLASS_DIR)/clock.cpp \
		   $(TEST_CLASS_DIR)/test.cpp \
		   $(TEST_CLASS_DIR)/tests/single_thread_tests.cpp \
		   $(TEST_CLASS_DIR)/tests/two_thread_tests.cpp \
		   $(TEST_CLASS_DIR)/tester.cpp
TEST_OBJ = $(TEST_SRC:.cpp=.o)
$(TEST_OBJ): BUILD_FLAGS := -I $(SRC_DIR) -I $(TEST_DIR) -g
$(TEST_OUT): LINK_FLAGS := -L$(TOP_DIR) -lsimple_rwlock -pthread
$(TEST_OUT): $(LIB_OUT) $(TEST_OBJ)
	$(CXX) -o $@ $(TEST_OBJ) $(LINK_FLAGS)

# Targets
.PHONY: lib test all clean
lib: $(LIB_OUT)
test: $(LIB_OUT) $(TEST_OUT)
all: lib test
clean:
	rm -f $(LIB_OUT)
	rm -f $(TEST_OUT)
	rm -f $(SRC_DIR)/*.o
	rm -f $(TEST_DIR)/*.o
	rm -f $(TEST_CLASS_DIR)/*.o
	rm -f $(TESTS_DIR)/*.o
