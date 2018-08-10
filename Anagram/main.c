#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
/** 
* Function returns true if word_1 and word_2 are anagrams to each other. Otherwise false.
* Case sensitivity doesn't matter.
*/


//returns word length without spaces
int world_length_without_spaces(const char* word, int length) 
{
    int charCount = 0;
    int i;

    for(i = 0; i < length; ++i)
    {
        if(!(word[i] == ' '))
            charCount++;
    }

    return charCount;
}

bool check_if_anagram(const char* word_1, int length_1, const char* word_2, int length_2)
{
    int i, j;
    bool match = false;

    //if number of chars without spaces differs then these words aren't anagrams
    char char_1, char_2;
    int firstWordLengthWithoutSpaces = world_length_without_spaces(word_1, length_1);
    int secondWordLengthWithoutSpaces = world_length_without_spaces(word_2, length_2);

    if(firstWordLengthWithoutSpaces != secondWordLengthWithoutSpaces)
        return false;

    for(i = 0; i < length_1; ++i)
    {
        for(j = 0; j < length_2; ++j)
        {
            //ignores spaces
            if(word_1[i] == ' ')
            {
                --j;
                break;
            }

            //ignores captials
            char_1 = tolower(word_1[i]);
            char_2 = tolower(word_2[j]);

            if(char_1 == char_2)
            {
                match = true;
                break;
            }
        }

        if(match == false)
            return false;

        match = false;
    }
	return true;
}

void test_cases()
{
	bool answer = check_if_anagram("LordVader", strlen("LordVader"), "VaderLord", strlen("VaderLord"));
	assert(answer == true);

	answer = check_if_anagram("silent", strlen("silent"), "listen", strlen("listen"));
	assert(answer == true);

	answer = check_if_anagram("funeral", strlen("funeral"), "real fun", strlen("real fun"));
	assert(answer == true);

	answer = check_if_anagram("Elon Musk", strlen("Elon Musk"), "Muskmelon", strlen("Muskmelon"));
	assert(answer == false);

	answer = check_if_anagram("Tieto", strlen("Tieto"), "Tietonator", strlen("Tietonator"));
	assert(answer == false);

	answer = check_if_anagram("Football", strlen("Football"), "Basketball", strlen("Basketball"));
	assert(answer == false);

	answer = check_if_anagram("F", strlen("F"), "", strlen(""));
	assert(answer == false);
}

int main()
{
	test_cases();
}