#include <string.h>
#include <assert.h>
#include <stdbool.h>
#include <ctype.h>
/**
* Function checks if a string has the same amount of 'x' and 'o's. Function must be case insensitive.
*/
bool XO(const char *word, const int length)
{
    int i;
    char loweredCaseChar;

    int amountOfOs = 0;
    int amountOfXs = 0;

    for(i = 0; i < length; ++i)
    {
        loweredCaseChar = tolower(word[i]);

        if(loweredCaseChar == 'x')
            amountOfXs++;
        else if(loweredCaseChar == 'o')
            amountOfOs++;
    }

    if(amountOfOs == amountOfXs)
        return true;
    else
        return false;
}

void test_cases()
{
	bool answer = XO("ooxx", strlen("ooxx"));
	assert(answer == true);

	answer = XO("xooxx", strlen("xooxx"));
	assert(answer == false);

	answer = XO("ooxXm", strlen("ooxXm"));
	assert(answer == true);

	answer = XO("zzoo", strlen("zzoo"));
	assert(answer == false);
}

int main(int argc, char *argv[])
{
	test_cases();
	return 0;
}