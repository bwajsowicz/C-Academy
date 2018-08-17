#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <string.h>
#include <iostream>
#include <string>

#include <sys/epoll.h>

#define MAX_EVENTS 10

#include <math.h>
#include <iomanip>

enum actions
{
	FOLD = 1,
	CHECK = 2,
	BET_or_CALL = 3,
	RAISE = 4
};

void error(char *message)
{
    perror(message);
    exit(1);
}

std::string drawSign(int amount, char sign)
{
	using namespace std;

	stringstream chars;
	for (int i = 0; i < amount; i++)
		chars << sign;

	return chars.str();
}

const int suits_count = 4;
const int ranks_count = 13;
const int sleep_time = 2;
const int player_index = 4;

std::string suits[suits_count];
std::string ranks[ranks_count];

class Card
{
public:
	int suit;
	int rank;
};

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
    int fd;
	std::string name;
	int money;
	Card cards[2];
	bool playing;
	int round;
	int goodToGo;
};

class Connection 
{
public:
    int players_registered;

    Connection() {}

    void init(int argc, char *argv[], Player players[6]) 
    {
        int server_socket = create_socket();
        int connection_socket;
        int event_count;
        struct sockaddr_in client_address;
        int client_address_length = sizeof(client_address);

        players_registered = 0;

        bind_socket(server_socket, argc, argv);
        set_socket_to_listen(server_socket);

        int epollfd = create_epoll();

        struct epoll_event ev, events[MAX_EVENTS];

        ev.events = EPOLLIN;
        ev.data.fd = server_socket;

        if(epoll_ctl(epollfd, EPOLL_CTL_ADD, server_socket, &ev) == -1)
            error("ERROR, failed to add server_socket to the epoll.");

        while(players_registered < 6)
        {
            event_count = epoll_wait(epollfd, events, MAX_EVENTS, -1);

            if(event_count == -1)
                error("ERROR, epoll_wait failed\n");

            for(int i = 0; i < event_count; i++)
            {
                if(events[i].data.fd == server_socket)
                {
                    connection_socket = accept(server_socket, (struct sockaddr *) &client_address, (socklen_t *) &client_address_length);

                    if(connection_socket == -1)
                        perror("ERROR, failed to create connection socket!\n");

                    set_non_blocking(connection_socket);
                    ev.events = EPOLLIN | EPOLLET;
                    ev.data.fd = connection_socket;

                    if(epoll_ctl(epollfd, EPOLL_CTL_ADD, connection_socket, &ev) == -1)
                        error("ERROR, failed to add server_socket to the epoll.");
           
                    register_player(connection_socket, client_address, players);
                }
            }
        }

        send_message_to_all("res:RDY", 256, players);
    }

    void send_message(int socket, char message[], int length)
    {
        if(write(socket, message, length) < 0)
            error("Failed to send message!");
    }

    void send_message_to_all(char message[], int length, Player players[6]) 
    {
        for(int i = 0; i < 6; i++)
            write(players[i].fd, message, length);
    }

    int read_message(int socket, int length) 
    {

        char buffer[10];
        bzero(buffer, 10);

		while(!strstr(buffer, "1"))
			read(socket, buffer, 10);

		return std::atoi(buffer);
    }

private:
    int create_socket()
    {
        int new_socket;

        new_socket = socket(AF_INET, SOCK_STREAM, 0);

        if(new_socket < 0)
            error("ERROR, can't open new socket!\n");

        return new_socket;
    }

    void bind_socket(int socket, int argc, char *argv[])
    {

        int port;
        struct sockaddr_in server_address;

        if(argc < 2)
            error("ERROR, no port provided!\n");
        
        setsockopt(socket, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, (int *)1, sizeof(int)); //bind socket forcefully

        port = atoi(argv[1]); 

        bzero((char *) &server_address, sizeof(server_address));

        server_address.sin_family = AF_INET;
        server_address.sin_port = htons(port); //port as network byte order
        server_address.sin_addr.s_addr = INADDR_ANY; 

        if(bind(socket, (struct sockaddr *) &server_address, sizeof(server_address)) < 0)
            error("ERROR, failed to bind socket!");
    }

