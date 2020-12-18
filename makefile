BIN_NAME = unito_so

GCC = gcc -Wall

MKD = mkdir -p
CP = cp -f
RM = rm -rf

SRCDIR = src
OBJDIR = obj
BINDIR = bin

#
.PHONY: build
build: init compile

#
.PHONY: compile
compile: $(OBJDIR)/main.o $(OBJDIR)/config.o $(OBJDIR)/msg.o $(OBJDIR)/sem.o $(OBJDIR)/shm.o $(OBJDIR)/student.o
	$(GCC) $^ -o $(BINDIR)/$(BIN_NAME)

#
$(OBJDIR)/main.o: $(SRCDIR)/main.c
	$(GCC) -c $< -o $@
	
#
$(OBJDIR)/config.o: $(SRCDIR)/config/config.c
	$(GCC) -c $< -o $@
	
#
$(OBJDIR)/msg.o: $(SRCDIR)/utils/msg.c
	$(GCC) -c $< -o $@
	
#
$(OBJDIR)/sem.o: $(SRCDIR)/utils/sem.c
	$(GCC) -c $< -o $@
	
#
$(OBJDIR)/shm.o: $(SRCDIR)/utils/shm.c
	$(GCC) -c $< -o $@

#
$(OBJDIR)/student.o: $(SRCDIR)/student/student.c
	$(GCC) -c $< -o $@

#
.PHONY: init
init:
	$(MKD) "./$(OBJDIR)"
	$(MKD) "./$(BINDIR)"
	$(CP) "./opt.conf" "./$(BINDIR)/opt.conf"

#
.PHONY: clean
clean:
	$(RM) $(OBJDIR) $(BINDIR)
