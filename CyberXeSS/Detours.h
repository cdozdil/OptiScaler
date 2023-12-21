#pragma once

#include <stdint.h>

namespace Detours
{
	static_assert(sizeof(uintptr_t) == sizeof(void *));

	const static uint32_t DISASM_MAX_INSTRUCTIONS	= 50;		// Maximum number of instructions to decode at once

	const static uint32_t OPT_MASK					= 0xFFF;	// Mask for all options
	const static uint32_t OPT_NONE					= 0x000;	// No options
	const static uint32_t OPT_BREAK_ON_FAIL			= 0x001;	// Throw INT3 on failure
	const static uint32_t OPT_DO_NOT_PAD_NOPS		= 0x002;	// Don't pad replaced instructions with NOPs

	struct JumpTrampolineHeader
	{
		uint32_t Magic;				// Used to verify the header
		uint32_t Random;			// Variable to change the code/data hash

		uintptr_t CodeOffset;		// Offset, in code, that was hooked:	"target"
		uintptr_t DetourOffset;		// User function that is called:		"destination"

		uintptr_t InstructionLength;// Length of the instructions that were replaced
		uintptr_t InstructionOffset;// Where the backed-up instructions are

		uintptr_t TrampolineLength;	// Length of the trampoline
		uintptr_t TrampolineOffset;	// Code offset where 'jmp (q/d)word ptr <user function>' occurs

		// Anything after this struct is null data or pure code (instructions/trampoline)
	};

	void SetGlobalOptions(uint32_t Options);
	uint32_t GetGlobalOptions();

	uint64_t DetourAlignAddress(uint64_t Address, uint8_t Align);

	bool DetourCopyMemory(uintptr_t Target, uintptr_t Memory, size_t Length);
	bool DetourFlushCache(uintptr_t Target, size_t Length);
	uintptr_t IATHook(uintptr_t Module, const char *ImportModule, const char *API, uintptr_t Detour);
	uintptr_t IATDelayedHook(uintptr_t Module, const char *ImportModule, const char *API, uintptr_t Detour);

#ifdef _M_IX86
	enum class X86Option
	{
		USE_JUMP,		// jmp <address>;
		USE_CALL,		// call <address>;
		USE_PUSH_RET,	// push <address>; retn;
	};

	namespace X86
	{
		// Redirects a single static function to another
		uintptr_t DetourFunction(uintptr_t Target, uintptr_t Detour, X86Option Options = X86Option::USE_JUMP);

		// Redirects a class member function (__thiscall) to another
		template<typename T>
		uintptr_t DetourFunctionClass(uintptr_t Target, T Detour, X86Option Options = X86Option::USE_JUMP)
		{
			return DetourFunction(Target, *(uintptr_t *)&Detour, Options);
		}

		// Removes a detoured function (Static or class member)
		bool DetourRemove(uintptr_t Trampoline);

		// Redirects an index in a virtual table
		uintptr_t DetourVTable(uintptr_t Target, uintptr_t Detour, uint32_t TableIndex);

		// Redirects a class member virtual function (__thiscall) to another
		template<typename T>
		uintptr_t DetourClassVTable(uintptr_t Target, T Detour, uint32_t TableIndex)
		{
			return DetourVTable(Target, *(uintptr_t *)&Detour, TableIndex);
		}

		// Removes a detoured virtual table index
		bool VTableRemove(uintptr_t Target, uintptr_t Function, uint32_t TableIndex);

		void DetourWriteStub(JumpTrampolineHeader *Header);
		bool DetourWriteJump(JumpTrampolineHeader *Header);
		bool DetourWriteCall(JumpTrampolineHeader *Header);
		bool DetourWritePushRet(JumpTrampolineHeader *Header);

		uint32_t DetourGetHookLength(X86Option Options);
	}
#endif // _M_IX86

#ifdef _M_AMD64
	enum class X64Option
	{
		USE_PUSH_RET,	// push <low 32 address>; [rsp+4h] = <hi 32 addr>; retn;
		USE_RAX_JUMP,	// mov rax, <address>; jmp rax;
		USE_REL32_JUMP, // jmp <address within +/- 2GB>
		USE_REL32_CALL, // call <address within +/- 2GB>
	};

	namespace X64
	{
		// Redirects a single static function to another
		uintptr_t DetourFunction(uintptr_t Target, uintptr_t Detour, X64Option Options = X64Option::USE_REL32_JUMP);

		// Redirects a class member function (__thiscall) to another
		template<typename T>
		uintptr_t DetourFunctionClass(uintptr_t Target, T Detour, X64Option Options = X64Option::USE_REL32_JUMP)
		{
			return DetourFunction(Target, *(uintptr_t *)&Detour, Options);
		}

		// Removes a detoured function (Static or class member)
		bool DetourRemove(uintptr_t Trampoline);

		// Redirects an index in a virtual table
		uintptr_t DetourVTable(uintptr_t Target, uintptr_t Detour, uint32_t TableIndex);

		// Redirects a class member virtual function (__thiscall) to another
		template<typename T>
		uintptr_t DetourClassVTable(uintptr_t Target, T Detour, uint32_t TableIndex)
		{
			return DetourVTable(Target, *(uintptr_t *)&Detour, TableIndex);
		}

		// Removes a detoured virtual table index
		bool VTableRemove(uintptr_t Target, uintptr_t Function, uint32_t TableIndex);

		void DetourWriteStub(JumpTrampolineHeader *Header);
		bool DetourWritePushRet(JumpTrampolineHeader *Header);
		bool DetourWriteRaxJump(JumpTrampolineHeader *Header);
		bool DetourWriteRel32Jump(JumpTrampolineHeader *Header);
		bool DetourWriteRel32Call(JumpTrampolineHeader *Header);

		uint32_t DetourGetHookLength(X64Option Options);
	}
#endif // _M_AMD64
}