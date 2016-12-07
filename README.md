# TAK Game Engine

## The Game
TAK is a strategy game similar to chess and checker is the sense that the board is square and has alternate dark and light squares. The rules of the game are quite simple and can be found at this [link](http://wargamespass.com/sites/default/files/TakBetaRules3-10-16.pdf). In short the objective of the game is to make a road from one end of the board to the other end which consists of only your colored pieces placed adjacent to each other. Another way to win is to have control over more pieces of the board in the case that any one player runs out of pieces.

## Objective
Our objective was to make a game engine which would skillfully play the game of TAK. The game engine required many differnt algorithm and search techniques and was in the end able to play at a **GrandMaster** level. Some of the algorithmic methods we implemented are discussed  below.

## Search
Any game engine must implement a good search strategy. In our case we used **Optimised Alpha-Beta Pruning** to conduct our search. It was opptimised because at each level of the search we sorted the moves based on an evaluation function so that it would lead to better pruning. Using this search approach we were able to achieve a search till a depth of 6 plys in a couple of seconds. 

## Evaluation
Pruning was made more effective by improving our **Evaluation Function**. This function would take the board state as an input and return a value pertaining to the strength of the players position on the board. A positive and high return value means that the player is in a strong commanding spot and vice-à-versa. The features that were a part of our evaluation function are listed below.  
+ **Influence** - Influence refers to the power that a certain piece has on its neighborhood. By keeping a high influence value we were able to keep our placed pieces closer together rather than scattered among the board.  
+ **Captured** - A tally of how many squares are captured by the player and his opponent. The differnce between them was analysed to see dominance over the number of captured pieces.  
+ **Composition** - Each row and column was analysed to see the nummber of captured squares of the player and the opponent. A greater number of captured pieces in a row/column meant an exponentially better reward.  
+ **Center Influence** - We noticed that the capstone did significantly better when placed closer to the centre of the board and resultingly we made a feature which would award more points if the capstone was closer to the centre of the board.  
+ **Piece Value** - We wanted the engine to play more pieces rather than capture for the same gain in evaluation. To counter this we added a parameter for piece value where negative points were given for each unused piece of a player. Hence it was more advantageous for a player to place a piece on the board.  
+ **Wall Disadvantage** - We kept a parameter to measure the disadvantage in a certain position due to a wall. A player's own wall would cause them a certain amount of disadvantage and an opponent's wall would cause a larger amount of disadvantage.  

## Learning
The weights of each feature were learned using a **Learning Script**. The learning script `learn.sh` allowed the game engine to play against itself but with slightly modified values of a certain specified feature. Based on the winner we were able to nudge the weight in the direction of the winning value. Continuing the process of learning for a long period of time would tune the weight close to its ideal value. Hence one by one all the weights were eventually tuned as the bot played better and better versions of itself.

## How to run
1) Compile the code using `compile.sh`.  
2) Set up the server using the command `python server.py <port no.> -n <n> -TL <time limit>`. Here **n** (default 5) is the board size and **time_limit** (default 120) will set the game time limit in seconds for each player working similar to a chess clock.  
3) Set up two clients on two different computers with the command `python client.py <server ip> <server port> run.sh -n <n> -mode <MODE>`. Here **n** (default 5) is the board size, *same as that of the server* and **MODE** (default GUI) can be one of either GUI or CUI. GUI mode will allow the player to see the board UI and CUI mode will hide the Board UI. `run.sh` should invoke the game engine code located in `main.cpp`.  
4) The game will continue running till a winner is crowned.  
5) After the game is over all the python instances can be simultaneaously closed by running the script `kill_python.sh`.

## Authors
* [Nikhil Gupta](https://github.com/NikhilGupta1997)
* [Ayush Bhardwaj](https://github.com/Ayushbh)

Course Project under [**Prof. Mausam**](http://homes.cs.washington.edu/~mausam/)
