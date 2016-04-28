########## INCLUDE the variables defined in Rules.make
include ../../../Rules.mk
########## DON'T REMOVE THIS !!!
########viariable define
TOPDIR=.

#targets
LIBTARGET = libwebbookticket.so

#objects
OBJS = WebBookTkt.o  HttpParser.o tkt.o AnaXC.o AnaDH.o AnaHH.o AnaNH.o AnaDaBa.o Ana12308.o Ana966800.o AnaGH.o AnaCT.o
INCLUDE = -I. -I$(ANALYZER_DIR)/include
CXXFLAGS += -shared $(INCLUDE)

.cpp.o:
	$(CXX) $(CXXFLAGS) -o $@ -c $<

.PHONY:all

all:lib exec

lib:$(LIBTARGET)
$(LIBTARGET):$(OBJS)
	$(CXX) $(CXXFLAGS) -o $(LIBTARGET) $(OBJS) $(LDFLAGS)
	mkdir -p $(FIRM_LIB_DIR)
	cd $(TOPDIR) && $(CP) $(LIBTARGET) $(FIRM_LIB_DIR)
	@echo -en $(SUCCESS);
	@echo "make lib success"
	@echo -en $(NORMAL);

exec:
	@echo -en $(SUCCESS);
	@echo "no exec need to compile";
	@echo -en $(NORMAL);

install: $(LIBTARGET)
	@mkdir -p $(FIRM_LIB_DIR)
	@cd $(TOPDIR) && $(CP) $(LIBTARGET) $(FIRM_LIB_DIR)
	@echo -en $(SUCCESS);
	@echo "install successfully";
	@echo -en $(NORMAL);

#########compile execs and upzip system
clean:
	@rm -fr $(LIBTARGET) $(OBJS)
	@echo -en $(SUCCESS);
	@echo "make clean successfully"
	@echo -en $(NORMAL);


######explain the variable
explain:
	@echo "the following modules will be make:"
	@echo "    exec_module: $(exec_module)"
	@echo "    test_module: $(test_module)"
	@echo "    sys_module: $(sys_module)"
	@echo "    all_module: $(all_module)"
	@echo "   DESTINATION FIRM_DIR:    ${FIRM_DIR}"
	@echo "   FIRM_SBIN_DIR:           ${FIRM_SBIN_DIR}"
	@echo "   FIRM_BIN_DIR:            ${FIRM_BIN_DIR}"
	@echo "   FIRM_USR_SBIN_DIR:       ${FIRM_USR_SBIN_DIR}"
	@echo "   FIRM_USR_BIN_DIR:        ${FIRM_USR_BIN_DIR}"
	@echo "   FIRM_CONFIG_DIR:         ${FIRM_CONFIG_DIR}"
	@echo "   FIRM_LIB_DIR:            ${FIRM_LIB_DIR}"
	@echo "   FIRM_MODULES_DIR:        ${FIRM_MODULES_DIR}"
	@echo "   CXXFLAGS:                ${CXXFLAGS}"
	@echo "   ARCHFLAGS:                ${ARCHFLAGS}"

