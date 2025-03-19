#include <stdio.h>
#include <string.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>
#include "alloc.h"

int main()
{
	//mmap to get page
	if(init_alloc())
		return 1;	//mmap failed

	char *str = alloc(512);
	char *str2 = alloc(512);
	printf("ALLOCATION COMPLETE");
	print_memory_layout();

	if(str == NULL || str2 == NULL)
	{
		printf("alloc failed\n");
		return 1;
	}

	strcpy(str, "Hello, world!");
	if(strcmp("Hello, world!", str))
	  printf("Hello, world! test failed\n");
	
	else
	  printf("Hello, world! test passed\n");
	
	dealloc(str);
	dealloc(str2);
	


	printf("Elementary tests passed\n");
	
	printf("Starting comprehensive tests (see details in code)\n");

	/*
	Comprehensive tests:
	1. Populate entire thing in blocks of for loop (a's, b's, c's, and d's) equal size.
	2. Dealloc c's, reallocate and replace with x's.
	3. 
	*/

	/*** test 1 ****/
	
	//Generating ideal strings for comparison
	char stringA[1024], stringB[1024], stringC[1024], stringD[1024], stringX[1024];
	for(int i = 0; i < 1023; i++)
	{
		stringA[i] = 'a';
		stringB[i] = 'b';
		stringC[i] = 'c';
		stringD[i] = 'd';
		stringX[i] = 'x';
	}

	stringA[1023] = stringB[1023] = stringC[1023] = stringD[1023] = stringX[1023] = '\0';

	char *strA = alloc(1024);
	char *strB = alloc(1024);
	char *strC = alloc(1024);
	char *strD = alloc(1024);

	for(int i = 0; i < 1023; i++)
	{
		strA[i] = 'a';
		strB[i] = 'b';
		strC[i] = 'c';
		strD[i] = 'd';
	}
	strA[1023] = strB[1023] = strC[1023] = strD[1023] = '\0';

	if(strcmp(stringA, strA) == 0 && strcmp(stringB, strB) == 0 && strcmp(stringC, strC) == 0 && strcmp(stringD, strD) == 0)
	  printf("Test 1 passed: allocated 4 chunks of 1KB each\n");
	else
	  printf("Test 1 failed: A: %d, B: %d, C: %d, D: %d\n", strcmp(stringA, strA), strcmp(stringB, strB), strcmp(stringC, strC), strcmp(stringD, strD));

	/**** test 2 ****/
	print_memory_layout();
	
	printf("Starting Test 2");
	dealloc(strC);
	print_memory_layout();

	char *strX = alloc(1024);
	for(int i = 0; i < 1023; i++)
	{
		strX[i] = 'x';
	}
	strX[1023] = '\0';

	if(strcmp(stringX, strX) == 0)
	  printf("Test 2 passed: dealloc and realloc worked\n");
	else
	  printf("Test 2 failed: X: %d\n", strcmp(stringX, strX));

	/*** test 3 ***/
	
	char stringY[512], stringZ[512];
	for(int i = 0; i < 511; i++)
	{
		stringY[i] = 'y';
		stringZ[i] = 'z';
	}
	stringY[511] = stringZ[511] = '\0';

	dealloc(strB);

	char *strY = alloc(512);
	char *strZ = alloc(512);
	
	for(int i = 0; i < 511; i++)
	{
		strY[i] = 'y';
		strZ[i] = 'z';
	}
	strY[511] = strZ[511] = '\0';

	if(strcmp(stringY, strY) == 0 && strcmp(stringZ, strZ) == 0)
	  printf("Test 3 passed: dealloc and smaller realloc worked\n");
	else
	  printf("Test 3 failed: Y: %d, Z: %d\n", strcmp(stringY, strY), strcmp(stringZ, strZ));


	// merge checks
	//test 4: free 2x512, allocate 1024
	
	dealloc(strZ);
	dealloc(strY);
	strY=alloc(1024);
	for(int i = 0; i < 1023; i++)
	{
		strY[i] = 'x';
	}
	strY[1023] = '\0';

	if(strcmp(stringX, strY) == 0)
	  printf("Test 4 passed: merge worked\n");
	else
	  printf("Test 4 failed: X: %d\n", strcmp(stringX, strX));

	//test 5: free 2x1024, allocate 2048
	
	dealloc(strX);
	dealloc(strY);
	strX= alloc(2048);
	char  stringM[2048];
	for (int i=0;i<2047;i++){
		strX[i]=stringM[i]='z';
	}
	strX[2047]=stringM[2047]='\0';
	if (!strcmp(stringM, strX))
		printf("Test 5 passed: merge alloc 2048 worked\n");
	else
		printf("Test5 failed\n");
	///////////////////////////

	
	if(cleanup())
		return 1;	//munmap failed
	return 0;

		/*** test 6 ***/

	char *str1 = alloc(128);
	char *str2 = alloc(256);
	char *str3 = alloc(512);

	if(str1 != NULL && str2 != NULL && str3 != NULL)
	{
    	printf("Test 6: Allocated 128, 256, 512 bytes\n");

    	// Deallocate in a different order
    	dealloc(str2);
    	dealloc(str1);
    	dealloc(str3);

    	printf("Test 6 passed: Multiple deallocations in random order worked\n");
	}
	else
	{
    	printf("Test 6 failed: Allocation failed for one or more blocks\n");
	}

	/*** test 7 ***/

	char *strBig = alloc(5000);  // Request more memory than the page size
	if (strBig == NULL)
	{
    	printf("Test 7 passed: Allocation larger than page size failed\n");
	}
	else
	{
    	printf("Test 7 failed: Allocation larger than page size succeeded unexpectedly\n");
	}

	/*** test 8 ***/

	char *str1 = alloc(1024);
	char *str2 = alloc(1024);

	dealloc(str1);
	dealloc(str2);

	char *strMerged = alloc(2048);  // Allocate the entire space after deallocation
	if (strMerged != NULL)
	{
	    printf("Test 8 passed: Allocation worked after merging adjacent free blocks\n");
	}
	else
	{
	    printf("Test 8 failed: Allocation failed after merging adjacent free blocks\n");
	}

	/*** test 9 ***/

	char *strSmall = alloc(8);  // Allocate minimum size
	if (strSmall != NULL)
	{
	    printf("Test 9 passed: Allocated minimum size of 8 bytes\n");
	    dealloc(strSmall);
	}
	else
	{
	    printf("Test 9 failed: Allocation failed for 8 bytes\n");
	}

	/*** test 10 ***/

	if (cleanup() == 0)
	{
	    printf("Test 10: Cleanup successful\n");

	    // Attempt to allocate after cleanup
	    char *strAfterCleanup = alloc(128);
	    if (strAfterCleanup == NULL)
	    {
	        printf("Test 10 passed: Allocation failed after cleanup\n");
	    }
	    else
	    {
	        printf("Test 10 failed: Allocation succeeded after cleanup\n");
	    }
	}
	else
	{
	    printf("Test 10 failed: Cleanup failed\n");
	}



}