    void set_socket_to_listen(int socket)
    {
        if(listen(socket, 10) < 0)
            perror("ERROR, can't set socket to listening\n");
    }

    void set_non_blocking(int socket)
    {
        int status = fcntl(socket, F_SETFL, fcntl(socket, F_GETFL, 0) | O_NONBLOCK);

        if(status == -1)
        error("ERROR, calling fcntl.\n");
    }

    int create_epoll()
    {
        int epollfd = epoll_create(MAX_EVENTS);

        if(epollfd == -1)
            perror("ERROR, failed to create epoll!\b");

        return epollfd;
    }

    void register_player(int socket, struct sockaddr_in client_address, Player players[6])
    {
		std::stringstream id_response;
        players[players_registered].fd = socket;
        players[players_registered].name = "XD";
        std::cout << "Player: " << socket << ", IP: " << client_address.sin_addr.s_addr << " joins!" << std::endl;

		id_response << players_registered;

        send_message(socket, (char *) id_response.str().c_str(), 256);
        players_registered++;
    }
};

class Poker 
{
public:
    
    Poker(int argc, char *argv[]) 
    {
        connection.init(argc, argv, players);
        player_index = 4;
        players_count = 6;

        startGame();
    }

private:
    Player players[6];
    Connection connection;

    Deck deck1;
	Card tableCards[5];
	int pot, action, bet, raise, betOn, winner, maxPoints, roundWinner;
    int player_with_chip = 0;
	int handPoints[6];
	int bestHand[6][3];

    int player_index, players_count;

