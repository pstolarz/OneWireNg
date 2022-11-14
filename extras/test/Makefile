.SILENT:
.PHONY: all build clean lib libclean analyze

LIBDIR=../../src
CXXFLAGS+=-DOWNG_TEST -Wall -DOWNG_CONFIG_FILE="\"test_config.h\"" -I$(LIBDIR) -I.

LIBOBJS=\
	$(LIBDIR)/OneWireNg.o \
	$(LIBDIR)/OneWireNg_BitBang.o \
	$(LIBDIR)/drivers/DSTherm.o

TESTS=\
	t01_OneWireNg_Test \
	t02_OneWireNg_BitBang_Test \
	t03_DSTherm_Test \
	t04_MAX31850_Test

t01_OneWireNg_Test: TDEFS=-DT01
t02_OneWireNg_BitBang_Test: TDEFS=-DT02
t03_DSTherm_Test: TDEFS=-DT03
t04_MAX31850_Test: TDEFS=-DT04

all: build
	for t in $(TESTS); do echo "TEST: $$t"; ./$$t || exit; echo; done;

build: $(TESTS)

lib: libclean $(LIBOBJS)

clean: libclean
	$(RM) -r $(TESTS) compile_commands.json report report-html

libclean:
	$(RM) $(LIBOBJS)

analyze: clean
	CodeChecker log -b "$(MAKE) build" -o compile_commands.json
	CodeChecker analyze compile_commands.json -o report --analyzers clangsa clang-tidy
	-CodeChecker parse -e html -o report-html report

%: %.cpp
	CXXFLAGS="$(TDEFS)" $(MAKE) lib
	$(CXX) $(CXXFLAGS) $(TDEFS) $< -o $@ $(LIBOBJS)

$(LIBDIR)/%.o: $(LIBDIR)/%.c $(LIBDIR)/%.h $(LIBDIR)/OneWire.h test_config.h
	$(CC) -c $(CFLAGS) $< -o $@
