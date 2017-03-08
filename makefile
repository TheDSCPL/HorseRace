COMPILER = g++
PROGRAM_NAME = BOP2_proj
MAIN_FILE = Main

BIN_DIR = bin
SRC_DIR = src
HEADRES_DIR = headers
LIBS_DIR = libs

FLAGS = -std=c++11 -pthread -Wnarrowing
PFLAGS = -Wall -pedantic
DFLAGS = -g
PACKFLAGS = -static -static-libgcc -static-libstdc++

MY_LIBS = SHA-256
DYNAMIC_LIBS = pq cryptopp

#LINKER_STATIC_LIBS_PATHS = libs
#LINKER_DYNAMIC_LIBS_PATHS = /usr/lib/postgresql/9.6/lib
#LINKER_LIBS_PATHS = $(patsubst %,-L%,$(LINKER_STATIC_LIBS_PATHS)) $(patsubst %,-L%,$(LINKER_DYNAMIC_LIBS_PATHS))

LINKER_LIBS =  $(patsubst %,-l%,$(DYNAMIC_LIBS))
#LINKER_LIBS =  -Wl, -Bdynamic $(patsubst %,-l%,$(DYNAMIC_LIBS))
#LINKER_LIBS += -Wl, -Bstatic $(patsubst %,-l%,$(STATIC_LIBS))

#LINKER_FLAGS = $(LINKER_LIBS_PATHS) $(LINKER_LIBS)
FILES = Log DBMS Sockets Client Race Properties Utils $(MAIN_FILE)
#FILES += $(MAIN_FILE)

#RED_BG=[31m
#GREEN_BG=[32m
#YELLOW_BG=[33m
#BLUE_BG=[34m
#DEFAULT=[0m
#RESET_COLORS=[37;40m
#BOLD=[1m

BIN_FILES = $(patsubst %,$(BIN_DIR)/%.o,$(FILES))
SRC_FILES = $(patsubst %,$(SRC_DIR)/%.cpp,$(FILES))
HEADER_FILES = $(patsubst %,$(HEADERS_DIR)/%.hpp,$(filter-out $(MAIN_FILE),$(FILES)))
MY_LIBS_FILES = $(patsubst %,$(LIBS_DIR)/%.o,$(MY_LIBS))

.PHONY: all rebuild pedantic debug set_pack set_pedantic set_debug show_start create_bin clean

#PLEASE COMPILE WITH THE "-s" FLAG AT THE END

#----------MAIN COMPILING MODES (all (normal compilation), pedantic (all warnings in the compilation) and debug (for GDB))------------#

all: $(PROGRAM_NAME)
	@printf '\033[1;4;32m'
	@echo -e '$(PROGRAM_NAME) compiled normally!'
	@printf '\033[0m'
	@echo -e
	@echo Finished building at `date +'20%y/%m/%d %Hh:%Mm:%Ss'`
	@echo -e

rebuild: clean all

pack: #set_pack $(PROGRAM_NAME)
	@echo Functionality not implemented yet!
	@#printf '\033[1;4;32m'
	@#echo -e '$(PROGRAM_NAME) compiled without external libraries!'
	@#printf '\033[0m'
	@#echo -e
	@#echo Finished building at `date +'20%y/%m/%d %Hh:%Mm:%Ss'`
	@#echo -e

pedantic: set_pedantic $(PROGRAM_NAME)
	@printf '\033[1;4;32m'
	@echo -e '$(PROGRAM_NAME) compiled with extra warnings!'
	@printf '\033[0m'
	@echo -e
	@echo Finished building at `date +'20%y/%m/%d %Hh:%Mm:%Ss'`
	@echo -e

debug: set_debug set_pedantic $(PROGRAM_NAME)
	@printf '\033[1;4;32m'
	@echo -e '$(PROGRAM_NAME) compiled a gdb-ready version of the program!'
	@printf '\033[0m'
	@echo -e
	@echo Finished building at `date +'20%y/%m/%d %Hh:%Mm:%Ss'`
	@echo -e

#----------SET THE FLAGS IN CASE OF "pedantic" OR "debug" COMPILATION------------#

set_pack:
	@$(eval FLAGS += $(PACKFLAGS))

set_pedantic:
	@$(eval FLAGS += $(PFLAGS))

set_debug:
	@$(eval FLAGS += $(DFLAGS))

#----------COMPILE LIBRARIES----------#
#$(shell echo $(patsubst $(BIN_DIR)/%.o,$(HEADRES_DIR)/%.hpp,$@) | grep -v $(HEADRES_DIR)/$(MAIN_FILE).hpp) = $(filter-out $(HEADRES_DIR)/$(MAIN_FILE).hpp,$(HEADRES_DIR)/%.hpp)
#the crazy shell command is what I came up with for not having a header when compiling the Main.cpp file
.SECONDEXPANSION:
$(BIN_DIR)/%.o: $(SRC_DIR)/%.cpp $$(wildcard $$(HEADRES_DIR)/%.hpp)
	@printf '\033[33m'
	@echo -n "\tCompiling '"$(patsubst $(BIN_DIR)/%.o,%,$@)"' binary: "
	@printf '\033[0m'
	@$(COMPILER) -c -o $@ $< $(FLAGS)
	@printf '\033[32m'
	@echo "Success!"
	@printf '\033[0m'
	@echo -e

#----------MAIN PROJECT RECEPY RECIPE------------#

_TEMP = $(COMPILER) $(BIN_FILES) $(MY_LIBS_FILES) -o $@ $(FLAGS) $(LINKER_LIBS)

$(PROGRAM_NAME): show_start create_bin $(BIN_FILES)
	@#echo $(_TEMP)
	@$(_TEMP)

#----------show_startING RECIPES------------#

show_start:
	@clear
	@echo -e
	@echo Started building at `date +'20%y/%m/%d %Hh:%Mm:%Ss'`
	@echo -e
	@#echo $(shell echo headers/Main.hpp | grep -v $(HEADRES_DIR)/(MAIN_FILE).hpp)
	@#echo "'"$(HEADRES_DIR)/$(MAIN_FILE).hpp"'"
	@#echo "'"headers/Main.hpp"'"
	@#rm -rf $(PROGRAM_NAME)

create_bin:
	@mkdir -p bin

clean:
	@rm -rf bin/*.o
