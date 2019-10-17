BUILDDIR = build
INCLUDEDIR = include
SOURCEDIR = src
 
INCLUDES = $(wildcard $(INCLUDEDIR)/*.hpp)
SOURCES = $(wildcard $(SOURCEDIR)/*.cpp)
OBJECTS = $(patsubst $(SOURCEDIR)/%.cpp,$(BUILDDIR)/%.o,$(SOURCES))

EXE_FNAME = $(BUILDDIR)/dvb_proxy
 
CXXFLAGS += $(INCLUDEDIR:%=-I%) -std=c++11 -pthread -DDEBUG -g
LDFLAGS += -pthread

.PHONY: build clean
 
build: $(EXE_FNAME)
 
dir:
	@mkdir -p $(BUILDDIR)

$(EXE_FNAME): $(OBJECTS)
	$(CXX) $(LDFLAGS) $(OBJECTS) -o $@

$(OBJECTS): $(BUILDDIR)/%.o : $(SOURCEDIR)/%.cpp $(INCLUDES)
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	$(RM) $(BUILDDIR)/*.o $(EXE_FNAME)