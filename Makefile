CC = gcc
INCLUDES = -Isource/include
CDEFINES = -D_CRT_SECURE_NO_WARNINGS
CFLAGS =
LIBS =

# Generating clang files
ifeq ($(CC), clang)
CL_CCJ   = @sed -e '1s/^/[\n''/' -e '$$s/,$$/\n'']/' $(OBJ_JSON) > compile_commands.json
CL_JSON = -MJ $@.json
endif

# Set Platform
PLATFORM = $(shell uname)
ifeq ($(PLATFORM), Windows_NT)
EXTENCION = .exe
CDEFINES += -DPLATFORM_WINDOWS
CFLAGS += -std=c99
LIBS += 

else ifeq ($(PLATFORM), Linux)
EXTENCION =
CDEFINES += -DPLATFORM_LINUX
CFLAGS += -std=gnu99
LIBS += -lm
endif

# Set Compile Mode
config = debug
ifeq ($(config), debug)
COMPILE_MODE = debug/
CFLAGS += -g -Wall
CDEFINES += -DDEBUG_MODE
CDEFINES += -D"CONTENT_PATH=$(abspath content/)/"
CDEFINES += -D"CONFIG_PATH=$(abspath config/)/"
else
COMPILE_MODE = release/
CFLAGS +=
CDEFINES += -DRELEASE_MODE
CDEFINES += -D"CONTENT_PATH=data/content/"
CDEFINES += -D"CONFIG_PATH=data/config/"

# Copy files in release mode
CREATE_DATA = @mkdir -p $(BUILD_DIR)/$(COMPILE_MODE)/data
COPY_FILES  = @cp -f -r "$(abspath content/)" "$(BUILD_DIR)/$(COMPILE_MODE)/data"
COPY_CONFIG = @cp -f -r "$(abspath config/)" "$(BUILD_DIR)/$(COMPILE_MODE)/data"
endif

# Set Auto Config
TARGET = $(notdir $(CURDIR))
BUILD_DIR = build/
OBJ_DIR = $(BUILD_DIR)obj/$(COMPILE_MODE)
SRC_DIR = source/
INCLUDE_DIR = $(SRC_DIR)/include/
CONTENT_DIR = content/
CONFIG_DIR = config/
SRC = $(wildcard $(SRC_DIR)*.c) $(wildcard $(SRC_DIR)*/*.c) $(wildcard $(SRC_DIR)*/*/*.c) $(wildcard $(SRC_DIR)*/*/*/*.c)
OBJ = $(addprefix $(OBJ_DIR), $(notdir $(SRC:.c=.o)))
OBJ_JSON = $(addprefix $(OBJ_DIR), $(notdir $(OBJ:.o=.o.json)))

all: $(TARGET)

install:
	@echo "Stating Compiling => Platform:$(PLATFORM), Compiler:$(CC), Mode:$(config)"
	@mkdir -p $(BUILD_DIR)
	@mkdir -p $(BUILD_DIR)$(COMPILE_MODE)
	@mkdir -p $(OBJ_DIR)
	@mkdir -p $(SRC_DIR)
	@mkdir -p $(INCLUDE_DIR)
	@mkdir -p $(CONTENT_DIR)
	@mkdir -p $(CONFIG_DIR)

clean:
	@echo "Cleaning Everything..."
	@rm -f -r $(BUILD_DIR)
	@rm -f -r .cache
	@rm -f -r .vscode
	@rm -f compile_commands.json

run: $(TARGET)
	@echo "Running => $(BUILD_DIR)$(COMPILE_MODE)$(TARGET)$(EXTENCION)"
	@$(BUILD_DIR)$(COMPILE_MODE)$(TARGET)$(EXTENCION)

$(TARGET): install $(OBJ)
	@echo "Linking => $(BUILD_DIR)$(COMPILE_MODE)$(TARGET)$(EXTENCION)"
	@$(CC) $(OBJ) $(INCLUDES) $(CFLAGS) $(CDEFINES) $(LIBS) -o $(BUILD_DIR)$(COMPILE_MODE)$(TARGET)$(EXTENCION)
	@$(CL_CCJ)
	@$(CREATE_DATA)
	@$(COPY_CONFIG)
	@$(COPY_FILES)

$(OBJ_DIR)%.o: $(SRC_DIR)%.c
	@echo "Compiling:$<..."
	@$(CC) $(CL_JSON) $< $(INCLUDES) $(CFLAGS) $(CDEFINES) -c -o $@

$(OBJ_DIR)%.o: $(SRC_DIR)*/%.c
	@echo "Compiling:$<..."
	@$(CC) $(CL_JSON) $< $(INCLUDES) $(CFLAGS) $(CDEFINES) -c -o $@

$(OBJ_DIR)%.o: $(SRC_DIR)*/*/%.c
	@echo "Compiling:$<..."
	@$(CC) $(CL_JSON) $< $(INCLUDES) $(CFLAGS) $(CDEFINES) -c -o $@

$(OBJ_DIR)%.o: $(SRC_DIR)*/*/*/%.c
	@echo "Compiling:$<..."
	@$(CC) $(CL_JSON) $< $(INCLUDES) $(CFLAGS) $(CDEFINES) -c -o $@
