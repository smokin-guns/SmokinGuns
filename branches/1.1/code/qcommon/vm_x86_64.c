/*
===========================================================================
Copyright (C) 1999-2005 Id Software, Inc.
Copyright (C) 2005 Ludwig Nussel <ludwig.nussel@web.de>

This file is part of Quake III Arena source code.

Quake III Arena source code is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

Quake III Arena source code is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Quake III Arena source code; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/
// vm_x86_64.c -- load time compiler and execution environment for x86-64

#include "vm_local.h"
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <time.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <stdarg.h>

#include <inttypes.h>

#ifdef __WIN64__
	#include <windows.h>
	#define CROSSCALL __attribute__ ((sysv_abi))//fool the vm we're SYSV ABI
	//#define __USE_MINGW_ANSI_STDIO 1 //very slow - avoid if possible
#else
	#include <sys/mman.h>
	#include <sys/wait.h>
	#define VM_X86_64_MMAP
	#define CROSSCALL
#endif

//#define DEBUG_VM

#ifdef DEBUG_VM
#define Dfprintf(fd, args...) fprintf(fd, ##args)
static FILE* qdasmout;
#else
#define Dfprintf(args...)
#endif

#define VM_FREEBUFFERS(vm) do {assembler_init(0); VM_Destroy_Compiled(vm);} while(0)

void assembler_set_output(char* buf);
size_t assembler_get_code_size(void);
void assembler_init(int pass);
void assemble_line(const char* input, size_t len);

static void VM_Destroy_Compiled(vm_t* self);

/*
 
  |=====================|
  ^       dataMask      ^- programStack rdi
  |
  +- r8

  eax	scratch
  ebx	scratch
  ecx	scratch (required for shifts)
  edx	scratch (required for divisions)
  rsi	stack pointer (opStack)
  rdi	program frame pointer (programStack)
  r8    pointer data (vm->dataBase)
  r10   start of generated code
*/


static int64_t CROSSCALL callAsmCall(int64_t callProgramStack, int64_t callSyscallNum)
{
	vm_t *savedVM;
	int64_t ret = 0x77;
	int64_t args[11];
//	int iargs[11];
	int i;

//	Dfprintf(stderr, "callAsmCall(%ld, %ld)\n", callProgramStack, callSyscallNum);
//	Com_Printf("-> callAsmCall %s, level %d, num %ld\n", currentVM->name, currentVM->callLevel, callSyscallNum);

	savedVM = currentVM;

	// save the stack to allow recursive VM entry
	currentVM->programStack = callProgramStack - 4;

	args[0] = callSyscallNum;
//	iargs[0] = callSyscallNum;
	for(i = 0; i < 10; ++i)
	{
//		iargs[i+1] = *(int *)((byte *)currentVM->dataBase + callProgramStack + 8 + 4*i);
		args[i+1] = *(int *)((byte *)currentVM->dataBase + callProgramStack + 8 + 4*i);
	}
	ret = currentVM->systemCall(args);

 	currentVM = savedVM;
//	Com_Printf("<- callAsmCall %s, level %d, num %ld\n", currentVM->name, currentVM->callLevel, callSyscallNum);

	return ret;
}

#ifdef DEBUG_VM
static char	*opnames[256] = {
	"OP_UNDEF", 

	"OP_IGNORE", 

	"OP_BREAK",

	"OP_ENTER",
	"OP_LEAVE",
	"OP_CALL",
	"OP_PUSH",
	"OP_POP",

	"OP_CONST",

	"OP_LOCAL",

	"OP_JUMP",

	//-------------------

	"OP_EQ",
	"OP_NE",

	"OP_LTI",
	"OP_LEI",
	"OP_GTI",
	"OP_GEI",

	"OP_LTU",
	"OP_LEU",
	"OP_GTU",
	"OP_GEU",

	"OP_EQF",
	"OP_NEF",

	"OP_LTF",
	"OP_LEF",
	"OP_GTF",
	"OP_GEF",

	//-------------------

	"OP_LOAD1",
	"OP_LOAD2",
	"OP_LOAD4",
	"OP_STORE1",
	"OP_STORE2",
	"OP_STORE4",
	"OP_ARG",

	"OP_BLOCK_COPY",

	//-------------------

	"OP_SEX8",
	"OP_SEX16",

	"OP_NEGI",
	"OP_ADD",
	"OP_SUB",
	"OP_DIVI",
	"OP_DIVU",
	"OP_MODI",
	"OP_MODU",
	"OP_MULI",
	"OP_MULU",

	"OP_BAND",
	"OP_BOR",
	"OP_BXOR",
	"OP_BCOM",

	"OP_LSH",
	"OP_RSHI",
	"OP_RSHU",

	"OP_NEGF",
	"OP_ADDF",
	"OP_SUBF",
	"OP_DIVF",
	"OP_MULF",

	"OP_CVIF",
	"OP_CVFI"
};
#endif // DEBUG_VM

