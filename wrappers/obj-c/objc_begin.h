//
// Copyright (c) 2015-2020 Microsoft Corporation and Contributors.
// SPDX-License-Identifier: Apache-2.0
//
/*
	If the source file or header file is contaminated by Windows headers,
	Wrap the pure Objective-C part with #include <objc_begin.h> and #include <objc_end.h>.
	These inclusions always have to come together and be balanced
	The primary goal is to take care of "interface" and "BOOL" conflicts
	
	// Whenever possible, avoid contaminated source or header.
	// For legacy,
	// - if the header is pure Objective-C, put the wrap in the contaminated source that includes it.
	// - if the header itself is contaminated, put the wrap in the header
*/

#if !__OBJC__
	#error "this file should be used only in Obj-C code"
#endif


//make sure "interface" is not #defined

//this check seemingly does nothing but it works around a freakish clang preprocessor bug
//that sometimes push_macro/pop_macro does not work. VSO:142920

#if defined(interface)
#endif

#pragma push_macro("interface")
#if defined(interface)
	#undef interface
#endif

//make sure "BOOL" is not #defined

//this check seemingly does nothing but it works around a freakish clang preprocessor bug

#if defined(BOOL)
#endif

#pragma push_macro("BOOL")
#if defined(BOOL)
	#undef BOOL
#endif

