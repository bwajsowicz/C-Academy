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

#include <math.h>
#include <iomanip>

#define MAX_EVENTS 10

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

class Card
{
public:
	int suit;
	int rank;
};

class Deck
{

public:
	Deck() 
	{
		initializeDeck();
	}

	void shuffle()
	{
		top_ = cardTally_ - 1;

		initializeDeck();

		int x;
		Card tempCard;
		for (int i = 0; i < cardTally_; i++)
		{
			x = rand() % cardTally_;
			tempCard = cards[i];
			cards[i] = cards[x];
			cards[x] = tempCard;
		}
	}

	Card hitme()
	{
		top_--;
		return cards[top_ + 1];
	}

private:
	int top_;
	static const int cardTally_ = 52;

	Card cards[cardTally_];

	void initializeDeck()
	{
		int nSuits = 4;
		int nRanks = 13;

		for (int i = 0; i < nSuits; i++)
			for (int j = 0; j < nRanks; j++)
			{
				cards[i * nRanks + j].suit = i;
				cards[i * nRanks + j].rank = j;
			}
	}
};

class Player
{
public:
    int fd;
	std::string name;
	int money;
	Card cards[2];
	bool isPlaying;
	bool isInRound;
	bool isGoodToGo;
};

class Connection 
{
public:
    int playersRegistered;

    Connection() {}

    void Init(int argc, char *argv[], Player players[6]) 
    {
        int serverSocket = CreateSocket();
        int connectionSocket;
        int nEvents;
        struct sockaddr_in clientAddress;
        int clientAddressLength = sizeof(clientAddress);

        playersRegistered = 0;

        BindSocket(serverSocket, argc, argv);
        SetSocketToListen(serverSocket);

        int epollfd = CreateEpoll();

        struct epoll_event ev, events[MAX_EVENTS];

        ev.events = EPOLLIN;
        ev.data.fd = serverSocket;

        if(epoll_ctl(epollfd, EPOLL_CTL_ADD, serverSocket, &ev) == -1)
            error("ERROR, failed to add serverSocket to the epoll.");

        while(playersRegistered < 6)
        {
            nEvents = epoll_wait(epollfd, events, MAX_EVENTS, -1);

            if(nEvents == -1)
                error("ERROR, epoll_wait failed\n");

            for(int i = 0; i < nEvents; i++)
            {
                if(events[i].data.fd == serverSocket)
                {
                    connectionSocket = accept(serverSocket, (struct sockaddr *) &clientAddress, (socklen_t *) &clientAddressLength);

                    if(connectionSocket == -1)
                        perror("ERROR, failed to create connection socket!\n");

                    SetSocketNoBlocking(connectionSocket);
                    ev.events = EPOLLIN | EPOLLET;
                    ev.data.fd = connectionSocket;

                    if(epoll_ctl(epollfd, EPOLL_CTL_ADD, connectionSocket, &ev) == -1)
                        error("ERROR, failed to add serverSocket to the epoll.");
           
                    RegisterPlayer(connectionSocket, clientAddress, players);
                }
            }
        }

        SendMessageToAll("res:rdy", 256, players);
    }

    void SendMessage(int socket, char message[], int length)
    {
        if(write(socket, message, length) < 0)
            error("Failed to send message!");
    }

    void SendMessageToAll(char message[], int length, Player players[6]) 
    {
        for(int i = 0; i < 6; i++)
            write(players[i].fd, message, length);
    }

	void SendMessageToAllExceptSomeone(char message[], int length, Player players[6], int playerIndex)
	{
		for(int i = 0; i < 6; i++)
		{	
			if(i != playerIndex)
				write(players[i].fd, message, length);
			else	
				continue;
		}
            
	}

    int ReadMessage(int socket, int length) 
    {
        char buffer[10];
        bzero(buffer, 10);

		while(buffer[0] == '\0')
			read(socket, buffer, 10);

		return std::atoi(buffer);
    }

private:
    int CreateSocket()
    {
        int newSocket;

        newSocket = socket(AF_INET, SOCK_STREAM, 0);

        if(newSocket < 0)
            error("ERROR, can't open new socket!\n");

        return newSocket;
    }

