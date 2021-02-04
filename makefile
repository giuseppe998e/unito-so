TARGET = unito_so

CC = gcc
CFLAGS = -Wall

ECHO = echo
MKD = mkdir
CP = cp
RM = rm

SRCDIR = src
OBJDIR = obj
BINDIR = bin

SRCS = main.o config/config.o utils/msg.o utils/sem.o utils/shm.o student/student.o

#
.PHONY: build
build: prepare $(BINDIR)/$(TARGET)
	@$(ECHO) "Done!"

#
.PHONY: run
run:
	@./$(BINDIR)/$(TARGET)

#
.PHONY: prepare
prepare:
	@$(MKD) -p "./$(OBJDIR)"
	@$(MKD) -p "./$(BINDIR)"
	@$(CP) -f "./opt.conf" "./$(BINDIR)/opt.conf"

#
.PHONY: clean
clean:
	@$(RM) -rf $(OBJDIR) $(BINDIR)

#
$(BINDIR)/$(TARGET): $(addprefix $(OBJDIR)/, $(SRCS))
	@$(CC) $(CFLAGS) $^ -o $@

#
$(OBJDIR)/%.o: $(SRCDIR)/%.c
	@$(MKD) -p $(dir $@)
	@$(CC) $(CFLAGS) -c $< -o $@
