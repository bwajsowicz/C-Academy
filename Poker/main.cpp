
/****************************************************************************/
/*                A simple Texas Hold'em Poker simulartion.                 */
/*                Program runs fine, but gameplay still has some bugs.      */
/****************************************************************************/

#include <iostream>
#include <iomanip>
#include <string>
#include <cstdlib>
#include <ctime>
#include <math.h>
#ifdef __linux__ 
#include <unistd.h>
#elif _WIN32
#include <windows.h>
#endif


enum actions
{
	FLOP = 1,
	CHECK = 2,
	BET_or_CALL= 3,
	RAISE = 4
};

const int suits_count = 4;
const int ranks_count = 13;
const int sleep_time = 2;
const int player_index = 4;

std::string suits[suits_count];
std::string ranks[ranks_count];

void press_Enter()
{
	printf("\nPress Enter to continue.\n");
	while (getchar() != '\n');
}

void _sleep(int time)
{
#ifdef __linux__ 
	usleep(time*1000);
#elif _WIN32
	Sleep(time);
#endif
}

class Card
{
public:
	int suit;
	int rank;
};

int compareCards(const void *card1, const void *card2)
{
	return (*(Card *)card1).rank - (*(Card *)card2).rank;
}

std::string drawSign(int amount, char sign)
{	
	using namespace std;

	std::stringstream chars;
	for(int i = 0; i < amount; i++)	
		chars << sign;
	
	return chars.str();
}

void displayCards(Card cards[], int size)
{
	using namespace std;
	int numberOfSpaces = 0;

	//**DISPLAY UP**
	cout << "      ";
	for(int i = 0; i < size; i++)
	{
		if(cards[i].rank >= 0)
			numberOfSpaces = suits[cards[i].suit].length() + 2;
		
		cout << ((cards[i].rank) >= 0 ? drawSign(numberOfSpaces, '_') : "___");
		
		cout << "   ";
	}
	cout << endl;
	//--------------

	//**DISPLAY RANKS**
	cout << "     ";
	for(int i = 0; i < size; i++)
	{
		if(cards[i].rank >= 0)
			numberOfSpaces = suits[cards[i].suit].length() - ranks[cards[i].rank].length();
		cout << "| " << ((cards[i].rank) >= 0 ? ranks[cards[i].rank] : " ") << ((cards[i].rank) >= 0 ? drawSign(numberOfSpaces, ' ') : "") << " | ";
	}
	cout << endl;
	//---------------
	
	//**DISPLAY SUITS**
	cout << "     ";
	for(int i = 0; i < size; i++)
	{
		cout << "| " << ((cards[i].rank) >= 0 ? suits[cards[i].suit] : " ") << " | ";
	}
	cout << endl;
	//---------------

	//**DISPLAY BOTTOM**
	cout << "     ";
	for(int i = 0; i < size; i++)
	{	
		if(cards[i].rank >= 0)
			numberOfSpaces = suits[cards[i].suit].length() + 2;
		
		cout << "|" << ((cards[i].rank) >= 0 ? drawSign(numberOfSpaces, '_') : "___") << "| ";		
	}
	cout << endl;
	//---------------
}

class Deck
{

private:
	int top;
	static const int card_tally = 52;

	Card cards[card_tally];

public:
	Deck()
	{
		for (int i = 0; i < suits_count; i++)
		{
			for (int j = 0; j < ranks_count; j++)
			{
				cards[i * ranks_count + j].suit = i;
				cards[i * ranks_count + j].rank = j;
			}
		}
		suits[0] = "Diamonds";
		suits[1] = "Spades";
		suits[2] = "Hearts";
		suits[3] = "Clubs";

		ranks[0] = "2";
		ranks[1] = "3";
		ranks[2] = "4";
		ranks[3] = "5";
		ranks[4] = "6";
		ranks[5] = "7";
		ranks[6] = "8";
		ranks[7] = "9";
		ranks[8] = "T";
		ranks[9] = "Joker";
		ranks[10] = "Queen";
		ranks[11] = "King";
		ranks[12] = "Ace";
	}

