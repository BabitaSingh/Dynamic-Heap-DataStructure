#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <memory.h>
#include <time.h>
#include <new>

//#define _STATS

// dummy malloc to prevent calls within the Container!
void * _DummyMalloc(size_t nSize)
{
	assert(false);
	printf("You're not allowed to call malloc/free or ordinary new/delete within the body of the container!\n");
	return NULL;
}

#define malloc _DummyMalloc
#include "container.h"
#undef malloc

static unsigned gs_nBufferSize = 0;
static char * gs_pBuffer = NULL;

#ifndef CDECL
#	if defined(__GNUC__) || defined(__clang__)
#		define CDECL __attribute__((cdecl))			// cover GCC and CLANG
#	elif defined(_MSC_VER)
#		define CDECL __cdecl						// Visual Studio
#	else
#		define CDECL								// unknown compiler...
#	endif
#endif

//-------------------------------------------------------------------------
//-------------------------------------------------------------------------

class CTestClass;
static void CheckAllCurrentElements(CContainer<CTestClass> const & oContainer);

///	this class only purpose is to control access to global new/delete operators. And yes, we know it's not pretty!
class CNewDeleteControl
{
	CNewDeleteControl() {}

private:
	static bool ms_bAllowNewAndDeleteOperators;

	friend void * CDECL operator new[](std::size_t nSize);
	friend void * CDECL operator new(std::size_t nSize);
	friend void CDECL operator delete[](void * pPtr) throw();
	friend void CDECL operator delete(void * pPtr) throw();

	friend void CheckAllCurrentElements(CContainer<CTestClass> const & oContainer);
	friend int main(int argc, char* argv[]);
};

bool CNewDeleteControl::ms_bAllowNewAndDeleteOperators = false;

//-------------------------------------------------------------------------

void * CDECL operator new[](std::size_t nSize)
{
	if (!CNewDeleteControl::ms_bAllowNewAndDeleteOperators)
	{
		assert(false);
		printf("You're not allowed to call malloc/free or ordinary new/delete within the body of the container!\n");
		return NULL;
	}

	return malloc(nSize);
}

//-------------------------------------------------------------------------

void * CDECL operator new(std::size_t nSize)
{
	if (!CNewDeleteControl::ms_bAllowNewAndDeleteOperators)
	{
		assert(false);
		printf("You're not allowed to call malloc/free or ordinary new/delete within the body of the container!\n");
		return NULL;
	}

	return malloc(nSize);
}

//-------------------------------------------------------------------------

void CDECL operator delete[](void * pPtr) throw()
{
	if (!CNewDeleteControl::ms_bAllowNewAndDeleteOperators)
	{
		assert(false);
		printf("You're not allowed to call malloc/free or ordinary new/delete within the body of the container!\n");
	}

	free(pPtr);
}

//-------------------------------------------------------------------------

void CDECL operator delete(void * pPtr) throw()
{
	if (!CNewDeleteControl::ms_bAllowNewAndDeleteOperators)
	{
		assert(false);
		printf("You're not allowed to call malloc/free or ordinary new/delete within the body of the container!\n");
	}

	free(pPtr);
}

//-------------------------------------------------------------------------
//-------------------------------------------------------------------------

/// this is the class we're going to use for this test. Adds are only allowed at defined times.
/// also we have special markers to make sure constructor/destructor is being called and that object is unique!
class CTestClass
{
	size_t			m_nAddr;
	unsigned		m_nAllocNum;
	static bool		ms_bAllowAdd;

public:
	CTestClass()
	{
		assert(ms_bAllowAdd);
		assert(m_nAddr == 0 && m_nAllocNum == 0);
		m_nAddr = (size_t)this;
		m_nAllocNum = 0;
	}

	~CTestClass()
	{
		assert(m_nAddr == (size_t)this);
		m_nAddr = 0;
		m_nAllocNum = 0;
	}

