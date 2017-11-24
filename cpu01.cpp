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

	void execute();

private:
	void	my_strncpy(char* dest, const char* src, size_t len);
	void	set_cmd_ptrs();
	size_t	get_cmd_num();
#define _CPU_CMDS_
#define CPU_CMD(name, cmdbinlen, argsnum, cmdnum, code)\
	void	name();
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

	int		g_flg;
	int		l_flg;
	int		e_flg;
};

CPU::CPU(const char* bin_str, size_t code_len):
ra_stack_	(NULL),
stack_		(NULL),
cmd_ptrs_	((size_t*) calloc (code_len, sizeof(size_t))),
code_len_	(code_len),
registers_	(NULL),
bin_str_	((char*) calloc (code_len_, sizeof(char))),
bin_offset_	(0),
ip_			(0),
cur_cmd_num_(0),
type1_		(0),
type2_		(0),
arg1_		(0),
arg2_		(0),
g_flg		(0),
l_flg		(0),
e_flg		(0)
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
#define CPU_CMD(name, cmdbinlen, argsnum, cmdnum, code)\
void CPU::name() 							\
{											\
	code									\
}
#include "supercmd.txt"
#undef CPU_CMD
#undef _CPU_CMDS_

size_t CPU::get_cmd_num()
{
	int res = 0;
	for (size_t i = 0; i < sizeof(int); i++)	
		((char*)&res)[i] = bin_str_[bin_offset_ + i];
	bin_offset_ += sizeof(int);
	return res;
}

void CPU::set_cmd_ptrs()
{
	ip_			= 0;
	bin_offset_ = 4;
	while (code_len_ - bin_offset_ >= sizeof(int)) 
	{
		cmd_ptrs_[ip_]	= bin_offset_;
		cur_cmd_num_	= (int) get_cmd_num();
#define _CPU_CMDS_
#define CPU_CMD(name, cmdbinlen, argsnum, cmdnum, code)\
		if (cur_cmd_num_ == cmdnum) bin_offset_ += cmdbinlen; 
#include "supercmd.txt"
#undef CPU_CMD
#undef _CPU_CMDS_
		ip_++;
	}
	ip_ = 0;
	bin_offset_ = 0;
}

void CPU::execute()
{
	set_cmd_ptrs();
 	ip_ = get_cmd_num(); //at the beginning there is the address of main label

	for (size_t i = 1; cmd_ptrs_[i] != 0; i++)
		printf("cmd_ptrs_[%lu]: %lu\n", i, cmd_ptrs_[i]);

	while (1)
	{
		bin_offset_  = cmd_ptrs_[ip_]; //to deal with a cmd that pointed at by ip_
		printf("bin_str[%lu]: %i\n", bin_offset_, bin_str_[bin_offset_]);
		printf("ip_ = %lu; cmd_ptrs_[%lu]: %lu\n", ip_, ip_, cmd_ptrs_[ip_]);
		cur_cmd_num_ = get_cmd_num();
#define _CPU_CMDS_
#define CPU_CMD(name, cmdbinlen, argsnum, cmdnum, code)\
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
			printf("next cmd num:[char %lu] %d\n", bin_offset_, next_one);
		}
		printf("=========================\n");
		getchar();
	}
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
	char* bin_str = (char*) calloc (binflen + 1, sizeof(char));
	if (!bin_str)
	{
		printf("CPU: error finding memory to keep .asm file\n");
		exit(EXIT_FAILURE);
	}

	printf ("binflen: %lu\n", binflen);

	size_t read = 0;
	if ((read = fread(bin_str, sizeof(char), binflen, bin_f)) < binflen)
		printf("warning: read less character than expected:\n\tread: %lu; expected: %lu\n", read, binflen);
	
	bin_str[binflen] = EOF;

	for (size_t i = 0; i < binflen; i++)
		printf("bin_str[%lu]: %i\n", i, bin_str[i]);
	printf("\n\n");

	CPU my_cpu(bin_str, binflen + 1);
	my_cpu.execute();
	free(bin_str);
}
