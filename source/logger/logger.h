#ifndef LOGGER_LOGGER_H_
#define LOGGER_LOGGER_H_

#ifndef LOGGER_DLL
#define LOGGER_LOGGER_API __declspec(dllexport)
#else
#define LOGGER_LOGGER_API __declspec(dllimport)
#endif

extern "C"
{
	class Logger
	{
	private:
	public:
		LOGGER_LOGGER_API Logger();
	};
}

#endif
