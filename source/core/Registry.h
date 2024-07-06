#ifndef CORE_REGISTRY_H_
#define CORE_REGISTRY_H_

#ifdef CORE_DLL
#define CORE_REGISTRY_API __declspec(dllexport)
#else
#define CORE_REGISTRY_API __declspec(dllimport)
#endif

extern "C"
{
	class Registry
	{
	private:
	public:
		CORE_REGISTRY_API Registry();
	};
}

#endif
