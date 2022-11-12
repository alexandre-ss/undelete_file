undelete: 
	@gcc -o undelete -Wextra main.c
	@./undelete
	@rm -rf undelete