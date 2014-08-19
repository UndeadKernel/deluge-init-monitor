# Project Name (executable)
PROJECT = deluge_monitor
# Source folder
SOURCE_DIR = src

# Compiler
CC = gcc

# Run Options       
COMMANDLINE_OPTIONS = -c /home/deluge/deluge_monitor.conf

# Compiler options during compilation
COMPILE_OPTIONS = -std=gnu99 -pedantic -Wall -O2

#Header include directories
HEADERS = -Ilibs/include
#Libraries for linking
LIBS = 
STATIC_LIBS = libs/lib/libconfig.a

# Dependency options
DEPENDENCY_OPTIONS = -MM


#-- Do not edit below this line --


# Subdirs to search for additional source files
SUBDIRS := $(shell ls -F $(SOURCE_DIR)/ | grep "\/" )
DIRS := $(SOURCE_DIR)/ $(SUBDIRS)
SOURCE_FILES := $(foreach d, $(DIRS), $(wildcard $(d)*.c) )

# Create an object file of every cpp file
OBJECTS = $(patsubst %.c, %.o, $(SOURCE_FILES))

# Dependencies
DEPENDENCIES = $(patsubst %.c, %.d, $(SOURCE_FILES))

# Create .d files
%.d: %.c
	$(CC) $(DEPENDENCY_OPTIONS) $< -MT "$*.o $*.d" -MF $*.d

# Make $(PROJECT) the default target
all: $(DEPENDENCIES) $(PROJECT)

$(PROJECT): $(OBJECTS)
	$(CC) -o $(PROJECT) $(OBJECTS) $(LIBS) $(STATIC_LIBS)

# Include dependencies (if there are any)
ifneq "$(strip $(DEPENDENCIES))" ""
  -include $(DEPENDENCIES)
endif

# Compile every cpp file to an object
%.o: %.c
	$(CC) -c $(COMPILE_OPTIONS) -o $@ $< $(HEADERS)

# Build & Run Project
run: $(PROJECT)
	./$(PROJECT) $(COMMANDLINE_OPTIONS)

# Create the TAGS file
.PHONY: etags
etags:
	find $(SOURCE_DIR)/ -name '*.[ch]' | xargs etags -o $(SOURCE_DIR)/TAGS

# Clean & Debug
.PHONY: makefile-debug
makefile-debug:

.PHONY: clean
clean:
	rm -f $(PROJECT) $(OBJECTS)

.PHONY: depclean
depclean:
	rm -f $(DEPENDENCIES)

clean-all: clean depclean
