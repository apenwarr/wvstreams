// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the IPSTREAMS_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// IPSTREAMS_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.
#ifdef IPSTREAMS_EXPORTS
#define IPSTREAMS_API __declspec(dllexport)
#else
#define IPSTREAMS_API __declspec(dllimport)
#endif

// This class is exported from the ipstreams.dll
class IPSTREAMS_API Cipstreams {
public:
	Cipstreams(void);
	// TODO: add your methods here.
};

extern IPSTREAMS_API int nipstreams;

IPSTREAMS_API int fnipstreams(void);
