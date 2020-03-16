.SILENT:
.PHONY: all clean

LIBDIR=../../src
CXXFLAGS+=-D__TEST__ -Wall -DOWNG_CONFIG_FILE="\"test_config.h\"" -I$(LIBDIR) -I.

TESTS=\
    OneWireNg_Test \
    OneWireNg_BitBang_Test

all: $(TESTS)
	./OneWireNg_Test
	./OneWireNg_BitBang_Test

clean:
	$(RM) $(LIBDIR)/*.o
	$(RM) *.o $(TESTS)

OneWireNg_BitBang_Test: $(LIBDIR)/OneWireNg.o

%_Test: %_Test.o $(LIBDIR)/%.o
	$(CXX) $(CXXFLAGS) $^ -o $@

$(LIBDIR)/OneWireNg_BitBang.o: $(LIBDIR)/OneWireNg.h

$(LIBDIR)/%.o: $(LIBDIR)/%.cpp $(LIBDIR)/%.h test_config.h
	$(CXX) -c $(CXXFLAGS) $< -o $@
