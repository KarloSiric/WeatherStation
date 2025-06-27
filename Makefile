CC = gcc
CFLAGS = -Wall -Wextra -Wpedantic -std=c99 -O2 -g -Iinclude
LDFLAGS = -lcurl -lcjson

SRCDIR = src
INCDIR = include
OBJDIR = obj

SOURCES = $(wildcard $(SRCDIR)/*.c)
OBJECTS = $(patsubst $(SRCDIR)/%.c,$(OBJDIR)/%.o,$(SOURCES))
TARGET = bin/weather_station

all: $(TARGET)

$(TARGET): $(OBJECTS)
	@mkdir -p bin
	$(CC) $(OBJECTS) -o $@ $(LDFLAGS)

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	@mkdir -p $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJDIR) bin

# Add test target for your test file
test: $(OBJDIR)/weather_api.o
	@mkdir -p bin
	$(CC) $(CFLAGS) test_weather.c $(OBJDIR)/weather_api.o -o bin/test_weather $(LDFLAGS)

.PHONY: all clean test