    void BindSocket(int socket, int argc, char *argv[])
    {

        int port;
        struct sockaddr_in serverAddress;

        if(argc < 2)
            error("ERROR, no port provided!\n");
        
        setsockopt(socket, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, (int *)1, sizeof(int)); //bind socket forcefully

        port = atoi(argv[1]); 

        bzero((char *) &serverAddress, sizeof(serverAddress));

        serverAddress.sin_family = AF_INET;
        serverAddress.sin_port = htons(port); //port as network byte order
        serverAddress.sin_addr.s_addr = INADDR_ANY; 

        if(bind(socket, (struct sockaddr *) &serverAddress, sizeof(serverAddress)) < 0)
            error("ERROR, failed to bind socket!");
    }

    void SetSocketToListen(int socket)
    {
        if(listen(socket, 10) < 0)
            perror("ERROR, can't set socket to listening\n");
    }

    void SetSocketNoBlocking(int socket)
    {
        int status = fcntl(socket, F_SETFL, fcntl(socket, F_GETFL, 0) | O_NONBLOCK);

        if(status == -1)
        error("ERROR, calling fcntl.\n");
    }

    int CreateEpoll()
    {
        int epollfd = epoll_create(MAX_EVENTS);

        if(epollfd == -1)
            perror("ERROR, failed to create epoll!\b");

        return epollfd;
    }

	//TODO: get players na from client
    void RegisterPlayer(int socket, struct sockaddr_in clientAddress, Player players[6])
    {
		std::stringstream id_response;
        players[playersRegistered].fd = socket;
        players[playersRegistered].name = "XD";
        std::cout << "Player: " << socket << ", IP: " << clientAddress.sin_addr.s_addr << " joins!" << std::endl;

		id_response << playersRegistered;

        SendMessage(socket, (char *) id_response.str().c_str(), 10);
        playersRegistered++;
    }
};

class Poker 
{
public:
	static const std::string suits[4];
	static const std::string ranks[13];

    Poker(int argc, char *argv[]) 
    {
        connection.Init(argc, argv, players);
		nPlayers = 6;

        StartGame();
    }

private:
    Player players[6];
    Connection connection;

    Deck deck;
	Card tableCards[5];
	int pot, action, bet, raise, betOn, winner, maxPoints, roundWinner;
	int handPoints[6];
	int bestHand[6][3];

    int nPlayers;

    static int CompareCards(const void *card1, const void *card2)
    {
        return (*(Card *)card1).rank - (*(Card *)card2).rank;
    }

	void SetPlayersDefaultValues()
	{
		for (int i = 0; i < nPlayers; i++)
			if (players[i].money < 1)
			{
				players[i].isPlaying = 0;
				players[i].isInRound = 0;
			}

		for (int i = 0; i < nPlayers; i++)
		{
			if (players[i].isPlaying)
				players[i].isInRound = 1;
			handPoints[i] = -1;
		}

		for (int i = 0; i < nPlayers; i++)
			for (int j = 0; j < 3; j++)
				bestHand[i][j] = -1;
	}

	void CheckForGameOver()
	{
		for(int i = 0; i < nPlayers; i++)
            if(players[i].isPlaying == 0)
          		connection.SendMessage(players[i].fd, "res:OVER", 256);
	}

	int GetPlayerWithDealerButton()
	{
		int playerWithDealerButton;
		for (int i = playerWithDealerButton; i < nPlayers + playerWithDealerButton; i++)
		{
			if (players[i % nPlayers].isPlaying)
			{
				playerWithDealerButton = i % nPlayers;
				std::cout << "** " << players[playerWithDealerButton].name << " Owns chip!" << " **" << std::endl;
				break;
			}
			else
				continue;
		}

		return playerWithDealerButton;
	}

	int GetPlayerPayingBlind(int playerWithDealerButton)
	{
		int playerPayingBlind;

		for (int i = playerWithDealerButton + 1; i < nPlayers + playerWithDealerButton; i++)
		{
			if (players[i % nPlayers].isPlaying)
			{
				playerPayingBlind = i % nPlayers;
				std::cout << "** " << players[playerPayingBlind].name << " Pays blind!" << " **" << std::endl;
				break;
			}
			else
				continue;
		}

		return playerPayingBlind;
	}

