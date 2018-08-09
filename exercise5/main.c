#include <string.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

/**
* Function duplicates letters based on their index. Each index means how many times the letter needs to be duplicated.
* See test cases for examples.
*/

float sum(int number_1, int number_2)
{
    return ((float)(number_1+number_2)/2)*number_2;
}

char* accumulate(const char *word, const int length)
{
    float returnLength = sum(1, length);

    char* returnString = malloc((returnLength+length-1)*sizeof(char));

    int i, j;
    int z = 0;

    for(i = 0; i < length; i++) 
    {
        for(j = i+1; j > 0; j--)
        {      
            if(j == i +1)
                returnString[z] = toupper(word[i]);  
            else
                returnString[z] = tolower(word[i]);
            z++; 
        }
        
        if(i != length - 1)
        {     
            //printf("%d\n", z);
            returnString[z] = '-';
        }

        z++; 
    }

    return returnString;
}

void test_cases()
{
	char* result = accumulate("abcd", strlen("abcd"));
	assert(strcmp(result, "A-Bb-Ccc-Dddd") == 0);

	result = accumulate("cwAt", strlen("cwAt"));
	assert(strcmp(result, "C-Ww-Aaa-Tttt") == 0);
}

int main(int argc, char *argv[])
{
    //printf("%s\n", accumulate("cwAt", strlen("cwAt")));
	test_cases();
	return 0;
}