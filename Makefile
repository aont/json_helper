TARGET	= prog

.PHONY: all clean

all: $(TARGET)

clean:
	$(RM) $(TARGET)

$(TARGET): main.cpp json.hpp
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $(TARGET) main.cpp

