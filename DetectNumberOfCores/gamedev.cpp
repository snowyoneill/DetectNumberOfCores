#include <windows.h>
#include <intrin.h>

static const unsigned PROCESSOR_UNKNOWN = 0;
static const unsigned PROCESSOR_AMD = 1;
static const unsigned PROCESSOR_INTEL = 2;


//============================================================================
// Read the CPU speed from the registry
//============================================================================
DWORD ReadCPUSpeedFromRegistry(DWORD dwCPU)
{
HKEY hKey;
DWORD dwSpeed;

	// Get the key name
	wchar_t szKey[256];
	_snwprintf(szKey, sizeof(szKey)/sizeof(wchar_t),
		L"HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\%d\\", dwCPU);

	// Open the key
	if(RegOpenKeyEx(HKEY_LOCAL_MACHINE,szKey, 0, KEY_QUERY_VALUE, &hKey) != ERROR_SUCCESS)
	{
		return 0;
	}

	// Read the value
	DWORD dwLen = 4;
	if(RegQueryValueEx(hKey, L"~MHz", NULL, NULL, (LPBYTE)&dwSpeed, &dwLen) != ERROR_SUCCESS)
	{
		RegCloseKey(hKey);
		return 0;
	}

	// Cleanup and return
	RegCloseKey(hKey);
    return dwSpeed;
}

//============================================================================
// Calculate and log the CPU speed and features
//============================================================================
void LogCPU()
{
unsigned nHighestFeature;
unsigned nHighestFeatureEx;
int nBuff[4];
char szMan[13];
char szFeatures[256];
unsigned nProcessorType;

	// Get CPU manufacturer and highest CPUID
	__cpuid(nBuff, 0);
	nHighestFeature = (unsigned)nBuff[0];
	*(int*)&szMan[0] = nBuff[1];
	*(int*)&szMan[4] = nBuff[3];
	*(int*)&szMan[8] = nBuff[2];
	szMan[12] = 0;
	if(strcmp(szMan, "AuthenticAMD") == 0)
		nProcessorType = PROCESSOR_AMD;
	else if(strcmp(szMan, "GenuineIntel") == 0)
		nProcessorType = PROCESSOR_INTEL;
	else
		nProcessorType = PROCESSOR_UNKNOWN;

	// Get highest extended feature
	__cpuid(nBuff, 0x80000000);
	nHighestFeatureEx = (unsigned)nBuff[0];

	// Get processor brand name
	if(nHighestFeatureEx >= 0x80000004)
	{
		char szCPUName[49];
		szCPUName[0] = 0;
		__cpuid((int*)&szCPUName[0], 0x80000002);
		__cpuid((int*)&szCPUName[16], 0x80000003);
		__cpuid((int*)&szCPUName[32], 0x80000004);
		szCPUName[48] = 0;
		for(int i=(int)strlen(szCPUName)-1; i>=0; --i)
		{
			if(szCPUName[i] == ' ')
				szCPUName[i] = '\0';
			else
				break;
		}

		ELog::Get().SystemFormat(L"PERF    : CPU: %S (%S)\n", szCPUName, szMan);
	}
	else
		ELog::Get().SystemFormat(L"PERF    : CPU: %S\n", szMan);

	// Get CPU features
	szFeatures[0] = 0;
	if(nHighestFeature >= 1)
	{
		__cpuid(nBuff, 1);
		if(nBuff[3] & 1<<0)
			strcat(szFeatures, "FPU ");
		if(nBuff[3] & 1<<23)
			strcat(szFeatures, "MMX ");
		if(nBuff[3] & 1<<25)
			strcat(szFeatures, "SSE ");
		if(nBuff[3] & 1<<26)
			strcat(szFeatures, "SSE2 ");
		if(nBuff[2] & 1<<0)
			strcat(szFeatures, "SSE3 ");

		// Intel specific:
		if(nProcessorType == PROCESSOR_INTEL)
		{
			if(nBuff[2] & 1<<9)
				strcat(szFeatures, "SSSE3 ");
			if(nBuff[2] & 1<<7)
				strcat(szFeatures, "EST ");
		}

		if(nBuff[3] & 1<<28)
			strcat(szFeatures, "HTT ");
	}

	// AMD specific:
	if(nProcessorType == PROCESSOR_AMD)
	{
		// Get extended features
		__cpuid(nBuff, 0x80000000);
		if(nHighestFeatureEx >= 0x80000001)
		{
			__cpuid(nBuff, 0x80000001);
			if(nBuff[3] & 1<<31)
				strcat(szFeatures, "3DNow! ");
			if(nBuff[3] & 1<<30)
				strcat(szFeatures, "Ex3DNow! ");
			if(nBuff[3] & 1<<22)
				strcat(szFeatures, "MmxExt ");
		}

		// Get level 1 cache size
		if(nHighestFeatureEx >= 0x80000005)
		{
			__cpuid(nBuff, 0x80000005);
			ELog::Get().SystemFormat(L"PERF    : L1 cache size: %dK\n", ((unsigned)nBuff[2])>>24);
		}
	}

	// Get cache size
	if(nHighestFeatureEx >= 0x80000006)
	{
		__cpuid(nBuff, 0x80000006);
		ELog::Get().SystemFormat(L"PERF    : L2 cache size: %dK\n", ((unsigned)nBuff[2])>>16);
	}

	// Log features
	ELog::Get().SystemFormat(L"PERF    : CPU Features: %S\n", szFeatures);

	// Get misc system info
	SYSTEM_INFO theInfo;
	GetSystemInfo(&theInfo);

	// Log number of CPUs and speeds
	ELog::Get().SystemFormat(L"PERF    : Number of CPUs: %d\n", theInfo.dwNumberOfProcessors);
	for(DWORD i=0; i<theInfo.dwNumberOfProcessors; ++i)
	{
		DWORD dwCPUSpeed = ReadCPUSpeedFromRegistry(i);
		ELog::Get().SystemFormat(L"PERF    : * CPU %d speed: ~%dMHz\n", i, dwCPUSpeed);
	}
}