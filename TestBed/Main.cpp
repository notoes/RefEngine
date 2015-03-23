#include "RefEngine.h"

#include "types.h"
#include <vector>

#include <OpenGLTypes.h>

#ifdef _WINDOWS
#include <direct.h>
#define GetCurrentDir _getcwd
#else
#include <unistd.h>
#define GetCurrentDir getcwd
#endif


int main()
{
	char cCurrentPath[FILENAME_MAX];
	if (GetCurrentDir(cCurrentPath, sizeof(cCurrentPath)))
	{
		printf("The current working directory is %s\n", cCurrentPath);
	}

	reng::RefEngine* pRefEng = new reng::RefEngine();
	if (!pRefEng->Init()) {
		return 1;
	}
	pRefEng->Run();

	return 0;
}

