#include <assert.h>
#include <stdlib.h>
/**
* Given a number, return the maximum number that could be formed with digits of the number given.
* For example: number = 7389, return 9873
*/

int cmpfunc (const void * a, const void * b) {
   return ( *(int*)b - *(int*)a );
}

int form_the_largest_number(int number)
{
    int temp = number;
    int digit;
    int lengthOfNumber = 0;

    //gets length of the number
    while(temp != 0) 
    {
        digit = temp % 10;
        temp = temp / 10;
        lengthOfNumber++;
    } 

    int i = 0;

    int* digits = malloc(lengthOfNumber*sizeof(int));

    //puts single digits into array
    while(number != 0)
    {
        digit = number % 10;
        digits[i] = digit;
        number = number / 10;
        i++;
    }

    //array sort
    qsort(digits, lengthOfNumber, sizeof(int), cmpfunc);

    int result = 0;

    //build result from sorted array
    for(i = 0; i < lengthOfNumber; i++)
    {
        result = result*10 + digits[i];
    }
    
    return result;
}


void test_cases()
{
	int result = form_the_largest_number(213);
	assert(result == 321);

	result = form_the_largest_number(7389);
	assert(result == 9873);

	result = form_the_largest_number(63729);
	assert(result == 97632);

	result = form_the_largest_number(566797);
	assert(result == 977665);
}

int main()
{
	test_cases();
}
