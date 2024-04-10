CC = g++
CFLAG += -std=c++11 -Wall -g -pthread
APP = threadpool
TARGET = $(wildcard *.cpp)
OBJ = $(patsubst %.cpp,%.o,$(TARGET))
HEADER = $(wildcard *.h)

$(APP) : $(OBJ) $(HEADER)
	$(CC) $^ -o $@ $(CFLAG)
%.o : %.cpp
	$(CC) -c $< -o $@ $(CFLAG)

.PHONY : all clean

all : $(TARGET)
 
clean : 
	 -rm -f ./$(OBJ)
	 -rm -f ./$(APP)
echo:
	@echo $(TARGET)
	@echo $(OBJ)
	@echo $(HEADER)