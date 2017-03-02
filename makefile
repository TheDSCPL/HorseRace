COMPILER = g++
FLAGS= -std=c++11 -pthread -lpq
PFLAGS= -Wall -pedantic
DFLAGS= -g
PROGRAM_NAME= BOP2_proj
BIN_DIR = bin
SRC_DIR = src
HEADRES_DIR = headers
MAIN_FILE = Main
FILES = Log DBMS Sockets Client Error Race Properties
FILES += $(MAIN_FILE)

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

.PHONY: all pedantic debug set_pedantic set_debug show_start clean

#PLEASE COMPILE WITH THE "-s" FLAG AT THE END

#----------MAIN COMPILING MODES (all (normal compilation), pedantic (all warnings in the compilation) and debug (for GDB))------------#

all: $(PROGRAM_NAME)
	printf '\033[1;4;32m'
	echo -e '$(PROGRAM_NAME) compiled normally!'
	printf '\033[0m'
	echo -e
	echo Finished building at `date +'20%y/%m/%d %Hh:%Mm:%Ss'`
	echo -e

pedantic: set_pedantic $(PROGRAM_NAME)
	printf '\033[1;4;32m'
	echo -e '$(PROGRAM_NAME) compiled with extra warnings!'
	printf '\033[0m'
	echo -e
	echo Finished building at `date +'20%y/%m/%d %Hh:%Mm:%Ss'`
	echo -e

debug: set_debug set_pedantic $(PROGRAM_NAME)
	printf '\033[1;4;32m'
	echo -e '$(PROGRAM_NAME) compiled a gdb-ready version of the program!'
	printf '\033[0m'
	echo -e
	echo Finished building at `date +'20%y/%m/%d %Hh:%Mm:%Ss'`
	echo -e

#----------SET THE FLAGS IN CASE OF "pedantic" OR "debug" COMPILATION------------#

set_pedantic:
	$(eval FLAGS += $(PFLAGS))

set_debug:
	$(eval FLAGS += $(DFLAGS))

#----------COMPILE LIBRARIES----------#
#$(shell echo $(patsubst $(BIN_DIR)/%.o,$(HEADRES_DIR)/%.hpp,$@) | grep -v $(HEADRES_DIR)/$(MAIN_FILE).hpp) = $(filter-out $(HEADRES_DIR)/$(MAIN_FILE).hpp,$(HEADRES_DIR)/%.hpp)
#the crazy shell command is what I came up with for not having a header when compiling the Main.cpp file
.SECONDEXPANSION:
$(BIN_DIR)/%.o: $(SRC_DIR)/%.cpp $$(wildcard $$(HEADRES_DIR)/%.hpp)
	printf '\033[33m'
	echo -n "\tCompiling '"$(patsubst $(BIN_DIR)/%.o,%,$@)"' binary"
	$(COMPILER) -c -o $@ $< $(FLAGS)
	echo -n ": "
	printf '\033[32m'
	echo "Success!"
	printf '\033[0m'
	echo -e

#----------MAIN PROJECT RECEPY RECIPE------------#

$(PROGRAM_NAME): show_start $(BIN_FILES)
	$(COMPILER) $(BIN_FILES) -o $@ $(FLAGS)

#----------show_startING RECIPES------------#

show_start:
	clear
	echo -e
	echo Started building at `date +'20%y/%m/%d %Hh:%Mm:%Ss'`
	echo -e
	#echo $(shell echo headers/Main.hpp | grep -v $(HEADRES_DIR)/(MAIN_FILE).hpp)
	#echo "'"$(HEADRES_DIR)/$(MAIN_FILE).hpp"'"
	#echo "'"headers/Main.hpp"'"
	#rm -rf $(PROGRAM_NAME)

clean:
	rm -rf bin/*.o
