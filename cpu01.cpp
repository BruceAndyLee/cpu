#include <cstdlib>
#include <cstdio>
#include <cstring>
#include "stack.h"

enum ARGS_TYPES
{
	TYPE_RGSR,
	TYPE_CNST,
	TYPE_ADDR,
	TYPE_LABL
};

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
	void	my_strncpy(char* dest, const char* src, size_t len);
	int		get_cmd_num();
#define _CPU_CMDS_
#define CPU_CMD(name, argsnum, cmdnum, code)\
	int	  name();
#include "supercmd.txt"
#undef CPU_CMD
#undef _CPU_CMDS_
	Stack*	ra_stack_; //return addrs stack
	Stack*  stack_;
	size_t*	cmd_ptrs_; //array keeps addresses of the beginnings of each cmd
	size_t	code_len_;
	double*	registers_;
	int*	flags_;
	char*	bin_str_;
	size_t  bin_offset_;
	size_t	ip_;
	int		cur_cmd_num_;
	int		type1_;
	int		type2_;
	int		arg1_;
	int		arg2_;
};

CPU::CPU(const char* bin_str, size_t code_len):
ra_stack_	(NULL),
stack_		(NULL),
cmd_ptrs_	((size_t*) calloc (code_len, sizeof(size_t))),
code_len_	(code_len),
registers_	(NULL),
flags_		(NULL),
bin_str_	((char*) calloc (code_len_, sizeof(char))),
bin_offset_	(0),
ip_			(0),
cur_cmd_num_(0),
type1_		(0),
type2_		(0),
arg1_		(0),
arg2_		(0)
{
	ra_stack_	= new Stack(10);
	stack_		= new Stack(10);
	if (!bin_str_)
	{
		printf("CPU: c_tor: invalid bin str\n");
		exit(EXIT_FAILURE);
	}
	my_strncpy(bin_str_, bin_str, code_len_);

	if (!cmd_ptrs_)
	{
		printf("CPU: c_tor: cannot find memory for cmd_ptrs_\n");
		exit(EXIT_FAILURE);
	}
	if (!(registers_ = (double*) calloc (10, sizeof(double))))
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
	free(cmd_ptrs_);
	delete ra_stack_;
	delete stack_;
}

void CPU::my_strncpy(char* dest, const char* src, size_t len)
{	
	size_t i = 0;
	for (i = 0; i < len && src[i] != EOF; i++)
		dest[i] = src[i];
	dest[i] = EOF; 
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

int CPU::get_cmd_num()
{
	int res = 0;
	for (size_t i = 0; i < sizeof(int); i++)	
		((char*)&res)[i] = bin_str_[bin_offset_ + i];
	bin_offset_ += sizeof(int);
	return res;
}

int CPU::execute()
{
	//set_cmd_ptrs();
	while(code_len_ - bin_offset_ >= sizeof(int))
	{
		printf("bin_str[%d]: %i\n", bin_offset_, bin_str_[bin_offset_]);
		cmd_ptrs_[ip_]	= bin_offset_;		//user expected to write code defining functions before calling them
		cur_cmd_num_	= get_cmd_num();
#define _CPU_CMDS_
#define CPU_CMD(name, argsnum, cmdnum, code)\
		if (cur_cmd_num_ == cmdnum)			\
			name();
#include "supercmd.txt"
#undef CPU_CMD
#undef _CPU_CMDS_
		if (code_len_ - bin_offset_ >= sizeof(int))
		{
			int next_one = 0;

			for (size_t i = 0; i < sizeof(int); i++)
				((char*)&next_one)[i] = bin_str_[bin_offset_ + i];
			printf("next cmd num: %d\n", next_one);
		}
		printf("=========================\n");
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
	FILE* bin_f = fopen(argv[1], "rb");
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

	printf ("binflen: %lu\n", binflen);
	if (fread(bin_str, sizeof(char), binflen, bin_f) < binflen)
		printf("warning: read less character than expected\n");
	
	bin_str[binflen - 1] = EOF;

	for (int i = 0; i < binflen; i++)
		printf("bin_str[%i]: %i\n", i, bin_str[i]);
	printf("\n\n");

	CPU my_cpu(bin_str, binflen);
	my_cpu.execute();
	free(bin_str);
}
