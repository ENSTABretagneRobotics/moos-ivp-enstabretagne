TARGET:=libxstypes.a
FILES := $(wildcard *.c)
OBJECTS := $(FILES:.c=.c.o)
CFLAGS+= -I../include/xsens

.PHONY: all xstypes
all: xstypes
xstypes: $(TARGET)

-include $(FILES:.c=.d)

%.c.o: %.c
	$(CC) -c $(CFLAGS) $< -o $@
	@$(CC) -MM $(CFLAGS) $< > $*.d
	@mv -f $*.d $*.d.tmp
	@sed -e 's|.*:|$*.c.o:|' < $*.d.tmp > $*.d
	@sed -e 's/.*://' -e 's/\\$$//' < $*.d.tmp | fmt -1 | \
	  sed -e 's/^ *//' -e 's/$$/:/' >> $*.d
	@rm -f $*.d.tmp

$(TARGET): $(OBJECTS)
	ar rcs $@ $^

clean:
	-$(RM) buildversion.h
	-$(RM) *.o *.d
	-$(RM) $(TARGET)
