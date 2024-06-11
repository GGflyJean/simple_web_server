# Makefile
CC = gcc
CFLAGS = -Wall -g

# Dir define
SRCDIR = src
OBJDIR = objs
BINDIR = bin

SRCS = $(wildcard $(SRCDIR)/*.c)
OBJS = $(SRCS:$(SRCDIR)/%.c=$(OBJDIR)/%.o)
TARGET = $(BINDIR)/sw-server

# Compile
all: ${TARGET}

# Link
$(TARGET): $(OBJS)
	@mkdir -p $(BINDIR)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)

# Target Create
$(OBJDIR)/%.o: $(SRCDIR)/%.c
	@mkdir -p $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Clean
clean:
	rm -f $(OBJDIR)/*.o $(TARGET)