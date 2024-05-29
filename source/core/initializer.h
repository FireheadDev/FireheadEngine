#ifndef CORE_INITIALIZER_H_
#define CORE_INITIALIZER_H_

#ifdef CORE_DLL
#define CORE_INITIALIZER_API __declspec(dllexport)
#else
#define CORE_INITIALIZER_API __declspec(dllimport)
#endif

extern "C"
{
	class Initializer
	{
	private:
	public:
		CORE_INITIALIZER_API Initializer();
	};
}

#endif
