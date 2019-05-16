.SILENT:
.PHONY: all clean

CXXFLAGS+=-D__TEST__ -Wall -DOWNG_CONFIG_FILE="\"test_config.h\"" -I../../src -I.

TESTS=\
    OneWireNg_Test \
    OneWireNg_BitBang_Test

all: $(TESTS)
	./OneWireNg_Test
	./OneWireNg_BitBang_Test

clean:
	$(RM) ../../src/*.o
	$(RM) *.o $(TESTS)

OneWireNg_BitBang_Test: ../../src/OneWireNg.o

%_Test: %_Test.o ../../src/%.o
	$(CXX) $(CXXFLAGS) $^ -o $@

../../src/OneWireNg_BitBang.o: ../../src/OneWireNg.h

../../src/%.o: ../../src/%.cpp ../../src/%.h test_config.h
	$(CXX) -c $(CXXFLAGS) $< -o $@