	void print()
	{
		std::cout << "Printing the deck..." << std::endl;
		_sleep(1);
		for (int i = 0; i < card_tally; i++)
		{
			std::cout << ranks[cards[i].rank] << suits[cards[i].suit] << std::endl;
		}
		printf("\n");
	}

	void shuffle()
	{
		top = card_tally - 1;

		for (int i = 0; i < suits_count; i++)
		{
			for (int j = 0; j < ranks_count; j++)
			{
				cards[i * ranks_count + j].suit = i;
				cards[i * ranks_count + j].rank = j;
			}
		}

		int x;
		Card tempCard;
		for (int i = 0; i < card_tally; i++)
		{
			x = rand() % card_tally;
			tempCard = cards[i];
			cards[i] = cards[x];
			cards[x] = tempCard;
		}
	}

	Card hitme()
	{
		top--;
		return cards[top + 1];
	}
};

class Player
{
public:
	std::string name;
	int money;
	Card cards[2];
	bool playing;
	int round;
	int goodToGo;
};

class PokerGame
{
public:
	int players_count;
	Player * players;
	std::string playerNames[10] = {"Wojciech", "Tristan", "Michal", "Jotaro", "Joseph", "Mielnik", "Diego", "Edyta", "Jan", "XD"};

	void start(const std::string &name, std::string &numberOfPlayers)
	{
		players_count = stoi(numberOfPlayers);
		players = (Player*)malloc(players_count * sizeof(Player));

		for (int i = 0; i < players_count; i++)
		{
			players[i].money = 100;
			players[i].playing = true;
		}

		players[4].money = 1000;

		for(int i = 0; i < players_count; i++)
		{
			if(i == player_index)
				players[player_index].name = name;
			else 
				players[i].name = playerNames[i];
		}
			
		startGame();
	}


	void deal()
	{
		for (int i = 0; i < players_count; i++)
		{
			for (int j = 0; j < 2; j++)
			{
				if (players[i].playing)
				{
					players[i].cards[j] = deck1.hitme();
				}
			}
		}
		for (int i = 0; i < 5; i++)
			tableCards[i].rank = -1;
	}

	void flop()
	{
		for (int i = 0; i < 3; i++)
			tableCards[i] = deck1.hitme();
	}

	void turn()
	{
		tableCards[3] = deck1.hitme();
	}

	void river()
	{
		tableCards[4] = deck1.hitme();
	}

