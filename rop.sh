if ! command -v valgrind &> /dev/null
then
	./rop-fk $1 > a.tmp; cat a.tmp; rm a.tmp
	exit
fi
valgrind ./rop-fk $1 2> /dev/null
