BUILDDIR = build
INCDIRS = include
SRCDIR = src
TESTDIR = test
 
OBJS = $(addprefix $(BUILDDIR)/, $(patsubst %.cpp, %.o, $(notdir $(wildcard $(SRCDIR)/*.cpp))))
TESTOBJS = $(addprefix $(BUILDDIR)/, $(patsubst %.cpp, %.o, $(notdir $(wildcard $(TESTDIR)/*.cpp))))
 
LIB_FNAME = $(BUILDDIR)/frontend.a
TEST_FNAME = $(BUILDDIR)/test_main
 
CXXFLAGS += $(INCDIRS:%=-I%) -std=c++1y -DDEBUG -g
LDFLAGS += $(LIB_FNAME) -lboost_unit_test_framework -pthread
 
.PHONY: all clean
 
all: $(LIB_FNAME) $(TEST_FNAME)
 
$(LIB_FNAME): $(OBJS)
	$(AR) $(ARFLAGS) $@ $^
 
$(TEST_FNAME): $(TESTOBJS) $(LIB_FNAME)
	$(CXX) -o $(TEST_FNAME) $^ $(LDFLAGS)
 
$(BUILDDIR)/%.o:$(SRCDIR)/%.cpp | $(BUILDDIR)
	$(COMPILE.cc) $< $(OUTPUT_OPTION)
	$(COMPILE.cc) -MM -MP -MT $@ $< -o $(BUILDDIR)/$*.d
 
$(BUILDDIR):
	@mkdir $(BUILDDIR)
 
$(BUILDDIR)/%.o:$(TESTDIR)/%.cpp | $(BUILDDIR)
	$(COMPILE.cc) $< $(OUTPUT_OPTION)
	$(COMPILE.cc) -MM -MP -MT $@ $< -o $(BUILDDIR)/$*.d
 
run_test:
	$(TEST_FNAME)
 
clean:
	$(RM) $(BUILDDIR)/*.d $(BUILDDIR)/*.o $(LIB_FNAME) $(TEST_FNAME)