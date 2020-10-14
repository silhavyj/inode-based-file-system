TARGET = fs
CCX    = g++
FLAGS  = -pthread -Wall -std=c++14 -pedantic-errors -Wextra
SRC    = src
BIN    = bin
SOURCE = $(wildcard $(SRC)/*.cpp)
OBJECT = $(patsubst %,$(BIN)/%, $(notdir $(SOURCE:.cpp=.o)))

$(TARGET) : $(OBJECT)
	$(CCX) $(FLAGS) -o $@ $^

$(BIN)/%.o : $(SRC)/%.cpp
	@mkdir -p $(BIN)
	$(CCX) $(FLAGS) -c $< -o $@

clean:
	rm -r $(BIN) $(TARGET)