	void printTable()
	{
		using std::cout;
		using std::endl;
		using std::setw;

		int upperHalfOfPlayers = ceil(players_count/2);

		cout << "---------------------------------------------------------------------------------------------" << endl;
		//--PLAYERS NAMES--
		cout << "  ";
		for(int i = 0; i < upperHalfOfPlayers; i++)
		{
			cout << ((players[i].playing) ? (players[i].name) : "      ") << "         ";
		}		
		cout << endl;
		//-----------------

		//--PLAYERS MONEY--
		cout << "   ";
		for(int i = 0; i < upperHalfOfPlayers; i++)
		{
			cout << "$" << setw(4) << ((players[i].playing) ? (players[i].money) : 0) << "          ";
		}
		cout << endl;
		//-----------------

		//----TABLE-TOP----
		int tableLength = 0;

		for(int i = 0; i < 5; i++)
		{
			tableLength += (tableCards[i].rank >= 0 ? suits[tableCards[i].suit].length() + 5 : 6);
		}
		cout << "    " << drawSign(tableLength, '_') << "_" << endl;
		//-----------------

		//--MOVING CHIP--
		cout << "   / ";
		for(int i = 0; i < 3; i++)
		{
			cout  << ((blind == i) ? "@" : " ");

			if(i != 2)
				cout << "          ";
		}
		cout << drawSign(tableLength - 23, ' ') << "\\" << endl;
		//---------------

		//----CARDS----
		
		displayCards(tableCards, 5);

		//--------------
		
		//--POT--
		int potMargin = (tableLength - 11) / 2;
		cout << "     " << drawSign(potMargin, ' ') << "Pot = $" << setw(4) << pot << drawSign(potMargin, ' ') << "  " << endl;
		//--------------
		
		//--MOVING CHIP--
		cout << "   \\ ";
		for(int i = 5; i > 2; i--)
		{
			cout  << ((blind  == i) ? "@" : " ");

			if(i != 3)
				cout << "          ";
		}
		cout << drawSign(tableLength - 23, ' ') << "/" << endl;
		//--------------

		//----TABLE-BOTTOM----
		cout << "    \\" << drawSign(tableLength-1, '_') << "/" << endl;
		//-----------------
		cout << endl;

		//--PLAYERS 4-6--
		cout << "  ";
		for(int i = players_count - 1; i > upperHalfOfPlayers - 1; i--)
		{
			cout << ((players[i].playing) ? (players[i].name) : "      ") << "         ";
		}		
		cout << endl;
		//-----------------

		//--PLAYERS MONEY 4-6--
		cout << "   ";
		for(int i = players_count - 1; i > upperHalfOfPlayers - 1; i--)
		{
			cout << "$" << setw(4) << ((players[i].playing) ? (players[i].money) : 0) << "          ";
		}
		cout << endl;
		//-----------------

		if (players[player_index].round)
		{
			cout << "   Your hand:" << endl;
			displayCards(players[player_index].cards, 2);
		}

		_sleep(3);
	}

private:
	Deck deck1;
	int blind;
	Card tableCards[5];
	int pot, action, bet, raise, betOn, winner, maxPoints, roundWinner;
	int handPoints[6];
	int bestHand[6][3];

	int playersLeft()
	{
		int count = 0;
		for (int i = 0; i < players_count; i++)
			if (players[i].money > 0)
				count++;
		return count;
	}

	int computerAction(int playerNum)
	{
		int rational = rand() % 2;
		if (rational)
		{
			if (players[playerNum].cards[0].rank < 8 && players[playerNum].cards[1].rank < 8)
			{
				if (players[playerNum].cards[0].rank != players[playerNum].cards[1].rank)
					return FLOP;
				else
					return CHECK;
			}
			else if (players[playerNum].cards[0].rank < 10 && players[playerNum].cards[1].rank < 10)
			{
				if (players[playerNum].cards[0].rank != players[playerNum].cards[1].rank)
					return CHECK;
				else if(players[playerNum].money >= betOn && betOn > 0)
					return RAISE;		
				else if(players[playerNum].money >= betOn)
					return BET_or_CALL;
				else
					return FLOP;
			}
			else if (players[playerNum].money >= betOn && betOn > 0)
				return RAISE;
			else if (players[playerNum].money >= betOn)
				return BET_or_CALL;
			else
				return FLOP;
		}
		else if (players[playerNum].money >= betOn && betOn > 0)
			return rand() % 4 + 1;
		else if (players[playerNum].money >= betOn)
			return rand() % 3 + 1;
		else
			return rand() % 2 + 1;
	}

	/*checks if someone still got bet/call*/
	bool playersToBet()
	{
		for (int i = 0; i < players_count; i++)
			if (players[i].round == 1 && players[i].goodToGo == 0)
				return true;

		return false;
	}

