ZINDAGI GAMES: PROGRAMMER C++ TEST


GETTING STARTED:
	The test has been compiled against Visual Studio Community 2013 (64 bit console app).

INSTRUCTIONS TO USE:
	Make sure the stress test capacity is lower than the number of command line arguments.

SCALABILITY AND EFFICIENCY:
	I have tried to maximize the number of object stored in buffer by allocating the new object at the deallocated addresses.

TIME LOG: Total 10hrs
	To understand the concept : 2hrs
	To Implement the algorithms : 3hrs
	To test all test cases and debugging the assertions : 5hrs
	Number of days I worked on it : 3days
	
WHY THE SOLUTION TO BE CONSIDERED:
	--I have implement all the necessary methods for the container
	--I have taken care of object allocation by using Placement new and I have properly destroyed each one of the object created by calling their destructor
	--I have handled all the assertions
	--I have maximized the number of objects stored in container by reusing the deallocated vacant blocks
	
********************************************** THANK YOU FOR THE OPPORTUNITY *******************************************************************************