static unsigned char op_argsize[256] = 
{
	[OP_ENTER]      = 4,
	[OP_LEAVE]      = 4,
	[OP_CONST]      = 4,
	[OP_LOCAL]      = 4,
	[OP_EQ]         = 4,
	[OP_NE]         = 4,
	[OP_LTI]        = 4,
	[OP_LEI]        = 4,
	[OP_GTI]        = 4,
	[OP_GEI]        = 4,
	[OP_LTU]        = 4,
	[OP_LEU]        = 4,
	[OP_GTU]        = 4,
	[OP_GEU]        = 4,
	[OP_EQF]        = 4,
	[OP_NEF]        = 4,
	[OP_LTF]        = 4,
	[OP_LEF]        = 4,
	[OP_GTF]        = 4,
	[OP_GEF]        = 4,
	[OP_ARG]        = 1,
	[OP_BLOCK_COPY] = 4,
};

void emit(const char* fmt, ...)
{
	va_list ap;
	char line[4096];
	va_start(ap, fmt);
	Q_vsnprintf(line, sizeof(line), fmt, ap);
	va_end(ap);
	assemble_line(line, strlen(line));
}

#define CHECK_INSTR_REG(reg) \
	emit("cmpl $%u, %%"#reg, header->instructionCount); \
	emit("jb jmp_ok_i_%08x", instruction); \
	emit("movq $%"PRIu64", %%rax", (uint64_t)jmpviolation); \
	emit("callq *%%rax"); \
	emit("jmp_ok_i_%08x:", instruction);

#define PREPARE_JMP(reg) \
	CHECK_INSTR_REG(reg) \
	emit("movq $%"PRIu64", %%rbx", (uint64_t)vm->instructionPointers); \
	emit("movl (%%rbx, %%rax, 4), %%eax"); \
	emit("addq %%r10, %%rax");

#define CHECK_INSTR(nr) \
	do { if(nr < 0 || nr >= header->instructionCount) { \
		VM_FREEBUFFERS(vm); \
		Com_Error( ERR_DROP, \
			"%s: jump target 0x%x out of range at offset %d", __func__, nr, pc ); \
	} } while(0)

#define JMPIARG \
	CHECK_INSTR(iarg); \
	emit("movq $%"PRIu64", %%rax", vm->codeBase+vm->instructionPointers[iarg]); \
	emit("jmpq *%%rax");
 
#define CONST_OPTIMIZE
#ifdef CONST_OPTIMIZE
#define MAYBE_EMIT_CONST() \
	if (got_const) \
	{ \
		got_const = 0; \
		vm->instructionPointers[instruction-1] = assembler_get_code_size(); \
		emit("addq $4, %%rsi"); \
		emit("movl $%d, 0(%%rsi)", const_value); \
	}
#else
#define MAYBE_EMIT_CONST()
#endif

// integer compare and jump
#define IJ(op) \
	MAYBE_EMIT_CONST(); \
	emit("subq $8, %%rsi"); \
	emit("movl 4(%%rsi), %%eax"); \
	emit("cmpl 8(%%rsi), %%eax"); \
	emit(op " i_%08x", instruction+1); \
	JMPIARG \
	neednilabel = 1;

#ifdef USE_X87
#define FJ(bits, op) \
	MAYBE_EMIT_CONST(); \
	emit("subq $8, %%rsi");\
	emit("flds 4(%%rsi)");\
	emit("fcomps 8(%%rsi)");\
	emit("fnstsw %%ax");\
	emit("testb $" #bits ", %%ah");\
	emit(op " i_%08x", instruction+1);\
	JMPIARG \
	neednilabel = 1;
#define XJ(x)
#else
#define FJ(x, y)
#define XJ(op) \
	MAYBE_EMIT_CONST(); \
	emit("subq $8, %%rsi");\
	emit("movss 4(%%rsi), %%xmm0");\
	emit("ucomiss 8(%%rsi), %%xmm0");\
	emit("jp i_%08x", instruction+1);\
	emit(op " i_%08x", instruction+1);\
	JMPIARG \
	neednilabel = 1;
#endif

#define SIMPLE(op) \
	MAYBE_EMIT_CONST(); \
	emit("subq $4, %%rsi"); \
	emit("movl 4(%%rsi), %%eax"); \
	emit(op " %%eax, 0(%%rsi)");

#ifdef USE_X87
#define FSIMPLE(op) \
	MAYBE_EMIT_CONST(); \
	emit("subq $4, %%rsi"); \
	emit("flds 0(%%rsi)"); \
	emit(op " 4(%%rsi)"); \
	emit("fstps 0(%%rsi)");
#define XSIMPLE(op)
#else
#define FSIMPLE(op)
#define XSIMPLE(op) \
	MAYBE_EMIT_CONST(); \
	emit("subq $4, %%rsi"); \
	emit("movss 0(%%rsi), %%xmm0"); \
	emit(op " 4(%%rsi), %%xmm0"); \
	emit("movss %%xmm0, 0(%%rsi)");
#endif

#define SHIFT(op) \
	MAYBE_EMIT_CONST(); \
	emit("subq $4, %%rsi"); \
	emit("movl 4(%%rsi), %%ecx"); \
	emit("movl 0(%%rsi), %%eax"); \
	emit(op " %%cl, %%eax"); \
	emit("movl %%eax, 0(%%rsi)");

#ifdef DEBUG_VM
#define RANGECHECK(reg, bytes) \
	emit("movl %%" #reg ", %%ecx"); \
	emit("andl $0x%x, %%ecx", vm->dataMask &~(bytes-1)); \
	emit("cmpl %%" #reg ", %%ecx"); \
	emit("jz rc_ok_i_%08x", instruction); \
	emit("movq $%"PRIu64", %%rax", (uint64_t)memviolation); \
	emit("callq *%%rax"); \
	emit("rc_ok_i_%08x:", instruction);
#elif 1
// check is too expensive, so just confine memory access
#define RANGECHECK(reg, bytes) \
	emit("andl $0x%x, %%" #reg, vm->dataMask &~(bytes-1));
#else
#define RANGECHECK(reg, bytes)
#endif

#ifdef DEBUG_VM
#define NOTIMPL(x) \
	do { Com_Error(ERR_DROP, "instruction not implemented: %s\n", opnames[x]); } while(0)
#else
#define NOTIMPL(x) \
	do { Com_Printf(S_COLOR_RED "instruction not implemented: %x\n", x); vm->compiled = qfalse; return; } while(0)
#endif

static void* getentrypoint(vm_t* vm)
{
       return vm->codeBase;
}

static void CROSSCALL block_copy_vm(unsigned dest, unsigned src, unsigned count)
{
	unsigned dataMask = currentVM->dataMask;

	if ((dest & dataMask) != dest
	|| (src & dataMask) != src
	|| ((dest+count) & dataMask) != dest + count
	|| ((src+count) & dataMask) != src + count)
	{
		Com_Error(ERR_DROP, "OP_BLOCK_COPY out of range!\n");
	}

	memcpy(currentVM->dataBase+dest, currentVM->dataBase+src, count);
}

static void CROSSCALL eop(void)
{
	Com_Error(ERR_DROP, "end of program reached without return!\n");
	exit(1);
}

static void CROSSCALL jmpviolation(void)
{
	Com_Error(ERR_DROP, "program tried to execute code outside VM\n");
	exit(1);
}

#ifdef DEBUG_VM
static void CROSSCALL memviolation(void)
{
	Com_Error(ERR_DROP, "program tried to access memory outside VM\n");
	exit(1);
}
#endif

/*
=================
VM_Compile
=================
*/
void VM_Compile( vm_t *vm, vmHeader_t *header ) {
	unsigned char op;
	int pc;
	unsigned instruction;
	char* code;
	unsigned iarg = 0;
	unsigned char barg = 0;
	int neednilabel = 0;
	struct timeval tvstart =  {0, 0};
#ifdef DEBUG_VM
	char fn_d[MAX_QPATH]; // disassembled
#endif

	int pass;
	size_t compiledOfs = 0;

	// const optimization
	unsigned got_const = 0, const_value = 0;
	
	vm->codeBase = NULL;

	gettimeofday(&tvstart, NULL);

	for (pass = 0; pass < 2; ++pass) {

	if(pass)
	{
		compiledOfs = assembler_get_code_size();
		vm->codeLength = compiledOfs;

		#ifdef VM_X86_64_MMAP
			vm->codeBase = mmap(NULL, compiledOfs, PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS, -1, 0);
			if(vm->codeBase == MAP_FAILED)
				Com_Error(ERR_FATAL, "VM_CompileX86_64: can't mmap memory");
		#elif __WIN64__
			// allocate memory with write permissions under windows.
			vm->codeBase = VirtualAlloc(NULL, compiledOfs, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
			if(!vm->codeBase)
				Com_Error(ERR_FATAL, "VM_CompileX86_64: VirtualAlloc failed");
		#else
			vm->codeBase = malloc(compiledOfs);
			if(!vm_codeBase)
				Com_Error(ERR_FATAL, "VM_CompileX86_64: Failed to allocate memory");
		#endif

		assembler_set_output((char*)vm->codeBase);
	}

	assembler_init(pass);

#ifdef DEBUG_VM
	strcpy(fn_d,vm->name);
	strcat(fn_d, ".qdasm");

	qdasmout = fopen(fn_d, "w");
#endif

	// translate all instructions
	pc = 0;
	code = (char *)header + header->codeOffset;

	for ( instruction = 0; instruction < header->instructionCount; ++instruction )
	{
		op = code[ pc ];
		++pc;

		vm->instructionPointers[instruction] = assembler_get_code_size();

		/* store current instruction number in r15 for debugging */
#if DEBUG_VM0
		emit("nop");
		emit("movq $%d, %%r15", instruction);
		emit("nop");
#endif

		if(op_argsize[op] == 4)
		{
			iarg = *(int*)(code+pc);
			pc += 4;
			Dfprintf(qdasmout, "%s %8u\n", opnames[op], iarg);
		}
		else if(op_argsize[op] == 1)
		{
			barg = code[pc++];
			Dfprintf(qdasmout, "%s %8hu\n", opnames[op], barg);
		}
		else
		{
			Dfprintf(qdasmout, "%s\n", opnames[op]);
		}

		if(neednilabel)
		{
			emit("i_%08x:", instruction);
			neednilabel = 0;
		}

		switch ( op )
		{
			case OP_UNDEF:
				NOTIMPL(op);
				break;
			case OP_IGNORE:
				MAYBE_EMIT_CONST();
				emit("nop");
				break;
			case OP_BREAK:
				MAYBE_EMIT_CONST();
				emit("int3");
				break;
			case OP_ENTER:
				MAYBE_EMIT_CONST();
				emit("subl $%d, %%edi", iarg);
				break;
			case OP_LEAVE:
				MAYBE_EMIT_CONST();
				emit("addl $%d, %%edi", iarg);          // get rid of stack frame
				emit("ret");
				break;
			case OP_CALL:
				RANGECHECK(edi, 4);
				emit("movl $%d, 0(%%r8, %%rdi, 1)", instruction+1);  // save next instruction
				if(got_const)
				{
					if ((int)const_value < 0)
						goto emit_do_syscall;

					CHECK_INSTR(const_value);
					emit("movq $%"PRIu64", %%rax", vm->codeBase+vm->instructionPointers[const_value]);
					emit("callq *%%rax");
					got_const = 0;
					break;
				}
				else
				{
					MAYBE_EMIT_CONST();
					emit("movl 0(%%rsi), %%eax");  // get instr from stack
					emit("subq $4, %%rsi");

					emit("orl %%eax, %%eax");
					emit("jl callSyscall%d", instruction);

					PREPARE_JMP(eax);
					emit("callq *%%rax");

					emit("jmp i_%08x", instruction+1);
					emit("callSyscall%d:", instruction);
				}
emit_do_syscall:
//				emit("fnsave 4(%%rsi)");
				emit("push %%rsi");
				emit("push %%rdi");
				emit("push %%r8");
				emit("push %%r9");
				emit("push %%r10");
				emit("movq %%rsp, %%rbx"); // we need to align the stack pointer
				emit("subq $8, %%rbx");    //   |
				emit("andq $127, %%rbx");  //   |
				emit("subq %%rbx, %%rsp"); // <-+
				emit("push %%rbx");
				if(got_const) {
					got_const = 0;
					emit("movq $%u, %%rsi", -1-const_value); // second argument in rsi
				} else {
					emit("negl %%eax");        // convert to actual number
					emit("decl %%eax");
					// first argument already in rdi
					emit("movq %%rax, %%rsi"); // second argument in rsi
				}
				emit("movq $%"PRIu64", %%rax", (uint64_t)callAsmCall);
				emit("callq *%%rax");
				emit("pop %%rbx");
				emit("addq %%rbx, %%rsp");
				emit("pop %%r10");
				emit("pop %%r9");
				emit("pop %%r8");
				emit("pop %%rdi");
				emit("pop %%rsi");
//				emit("frstor 4(%%rsi)");
				emit("addq $4, %%rsi");
				emit("movl %%eax, (%%rsi)"); // store return value
				neednilabel = 1;
				break;
			case OP_PUSH:
				MAYBE_EMIT_CONST();
				emit("addq $4, %%rsi");
				break;
			case OP_POP:
				MAYBE_EMIT_CONST();
				emit("subq $4, %%rsi");
				break;
			case OP_CONST:
				MAYBE_EMIT_CONST();
#ifdef CONST_OPTIMIZE
				got_const = 1;
				const_value = iarg;
#else
				emit("addq $4, %%rsi");
				emit("movl $%d, 0(%%rsi)", iarg);
#endif
				break;
			case OP_LOCAL:
				MAYBE_EMIT_CONST();
				emit("movl %%edi, %%ebx");
				emit("addl $%d,%%ebx", iarg);
				emit("addq $4, %%rsi");
				emit("movl %%ebx, 0(%%rsi)");
				break;
			case OP_JUMP:
				if(got_const) {
					iarg = const_value;
					got_const = 0;
					JMPIARG;
				} else {
					emit("movl 0(%%rsi), %%eax"); // get instr from stack
					emit("subq $4, %%rsi");

					PREPARE_JMP(eax);
					emit("jmp *%%rax");
				}
				break;
			case OP_EQ:
				IJ("jne");
				break;
			case OP_NE:
				IJ("je");
				break;
			case OP_LTI:
				IJ("jnl");
				break;
			case OP_LEI:
				IJ("jnle");
				break;
			case OP_GTI:
				IJ("jng");
				break;
			case OP_GEI:
				IJ("jnge");
				break;
			case OP_LTU:
				IJ("jnb");
				break;
			case OP_LEU:
				IJ("jnbe");
				break;
			case OP_GTU:
				IJ("jna");
				break;
			case OP_GEU:
				IJ("jnae");
				break;
			case OP_EQF:
				FJ(0x40, "jz");
				XJ("jnz");
				break;
			case OP_NEF:
				FJ(0x40, "jnz");
#ifndef USE_X87
				MAYBE_EMIT_CONST();
				emit("subq $8, %%rsi");
				emit("movss 4(%%rsi), %%xmm0");
				emit("ucomiss 8(%%rsi), %%xmm0");
				emit("jp dojump_i_%08x", instruction);
				emit("jz i_%08x", instruction+1);
				emit("dojump_i_%08x:", instruction);
				JMPIARG
				neednilabel = 1;
#endif
				break;
			case OP_LTF:
				FJ(0x01, "jz");
				XJ("jnc");
				break;
			case OP_LEF:
				FJ(0x41, "jz");
				XJ("ja");
				break;
			case OP_GTF:
				FJ(0x41, "jnz");
				XJ("jbe");
				break;
			case OP_GEF:
				FJ(0x01, "jnz");
				XJ("jb");
				break;
			case OP_LOAD1:
				MAYBE_EMIT_CONST();
				emit("movl 0(%%rsi), %%eax"); // get value from stack
				RANGECHECK(eax, 1);
				emit("movb 0(%%r8, %%rax, 1), %%al"); // deref into eax
				emit("andq $255, %%rax");
				emit("movl %%eax, 0(%%rsi)"); // store on stack
				break;
			case OP_LOAD2:
				MAYBE_EMIT_CONST();
				emit("movl 0(%%rsi), %%eax"); // get value from stack
				RANGECHECK(eax, 2);
				emit("movw 0(%%r8, %%rax, 1), %%ax"); // deref into eax
				emit("movl %%eax, 0(%%rsi)"); // store on stack
				break;
			case OP_LOAD4:
				MAYBE_EMIT_CONST();
				emit("movl 0(%%rsi), %%eax"); // get value from stack
				RANGECHECK(eax, 4); // not a pointer!?
				emit("movl 0(%%r8, %%rax, 1), %%eax"); // deref into eax
				emit("movl %%eax, 0(%%rsi)"); // store on stack
				break;
			case OP_STORE1:
				MAYBE_EMIT_CONST();
				emit("movl 0(%%rsi), %%eax"); // get value from stack
				emit("andq $255, %%rax");
				emit("movl -4(%%rsi), %%ebx"); // get pointer from stack
				RANGECHECK(ebx, 1);
				emit("movb %%al, 0(%%r8, %%rbx, 1)"); // store in memory
				emit("subq $8, %%rsi");
				break;
			case OP_STORE2:
				MAYBE_EMIT_CONST();
				emit("movl 0(%%rsi), %%eax"); // get value from stack
				emit("movl -4(%%rsi), %%ebx"); // get pointer from stack
				RANGECHECK(ebx, 2);
				emit("movw %%ax, 0(%%r8, %%rbx, 1)"); // store in memory
				emit("subq $8, %%rsi");
				break;
			case OP_STORE4:
				MAYBE_EMIT_CONST();
				emit("movl -4(%%rsi), %%ebx"); // get pointer from stack
				RANGECHECK(ebx, 4);
				emit("movl 0(%%rsi), %%ecx"); // get value from stack
				emit("movl %%ecx, 0(%%r8, %%rbx, 1)"); // store in memory
				emit("subq $8, %%rsi");
				break;
			case OP_ARG:
				MAYBE_EMIT_CONST();
				emit("subq $4, %%rsi");
				emit("movl 4(%%rsi), %%eax"); // get value from stack
				emit("movl $0x%hx, %%ebx", barg);
				emit("addl %%edi, %%ebx");
				RANGECHECK(ebx, 4);
				emit("movl %%eax, 0(%%r8,%%rbx, 1)"); // store in args space
				break;
			case OP_BLOCK_COPY:

				MAYBE_EMIT_CONST();
				emit("subq $8, %%rsi");
				emit("push %%rsi");
				emit("push %%rdi");
				emit("push %%r8");
				emit("push %%r9");
				emit("push %%r10");
				emit("movq %%rsp, %%rbx"); // we need to align the stack pointer
				emit("subq $8, %%rbx");    //   |
				emit("andq $127, %%rbx");  //   |
				emit("subq %%rbx, %%rsp"); // <-+
				emit("push %%rbx");
				emit("movl 4(%%rsi), %%edi");  // 1st argument dest
				emit("movl 8(%%rsi), %%esi");  // 2nd argument src
				emit("movl $%d, %%edx", iarg); // 3rd argument count
				emit("movq $%"PRIu64", %%rax", (uint64_t)block_copy_vm);
				emit("callq *%%rax");
				emit("pop %%rbx");
				emit("addq %%rbx, %%rsp");
				emit("pop %%r10");
				emit("pop %%r9");
				emit("pop %%r8");
				emit("pop %%rdi");
				emit("pop %%rsi");

				break;
			case OP_SEX8:
				MAYBE_EMIT_CONST();
				emit("movw 0(%%rsi), %%ax");
				emit("andq $255, %%rax");
				emit("cbw");
				emit("cwde");
				emit("movl %%eax, 0(%%rsi)");
				break;
			case OP_SEX16:
				MAYBE_EMIT_CONST();
				emit("movw 0(%%rsi), %%ax");
				emit("cwde");
				emit("movl %%eax, 0(%%rsi)");
				break;
			case OP_NEGI:
				MAYBE_EMIT_CONST();
				emit("negl 0(%%rsi)");
				break;
			case OP_ADD:
				SIMPLE("addl");
				break;
			case OP_SUB:
				SIMPLE("subl");
				break;
			case OP_DIVI:
				MAYBE_EMIT_CONST();
				emit("subq $4, %%rsi");
				emit("movl 0(%%rsi), %%eax");
				emit("cdq");
				emit("idivl 4(%%rsi)");
				emit("movl %%eax, 0(%%rsi)");
				break;
			case OP_DIVU:
				MAYBE_EMIT_CONST();
				emit("subq $4, %%rsi");
				emit("movl 0(%%rsi), %%eax");
				emit("xorq %%rdx, %%rdx");
				emit("divl 4(%%rsi)");
				emit("movl %%eax, 0(%%rsi)");
				break;
			case OP_MODI:
				MAYBE_EMIT_CONST();
				emit("subq $4, %%rsi");
				emit("movl 0(%%rsi), %%eax");
				emit("xorl %%edx, %%edx");
				emit("cdq");
				emit("idivl 4(%%rsi)");
				emit("movl %%edx, 0(%%rsi)");
				break;
			case OP_MODU:
				MAYBE_EMIT_CONST();
				emit("subq $4, %%rsi");
				emit("movl 0(%%rsi), %%eax");
				emit("xorl %%edx, %%edx");
				emit("divl 4(%%rsi)");
				emit("movl %%edx, 0(%%rsi)");
				break;
			case OP_MULI:
				MAYBE_EMIT_CONST();
				emit("subq $4, %%rsi");
				emit("movl 0(%%rsi), %%eax");
				emit("imull 4(%%rsi)");
				emit("movl %%eax, 0(%%rsi)");
				break;
			case OP_MULU:
				MAYBE_EMIT_CONST();
				emit("subq $4, %%rsi");
				emit("movl 0(%%rsi), %%eax");
				emit("mull 4(%%rsi)");
				emit("movl %%eax, 0(%%rsi)");
				break;
			case OP_BAND:
				SIMPLE("andl");
				break;
			case OP_BOR:
				SIMPLE("orl");
				break;
			case OP_BXOR:
				SIMPLE("xorl");
				break;
			case OP_BCOM:
				MAYBE_EMIT_CONST();
				emit("notl 0(%%rsi)");
				break;
			case OP_LSH:
				SHIFT("shl");
				break;
			case OP_RSHI:
				SHIFT("sarl");
				break;
			case OP_RSHU:
				SHIFT("shrl");
				break;
			case OP_NEGF:
				MAYBE_EMIT_CONST();
#ifdef USE_X87
				emit("flds 0(%%rsi)");
				emit("fchs");
				emit("fstps 0(%%rsi)");
#else
				emit("movl $0x80000000, %%eax");
				emit("xorl %%eax, 0(%%rsi)");
#endif
				break;
			case OP_ADDF:
				FSIMPLE("fadds");
				XSIMPLE("addss");
				break;
			case OP_SUBF:
				FSIMPLE("fsubs");
				XSIMPLE("subss");
				break;
			case OP_DIVF:
				FSIMPLE("fdivs");
				XSIMPLE("divss");
				break;
			case OP_MULF:
				FSIMPLE("fmuls");
				XSIMPLE("mulss");
				break;
			case OP_CVIF:
				MAYBE_EMIT_CONST();
#ifdef USE_X87
				emit("filds 0(%%rsi)");
				emit("fstps 0(%%rsi)");
#else
				emit("movl 0(%%rsi), %%eax");
				emit("cvtsi2ss %%eax, %%xmm0");
				emit("movss %%xmm0, 0(%%rsi)");
#endif
				break;
			case OP_CVFI:
				MAYBE_EMIT_CONST();
#ifdef USE_X87
				emit("flds 0(%%rsi)");
				emit("fnstcw 4(%%rsi)");
				emit("movw $0x0F7F, 8(%%rsi)"); // round toward zero
				emit("fldcw 8(%%rsi)");
				emit("fistpl 0(%%rsi)");
				emit("fldcw 4(%%rsi)");
#else
				emit("movss 0(%%rsi), %%xmm0");
				emit("cvttss2si %%xmm0, %%eax");
				emit("movl %%eax, 0(%%rsi)");
#endif
				break;
			default:
				NOTIMPL(op);
				break;
		}


	}

	if(got_const)
	{
		VM_FREEBUFFERS(vm);
		Com_Error(ERR_DROP, "leftover const\n");
	}

	emit("movq $%"PRIu64", %%rax", (uint64_t)eop);
	emit("callq *%%rax");

	} // pass loop

	assembler_init(0);

	#ifdef VM_X86_64_MMAP
		if(mprotect(vm->codeBase, compiledOfs, PROT_READ|PROT_EXEC))
			Com_Error(ERR_FATAL, "VM_CompileX86_64: mprotect failed");
	#elif __WIN64__
		{
			DWORD oldProtect = 0;
			
			// remove write permissions; give exec permision
			if(!VirtualProtect(vm->codeBase, compiledOfs, PAGE_EXECUTE_READ, &oldProtect))
				Com_Error(ERR_FATAL, "VM_CompileX86_64: VirtualProtect failed");
		}
	#endif

	vm->destroy = VM_Destroy_Compiled;

#ifdef DEBUG_VM
	fflush(qdasmout);
	fclose(qdasmout);

#if 0
	strcpy(fn_d,vm->name);
	strcat(fn_d, ".bin");
	qdasmout = fopen(fn_d, "w");
	fwrite(vm->codeBase, compiledOfs, 1, qdasmout);
	fflush(qdasmout);
	fclose(qdasmout);
#endif
#endif

	#ifndef __WIN64__ //timersub and gettimeofday
		if(vm->compiled)
		{
			struct timeval tvdone =  {0, 0};
			struct timeval dur =  {0, 0};
			Com_Printf( "VM file %s compiled to %i bytes of code (%p - %p)\n", vm->name, vm->codeLength, vm->codeBase, vm->codeBase+vm->codeLength );

			gettimeofday(&tvdone, NULL);
			timersub(&tvdone, &tvstart, &dur);
			Com_Printf( "compilation took %"PRIu64".%06"PRIu64" seconds\n", dur.tv_sec, dur.tv_usec );
		}
	#endif
}


void VM_Destroy_Compiled(vm_t* self)
{
	if(self && self->codeBase)
	{
#ifdef VM_X86_64_MMAP
		munmap(self->codeBase, self->codeLength);
#elif __WIN64__
		VirtualFree(self->codeBase, 0, MEM_RELEASE);
#else
		free(self->codeBase);
#endif
	}
}

/*
==============
VM_CallCompiled

This function is called directly by the generated code
==============
*/

#ifdef DEBUG_VM
static char* memData;
#endif

int	VM_CallCompiled( vm_t *vm, int *args ) {
	int		programCounter;
	int		programStack;
	int		stackOnEntry;
	byte	*image;
	void	*entryPoint;
	void	*opStack;
	int stack[1024] = { 0xDEADBEEF };

	currentVM = vm;

//	Com_Printf("entering %s level %d, call %d, arg1 = 0x%x\n", vm->name, vm->callLevel, args[0], args[1]);

	// interpret the code
	vm->currentlyInterpreting = qtrue;

//	callMask = vm->dataMask;

	// we might be called recursively, so this might not be the very top
	programStack = vm->programStack;
	stackOnEntry = programStack;

	// set up the stack frame 
	image = vm->dataBase;
#ifdef DEBUG_VM
	memData = (char*)image;
#endif

	programCounter = 0;

	programStack -= 48;

	*(int *)&image[ programStack + 44] = args[9];
	*(int *)&image[ programStack + 40] = args[8];
	*(int *)&image[ programStack + 36] = args[7];
	*(int *)&image[ programStack + 32] = args[6];
	*(int *)&image[ programStack + 28] = args[5];
	*(int *)&image[ programStack + 24] = args[4];
	*(int *)&image[ programStack + 20] = args[3];
	*(int *)&image[ programStack + 16] = args[2];
	*(int *)&image[ programStack + 12] = args[1];
	*(int *)&image[ programStack + 8 ] = args[0];
	*(int *)&image[ programStack + 4 ] = 0x77777777;	// return stack
	*(int *)&image[ programStack ] = -1;	// will terminate the loop on return

	// off we go into generated code...
	entryPoint = getentrypoint(vm);
	opStack = &stack;

	__asm__ __volatile__ (
		"	movq %5,%%rsi		\r\n" \
		"	movl %4,%%edi		\r\n" \
		"	movq %2,%%r10		\r\n" \
		"	movq %3,%%r8		\r\n" \
		"       subq $24, %%rsp # fix alignment as call pushes one value \r\n" \
		"	callq *%%r10		\r\n" \
		"       addq $24, %%rsp          \r\n" \
		"	movl %%edi, %0		\r\n" \
		"	movq %%rsi, %1		\r\n" \
		: "=m" (programStack), "=m" (opStack)
		: "m" (entryPoint), "m" (vm->dataBase), "m" (programStack), "m" (opStack)
		: "%rsi", "%rdi", "%rax", "%rbx", "%rcx", "%rdx", "%r8", "%r10", "%r15", "%xmm0"
	);

	if ( opStack != &stack[1] ) {
		Com_Error( ERR_DROP, "opStack corrupted in compiled code (offset %"PRId64")\n", (int64_t) ((void *) &stack[1] - opStack));
	}
	if ( programStack != stackOnEntry - 48 ) {
		Com_Error( ERR_DROP, "programStack corrupted in compiled code\n" );
	}

//	Com_Printf("exiting %s level %d\n", vm->name, vm->callLevel);
	vm->programStack = stackOnEntry;

	return *(int *)opStack;
}