	void takeBets()
	{
		using std::cout;
		using std::cin;
		using std::endl;

		betOn = 0;
		for (int k = 0; k < players_count; k++)
			players[k].goodToGo = 0;

		for (int k = blind; k < blind + players_count; k++)
		{
			/* human player actions */
			if (k % players_count == 4 && players[player_index].round)
			{
				if (betOn)
				{
					if(players[player_index].money >= betOn)
						cout << "\t\t\t\t\tYour action: (1) FLOP (3) BET/CALL (4) RRRRRAISE? ";
					else
						cout << "\t\t\t\t\tYour action: (1) FLOP";
					cin >> action;

					if(players[player_index].money >= betOn)
					{
						while (action != FLOP && action != BET_or_CALL && action != RAISE)
						{
							cout << "Invalid number pressed." << endl;
							cout << "\t\t\t\t\tYour action: (1) FLOP (3) BET/CALL (4) RRRRAISE?";
							cin >> action;
						}
					}
					else
					{
						while (action != FLOP)
						{
							cout << "Invalid number pressed." << endl;
							cout << "\t\t\t\t\tYour action: (1) FLOP";
							cin >> action;
						}
					}	
				}
				else
				{
					if(players[player_index].money > 0) 
						cout << "\t\t\t\t\tYour action: (1) FLOP (2) CHECK (3) BET/CALL ";						
					else
						cout << "\t\t\t\t\tYour action: (1) FLOP (2) CHECK ";
					cin >> action;


					if(players[player_index].money > 0)
					{
						while (action != FLOP && action != CHECK && action != BET_or_CALL)
						{
							cout << "Invalid number pressed." << endl;
							cout << "\t\t\t\t\tYour action: (1) FLOP (2) CHECK (3) BET/CALL ";						
							cin >> action;
						}
					}
					else
					{
						while (action != FLOP && action != CHECK)
						{
							cout << "Invalid number pressed." << endl;
							cout << "\t\t\t\t\tYour action: (1) FLOP (2) CHECK ";
							cin >> action;
						}
					}
					
				}

				cout << endl;

				if (action == FLOP)
				{
					players[player_index].round = 0;
					cout << "\t- " << players[player_index].name << " flops...\n";
				}
				else if (action == CHECK)
				{
					cout << "\t+ " << players[player_index].name << " checks.\n";
					continue;
				}
				else if (action == BET_or_CALL)
				{
					if (betOn)
					{
						pot += betOn;
						players[player_index].money -= betOn;
						players[player_index].goodToGo = 1;
						cout << "\t+ " << players[player_index].name << " bets " << betOn << "$\n";
					}
					else
					{
						cout << "How much do you want to bet: ";
						cin >> bet;
						while (bet > players[player_index].money || bet < 1)
						{
							cout << "Invalid number to bet." << endl;
							cout << "How much do you want to bet: ";
							cin >> bet;
							cout << endl << endl;
						}
						pot += bet;
						players[player_index].money -= bet;
						betOn = bet;
						players[player_index].goodToGo = 1;

						cout << "\t+ " << players[player_index].name << " bets " << bet << "$\n";
					}
				}
				else 
				{
					cout << "How much do you want to raise: ";
					cin >> raise;
					while (raise > players[player_index].money || raise <= betOn || raise <= 0)
					{
						cout << "Invalid number to raise." << endl;
						cout << "How much do you want to raise: ";
						cin >> raise;
						cout << endl << endl;
					}

					pot += raise;
					players[player_index].money -= raise;
					betOn += raise;
					players[player_index].goodToGo = 1;

					cout << "\t+ " << players[player_index].name << " r-r-r-r-raises! " << raise << "$\n";
				}
			}
			/* computers actions */
			else
			{
				if (players[k % players_count].round == 0)
				{
					continue;
				}

				action = computerAction(k % players_count);

				if (action == FLOP)
				{
					players[k % players_count].round = 0;
					cout << "\t- " << players[k % players_count].name << " flops..." << endl;
				}
				else if (action == CHECK)
				{
					cout << "\t+ " << players[k % players_count].name << " checks." << endl;
					continue;
				}
				else if (action == BET_or_CALL)
				{
					if (betOn)
					{
						pot += betOn;
						players[k % players_count].money -= betOn;
						cout << "\t++ " << players[k % players_count].name << " calls!" << endl;
						players[k % players_count].goodToGo = 1;
					}
					else
					{
						bet = (rand() % players[k % players_count].money) + 1;
						pot += bet;
						players[k % players_count].money -= bet;
						cout << '\a';
						cout << "\t+ " << players[k % players_count].name << " bets " << bet << "$" << endl;
						betOn = bet;
						players[k % players_count].goodToGo = 1;
					}
				}
				else 
				{
					raise = (rand() % players[k % players_count].money) + 1;
					pot += raise;
					players[k % players_count].money -= raise;
					cout << '\a';
					cout << "\t+ " << players[k % players_count].name << " r-r-r-r-r-raises! " << raise << "$" << endl;
					betOn += bet;
					players[k % players_count].goodToGo = 1;
				}
				_sleep(1);
			}
		}

		if (betOn && playersToBet())
		{
			for (int k = blind + 1; k < blind + 7; k++)
			{
				if (k % players_count == 4)
				{
					if (players[player_index].round && players[player_index].goodToGo == 0)
					{
						cout << "\t\t\t\t\tYour action: (1) FLOP (3) BET/CALL ";
						cin >> action;
						while (action != FLOP && action != BET_or_CALL)
						{
							cout << "Invalid number pressed." << endl;
							cout << "\t\t\t\t\tYour action: (1) FLOP (3) BET/CALL ";
							cin >> action;
							cout << endl << endl;
						}
						if (action == FLOP)
						{
							cout << "\t- " << players[player_index].name << " flops...\n";
							players[player_index].round = 0;
						}
						else
						{
							pot += betOn;
							players[player_index].money -= betOn;
							players[player_index].goodToGo = 1;

							cout << "\t+ " << players[player_index].name << " bets " << betOn << "$\n";
						}
					}
				}

				else
				{
					if (players[k % players_count].round == 0 || players[k % players_count].goodToGo == 1)
						continue;
					action = rand() % 2;
					if (action == 0 || players[k % players_count].money < betOn)
					{
						players[k % players_count].round = 0;
						cout << "\t- " << players[k % players_count].name << " flops..." << endl;
					}
					else
					{
						pot += betOn;
						players[k % players_count].money -= betOn;
						cout << "\t++ " << players[k % players_count].name << " calls!" << endl;
						players[k % players_count].goodToGo = 1;
					}
				}
			}
		}
	}

