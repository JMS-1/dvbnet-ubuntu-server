BUILDDIR = build
INCDIRS = include
SOURCEDIR = src
 
SOURCES = $(wildcard $(SOURCEDIR)/*.cpp)
OBJECTS = $(patsubst $(SOURCEDIR)/%.cpp,$(BUILDDIR)/%.o,$(SOURCES))

EXE_FNAME = $(BUILDDIR)/dvb_proxy
 
CXXFLAGS += $(INCDIRS:%=-I%) -std=c++1y -DDEBUG -g
LDFLAGS += -std=c++1y -pthread

.PHONY: build clean
 
build: $(EXE_FNAME)
 
dir:
	@mkdir -p $(BUILDDIR)

$(EXE_FNAME): $(OBJECTS)
	$(CXX) $(LDFLAGS) $(OBJECTS) -o $@

$(OBJECTS): $(BUILDDIR)/%.o : $(SOURCEDIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	$(RM) $(BUILDDIR)/*.o $(EXE_FNAME)