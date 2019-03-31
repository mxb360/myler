CC = gcc
LD = gcc
CCFLAGS = -g -Wall -fexec-charset=GBK
LDFLAGS = -lwinmm

target = myler.exe
objdir = obj/

src = $(wildcard *.c)
obj = $(addprefix $(objdir), $(patsubst %c, %o, $(src)))
vpath %.d $(objdir)

all: $(target)

$(target): $(obj)
	$(LD) $^ -o $@ $(LDFLAGS)

$(objdir)%.o: %.c
	$(CC) -c $< -o $@ $(CCFLAGS)

$(objdir)%.d: %.c
	@set -e; rm -f $@; $(CC) -MM $<  > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,$(objdir)\1.o $@: ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$

-include $(patsubst %o, %d, $(obj))

clean: 
	rm -f $(objdir)* $(target)