    static int compareCards(const void *card1, const void *card2)
    {
        return (*(Card *)card1).rank - (*(Card *)card2).rank;
    }
    void startGame()
	{
		int i = 0;
		int player_paying_blind = 0;

        set_players_money(1000);

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
            for(int i = 0; i < players_count; i++)
            {
                if(players[i].playing == 0)
                    connection.send_message(players[i].fd, "res:OVER", 256);
            }

			for (int i = player_with_chip; i < players_count + player_with_chip; i++)
			{
				if (players[i % players_count].playing)
				{
					player_with_chip = i % players_count;
					std::cout << "** " << players[player_with_chip].name << " Owns chip!" << " **" << std::endl;
					break;
				}
				else
					continue;
			}

			for (int i = player_with_chip + 1; i < players_count + player_with_chip; i++)
			{
				if (players[i % players_count].playing)
				{
					player_paying_blind = i % players_count;
					std::cout << "** " << players[player_paying_blind].name << " Pays blind!" << " **" << std::endl;
					break;
				}
				else
					continue;
			}

			/* paying blind */
			pot = 20;
			if (players[player_paying_blind].money >= 20)
				players[player_paying_blind].money -= 20;
			else
				players[player_paying_blind].playing = 0;

			//std::cout << "\n\n\n";
			//std::cout << "\t\t\t\t\t ------ ROUND " << i + 1 << " ------\n\n\n";
			deck1.shuffle();

			/* pre-flop */
			deal();
			send_table();
			takeBets();
			if (oneLeft())
			{
				winner = getWinner();
				players[winner].money += pot;
				std::cout << players[winner].name << " wins $" << pot << "\n\n";
				//printAllHands(winner);
				player_with_chip = (player_with_chip + 1) % players_count;
				i++;
				continue;
			}

			/* flop */
			flop();
			std::cout << std::endl;
			send_table();
			takeBets();
			if (oneLeft())
			{
				winner = getWinner();
				players[winner].money += pot;
				std::cout << players[winner].name << " wins $" << pot << "\n\n";
				//printAllHands(winner);
				player_with_chip = (player_with_chip + 1) % players_count;
				i++;
				continue;
			}

			/* turn */
			turn();
			std::cout << std::endl;
			send_table();
			takeBets();
			if (oneLeft())
			{
				winner = getWinner();
				players[winner].money += pot;
				std::cout << players[winner].name << " wins $" << pot << "\n\n";
				//printAllHands(winner);
				player_with_chip = (player_with_chip + 1) % players_count;
				i++;
				continue;
			}

			/* river */
			river();
			std::cout << std::endl;
			send_table();
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
			//printAllHands(roundWinner);

			players[roundWinner].money += pot;

			player_with_chip = (player_with_chip + 1) % players_count;
			i++;
		}
    }

    void set_players_money(int start_value)
	{
		for (int i = 0; i < 6; i++)
		{
			players[i].money = start_value;
			players[i].playing = true;
		}
    }

    int playersLeft()
	{
		int count = 0;
		for (int i = 0; i < players_count; i++)
			if (players[i].money > 0)
				count++;
		return count;
	}

    void deal()
	{
		for (int i = 0; i < players_count; i++)
		{
			for (int j = 0; j < 2; j++)
			{
				if (players[i].playing)
					players[i].cards[j] = deck1.hitme();
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

    int getWinner()
	{
		int winner;
		for (int k = 0; k < players_count; k++)
			if (players[k].round)
				winner = k;
		return winner;
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

		std::stringstream current_player;
		std::stringstream players_action;
		std::stringstream round_info;

		betOn = 0;
		for (int k = 0; k < players_count; k++)
			players[k].goodToGo = 0;


		for (int k = player_with_chip; k < player_with_chip + players_count; k++)
		{
			current_player.str(std::string());
			current_player.clear();

			players_action.str(std::string());
			players_action.clear();

			round_info.str(std::string());
			round_info.clear();

			current_player << "res:" << k << "goes";

			connection.send_message_to_all((char *) current_player.str().c_str(), 256, players);

				if (betOn)
				{
					if (players[k].money >= betOn)
					{
						players_action << "\t\t\t\t\tYour action: (1) FOLD (3) BET/CALL (4) RRRRRAISE? ";
						connection.send_message(players[k].fd, (char* )players_action.str().c_str(), 256);
					}
					else
					{
						players_action << "\t\t\t\t\tYour action: (1) FOLD";
						connection.send_message(players[k].fd, (char* )players_action.str().c_str(), 256);
					}
					
					action = connection.read_message(players[k].fd, 1);

					if (players[k].money >= betOn)
					{
						while (action != FOLD && action != BET_or_CALL && action != RAISE)
						{
							cout << "Invalid number pressed." << endl;
							cout << "\t\t\t\t\tYour action: (1) FOLD (3) BET/CALL (4) RRRRRAISE?";
							cin >> action;
						}
					}
					else
					{
						while (action != FOLD)
						{
							cout << "Invalid number pressed." << endl;
							cout << "\t\t\t\t\tYour action: (1) FOLD";
							cin >> action;
						}
					}
				}
				else
				{
					if (players[k].money > 0)
					{
						players_action << "\t\t\t\t\tYour action: (1) FOLD (2) CHECK (3) BET/CALL ";
						connection.send_message(players[k].fd, (char* )players_action.str().c_str(), 256);
					}
					else
					{
						players_action << "\t\t\t\t\tYour action: (1) FOLD (2) CHECK ";
						connection.send_message(players[k].fd, (char* )players_action.str().c_str(), 256);
					}
						
					action = connection.read_message(players[k].fd, 1);


					if (players[k].money > 0)
					{
						while (action != FOLD && action != CHECK && action != BET_or_CALL)
						{
							cout << "Invalid number pressed." << endl;
							cout << "\t\t\t\t\tYour action: (1) FOLD (2) CHECK (3) BET/CALL ";
							cin >> action;
						}
					}
					else
					{
						while (action != FOLD && action != CHECK)
						{
							cout << "Invalid number pressed." << endl;
							cout << "\t\t\t\t\tYour action: (1) FOLD (2) CHECK ";
							cin >> action;
						}
					}
				}

				cout << endl;

				if (action == FOLD)
				{
					players[k].round = 0;
					round_info << "\t- " << players[k].name << " folds...\n";
					connection.send_message_to_all((char *) round_info.str().c_str(), 256, players);
				}
				else if (action == CHECK)
				{
					round_info << "\t+ " << players[k].name << " checks.\n";
					connection.send_message_to_all((char *) round_info.str().c_str(), 256, players);
					continue;
				}
				else if (action == BET_or_CALL)
				{
					if (betOn)
					{
						pot += betOn;
						players[k].money -= betOn;
						players[k].goodToGo = 1;
						round_info << "\t+ " << players[k].name << " bets " << betOn << "$\n";
						connection.send_message_to_all((char *) round_info.str().c_str(), 256, players);
					}
					else
					{
						players_action << "How much do you want to bet: ";
						connection.send_message(players[k].fd, (char *) players_action.str().c_str(), 256);

						bet = connection.read_message(players[k].fd, 1);

						while (bet > players[k].money || bet < 1)
						{
							cout << "Invalid number to bet." << endl;
							cout << "How much do you want to bet: ";
							cin >> bet;
							cout << endl << endl;
						}
						pot += bet;
						players[k].money -= bet;
						betOn = bet;
						players[k].goodToGo = 1;

						round_info << "\t+ " << players[k].name << " bets " << bet << "$\n";
						connection.send_message_to_all((char *) round_info.str().c_str(), 256, players);
					}
				}
				else
				{
					players_action << "How much do you want to raise: ";
					raise = connection.read_message(players[k].fd, 1);
					while (raise > players[k].money - betOn || raise <= betOn || raise <= 0)
					{
						cout << "Invalid number to raise." << endl;
						cout << "How much do you want to raise: ";
						cin >> raise;
						cout << endl << endl;
					}

					pot += raise;
					betOn += raise;
					players[k].money -= betOn;
					players[k].goodToGo = 1;

					round_info << "\t+ " << players[k].name << " r-r-r-r-raises! " << raise << "$\n";
					connection.send_message_to_all((char *) round_info.str().c_str(), 256, players);
				}			

		if (betOn && playersToBet())
		{
			for (int k = player_with_chip + 1; k < player_with_chip + 7; k++)
			{
				if (k % players_count == 4)
				{
					if (players[k].round && players[k].goodToGo == 0)
					{
						cout << "\t\t\t\t\tYour action: (1) FOLD (3) BET/CALL ";
						cin >> action;
						while (action != FOLD && action != BET_or_CALL)
						{
							cout << "Invalid number pressed." << endl;
							cout << "\t\t\t\t\tYour action: (1) FOLD (3) BET/CALL ";
							cin >> action;
							cout << endl << endl;
						}
						if (action == FOLD)
						{
							cout << "\t- " << players[k].name << " folds...\n";
							players[k].round = 0;
						}
						else
						{
							pot += betOn;
							players[k].money -= betOn;
							players[k].goodToGo = 1;

							cout << "\t+ " << players[k].name << " bets " << betOn << "$\n";
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
						cout << "\t- " << players[k % players_count].name << " folds..." << endl;
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
	}

	std::stringstream displayCards(Card cards[], int size)
	{
		using namespace std;
		int numberOfSpaces = 0;

		stringstream display;

		//**DISPLAY UP**
		display << "      ";
		for (int i = 0; i < size; i++)
		{
			if (cards[i].rank >= 0)
				numberOfSpaces = suits[cards[i].suit].length() + 2;

			display << ((cards[i].rank) >= 0 ? drawSign(numberOfSpaces, '_') : "___");

			display << "   ";
		}
		display << endl;
		//--------------

		//**DISPLAY RANKS**
		display << "     ";
		for (int i = 0; i < size; i++)
		{
			if (cards[i].rank >= 0)
				numberOfSpaces = suits[cards[i].suit].length() - ranks[cards[i].rank].length();
			display << "| " << ((cards[i].rank) >= 0 ? ranks[cards[i].rank] : " ") << ((cards[i].rank) >= 0 ? drawSign(numberOfSpaces, ' ') : "") << " | ";
		}
		display << endl;
		//---------------

		//**DISPLAY SUITS**
		display << "     ";
		for (int i = 0; i < size; i++)
		{
			display << "| " << ((cards[i].rank) >= 0 ? suits[cards[i].suit] : " ") << " | ";
		}
		display << endl;
		//---------------

		//**DISPLAY BOTTOM**
		display << "     ";
		for (int i = 0; i < size; i++)
		{
			if (cards[i].rank >= 0)
				numberOfSpaces = suits[cards[i].suit].length() + 2;

			display << "|" << ((cards[i].rank) >= 0 ? drawSign(numberOfSpaces, '_') : "___") << "| ";
		}
		display << endl;
		//---------------

		return display;
	}

	void send_table()
	{
		using std::cout;
		using std::endl;
		using std::setw;

		std::stringstream gui;

		int upperHalfOfPlayers = ceil((float)players_count / 2);

		gui << "---------------------------------------------------------------------------------------------" << endl;
		//--PLAYERS NAMES 1-3--
		gui << "     ";
		for (int i = 0; i < upperHalfOfPlayers; i++)
			gui << std::left << setw(16) << ((players[i].playing) ? (players[i].name) : "      ");

		gui << endl;
		//-----------------

		//--PLAYERS MONEY 1-3--
		gui << "     ";
		for (int i = 0; i < upperHalfOfPlayers; i++)
			gui << std::left << "$" << setw(15) << ((players[i].playing) ? (players[i].money) : 0);
		gui << endl;
		//-----------------

		//----TABLE-TOP----
		int tableLength = 0;

		tableLength += upperHalfOfPlayers * 16;

		gui << "    " << drawSign(tableLength, '_') << endl;
		//-----------------

		//--MOVING CHIP--
		gui << "   /";
		for (int i = 0; i < upperHalfOfPlayers; i++)
			gui << ((player_with_chip == i) ? " @              " : "                ");
		gui << "\\" << endl;
		//---------------

		//----CARDS----
		gui << displayCards(tableCards, 5).str();
		//--------------

		//--POT--
		int potMargin = (tableLength - 11) / 2;
		gui << "     " << drawSign(potMargin, ' ') << "Pot = $" << setw(4) << pot << drawSign(potMargin, ' ') << "  " << endl;
		//--------------

		//--MOVING CHIP--
		gui << "   \\";
		for (int i = players_count - 1; i > upperHalfOfPlayers - 1; i--)
			gui << ((player_with_chip == i) ? " @              " : "                ");
		gui << "/" << endl;
		//--------------

		//----TABLE-BOTTOM----
		gui << "    \\" << drawSign(tableLength - 1, '_') << "/" << endl;
		//-----------------
		gui << endl;

		//--PLAYER NAMES4-6--
		gui << "     ";
		for (int i = players_count - 1; i > upperHalfOfPlayers - 1; i--)
			gui << std::left << setw(16) << ((players[i].playing) ? (players[i].name) : "      ");

		gui << endl;
		//-----------------

		//--PLAYERS MONEY 4-6--
		gui << "     ";
		for (int i = players_count - 1; i > upperHalfOfPlayers - 1; i--)
			gui << std::left << "$" << setw(15) << ((players[i].playing) ? (players[i].money) : 0);

		gui << endl << endl;
		//-----------------

		connection.send_message_to_all((char *)gui.str().c_str(), 2048, players);


		std::stringstream players_hand;

		for(int i = 0; i < players_count; i++)
		{
			players_hand.str(std::string());
			players_hand.clear();
			if(players[i].round)
			{
				players_hand <<  " Your hand:" << endl << displayCards(players[i].cards, 2).str() << endl;
				connection.send_message(players[i].fd, (char *) players_hand.str().c_str(), 2048);
			}
		}
	}
};


int main(int argc, char *argv[])
{
    Poker *poker = new Poker(argc, argv);

    return 0;
}