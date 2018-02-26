/* Copyright Zaurmann Software 2010 */
#ifndef _MY_STATIC_ASSERTION_H_
#define _MY_STATIC_ASSERTION_H_

#ifdef __cplusplus

namespace StaticAssertionPrivateNamespace
{
	//Is defined only for true template parameter.
	//So sizeof with false template parameter must fail.
	template<bool Ok>
	struct StaticAssertionStructure;

	template<>
	struct StaticAssertionStructure<true>{};
}

#define StaticAssert(Condition)				\
	typedef char mychararray[ sizeof( StaticAssertionPrivateNamespace::StaticAssertionStructure< (Condition) == true  >  ) ]; 

#define CheckSize(MyType, MySize) StaticAssert(sizeof(MyType) == (MySize))

#else

#define StaticAssert(c)
#define CheckSize(t,s)

#endif

#endif
