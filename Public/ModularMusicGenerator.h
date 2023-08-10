#pragma once

#if defined(_WIN32) || defined(__CYGWIN__)

	#if defined(MMG_EXPORT)

		#if defined(__GNUC__)
			#define MMG_API			__attribute__((dllexport))
		#else
			#define MMG_API			__declspec(dllexport)
		#endif

		#define MMG_TEMPLATE_API_DEF 	MMG_API
		#define MMG_TEMPLATE_API(...)	extern template class __VA_ARGS__

	#else

		#if defined(__GNUC__)
			#define MMG_API __attribute__((dllimport))
		#else
			#define MMG_API __declspec(dllimport)
		#endif

		#define MMG_TEMPLATE_API(...)	template class MMG_API __VA_ARGS__

  #endif

	#define MMG_INTERNAL

#else

	#if __GNUC__ >= 4

		#if defined(MMG_EXPORT)

			#define MMG_API				__attribute__((visibility("default")))
			#define MMG_INTERNAL			__attribute__((visibility("hidden")))
			#define MMG_TEMPLATE_API_DEF
			#define MMG_TEMPLATE_API(...)	extern template class MMG_API __VA_ARGS__

		#else

			#define MMG_API
			#define MMG_INTERNAL
			#define MMG_TEMPLATE_API(...)

		#endif

	#else

		#define MMG_API
		#define MMG_INTERNAL
		#define MMG_TEMPLATE_API(...)

	#endif

#endif


// MyCFuncs.h
#ifdef __cplusplus
extern "C" {  // only need to export C interface if
              // used by C++ source code
#endif

MMG_API void MyCFunc();

#ifdef __cplusplus
}

#include <vector>
#include <memory>

#include "LayerManager.h"

class ModularMusicGenerator
{
public:
	// fluid_synth_t* synth = nullptr;

	LayerManager layerManager;
};


#endif