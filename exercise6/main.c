#include <assert.h>
#include <stdbool.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
/** 
* Function returns an array of 5 elements.
* Those 5 elements are created randomly in the range 1 - 49.
* Numbers can't repeat.
*/

bool check_randomness(int* array)
{
    int i,j;
    int match = 0;

    for(i = 0; i < 5; i++)
    {
        match = 0;
        for(j = 0; j < 5; j++)
        {
            if(array[i] == array[j])
                match++;

            if(match > 1)
                 return false;
        }
    }

    return true;
}

int* Lotto_drawing()
{
    srand(time(NULL));

	int* randomElements = malloc(5*sizeof(int));

    int i;

    for(i = 0; i < 5; i++)
        randomElements[i] = rand() % (49 + 1 - 0) + 0;

    return randomElements;
}

/* Please create test cases for this program. test_cases() function can return void, bool or int. */
bool test_cases()
{
    bool result = check_randomness(Lotto_drawing());
    assert(result == true);

    int firstTestArray[5] = {1, 1, 1, 1, 2};
    int secondTestArray[5] = {1, 12, 15, 23, 2};

    result = check_randomness(firstTestArray);
    assert(result == false);

    result = check_randomness(secondTestArray);
    assert(result == true);
}

int main()
{
    test_cases();
}
