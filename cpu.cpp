#include <cstdlib>
#include <cstdio>
#include <cstring>
#include "stack.h"

size_t getflen(FILE* f)
{
	if (!f)
	{
		printf("flen: error: invalid descriptor pointer\n");
		return 1;
	}
	long cur = ftell(f);
	size_t len = 0;
	fseek(f, 0, SEEK_END);
	len = (size_t) ftell(f);
	fseek(f, cur, SEEK_SET);
	return len;
}

class CPU
{
public:
	CPU(const char* bin_str, size_t code_len);
	~CPU();

	int execute();

private:
	size_t	get_cmd_num();
#define _CPU_CMDS_
#define CPU_CMD(name, argsnum, cmdnum, code)\
	int	  name();
#include "supercmd.txt"
#undef CPU_CMD
#undef _CPU_CMDS_
	Stack*	ra_stack_; //return addrs stack
	size_t	code_len_;
	int*	registers_;
	int*	flags_;
	char*	bin_str_;
	size_t	ip_;
	size_t	cur_cmd_num_;
	int		type1_;
	int		type2_;
	int		arg1_;
	int		arg2_;
};

CPU::CPU(const char* bin_str, size_t code_len):
ra_stack_	(NULL),
code_len_	(code_len),
registers_	(NULL),
flags_		(NULL),
bin_str_	((char*) calloc (code_len_, sizeof(char))),
ip_			(0),
cur_cmd_num_(0),
type1_		(0),
type2_		(0),
arg1_		(0),
arg2_		(0)
{
	ra_stack_ = new Stack(10);
	if (!bin_str_)
	{
		printf("CPU: c_tor: invalid bin str\n");
		exit(EXIT_FAILURE);
	}
	strncpy(bin_str_, bin_str, code_len_);
	if (!(registers_ = (int*) calloc (10, sizeof(int))))
	{
		printf("CPU: error finding memory to serve registers\n");
		exit(EXIT_FAILURE);
	}
	if (!(flags_ = (int*) calloc (3, sizeof(int))))
	{
		printf("CPU: error finding memory to serve flags");
		exit(EXIT_FAILURE);
	}
}

CPU::~CPU()
{
	free(registers_);
	free(flags_);
	free(bin_str_);
}

#define _CPU_CMDS_
#define CPU_CMD(name, argsnum, cmdnum, code)\
int CPU::name() 							\
{											\
	code									\
	return 0;								\
}
#include "supercmd.txt"
#undef CPU_CMD
#undef _CPU_CMDS_

size_t CPU::get_cmd_num()
{

}

int CPU::execute()
{
	while(ip_ < code_len_)
	{
		cur_cmd_num_ = get_cmd_num();
		printf("got cmd number %lu\n", cur_cmd_num_);
#define _CPU_CMDS_
#define CPU_CMD(name, argsnum, cmdnum, code)\
		if (cur_cmd_num_ == cmdnum)			\
			name();							
#include "supercmd.txt"
#undef CPU_CMD
#undef _CPU_CMDS_
		ip_++;
	}
	return 0;
}

int main(int argc, char* argv[])
{
	if (argc < 2)
	{
		printf("Usage: %s [file.asm]\n", argv[0]);
		exit(EXIT_FAILURE);
	}
	FILE* bin_f = fopen(argv[1], "r");
	if (!bin_f)
	{
		printf("CPU: error reading .asm file\n");
		exit(EXIT_FAILURE);
	}
	
	size_t binflen = getflen(bin_f);
	char* bin_str = (char*) calloc (getflen(bin_f), sizeof(char));
	if (!bin_str)
	{
		printf("CPU: error finding memory to keep .asm file\n");
		exit(EXIT_FAILURE);
	}

	if (fread(bin_str, sizeof(int), binflen, bin_f) < binflen)
		printf("warning: read less character than expected\n");
		
	CPU my_cpu(bin_str, binflen);
	my_cpu.execute();
	free(bin_str);
}