	void SetAllocNum(unsigned nNumAlloc)
	{
		m_nAllocNum = nNumAlloc;
	}

	unsigned GetAllocNum() const
	{
		return m_nAllocNum;
	}

	size_t GetAddress() const
	{
		return m_nAddr;
	}

protected:
	static void AllowAdd(bool bAllowed)
	{
		ms_bAllowAdd = bAllowed;
	}

	friend void StressTest(CContainer<CTestClass> & oContainer, unsigned const nNumAllocs);
};

bool CTestClass::ms_bAllowAdd = false;

//-------------------------------------------------------------------------

inline bool IsInBuffer(void const * pAddr)
{
	return ((char const *)pAddr >= gs_pBuffer) && ((char const *)pAddr <= (gs_pBuffer + gs_nBufferSize - sizeof(CTestClass)));
}

//-------------------------------------------------------------------------

int CompareTestClassAddr(void const * pX, void const * pY)
{
	CTestClass const ** pE0 = (CTestClass const **)pX;
	CTestClass const ** pE1 = (CTestClass const **)pY;

	if (*pE0 < *pE1)
	{
		return -1;
	}
	else if (*pE0 > *pE1)
	{
		return 1;
	}

	return 0;
}

//-------------------------------------------------------------------------

static void CheckAllCurrentElements(CContainer<CTestClass> const & oContainer)
{
	/// _DEBUG is visual studio definition (DEBUG == 1) is XCode, make sure this is called when the solution is compiled in debug.
	/// you may need to add your own define (for GCC or other compiler you may use)
#if defined(_DEBUG) || (DEBUG == 1)
	int const nCount = oContainer.Count();

	/// this is used to check that any given time we're going to hit all the elements in the array.
	/// First allocate the a temp array
	CTestClass const ** ppElementAddr;
	CNewDeleteControl::ms_bAllowNewAndDeleteOperators = true;
	ppElementAddr = new CTestClass const *[nCount];

	/// clear it
	memset(ppElementAddr, 0, nCount * sizeof(CTestClass const *));

	/// now populate it with valid elements
	for (int i = 0; i < nCount; ++i)
	{
		CTestClass const * pObj = oContainer[i];

		assert(IsInBuffer(pObj) && pObj->GetAddress() == (size_t)pObj);	///======	sanity check
		ppElementAddr[i] = pObj;
	}

	/// now go and sort all these addresses
	qsort(ppElementAddr, nCount, sizeof(CTestClass const *), CompareTestClassAddr);

	/// finally, go and make sure all the elements are in order and no dups!
	for (int i = 0; i < nCount - 1; ++i)
	{
		assert(ppElementAddr[i] < ppElementAddr[i + 1]);
	}

	delete[] ppElementAddr;
	CNewDeleteControl::ms_bAllowNewAndDeleteOperators = false;
#endif
}

//-------------------------------------------------------------------------

