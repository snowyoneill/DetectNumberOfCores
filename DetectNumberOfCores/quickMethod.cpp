#include <stdio.h>
#include <windows.h>

//// Get number of Logical CPUs Per Package
//// For example:
//// HyperThreading P4 CPU will return 2
//// Regular P4 will return 1
//// Dual Xeon with HT will return 2
//// Dual Xeon w/o HT will return 1
//// Dual-core Pentium-D will return 2
//// Dual-core Pentium-D XE will return 4
//// Dual-core Athlon 64 will return 2
//int NumCpuPerPackage() {
//	// Number of Logical Cores per Physical Processor
//	int nCoreCount = 1;
//	// Initialize to 1 to support older processors.
//	_asm {
//		mov		eax, 1
//		cpuid
//		// Test for HTT bit
//		test	edx, 0x10000000
//		jz		Unp
//		// Multi-core or Hyperthreading supported...
//		// Read the "# of Logical Processors per Physical Processor" field:
//		mov		eax, ebx
//		and		eax, 0x00FF0000 // Mask the "logical core counter" byte
//		shr		eax, 16 // Shift byte to be least-significant
//		mov		nCoreCount, eax
//		// Uniprocessor (i.e. Pentium III or any AMD CPU excluding their new dual-core A64)
//		Unp:
//		// nCoreCount will contain 1.
//	}
//	return nCoreCount;
//}

int NumCpuPerPackage() {
	// Number of Logical Cores per Physical Processor
	int HyperThread = 0;
	// Initialize to 1 to support older processors.
	_asm {
		mov		eax, 1
		cpuid
		test	edx, 0x10000000 // Test for HTT bit
		jz		Unp
		mov HyperThread , 3
		Unp:
	}
	return HyperThread;
}

int main(int argc, char* argv[])
{
	unsigned int count = 1;

	SYSTEM_INFO si;
	GetSystemInfo( &si );
	count = si.dwNumberOfProcessors;

	int LogicalPerSocket = 0;
	__asm
	{
		mov eax, 1
		cpuid
		and ebx, 0x00FF0000// Select bits 16 to 23 of EBX
		shr ebx, 16
		mov ebx, LogicalPerSocket
	}

	printf("No: %d , LogicalPerSocket: %d", count, NumCpuPerPackage());

	getchar();
}