	bool oneLeft()
	{
		int count = 0;
		for (int k = 0; k < players_count; k++)
			if (players[k].round)
				count++;
		if (count == 1)
			return true;
		else
			return false;
	}

	int getWinner()
	{
		int winner;
		for (int k = 0; k < players_count; k++)
			if (players[k].round)
				winner = k;
		return winner;
	}

	int getScore(Card hand[])
	{
		qsort(hand, 5, sizeof(Card), compareCards);
		int straight, flush, three, four, full, pairs, high;
		int k;

		straight = flush = three = four = full = pairs = high = 0;
		k = 0;

		/*checks for flush*/
		while (k < 4 && hand[k].suit == hand[k + 1].suit)
			k++;
		if (k == 4)
			flush = 1;

		/* checks for straight*/
		k = 0;
		while (k < 4 && hand[k].rank == hand[k + 1].rank - 1)
			k++;
		if (k == 4)
			straight = 1;

		/* checks for fours */
		for (int i = 0; i < 2; i++)
		{
			k = i;
			while (k < i + 3 && hand[k].rank == hand[k + 1].rank)
				k++;
			if (k == i + 3)
			{
				four = 1;
				high = hand[i].rank;
			}
		}

		/*checks for threes and fullhouse*/
		if (!four)
		{
			for (int i = 0; i < 3; i++)
			{
				k = i;
				while (k < i + 2 && hand[k].rank == hand[k + 1].rank)
					k++;
				if (k == i + 2)
				{
					three = 1;
					high = hand[i].rank;
					if (i == 0)
					{
						if (hand[3].rank == hand[4].rank)
							full = 1;
					}
					else if (i == 1)
					{
						if (hand[0].rank == hand[4].rank)
							full = 1;
					}
					else
					{
						if (hand[0].rank == hand[1].rank)
							full = 1;
					}
				}
			}
		}

		if (straight && flush)
			return 170 + hand[4].rank;
		else if (four)
			return 150 + high;
		else if (full)
			return 130 + high;
		else if (flush)
			return 110;
		else if (straight)
			return 90 + hand[4].rank;
		else if (three)
			return 70 + high;

		/* checks for pairs*/
		for (k = 0; k < 4; k++)
			if (hand[k].rank == hand[k + 1].rank)
			{
				pairs++;
				if (hand[k].rank>high)
					high = hand[k].rank;
			}

		if (pairs == 2)
			return 50 + high;
		else if (pairs)
			return 30 + high;
		else
			return hand[4].rank;
	}

