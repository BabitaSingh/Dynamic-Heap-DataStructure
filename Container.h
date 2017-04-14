///============================================================================
///======	class CContainer
///============================================================================
#include <stdlib.h>
#include <new>
#include <windows.h>
//#pragma warning( disable : 4290 )
template<typename T>
class CContainer
{
public:
	CContainer(char * pBuffer, unsigned nBufferSize);	///======	Constructs the container from a pre-defined buffer.
	~CContainer();

	T *				Add();				///======	Adds an element to the container, constructs it and returns it to the caller.
	///======	Note that the return address needs to be persistent during the lifetime of the object.

	void			Remove(T *);		///======	Removes an object from the container.

	unsigned		Count() const;		///======	Number of elements currently in the container.
	unsigned		Capacity() const;	///======	Max number of elements the container can contain. You can limit the capacity to 65536 elements if this makes your implementation easier.

	bool			IsEmpty() const;	///======	Is container empty?
	bool			IsFull() const;		///======	Is container full?	

	char const *	GetAuthor() const
	{
		return "Babita Singh submitted on May 30th, 2016";	///======	put your first and last name as well as the date you took the test.
	}

	T const *		operator [] (unsigned nIndex) const;	///======	Returns the n th element of the container [0..Count -1].
	T *				operator [] (unsigned nIndex);			///======	Returns the n th element of the container [0..Count -1].
	char *				LowestAvailableAddress();
private:
	char *containerBuff;	//private local buffer for preallocated memory buffer
	unsigned count;			//variable to keep track of added elements
	unsigned capacity;		//Total capacity of the container 
	T *templateObj;			//Object declaration which would be defined in add for allocation on preallocated buffer
	char* add;				//This address is being incremented for every addition of an object just to point at the starting address of available buffer
	char* startingAdd;		//This is being used while iterating the buffer
	size_t currCapacity;    //This stores the current Non Removed Objects
	char* lowestZeroAddr;	//this maintains the lowest available memory addr
};

//Constructor for the Container
template<typename T>
CContainer<T>::CContainer(char * pBuffer, unsigned nBufferSize)
{
	capacity = nBufferSize / sizeof(T);
	containerBuff = pBuffer;// allocate memory // zeroized memory
	if (containerBuff == NULL) {
		printf("%s", "memory allocation failed for containerBuff\n");
		exit(EXIT_FAILURE);
	}
	count = 0;
	add = containerBuff;
	startingAdd = containerBuff;
	lowestZeroAddr = containerBuff;
	currCapacity = 0;

};
//Destructor for the Container
template<typename T>
CContainer<T>::~CContainer()
{
};
//Adding Object to the Container and handling the deallocated addresses by adding elements at new spots
template<typename T>
T *	CContainer<T>::Add()
{
	templateObj = new(lowestZeroAddr)T();
	lowestZeroAddr += sizeof(T);
	count++;
	currCapacity++;	
	return templateObj;
};

template<typename T>
void CContainer<T>::Remove(T *tempObj)
{
	//calling the destructor
	lowestZeroAddr = (char*)tempObj;
	tempObj->~T();
	count--;
};
//count of elements in Container
template<typename T>
unsigned CContainer<T>::Count() const
{
	return count;
};
//Capacity of the container
template<typename T>
unsigned CContainer<T>::Capacity() const
{
	return capacity;
};
//is container empty
template<typename T>
bool CContainer<T>::IsEmpty() const
{
	if (count > 0)
		return false;
	return true;
};
//is container full
template<typename T>
bool CContainer<T>::IsFull() const
{
	if (currCapacity == capacity)
		return true;
	return false;
};

//subscript operator for const T overloading
template<typename T>
T const * CContainer<T>::operator [] (unsigned nIndex) const
{
	char* returnPtr = startingAdd;
	unsigned loopIndex = 0;
	T* curObj = (T*)(startingAdd);//getting the starting addr of the container


	//Checking if starting addr is zero ? move to next block :  goto next loop
	while ((curObj->GetAllocNum() == 0) && (curObj->GetAddress() == 0))
	{
		returnPtr += sizeof(T);
		curObj = (T*)returnPtr;
	}
	//increment loopindex until nIndex and return current pointer
	while (loopIndex != nIndex){
		if (((curObj->GetAllocNum() == 0) && (curObj->GetAddress() == 0)))
		{
			returnPtr += sizeof(T);
			curObj = (T*)returnPtr;
		}
		else{
			returnPtr += sizeof(T);
			curObj = (T*)returnPtr;
			//Increment loopIndex only if curr block is not removed
			if (!((curObj->GetAllocNum() == 0) && (curObj->GetAddress() == 0)))
				loopIndex++;
		}

		returnPtr -= sizeof(T);
		curObj = (T*)returnPtr;

	}

	return (T*)(returnPtr);
};

//subscript operatoroverloading
template<typename T>
T * CContainer<T>::operator [] (unsigned nIndex)
{
	char* itrAdd = startingAdd;
	char* returnPtr = startingAdd;
	unsigned loopIndex = 0;
	T* curObj = (T*)(startingAdd);//getting the starting addr of the container


	//Checking if starting addr is zero ? move to next block :  goto next loop
	while ((curObj->GetAllocNum() == 0) && (curObj->GetAddress() == 0))
	{
		returnPtr += sizeof(T);
		curObj = (T*)returnPtr;
	}
	//increment loopindex until nIndex and return current pointer
	while (loopIndex != nIndex){
		if (((curObj->GetAllocNum() == 0) && (curObj->GetAddress() == 0)))
		{
			returnPtr += sizeof(T);
			curObj = (T*)returnPtr;
		}
		else{
			returnPtr += sizeof(T);
			curObj = (T*)returnPtr;
			//Increment loopIndex only if curr block is not removed
			if (!((curObj->GetAllocNum() == 0) && (curObj->GetAddress() == 0)))
				loopIndex++;
		}

		returnPtr -= sizeof(T);
		curObj = (T*)returnPtr;

	}

	return (T*)(returnPtr);
};


