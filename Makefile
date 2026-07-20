#
#
#
ifeq ($(CC),)
  CC=gcc
endif

ifeq ($(CXX),)
  CXX=g++
endif

BUILD=./build
SRCDIR = .

TARGET = $(BUILD)/DPasm

INC_DIRS := ./
INC_FLAGS := $(addprefix -I,$(INC_DIRS))

SRCS := $(shell find . -name "*.cpp")

CFLAGS+=-Wall -g $(MSTATIC)
CXXFLAGS += $(CFLAGS) $(INC_FLAGS)

OBJ=$(BUILD)/main.o \
		$(BUILD)/FileUtils.o \
		$(BUILD)/Section.o \
		$(BUILD)/StrUtils.o \
		$(BUILD)/All-Directives.o \
		$(BUILD)/All-Operators.o \
		$(BUILD)/Assembler.o \
		$(BUILD)/CodeLine.o \
		$(BUILD)/Errors.o \
		$(BUILD)/ExpVector.o \
		$(BUILD)/Expression.o \
		$(BUILD)/Operator.o \
		$(BUILD)/Parser.o \
		$(BUILD)/Z80-Instructions.o \
		$(BUILD)/Z180-Instructions.o \
		$(BUILD)/Z80-Operands.o

all: $(BUILD) $(TARGET)

$(TARGET): $(OBJ)
	@echo "Linking $(TARGET)..."
	$(CXX) $(CXXFLAGS) $^ -o $@

$(BUILD)/%.o: $(SRCDIR)/%.cpp | $(BUILD)
	@echo "Compiling $<..."
	@$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILD):
	mkdir -p $(BUILD)

clean:
	rm -rf $(BUILD)

.PHONY: all clean