	int tryHand(int array[], int player)
	{
		Card hand[5];

		/* get cards from table and player */
		for (int i = 1; i < 4; i++)
			hand[i - 1] = tableCards[array[i]];

		for (int i = 0; i < 2; i++)
			hand[i + 3] = players[player].cards[i];

		return getScore(hand);

	}

	void evaluateHands()
	{
		int stack[10], k;
		int currentPoints;

		for (int q = 0; q < players_count; q++)
		{
			if (players[q].round)
			{
				stack[0] = -1; /* -1 is not considered as part of the set */
				k = 0;
				while (1) {
					if (stack[k] < 4)
					{
						stack[k + 1] = stack[k] + 1;
						k++;
					}

					else
					{
						stack[k - 1]++;
						k--;
					}

					if (k == 0)
						break;

					if (k == 3)
					{
						currentPoints = tryHand(stack, q);
						if (currentPoints > handPoints[q])
						{
							handPoints[q] = currentPoints;
							for (int x = 0; x < 3; x++)
								bestHand[q][x] = stack[x + 1];
						}
					}
				}

			}
		}
	} /*end of evaluateHands() */

	void printAllHands(int winner)
	{
		using std::cout;
		using std::endl;

		Card cards[5];

		for(int i = 0; i < 3; i++)
			cards[i+2] = tableCards[bestHand[winner][i]];

		for(int i = 0; i < players_count; i++)
		{
			if(players[i].playing)
			{
				for(int j = 0; j < 2; j++)
				cards[j] = players[i].cards[j];

				qsort(cards, 5, sizeof(Card), compareCards);
	
				if(i == winner)
					cout << "[WINNER] " << players[i].name << "'s " << " hand:" << endl;
				else	
					cout << players[i].name << "'s " << " hand:" << endl;
				
				cout << endl;
				displayCards(cards, 5);
				cout << endl;
			}
		}

		_sleep(3);
	}

