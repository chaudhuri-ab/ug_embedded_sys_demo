#
# Makfile for inflection server examples (registration, access, and clear)
#

all:		example_acc example_reg example_clr

clean:
		rm -f example_acc example_reg example_clr

example_reg:	example_reg.c
		gcc -lnsl -o example_reg example_reg.c

example_acc:	example_acc.c
		gcc -lnsl -o example_acc example_acc.c

example_clr:	example_clr.c
		gcc -lnsl -o example_clr example_clr.c
