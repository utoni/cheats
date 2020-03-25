#ifndef GHACK_H_INCLUDED
#define GHACK_H_INCLUDED

#ifndef GHACK_VERSION
#define GHACK_VERSION "0.4a"
#endif

#ifdef BUILD_DLL
#define DLL_EXPORT __declspec(dllexport)
#else
#define DLL_EXPORT __declspec(dllimport)
#endif

#include <log.h>
#include <mem.h>
#include <radar.h>

#endif // GHACK_H_INCLUDED
