##############################################################################################################
## COMPANY     : Ruhr-Universität Bochum, Chair for Security Engineering									##
## AUTHOR      : Pascal Sasdrich (pascal.sasdrich@rub.de)													##
## DESCRIPTION : Simple and configurable C/C++ Makefile														##
##																											##
## Copyright (c) 2020, Pascal Sasdrich. All rights reserved.												##
##############################################################################################################

##############################################################################################################
## CONFIGURATION																							##
##############################################################################################################

# DIRECTORIES
SOURCE_DIR	 	:= ./src
INCLUDE_DIR		:= ./inc
LIBRARY_DIR		:= ./lib
BINARY_DIR		:= ./bin
BUILD_DIR		:= ./build

# COMPILER/LINKER OPTIONS
CC					:= gcc
C_RELEASE_FLAGS		:= -m64 -march=native -O3 -fomit-frame-pointer -std=c11
C_DEBUG_FLAGS		:= -Wall -Wextra -pedantic -m64 -march=native -g3 -Og -fsanitize=address -std=c11

CXX					:= g++
CXX_RELEASE_FLAGS	:= -m64 -march=native -O3 -fomit-frame-pointer -std=c++17 -fopenmp
CXX_DEBUG_FLAGS		:= -Wall -Wextra -pedantic -m64 -march=native -g3 -Og -fsanitize=address -std=c++17 -fopenmp

LD_RELEASE_FLAGS	:= -m64
LD_DEBUG_FLAGS		:= -m64 -fsanitize=address

# EXTERNAL INCLUDES
INCLUDES			:= "/mnt/c/Program Files/boost/boost_1_73_0"

# LIBRARIES
LIBRARIES			:= -lcudd -lboost_program_options -lgomp

# TARGET PROGRAM NAME
TARGET				:= verify

# DEFAULT VERBOSITY
VERBOSITY			:= 0

##############################################################################################################
## CORE																										##
##############################################################################################################

.PHONY: all build check clean compile debug release

# store make invocation time
START_TIME := $(shell date +%s%3N)

# if verbosity is set to 0, pipe outputs to null and supress command printing
ifeq ($(VERBOSITY),0)
	SUPPRESS := @
	PIPE := > /dev/null
endif

# tell make to not print spam on recursive calls
MAKEFLAGS += --no-print-directory

# clang/gcc options to generate dependency files
DEPENDENCYFLAGS = -MT $@ -MMD -MP -MF $(OBJECT_DIR)/$*.d

# select appropriate flags
ifeq ($(DEBUG),1)
	C_FLAGS 	= $(C_DEBUG_FLAGS)
	CXX_FLAGS 	= $(CXX_DEBUG_FLAGS)
	LD_FLAGS 	= $(LD_DEBUG_FLAGS)
else
	C_FLAGS 	= $(C_RELEASE_FLAGS)
	CXX_FLAGS 	= $(CXX_RELEASE_FLAGS)
	LD_FLAGS 	= $(LD_RELEASE_FLAGS)
endif

# list all .c and .cpp source files
C_SOURCES	:= $(shell find $(SOURCE_DIR) -name '*.c' | sort -k 1nr | cut -f2-)
CXX_SOURCES	:= $(shell find $(SOURCE_DIR) -name '*.cpp' | sort -k 1nr | cut -f2-)

#select appropriate linker
ifeq ($(CXX_SOURCES),)
	LINK	:= $(CC)
else
	LINK	:= $(CXX)
endif

# create object file names in the obj directory
OBJECTS := $(patsubst $(SOURCE_DIR)/%,$(OBJECT_DIR)/%, $(C_SOURCES:.c=.o)) $(patsubst $(SOURCE_DIR)/%,$(OBJECT_DIR)/%, $(CXX_SOURCES:.cpp=.o))

##############################################################################################################
## USER TARGETS																								##
##############################################################################################################

all: debug release

clean:
	@echo  Removing build artifacts...
	-@rm -rvf $(BUILD_DIR)/*
	-@rm -rvf $(BINARY_DIR)/*

debug:
	@+make compile DEBUG=1 TARGET_DIR=$(BINARY_DIR)/debug OBJECT_DIR=$(BUILD_DIR)/debug

release:
	@+make compile DEBUG=0 TARGET_DIR=$(BINARY_DIR)/release OBJECT_DIR=$(BUILD_DIR)/release

help:

##############################################################################################################
## INTERNAL TARGETS																							##
##############################################################################################################

check:
ifeq ($(TARGET_DIR),)
	$(info Unsupported command.)
	$(error )
endif

compile: check build $(TARGET_DIR)/$(TARGET)
	@diff=$$(($(shell date +%s%3N) - $(START_TIME))); echo 'Build completed in '$$(($$diff / 1000))'.'$$(($$diff % 1000))'s.'
	@echo

build: check
ifeq ($(DEBUG), 1)
	@echo '_____ BUILDING (DEBUG) _____'
else
	@echo '____ BUILDING (RELEASE) ____'
endif
	@mkdir -p $(TARGET_DIR)
	@mkdir -p $(OBJECT_DIR)

# compile .c files
$(OBJECT_DIR)/%.o: $(SOURCE_DIR)/%.c
	@mkdir -p '$(dir $@)'
	$(SUPPRESS)$(CC) -c $< -o $@ $(DEPENDENCYFLAGS) $(C_FLAGS) -I $(INCLUDE_DIR) -I $(INCLUDES)
	@touch $@

# compile .cpp files
$(OBJECT_DIR)/%.o: $(SOURCE_DIR)/%.cpp
	@mkdir -p '$(dir $@)'
	$(SUPPRESS)$(CXX) -c $< -o $@ $(DEPENDENCYFLAGS) $(CXX_FLAGS) -I $(INCLUDE_DIR) -I $(INCLUDES) -L$(LIBRARY_DIR) $(LIBRARIES) 
	@touch $@

# link target
$(TARGET_DIR)/$(TARGET) : $(OBJECTS)
	@echo Linking
	$(SUPPRESS)$(LINK) -o $(TARGET_DIR)/$(TARGET) $(OBJECTS) $(LD_FLAGS) -I $(INCLUDE_DIR) -I $(INCLUDES) -L$(LIBRARY_DIR) $(LIBRARIES)

#Pull in dependency info for *existing* .o files
-include $(OBJECTS:.o=.d)