#ifndef BE_BUILD_H
#define BE_BUILD_H


#define _UNUSED(argument) argument
#define _PACKED __attribute__((packed))
#define _PRINTFLIKE(_format_, _args_) \
		__attribute__((format(__printf__, _format_, _args_)))


#endif // BE_BUILD_H