void StressTest(CContainer<CTestClass> & oContainer, unsigned const nNumAllocs)
{
	unsigned const nCapacity = oContainer.Capacity();

	unsigned nNballocs = 0;
	unsigned nNbFrees = 0;
#ifdef _STATS
	unsigned nNbEmpty = 0;
	unsigned nNbFull = 0;
	unsigned nMaxSize = 0;
#endif

	///======	pre-fill up 3/4 of the container.
	CTestClass::AllowAdd(true);
	for (unsigned i = 0; i < 3 * nCapacity / 4; ++i)
	{
		CTestClass * pObj = oContainer.Add();
		pObj->SetAllocNum(nNballocs);
		nNballocs++;
	}
	CTestClass::AllowAdd(false);

	///======	loop until we reach a certain number of allocations
	while (nNballocs < nNumAllocs)
	{
		///======	randomly add or remove object from the managed container
		bool bAdd = ((rand() & 0x1f) >= 16) ? true : false;

		if (bAdd && oContainer.IsFull())
		{
			bAdd = false;
		}
		else if (!bAdd && oContainer.IsEmpty())
		{
			bAdd = true;
		}

		if (bAdd)
		{
			CTestClass::AllowAdd(true);
			CTestClass * pObj = oContainer.Add();
			CTestClass::AllowAdd(false);
			assert(IsInBuffer(pObj));
			pObj->SetAllocNum(nNballocs);
			assert(pObj->GetAddress() == (size_t)pObj);	///======	sanity check
			nNballocs++;
		}
		else
		{
			int nIndex = (oContainer.Count() > 1) ? rand() % (oContainer.Count() - 1) : 0;

			CTestClass * pObj = oContainer[nIndex];
			assert(pObj->GetAddress() == (size_t)pObj);	///======	sanity check
			oContainer.Remove(pObj);
			assert(pObj->GetAddress() == 0);			///======	if this assert trips then you haven't called the destructor.
			nNbFrees++;
		}

#ifdef _STATS
		nMaxSize = oContainer.Count() > nMaxSize ? oContainer.Count() : nMaxSize;
		if (oContainer.IsEmpty())
		{
			++nNbEmpty;
		}
		else if (oContainer.IsFull())
		{
			++nNbFull;
		}
#endif

		if ((nNballocs & 32767) == 0)
		{
			CheckAllCurrentElements(oContainer);

			printf("Container => %d allocs, %d frees\r", nNballocs, nNbFrees);
		}
	}

#ifdef _STATS
	printf("\nMaxSize: %d (seed %d), Nb full containers: %d, Nb empty containers: %d\n", nMaxSize, nSeed, nNbFull, nNbEmpty);
#endif

	///======	clean up
	while (oContainer.Count() > 0)
	{
		CTestClass * pObj = oContainer[0];
		assert(pObj->GetAddress() == (size_t)pObj);	///======	sanity check
		oContainer.Remove(pObj);
	}
	assert(oContainer.Count() == 0);
}

//-------------------------------------------------------------------------

int main(int argc, char* argv[])
{
	if (argc < 2)
	{
		printf("Usage:\tContainerTest.exe buffersize (in KB)\n");
		printf("\ttypical test parameters are 2, 20, 200 and 2000.\n");
		printf("\tLooking for implementation correctness as well as solution scalability.\n");
		printf("\tSolution will be compiled in debug and release mode (32 & 64 bits).\n");
		return EXIT_FAILURE;
	}

	CNewDeleteControl::ms_bAllowNewAndDeleteOperators = true;		///======	allow new & delete

	assert(sizeof(CContainer<CTestClass>) < 256);	///======	this is an arbitrary, but why should we need more?

	/// seed the random number generator
	time_t oTime;
	time(&oTime);

	srand((unsigned)oTime);

	/// allocate storage buffer
	gs_nBufferSize = atoi(argv[1]) * 1024;

	gs_pBuffer = new char[gs_nBufferSize];

	memset(gs_pBuffer, 0, sizeof(char) * gs_nBufferSize);

	CContainer<CTestClass> * pContainer = new CContainer<CTestClass>(gs_pBuffer, gs_nBufferSize);

	printf("Managed Container Capacity: %d\n", pContainer->Capacity());
	printf("\tSolution by: %s\n", pContainer->GetAuthor());

	CNewDeleteControl::ms_bAllowNewAndDeleteOperators = false;		///======	standard new & delete are now forbidden

	clock_t oStartTime = clock();
	StressTest(*pContainer, 20000);
	clock_t oEndTime = clock();

	CNewDeleteControl::ms_bAllowNewAndDeleteOperators = true;		///======	allow new & delete

	double fElapsedTime = (double)(oEndTime - oStartTime) / CLOCKS_PER_SEC;

	printf("\nTime elapsed: %f seconds\n", fElapsedTime);

	/// delete container.
	delete pContainer;
	pContainer = NULL;

	/// free memory on our way out
	delete[] gs_pBuffer;

	return EXIT_SUCCESS;
}
