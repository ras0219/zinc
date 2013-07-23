###########################################################
#### Top matter purloined from the Coq proof assistant ####
###########################################################

VERBOSE=
FIND_VCS_CLAUSE:='(' \
  -name '{arch}' -o \
  -name '.svn' -o \
  -name '_darcs' -o \
  -name '.git' -o \
  -name '.bzr' -o \
  -name 'debian' -o \
  -name "$${GIT_DIR}" -o \
  -name '_build' \
')' -prune -o

define find
 $(shell find . $(FIND_VCS_CLAUSE) '(' -name $(1) ')' -print | sed 's|^\./||')
endef

# The SHOW and HIDE variables control whether make will echo complete commands 
# or only abbreviated versions. 
# Quiet mode is ON by default except if VERBOSE=1 option is given to make

SHOW := $(if $(VERBOSE),@true "",@echo "")
HIDE := $(if $(VERBOSE),,@)

SOURCEFILES:=$(call find, '*.cpp')
OBJFILES:=$(SOURCEFILES: .cpp = .o)
CXXFLAGS:=-std=c++11
CC=$(CXX)
CCFLAGS=$(CXXFLAGS)

###########################################################

ALLDEPS:=$(addsuffix .d, $(SOURCEFILES))

all: zinc

-include $(ALLDEPS)

###########################################################
%.cpp.d: %.cpp
	$(SHOW)'CXXDEPS $<'
	$(HIDE)$(CXX) $(CPPFLAGS) $(CXXFLAGS) -MM -MP -MG -MT "$(<:.cpp=.o)" -o $@ $<

%.o: %.cpp
	$(SHOW)'COMPILE $<'
	$(HIDE)$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c -o $@ $<

zinc: $(OBJFILES)
	$(SHOW)'LINK $@'
	$(HIDE)$(CXX) $(LDFLAGS) -o $@ $^
