CPPFLAGS := -Iinclude -MMD -MP
CFLAGS   := -std=c99 -g -Wall -Wextra -Wno-missing-field-initializers
LDLIBS   := -lm

.PHONY: all clean

all: 
	$(CC) $(CPPFLAGS) $(CFLAGS) $(LDLIBS) -o harkka harkka.c

clean:
	@$(RM) -rv harkka harkka.d harkka.dSYM