	/* main gameplay function*/
	void startGame()
	{
		int i = 0;
		int player_paying_blind = 0;

		while (playersLeft() > 1)
		{
			/* starting default values*/
			for (int z = 0; z < players_count; z++)
				if (players[z].money<1)
				{
					players[z].playing = 0;
					players[z].round = 0;
				}
			for (int z = 0; z < players_count; z++)
			{
				if (players[z].playing)
					players[z].round = 1;
				handPoints[z] = -1;
			}
			for (int x = 0; x < players_count; x++)
			{
				for (int y = 0; y<3; y++)
				{
					bestHand[x][y] = -1;
				}
			}

			/* checking for game over*/
			if (players[player_index].playing == 0)
			{
				std::cout << "You are out of money, sorry." << std::endl;
				std::cout << "Game over." << std::endl;
				break;
			}

			blind = i % players_count;

			for(int i = 1; i <= players_count; i++)
			{
				if(players[i].playing)
				{
					player_paying_blind = (blind + i) % players_count;
					break;	
				}
			}

			/* paying blind */
			pot = 20;
			if (players[player_paying_blind].money >= 20)
				players[player_paying_blind].money -= 20;
			else
				players[player_paying_blind].playing = 0;

			std::cout << "\n\n\n";
			std::cout << "\t\t\t\t\t ------ ROUND " << i + 1 << " ------\n\n\n";
			_sleep(1000);
			deck1.shuffle();

			/* pre-flop */
			deal();
			_sleep(sleep_time);
			printTable();
			takeBets();
			if (oneLeft())
			{
				winner = getWinner();
				players[winner].money += pot;
				std::cout << players[winner].name << " wins $" << pot << "\n\n";
				printAllHands(winner);
				i++;
				continue;
			}

			/* flop */
			flop();
			std::cout << std::endl;
			_sleep(sleep_time);
			printTable();
			takeBets();
			if (oneLeft())
			{
				winner = getWinner();
				players[winner].money += pot;
				std::cout << players[winner].name << " wins $" << pot << "\n\n";
				printAllHands(winner);
				i++;
				continue;
			}

			/* turn */
			turn();
			std::cout << std::endl;
			_sleep(sleep_time);
			printTable();
			takeBets();
			if (oneLeft())
			{
				winner = getWinner();
				players[winner].money += pot;
				std::cout << players[winner].name << " wins $" << pot << "\n\n";
				printAllHands(winner);
				i++;
				continue;
			}

			/* river */
			river();
			std::cout << std::endl;
			_sleep(sleep_time);
			printTable();
			takeBets();

			evaluateHands();

			/* find and declare round winner */
			maxPoints = 0;
			for (int q = 0; q < players_count; q++)
			{
				if (players[q].round)
				{
					if (handPoints[q] > maxPoints)
					{
						maxPoints = handPoints[q];
						roundWinner = q;
					}
				}
			}
			std::cout << std::endl;
			std::cout << players[roundWinner].name << " wins $" << pot << " with ";
			if (maxPoints < 30)
				std::cout << "HIGH CARD";
			else if (maxPoints < 50)
				std::cout << "SINGLE PAIR";
			else if (maxPoints < 70)
				std::cout << "TWO PAIRS";
			else if (maxPoints < 90)
				std::cout << "THREE OF A KIND";
			else if (maxPoints < 110)
				std::cout << "STRAIGHT";
			else if (maxPoints < 130)
				std::cout << "FLUSH";
			else if (maxPoints < 150)
				std::cout << "FULL HOUSE";
			else if (maxPoints < 170)
				std::cout << "FOUR OF A KIND";
			else
				std::cout << "STRAIGHT FLUSH";
			std::cout << "\n\n";

			//printWinningHand(roundWinner);
			printAllHands(roundWinner);

			players[roundWinner].money += pot;

			i++;
		}

	}
};


int main()
{
	std::string name;
	PokerGame game1;

	srand(time(NULL));

	using std::cout;
	using std::endl;

	cout << "Welcome to..." << endl << endl;
	cout << "#######                        ###### " << endl;
	cout << "   #    ###### #    # #####    #     #  ####  #    # ###### #####" << endl;
	cout << "   #    #       #  #    #      #     # #    # #   #  #      #    #" << endl;
	cout << "   #    #####    ##     #      ######  #    # ####   #####  #    #" << endl;
	cout << "   #    #        ##     #      #       #    # #  #   #      #####" << endl;
	cout << "   #    #       #  #    #      #       #    # #   #  #      #   #" << endl;
	cout << "   #    ###### #    #   #      #        ####  #    # ###### #    #" << endl << endl;

	std::string buffer;

	cout << "Please type your name: ";
	std::cin >> name;
	cout << "Selec number of players: ";
	std::cin >> buffer;

	cout << "OK " << name << " let's play some poker!" << endl << endl;

	game1.start(name, buffer);

	return 0;
}
