#ifndef CORE_FHEMACROS_H_
#define CORE_FHEMACROS_H_


#ifdef NDEBUG
	#define IS_DEBUGGING_TERNARY(valueOnTrue, valueOnFalse) valueOnFalse
#else
	#define IS_DEBUGGING_TERNARY(valueOnTrue, valueOnFalse) valueOnTrue
#endif


#endif