    void StartGame()
	{
		int i = 0;
		int playerPayingBlind;
		int playerWithDealerButton;

        SetPlayersMoney(1000);

		while (PlayersLeft() > 1)
		{

			SetPlayersDefaultValues();		
			CheckForGameOver();

			playerWithDealerButton = GetPlayerWithDealerButton();
			playerPayingBlind = GetPlayerPayingBlind(playerWithDealerButton);

			//std::cout << "\n\n\n";
			//std::cout << "\t\t\t\t\t ------ ROUND " << i + 1 << " ------\n\n\n";
			connection.SendMessageToAll("res:roundstarts", 256, players);
			deck.shuffle();

			/* pre-flop */
			connection.SendMessageToAll("res:preflop()", 256, players);

			Deal();
			SendTable(playerWithDealerButton);
			TakeBets(playerWithDealerButton);

			connection.SendMessageToAll("res:stageended", 256, players);

			if (OneLeft())
			{
				winner = GetWinner();
				players[winner].money += pot;
				std::cout << players[winner].name << " wins $" << pot << "\n\n";
				//printAllHands(winner);
				playerWithDealerButton = (playerWithDealerButton + 1) % nPlayers;
				i++;
				continue;
			}

			/* flop */
			connection.SendMessageToAll("res:flop", 256, players);

			Flop();
			std::cout << std::endl;
			SendTable(playerWithDealerButton);
			TakeBets(playerWithDealerButton);

			connection.SendMessageToAll("res:stageended", 256, players);

			if (OneLeft())
			{
				winner = GetWinner();
				players[winner].money += pot;
				std::cout << players[winner].name << " wins $" << pot << "\n\n";
				//printAllHands(winner);
				playerWithDealerButton = (playerWithDealerButton + 1) % nPlayers;
				i++;
				continue;
			}

			/* turn */
			connection.SendMessageToAll("res:turn", 256, players);

			Turn;
			std::cout << std::endl;
			SendTable(playerWithDealerButton);
			TakeBets(playerWithDealerButton);

			connection.SendMessageToAll("res:stageended", 256, players);

			if (OneLeft())
			{
				winner = GetWinner();
				players[winner].money += pot;
				std::cout << players[winner].name << " wins $" << pot << "\n\n";
				//printAllHands(winner);
				playerWithDealerButton = (playerWithDealerButton + 1) % nPlayers;
				i++;
				continue;
			}

			/* River */
			connection.SendMessageToAll("res:River", 256, players);

			River();
			std::cout << std::endl;
			SendTable(playerWithDealerButton);
			TakeBets(playerWithDealerButton);

			EvaluateHands();

			connection.SendMessageToAll("res:roundended", 256, players);

			/* find and declare round winner */
			maxPoints = 0;
			for (int q = 0; q < nPlayers; q++)
			{
				if (players[q].isInRound)
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

			playerWithDealerButton = (playerWithDealerButton + 1) % nPlayers;
			i++;
		}
    }

    void SetPlayersMoney(int start_value)
	{
		for (int i = 0; i < 6; i++)
		{
			players[i].money = start_value;
			players[i].isPlaying = true;
		}
    }

    int PlayersLeft()
	{
		int nPlayersLeft = 0;
		for (int i = 0; i < nPlayers; i++)
			if (players[i].money > 0)
				nPlayersLeft++;

		return nPlayersLeft;
	}

    void Deal()
	{
		for (int i = 0; i < nPlayers; i++)
		{
			for (int j = 0; j < 2; j++)
			{
				if (players[i].isPlaying)
					players[i].cards[j] = deck.hitme();
			}
		}
		for (int i = 0; i < 5; i++)
			tableCards[i].rank = -1;
	}

	void Flop()
	{
		for (int i = 0; i < 3; i++)
			tableCards[i] = deck.hitme();
	}

	void Turn()
	{
		tableCards[3] = deck.hitme();
	}

	void River()
	{
		tableCards[4] = deck.hitme();
	}

    int GetWinner()
	{
		int winner;
		for (int i = 0; i < nPlayers; i++)
			if (players[i].isInRound)
				winner = i;

		return winner;
	}

    void EvaluateHands()
	{
		int stack[10], k;
		int currentPoints;

		for (int i = 0; i < nPlayers; i++)
		{
			if (players[i].isInRound)
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
						currentPoints = TryHand(stack, i);
						if (currentPoints > handPoints[i])
						{
							handPoints[i] = currentPoints;
							for (int x = 0; x < 3; x++)
								bestHand[i][x] = stack[x + 1];
						}
					}
				}

			}
		}
	} 

    int GetScore(Card hand[])
	{
		qsort(hand, 5, sizeof(Card), CompareCards);
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

	int TryHand(int array[], int player)
	{
		Card hand[5];

		/* get cards from table and player */
		for (int i = 1; i < 4; i++)
			hand[i - 1] = tableCards[array[i]];

		for (int i = 0; i < 2; i++)
			hand[i + 3] = players[player].cards[i];

		return GetScore(hand);
	}

    bool OneLeft()
	{
		int nPlayersLeft = 0;
		for (int i = 0; i < nPlayers; i++)
			if (players[i].isInRound)
				nPlayersLeft++;

		if (nPlayersLeft == 1)
			return true;
		else
			return false;
	}

    bool PlayersToBet()
	{
		for (int i = 0; i < nPlayers; i++)
			if (players[i].isInRound == 1 && players[i].isGoodToGo == 0)
				return true;

		return false;
	}

    void TakeBets(int playerWithDealerButton)
	{
		using std::cout;
		using std::cin;
		using std::endl;

		std::stringstream currentPlayer;
		std::stringstream playerActionOptions;
		std::stringstream playerAction;

		betOn = 0;
		for (int k = 0; k < nPlayers; k++)
			players[k].isGoodToGo = 0;


		for (int k = playerWithDealerButton; k < playerWithDealerButton + nPlayers; k++)
		{
			currentPlayer.str(std::string());
			currentPlayer.clear();

			playerActionOptions.str(std::string());
			playerActionOptions.clear();

			playerAction.str(std::string());
			playerAction.clear();

			currentPlayer << "res:" << k << "goes";

			connection.SendMessageToAll((char *) currentPlayer.str().c_str(), 256, players);

				if (betOn)
				{
					if (players[k].money >= betOn)
					{
						playerActionOptions << "\t\t\t\t\tYour action: (1) FOLD (3) BET/CALL (4) RRRRRAISE? ";
						connection.SendMessage(players[k].fd, (char* )playerActionOptions.str().c_str(), 256);
					}
					else
					{
						playerActionOptions << "\t\t\t\t\tYour action: (1) FOLD";
						connection.SendMessage(players[k].fd, (char* )playerActionOptions.str().c_str(), 256);
					}
					
					action = connection.ReadMessage(players[k].fd, 10);

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
						playerActionOptions << "\t\t\t\t\tYour action: (1) FOLD (2) CHECK (3) BET/CALL ";
						connection.SendMessage(players[k].fd, (char* )playerActionOptions.str().c_str(), 256);
					}
					else
					{
						playerActionOptions << "\t\t\t\t\tYour action: (1) FOLD (2) CHECK ";
						connection.SendMessage(players[k].fd, (char* )playerActionOptions.str().c_str(), 256);
					}
						
					action = connection.ReadMessage(players[k].fd, 1);


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
					players[k].isInRound = 0;
					playerAction << "\t- " << players[k].name << " folds...\n";
					connection.SendMessageToAllExceptSomeone((char *) playerAction.str().c_str(), 256, players, k);
				}
				else if (action == CHECK)
				{
					playerAction << "\t+ " << players[k].name << " checks.\n";
					connection.SendMessageToAllExceptSomeone((char *) playerAction.str().c_str(), 256, players, k);
					continue;
				}
				else if (action == BET_or_CALL)
				{
					if (betOn)
					{
						playerActionOptions.str(std::string());
						playerActionOptions.clear();
						playerActionOptions << "call";
						connection.SendMessage(players[k].fd, (char *) playerActionOptions.str().c_str(), 256);
						
						pot += betOn;
						players[k].money -= betOn;
						players[k].isGoodToGo = 1;
						playerAction << "\t+ " << players[k].name << " bets " << betOn << "$\n";
						connection.SendMessageToAllExceptSomeone((char *) playerAction.str().c_str(), 256, players, k);
					}
					else
					{
						playerActionOptions.str(std::string());
						playerActionOptions.clear();
						playerActionOptions << "bet";
						connection.SendMessage(players[k].fd, (char *) playerActionOptions.str().c_str(), 256);

						bet = connection.ReadMessage(players[k].fd, 1);

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
						players[k].isGoodToGo = 1;

						playerAction << "\t+ " << players[k].name << " bets " << bet << "$\n";
						connection.SendMessageToAllExceptSomeone((char *) playerAction.str().c_str(), 256, players, k);
					}
				}
				else
				{
					playerActionOptions << "How much do you want to raise: ";
					raise = connection.ReadMessage(players[k].fd, 1);
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
					players[k].isGoodToGo = 1;

					playerAction << "\t+ " << players[k].name << " r-r-r-r-raises! " << raise << "$\n";
					connection.SendMessageToAllExceptSomeone((char *) playerAction.str().c_str(), 256, players, k);
				}			

		
		}

		if (betOn && PlayersToBet())
		{
			for (int k = playerWithDealerButton + 1; k < playerWithDealerButton + 7; k++)
			{
				if (k % nPlayers == 4)
				{
					if (players[k].isInRound && players[k].isGoodToGo == 0)
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
							players[k].isInRound = 0;
						}
						else
						{
							pot += betOn;
							players[k].money -= betOn;
							players[k].isGoodToGo = 1;

							cout << "\t+ " << players[k].name << " bets " << betOn << "$\n";
						}
					}
				}

				else
				{
					if (players[k % nPlayers].isInRound == 0 || players[k % nPlayers].isGoodToGo == 1)
						continue;
					action = rand() % 2;
					if (action == 0 || players[k % nPlayers].money < betOn)
					{
						players[k % nPlayers].isInRound = 0;
						cout << "\t- " << players[k % nPlayers].name << " folds..." << endl;
					}
					else
					{
						pot += betOn;
						players[k % nPlayers].money -= betOn;
						cout << "\t++ " << players[k % nPlayers].name << " calls!" << endl;
						players[k % nPlayers].isGoodToGo = 1;
					}
				}
			}
		}
	}

	std::stringstream DisplayCards(Card cards[], int size)
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

	void SendTable(int playerWithDealerButton)
	{
		using std::cout;
		using std::endl;
		using std::setw;

		std::stringstream gui;

		int upperHalfOfPlayers = ceil((float)nPlayers / 2);

		gui << "---------------------------------------------------------------------------------------------" << endl;
		//--PLAYERS NAMES 1-3--
		gui << "     ";
		for (int i = 0; i < upperHalfOfPlayers; i++)
			gui << std::left << setw(16) << ((players[i].isPlaying) ? (players[i].name) : "      ");

		gui << endl;
		//-----------------

		//--PLAYERS MONEY 1-3--
		gui << "     ";
		for (int i = 0; i < upperHalfOfPlayers; i++)
			gui << std::left << "$" << setw(15) << ((players[i].isPlaying) ? (players[i].money) : 0);
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
			gui << ((playerWithDealerButton == i) ? " @              " : "                ");
		gui << "\\" << endl;
		//---------------

		//----CARDS----
		gui << DisplayCards(tableCards, 5).str();
		//--------------

		//--POT--
		int potMargin = (tableLength - 11) / 2;
		gui << "     " << drawSign(potMargin, ' ') << "Pot = $" << setw(4) << pot << drawSign(potMargin, ' ') << "  " << endl;
		//--------------

		//--MOVING CHIP--
		gui << "   \\";
		for (int i = nPlayers - 1; i > upperHalfOfPlayers - 1; i--)
			gui << ((playerWithDealerButton == i) ? " @              " : "                ");
		gui << "/" << endl;
		//--------------

		//----TABLE-BOTTOM----
		gui << "    \\" << drawSign(tableLength - 1, '_') << "/" << endl;
		//-----------------
		gui << endl;

		//--PLAYER NAMES4-6--
		gui << "     ";
		for (int i = nPlayers - 1; i > upperHalfOfPlayers - 1; i--)
			gui << std::left << setw(16) << ((players[i].isPlaying) ? (players[i].name) : "      ");

		gui << endl;
		//-----------------

		//--PLAYERS MONEY 4-6--
		gui << "     ";
		for (int i = nPlayers - 1; i > upperHalfOfPlayers - 1; i--)
			gui << std::left << "$" << setw(15) << ((players[i].isPlaying) ? (players[i].money) : 0);

		gui << endl << endl;
		//-----------------

		connection.SendMessageToAll((char *)gui.str().c_str(), 2048, players);


		std::stringstream players_hand;

		for(int i = 0; i < nPlayers; i++)
		{
			players_hand.str(std::string());
			players_hand.clear();
			if(players[i].isPlaying)
			{
				players_hand <<  " Your hand:" << endl << DisplayCards(players[i].cards, 2).str() << endl;
				connection.SendMessage(players[i].fd, (char *) players_hand.str().c_str(), 2048);
			}
		}
	}
};

const std::string Poker::suits[] = {"Diamonds", "Spades", "Hearts", "Clubs"};
const std::string Poker::ranks[] = {"2", "3", "4", "5", "6", "7", "8", "9", "Ten", "Joker", "Queen", "King", "Ace"};

int main(int argc, char *argv[])
{
    Poker *poker = new Poker(argc, argv);

    return 0;
}