#include <iostream>
#include <stack>
#include <string>
#include <unordered_map>
#include <vector>
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <sstream>
#include <math.h>
#include <algorithm>
#include <stack>
#include <limits.h>
#include <time.h>
#include <queue>
#include <fstream>

using namespace std;

// This is the encoding of all the board pieces
#define my_flatstone 0
#define my_standstone 1
#define my_capstone 2
#define your_flatstone 3
#define your_standstone 4
#define your_capstone 5

// Some cout variables
static int best_called = 0;
static int last_heur_val = 0;

// Evaluation function variables
static double diff[8] = {0,35,75,120,170,235,285,400};
static float against_wall = 40, for_wall = 32;
static float center_weight = 5;
static float flat = 3;
static float wall = 2.98;
static float cap = 2.99;

// Define some global arrays (max board size assumed = 8)
static float infl[8][8];
static int neighbors[1000];
static int DFS_board[8][8];
static string all_moves[5000];

// Define some global variables
static int player_id;
static int board_size;
static int current_player;

// Clock variables
static float time_player = 0.0;
static clock_t begin_time;
static clock_t end_time;
static clock_t ids_start;
static clock_t ids_end;
static float time_threshold = 8;

// Global edge variables
static int top_edge = 0;
static int left_edge = 0;
static int bottom_edge = 0;
static int right_edge = 0;

// Global partition vector used to generate moves
vector< vector<int> > part_list;

// Used to sort the elements in a priority queue in decreasing order
struct Compare_min {
    bool operator()(pair<double,string> p1,pair<double,string> p2) {
        return p1.first<p2.first;
    }
};

// Used to sort the elements in a priority queue in decreasing order
struct Compare_max {
    bool operator()(pair<double,string> p1,pair<double,string> p2) {
        return p1.first>p2.first;
    }
};

// Depicts the state of any one square on the board
class state {
  public:   
    stack<int> state_stack; // Composition of pieces on the sqare
    int captured; // Which piece controls the state
    
    state() {
        captured = -1; // Initially square is not captured
    }
};

// The class player stores info of the current pieces left in hand
class Player {
  public:
    int no_flat;
    int capstone;
    int id; // For player id (1 or 2)

    void assign (int n) {
        if(n == 5) {
            this->no_flat=21;
            this->capstone=1;
        }
        else if(n == 6) {
            this->no_flat=30;
            this->capstone=1;   
        }
        else if(n == 7) {
            this->no_flat=40;
            this->capstone=1;
        }
    }
} cur_player, other_player; // Total two players (current player and opponent)

// This perform the input move and updates the Board state
void perform_move(string move, int id, state myBoard[8][8], int &crushed, int debug) {
    // Get coordinates of square on which move is performed
    int j = (int)(move[1]) - 96; 
    int i = (int)(move[2]) - 48;
    // If it is a 'place' move   
    if(!isdigit(move[0])) {
        assert(myBoard[i-1][j-1].captured == -1); // Failsafe
        
        // Get piece encoding
        int x;
        if(move[0] == 'F') 
            x = 0;
        else if(move[0] == 'S') 
            x = 1;
        else if(move[0] == 'C') 
            x = 2;
        if(id != player_id) 
            x = x+3;

        // Update board state
        myBoard[i-1][j-1].captured = x;
        myBoard[i-1][j-1].state_stack.push(x);

        if(debug) {
            cerr<<"The move is "<<move<<" x = "<<x<<endl;
            cerr<<"captured val = "<<myBoard[i-1][j-1].captured<<" top = "<<myBoard[i-1][j-1].state_stack.top()<<endl;
        }
        
        // Update piece count
        if(move[0] == 'F' || move[0] == 'S') {
            if(id == player_id)
                cur_player.no_flat--;       
            else  
                other_player.no_flat--;     
        }   
        else {   
            if(id == player_id)
                cur_player.capstone--;
            else 
                other_player.capstone--;
        } 
    }
    // If its not a 'place' move
    else {
        int no_picked = (int)move[0] - 48; // No of pieces picked
        stack<int> picked; // Stack of picked pieces

        for(int l = 0; l < no_picked; l++) {
            int top = myBoard[i-1][j-1].state_stack.top(); // Get top element of stack
            myBoard[i-1][j-1].state_stack.pop();
            picked.push(top);
        }

        // Update captured state of the square
        if(myBoard[i-1][j-1].state_stack.size() != 0)
            myBoard[i-1][j-1].captured = myBoard[i-1][j-1].state_stack.top();
        else
            myBoard[i-1][j-1].captured = -1; 

        vector<int> drop; // Vector containing the partition of stack during move
        for(int l = 4; l < move.length(); l++) {
            int num = (int)move[l] - 48; // get number from character encoding
            drop.push_back(num);
        }

        char dir = move[3]; // obtain direction of the move
        int mi, mj; // temp variables
        if(dir == '+') {
            mi = 1; mj = 0; // move down
        }
        else if(dir == '-') {
            mi = -1; mj = 0; // move up
        }
        else if(dir == '>') { 
            mi = 0; mj = 1; // move right
        }
        else {
            mi = 0; mj = -1; // move left
        }

        int w1, w2; // new position coordinates
        for(int k = 1; k <= drop.size(); k++) {
            w1 = i-1 + (k * mi); 
            w2 = j-1 + (k * mj);
            // Get stack at new state
            stack<int> new_stack(myBoard[w1][w2].state_stack);

            // Start dropping pieces
            for(int x1 = 1; x1 <= drop[k-1]; x1++) {
                int top = picked.top();
                picked.pop();
                // Check if wall is crushed
                if(drop[k-1] == 1 && myBoard[w1][w2].captured % 3 == 1) {
                    int old_wall = new_stack.top();
                    new_stack.pop();
                    new_stack.push(old_wall-1); // Convert wall to flatstone 
                    crushed = 1;  
                }
                new_stack.push(top);             
            }
            // Set new captured state of square
            myBoard[w1][w2].captured = new_stack.top();
            swap(new_stack, myBoard[w1][w2].state_stack); // Update square stack  
        } 
    }
}

// This undos the input move and updates the Board state
void undo_move(string move, int id,state gen_Board[8][8], int crushed) {
    // Get coordinates of square on which move is performed
    int j = (int)(move[1]) - 96;
    int i = (int)(move[2]) - 48;
    // If it is a 'place' move
    if(!isdigit(move[0])) {
        // Update board state
        gen_Board[i-1][j-1].captured = -1;
        gen_Board[i-1][j-1].state_stack.pop();
        
        // Update piece count
        if(move[0] == 'F' || move[0] == 'S') {
            if(id == player_id) 
                cur_player.no_flat++;       
            else 
                other_player.no_flat++;     
        }
        else {   
            if(id == player_id) 
                cur_player.capstone++;
            else 
                other_player.capstone++;
        }
    }  
    // If its not a 'place' move    
    else {   
        int no_picked = (int)move[0] - 48; // No of pieces picked
        int drops = move.length()-4; // The number of drops in the move
        int dropped[8]; // Array containing the size of each drop
        
        for(int l = 4; l < move.length(); l++) {
            int num = (int)move[l] -48;
            dropped[l-4] = num;
        }

        char dir = move[3]; // obtain direction of the move
        int mi, mj; // temp variables
        if(dir == '+') {
            mi = 1; mj = 0; // move down
        }
        else if(dir == '-') {
            mi = -1; mj = 0; // move up
        }
        else if(dir == '>') {
            mi = 0; mj = 1; // move right
        }
        else {
            mi = 0; mj = -1; // move left
        }

        stack<int> reverse_drop; // Stack of the picked up pieces
        int w1, w2; // new position coordinates
        for(int k = drops; k > 0; k--) {
            w1 = i-1 + (k * mi);
            w2 = j-1 + (k * mj);
            int pick_up = dropped[k-1]; // No. of pieces being picked up                   
            int captured = gen_Board[w1][w2].captured; // Maintains the current capturing piece
            // If there was a possiblity that a wall was crushed
            if(k == drops && gen_Board[w1][w2].state_stack.size() >= 2 && pick_up == 1 && captured % 3 == 2) {
                reverse_drop.push(captured);
                gen_Board[w1][w2].state_stack.pop();
                captured = gen_Board[w1][w2].state_stack.top();
                  
                // Check to see if a wall was crushed during the move
                if(captured % 3 == 0 && crushed == 1) {
                    int new_wall = captured;
                    gen_Board[w1][w2].state_stack.pop();
                    new_wall++; // Upgrade flatstone to wall
                    gen_Board[w1][w2].state_stack.push(new_wall); 
                    gen_Board[w1][w2].captured = new_wall;
                }
                else 
                    gen_Board[w1][w2].captured = captured;  
            }
            else {   
                for(int x1 = 1; x1 <= pick_up; x1++) { // Pick up pieces 
                    reverse_drop.push(gen_Board[w1][w2].state_stack.top());
                    gen_Board[w1][w2].state_stack.pop();    
                }
                // Update the captured state of square
                if(gen_Board[w1][w2].state_stack.size() == 0)
                    gen_Board[w1][w2].captured = -1;
                else
                    gen_Board[w1][w2].captured = gen_Board[w1][w2].state_stack.top();
            }
        }
        // Add the picked up pieces to the original stack where they were picked from
        while(reverse_drop.size() != 0) {
            int picking = reverse_drop.top();
            reverse_drop.pop();
            gen_Board[i-1][j-1].state_stack.push(picking);
        }
        gen_Board[i-1][j-1].captured = gen_Board[i-1][j-1].state_stack.top();       
    }
}

// This function helps create all possible paritions of a stack of size 'stack_size'
vector< vector<int> > partition(int stack_size) {
    vector< vector<int> > answer;
    if(stack_size <= 0) 
        return answer; // No partitions can be formed 
    if(stack_size == 1) {
        vector<int> temp1; 
        temp1.push_back(1);
        answer.push_back(temp1);
        return answer;
    }       
    for(int i = 1; i < stack_size; i++) {
        int j = stack_size-i;
        vector<int> temp2;
        vector<vector<int> > part = partition(j);
        for(int k = 0; k < part.size(); k++) {
            temp2 = part[k];
            temp2.insert(temp2.begin(),i);
            answer.push_back(temp2);
        }   
    }
    vector<int> temp3;
    temp3.push_back(stack_size);
    answer.push_back(temp3);
    return answer; // Vector containing vectors, each one which contains a possible partition of the stack 
}

// Obtains a List of unvisited neighbors around a given square on the board
int get_neighbors(int i, int j, int index) {
    int count = index; // This count keeps track of the current position in the global 'neighbors' array
    if(i != 0 && DFS_board[i-1][j] == 0) { // Check for neighbor above
        neighbors[count] = ((i-1)*board_size + j); // Encode a 2-D position into 1-D
        count++;
    }
    if(j != 0 && DFS_board[i][j-1] == 0) { // Check for neighbor to the left
        neighbors[count] = (i*board_size + j-1); // Encode a 2-D position into 1-D
        count++;
    }
    if(i != board_size-1 && DFS_board[i+1][j] == 0) { // Check for neighbor below
        neighbors[count] = ((i+1)*board_size + j); // Encode a 2-D position into 1-D
        count++;
    }
    if(j != board_size-1 && DFS_board[i][j+1] == 0) { // Check for neighbor to the right
        neighbors[count] = (i*board_size + j+1); // Encode a 2-D position into 1-D
        count++;
    }
    return count;
}

// DFS check to see if a road is formed or not
int DFS(int i, int j, int myboard[8][8],  int player_id, state gen_board[8][8], int neigh_index) {
    bool road = false; // Initialise to false
    int size = get_neighbors(i, j, neigh_index); // Keeps track of the position in the global 'neighbors' array
    int curr, curr_i, curr_j;
    for(int i = neigh_index; i < size; i++) {
        curr = neighbors[i];
        // Extract 2-D coordinates from 1-D encoding
        curr_i = curr / board_size;
        curr_j = curr % board_size;
        if(myboard[curr_i][curr_j] == 1) // If visited then ignore (failsafe)
            continue;
        myboard[curr_i][curr_j] = 1; // Mark as visited

        if(gen_board[curr_i][curr_j].captured == (0 + player_id * 3) || gen_board[curr_i][curr_j].captured == (2 + player_id * 3)) {
            // Marks if a certain edge is visited / connected
            if(curr_i == 0) 
                top_edge = 1;
            if(curr_i == board_size-1) 
                bottom_edge = 1;
            if(curr_j == 0) 
                left_edge = 1;
            if(curr_j == board_size-1) 
                right_edge = 1;    

            // Check to see if a path exists between two opposite edges   
            if(top_edge == 1 && bottom_edge == 1)
                return true;
            if(left_edge == 1 && right_edge == 1)
                return true;    
            road = DFS(curr_i, curr_j, myboard, player_id, gen_board, size);
            if(road)
                return road;
        }
    }
    return road;
}

// Re-initiallises any input array to zero
void reset_visited(int matrix[8][8]) {
    for(int i = 0; i < board_size; i++) {
        for(int j = 0; j < board_size; j++) {
            matrix[i][j] = 0;
        }
    }
}

bool flat_win(state gen_board[8][8], double &value) {
    float my_count = 0;
    float your_count = 0;
    int capt;
    bool flag = false;
    int cur_player_count = cur_player.no_flat;
    int other_player_count = other_player.no_flat;

    // Check to see if any player is out of flatstone
    if(cur_player_count == 0 || other_player_count == 0)
        flag = true;
    for(int i = 0; i < board_size; i++) {
        for(int j = 0; j < board_size; j++) { 
            capt = gen_board[i][j].captured;
            if(capt == -1 && !flag)
                return false;
            else if(capt == 0 || capt == 2)
                my_count++;
            else if(capt == 3 || capt == 5)
                your_count++;
        }
    }

    // Caluculate ratio of captured pieces
    if(my_count > your_count)
        value += (float)(my_count/(float)(my_count + your_count));
    else if(my_count < your_count)
        value -= (float)(your_count/(float)(my_count + your_count));
    else 
        value += 10/LONG_MAX;
    return true;
}

// Reinitialise the edge variables
void initialise_edges() {
    top_edge = 0;
    left_edge = 0;
    bottom_edge = 0;
    right_edge = 0;
}

// Checks to see if the Board is in an endstate position
double at_endstate(state gen_board[8][8], bool score, int move){
    bool road = false;
    reset_visited(DFS_board);
    if(move == 0) { // First check if you have a roadwin and then check for opponent
        // Check for your roadwin from left_edge to right_edge
        for(int i = 0; i < board_size; i++) {
            int capt_i = gen_board[i][0].captured;
            if((capt_i == 0 || capt_i == 2) && (DFS_board[i][0] == 0)) {
                DFS_board[i][0] = 1;
                if(i == 0) 
                    top_edge = 1;
                left_edge = 1;
                road = DFS(i, 0, DFS_board, 0, gen_board, 0);
                initialise_edges();
                if(road)
                    return 10.0;
            }
        }
        // Check for your roadwin from top_edge to bottom_edge
        for(int j = 1; j < board_size; j++) {
            int capt_j = gen_board[0][j].captured;
            if((capt_j == 0 || capt_j == 2) && (DFS_board[0][j] == 0)) {
                DFS_board[0][j] = 1;
                top_edge = 1;
                road = DFS(0, j, DFS_board, 0, gen_board, 0);
                initialise_edges();
                if(road)
                    return 10.0;
            }
        }
        reset_visited(DFS_board);

        // Check for opponent roadwin from left_edge to right_edge
        for(int i = 0; i < board_size; i++) {
            int capt_i = gen_board[i][0].captured;
            if((capt_i == 3 || capt_i == 5) && (DFS_board[i][0] == 0)) {
                DFS_board[i][0] = 1;
                if(i == 0) 
                    top_edge = 1;
                left_edge = 1;
                road = DFS(i, 0, DFS_board, 1, gen_board, 0);
                initialise_edges();
                if(road)
                    return -10.0;
            } 
        }
        // Check for opponent roadwin from top_edge to bottom_edge
        for(int j = 1; j < board_size; j++) {
            int capt_j = gen_board[0][j].captured;
            if((capt_j == 3 || capt_j == 5) && (DFS_board[0][j] == 0)) {
                DFS_board[0][j] = 1;
                top_edge = 1;
                road = DFS(0, j, DFS_board,  1, gen_board, 0);
                initialise_edges();
                if(road)
                    return -10.0;
            }    
        }
    }
    else { // First check if opponent has roadwin and then check for yours
        // Check for opponent roadwin from left_edge to right_edge
        for(int i = 0; i < board_size; i++) {
            int capt_i = gen_board[i][0].captured;
            if((capt_i == 3 || capt_i == 5) && (DFS_board[i][0] == 0)) {
                DFS_board[i][0] = 1;
                if(i == 0) 
                    top_edge = 1;
                left_edge = 1;
                road = DFS(i, 0, DFS_board, 1, gen_board, 0);
                initialise_edges();
                if(road)
                    return -10.0;
            } 
        }
        // Check for opponent roadwin from top_edge to bottom_edge
        for(int j = 1; j < board_size; j++) {
            int capt_j = gen_board[0][j].captured;
            if((capt_j == 3 || capt_j == 5) && (DFS_board[0][j] == 0)) {
                DFS_board[0][j] = 1;
                top_edge = 1;
                road = DFS(0, j, DFS_board,  1, gen_board, 0);
                initialise_edges();
                if(road)
                    return -10.0;
            }    
        }
        reset_visited(DFS_board);

        // Check for your roadwin from left_edge to right_edge
        for(int i = 0; i < board_size; i++) {
            int capt_i = gen_board[i][0].captured;
            if((capt_i == 0 || capt_i == 2) && (DFS_board[i][0] == 0)) {
                DFS_board[i][0] = 1;
                if(i == 0) 
                    top_edge = 1;
                left_edge = 1;
                road = DFS(i, 0, DFS_board, 0, gen_board, 0);
                initialise_edges();
                if(road)
                    return 10.0;
            }
        }
        // Check for your roadwin from top_edge to bottom_edge
        for(int j = 1; j < board_size; j++) {
            int capt_j = gen_board[0][j].captured;
            if((capt_j == 0 || capt_j == 2) && (DFS_board[0][j] == 0)) {
                DFS_board[0][j] = 1;
                top_edge = 1;
                road = DFS(0, j, DFS_board, 0, gen_board, 0);
                initialise_edges();
                if(road)
                    return 10.0;
            }
        }
    }
    double flat_val = 0.0;
    bool flat_win_check = flat_win(gen_board,flat_val);
    if(!flat_win_check)
        return 0.0;
    else 
        return flat_val;
}

// This maintains an influence value for each board square based on pieces on and around that square
void influence(float infl_arr[8][8], state gen_board[8][8], float flat, float wall, float cap) {
    // Reset the global 'infl_arr'
    for(int i = 0; i < board_size; i++) {
        for(int j = 0; j < board_size; j++) {
            infl_arr[i][j] = 0.0;
        }
    }
    for(int i = 0; i < board_size; i++) {
        for(int j = 0; j < board_size; j++) {
            float k;
            int capt = gen_board[i][j].captured;
            int temp;
            // Set the value of update variable 'k'
            if(capt == -1)
                k = 0;
            else if(capt < 3) {
                if(capt == 0)
                    k = flat;
                else if(capt == 1)
                    k = wall;
                else
                    k = cap;
            }
            else if(capt >= 3) {
                if(capt == 3)
                    k = -flat;
                else if(capt == 4)
                    k = -wall;
                else
                    k = -cap;
            }

            if(k != 0) {
                // Change current
                infl_arr[i][j] += k;
                // Change up
                temp = gen_board[i-1][j].captured;
                if(i-1 >= 0 && (temp == -1 || temp %3 == 0 || (capt%3 == 2 && temp %3 != 2)))
                    infl_arr[i-1][j] += k;
                // Change down
                temp = gen_board[i+1][j].captured;
                if(i+1 < board_size && (temp == -1 || temp % 3 == 0 || (capt%3 == 2 && temp %3 != 2)))
                    infl_arr[i+1][j] += k;
                // Change left
                temp = gen_board[i][j-1].captured;
                if(j-1 >= 0 && (temp == -1 || temp % 3 == 0 || (capt%3 == 2 && temp %3 != 2)))
                    infl_arr[i][j-1] += k;
                // Change right
                temp = gen_board[i][j+1].captured;
                if(j+1 < board_size-1 && (temp == -1 || temp % 3 == 0 || (capt%3 == 2 && temp %3 != 2)))
                    infl_arr[i][j+1] += k;
            }
        }
    }
}

// The evaluation function
double get_heuristic(state gen_board[8][8]) {
    // Initialize our global array
    int arr[16][6]; 
    for(int i=0;i<2*board_size;i++)
        for(int j=0;j<6;j++)
            arr[i][j]=0;
    
    // Calculate no. of pieces in each row and column
    for(int i = 0; i < board_size; i++) {
        for(int j = 0; j < board_size; j++) {
            int capt = gen_board[i][j].captured;
            if(capt == -1) 
                continue;
            else {
                arr[i][capt]++;
                arr[j+board_size][capt]++;
            }
        }
    }

    // NUMBER SQUARES CAPTURED
        double captured = 0.0;
        for(int i = 0; i < board_size; i++) {
            captured += (arr[i][0]-arr[i][3])*50 + (arr[i][2]-arr[i][5])*80; // Measure the captured advantage parameter
        }

    // SAME ROW PIECES HAS MORE WEIGHT
        double composition_value = 0.0;
        double center_value = 0.0;
        for(int i=0;i<2*board_size;i++){
            int flat_capt_me = arr[i][0];
            int wall_capt_me = arr[i][1];
            int cap_capt_me = arr[i][2];
            int my_capt = flat_capt_me + cap_capt_me;

            int flat_capt_you = arr[i][3];
            int wall_capt_you = arr[i][4];
            int cap_capt_you = arr[i][5];
            int your_capt = flat_capt_you + cap_capt_you;

            // Calculate row/column capture advantage
            int capt_diff = my_capt - your_capt;
            double capture_advantage = diff[my_capt];
            double capture_disadvantage = diff[your_capt];
            
            // Set wall dissadvantage factor based on board size
            float wallFactor;
            if(board_size == 5) {
                if(cur_player.id == 2 && last_heur_val < -200) {
                    wallFactor = 0.7;
                }
                else
                    wallFactor = 0.9;
            }
            if(board_size == 6) {
                if(cur_player.id == 2 && last_heur_val < -200) {
                    wallFactor = 0.7;
                }
                else
                    wallFactor = 0.9;
            }
            if(board_size == 7) {
                if(cur_player.id == 2 && last_heur_val < -200) {
                    wallFactor = 0.7;
                }
                else
                    wallFactor = 0.9;
            }

            // Calculate wall disadvantage
            double wall_disadvantage;
            if(capt_diff > 0){
                wall_disadvantage = wall_capt_me*for_wall + wall_capt_you*against_wall + diff[capt_diff]*(wall_capt_me + wall_capt_you)*2/board_size;
            }
            else if(capt_diff < 0){
                wall_disadvantage = (wall_capt_me*for_wall + wall_capt_you*against_wall - diff[-1*capt_diff]*(wall_capt_me + wall_capt_you)*2/board_size);
            }
            else
                wall_disadvantage = wall_capt_me*for_wall + wall_capt_you*for_wall;
            
            composition_value += capture_advantage - capture_disadvantage - wallFactor*wall_disadvantage;
            
            // CENTER CONTROL
            // Advantage for placing the capstone closer to the center
            if(i < board_size)
                center_value += (cap_capt_me - cap_capt_you)*(board_size-i-1)*i*center_weight;
            else
                center_value += (cap_capt_me - cap_capt_you)*(2*board_size-i-1)*(i-board_size)*center_weight;
        }

    // CURRENT PIECE HOLDINGS
        // Give disadvantage for holding an unused piece
        // Check to see if capstone is available
        double piece_val = 0.0;
        if(other_player.capstone == 1)
            piece_val -= 60;
        if(cur_player.capstone == 1)
            piece_val += 60;

        // See how many flatstones left
        piece_val -= 24*cur_player.no_flat;
        piece_val += 24*other_player.no_flat;

    // INFLUENCE
        // Calculate the influence advantage
        double infl_value = 0;
        influence(infl, gen_board, flat, wall, cap);
        for(int i = 0; i < board_size; i++){
            for(int j = 0; j < board_size; j++){
                int temp = infl[i][j];
                if(temp > 0)
                    infl_value += pow(temp,1.5);
                else
                    infl_value -= pow(-temp,1.5);
            }
        }

    // MOVE ADVANTAGE
        double move_advantage = 0.0;
        if(current_player == -1)
            move_advantage -= 250;
        else if(current_player == 1)
            move_advantage += 250;

    // Different weights for parameters for different board sizes
    double heuristic_value;
    if(board_size == 5)
        heuristic_value = move_advantage + 1.4*captured + 1.55*composition_value + piece_val + 0.9*infl_value + 1.1*center_value;
    else if(board_size == 6)
        heuristic_value = move_advantage + 1.5*captured + 1*composition_value + piece_val + 0.9*infl_value + 0.5*center_value;
    else
        heuristic_value = move_advantage + 1.6*captured + 0.75*composition_value + piece_val + 0.9*infl_value + 0.2*center_value;
    return heuristic_value;
}

// Helps to generate all possible moves that can be taken by a player for a given Board state
void generate_all_moves(int id, state gen_board[8][8], int &size) {
    // Initialize variables
    size = 0;
    string move;
    bool valid;
    Player the_player;
    int index, capt, curr_capt;

    // Check if it is our move
    if(id == player_id) {
        the_player = cur_player;
        index = 0;
    }
    // Check if it is opponent's move
    else {
        the_player = other_player;
        index = 1;
    }
    for(int i = 0; i < board_size; i++) {
        for(int j = 0; j < board_size; j++) {
            capt = gen_board[i][j].captured;
            if(capt == -1){
                // Add the Flatstone moves
                if(the_player.no_flat != 0) { // If 'the_player' still has some flatstones left                    
                    move = "F" ; 
                    move += (char)(97+j); // Encode to alphabet character
                    move += (char)(49+i); // Encode to numeric character
                    all_moves[size] = move; 
                    size++;
                }
            }
            // Add all the 'stack pick and place moves'
            else if(capt >= 3*index && capt < 3 + 3*index){ // Checks to see if square is captured by 'the_player' 
                int temp_size = gen_board[i][j].state_stack.size(); // Size of the stack on current square
                int stack_size = min(temp_size, board_size); // Maximum size of stzk that can be picked up by 'the_player'
                
                // Set distances of edges from current square
                int dist_up = i;
                int dist_down = board_size - 1 - i;
                int dist_left = j; 
                int dist_right = board_size - 1 - j;
            
                for(int x = 1; x <= stack_size; x++) {
                    part_list = partition(x); // Get possible partition list for a stack of size 'x'
                    for(int y = 0; y < part_list.size(); y++) { 
                        int part_size = part_list[y].size(); 
                        
                        // Check if its possible to spread the stack upwards
                        if(part_size <= dist_up) { 
                            valid = true;
                            move = ""; 
                            move += (char)(48+x); // Encode the number of pieces picked up
                            move += (char)(97+j); // Encode to alphabet character
                            move += (char)(49+i); // Encode to numeric character
                            move += "-"; // Indicating that pieces are placed in upward direction
                            for(int z = 0; z < part_size; z++) {
                                curr_capt = gen_board[i-z-1][j].captured;
                                // Check to see there is no capstone or standing stone in the way
                                if(curr_capt % 3 == 0 || curr_capt == -1) 
                                    move += (char)(48+part_list[y][z]); // Encode the number of pieces picked up
                                else {
                                    // Check to see if capstone can crush the wall in its path
                                    if(z == part_size-1) {
                                        if((part_list[y][z] == 1) && ((curr_capt % 3 == 1) ) && (capt == 2 + index*3))
                                            move += "1";
                                        else
                                            valid = false;
                                    }
                                    else {
                                        valid = false;
                                        break;
                                    }
                                }
                            }
                            if(valid) {
                                all_moves[size] = move; 
                                size++;
                            }
                        }

                        // Check if its possible to spread the stack downwards
                        if(part_size <= dist_down) { 
                            valid = true;
                            move = ""; 
                            move += (char)(48+x); // Encode the number of pieces picked up
                            move += (char)(97+j); // Encode to alphabet character
                            move += (char)(49+i); // Encode to numeric character
                            move += "+"; // Indicating that pieces are placed in downward direction
                            int z1 = -1;
                            for(int z = 0; z < part_size; z++) {
                                curr_capt = gen_board[i+z+1][j].captured;
                                if(curr_capt % 3 == 0 || curr_capt == -1)
                                    move += (char)(48+part_list[y][z]); 
                                else {
                                    // Check to see if capstone can crush the wall in its path
                                    if(z == part_size-1) {
                                        if((part_list[y][z] == 1) && ((curr_capt % 3 == 1) ) && (capt == 2 + index*3))
                                            move += "1";
                                        else
                                            valid = false;
                                    }
                                    else {
                                        valid = false;
                                        break;
                                    }
                                }
                            }
                            if(valid) {
                                all_moves[size] = move; 
                                size++;
                            }
                        }

                        // Check if its possible to spread the stack to the left
                        if(part_size <= dist_left) {
                            valid = true;
                            move = ""; 
                            move += (char)(48+x); // Encode the number of pieces picked up
                            move += (char)(97+j); // Encode to alphabet character
                            move += (char)(49+i); // Encode to numeric character
                            move += "<"; // Indicating that pieces are placed in left direction
                            int z1 = -1;
                            for(int z = 0; z < part_size; z++) {
                                curr_capt = gen_board[i][j-z-1].captured;
                                if(curr_capt % 3 == 0 || curr_capt == -1)
                                    move += (char)(48+part_list[y][z]);  
                                else {
                                    // Check to see if capstone can crush the wall in its path
                                    if(z == part_size-1) {
                                        if((part_list[y][z] == 1) && ((curr_capt % 3 == 1) ) && (capt == 2 + index*3))
                                            move += "1";
                                        else
                                            valid = false;
                                    }
                                    else {
                                        valid = false;
                                        break;
                                    }
                                }
                            }
                            if(valid) {
                                all_moves[size] = move; 
                                size++;
                            }
                        }

                        // Check if its possible to spread the stack to the right
                        if(part_size <= dist_right) {
                            valid = true;
                            move = ""; 
                            move += (char)(48+x); // Encode the number of pieces picked up
                            move += (char)(97+j); // Encode to alphabet character
                            move += (char)(49+i); // Encode to numeric character
                            move += ">"; // Indicating that pieces are placed in right direction
                            int z1 = -1;
                            for(int z = 0; z < part_size; z++){
                                curr_capt = gen_board[i][j+z+1].captured;
                                if(curr_capt % 3 == 0 || curr_capt == -1)
                                    move += (char)(48+part_list[y][z]); 
                                else {
                                    // Check to see if capstone can crush the wall in its path
                                    if(z == part_size-1) {
                                        if((part_list[y][z] == 1) && ((curr_capt % 3 == 1) ) && (capt == 2 + index*3))
                                            move += "1";
                                        else
                                            valid = false;
                                    }
                                    else {
                                        valid = false;
                                        break;
                                    }
                                }
                            }
                            if(valid) {
                                all_moves[size] = move; 
                                size++;
                            }
                        }
                    }
                }
            }
        }
    }
    // Add capstone moves
    for(int i = 0; i < board_size; i++) {
        for(int j = 0; j < board_size; j++) {
            capt = gen_board[i][j].captured;
            if(capt == -1){
                if(the_player.capstone != 0) {
                    move = "C" ; 
                    move += (char)(97+j); // Encode to alphabet character
                    move += (char)(49+i); // Encode to numeric character
                    all_moves[size] = move; 
                    size++;                            
                }
            }
        }
    }
    // Add standing stone moves
    for(int i = 0; i < board_size; i++) {
        for(int j = 0; j < board_size; j++) {
            capt = gen_board[i][j].captured;
            if(capt == -1){
                if(the_player.no_flat != 0) {                            
                    move = "S" ; 
                    move += (char)(97+j); // Encode to alphabet character
                    move += (char)(49+i); // Encode to numeric character
                    all_moves[size] = move; 
                    size++;                           
                }
            }
        }
    }
}

// Calculates the score at current board state
double current_score(state myboard[8][8], int los_player_id) { // 0 means our player has lost
    double count_loser_score = 0.0;
    double count_winner_score = board_size*board_size; // initialise winner score

    for(int i=0;i<board_size;i++) {
        for(int j=0; j<board_size; j++) {
            int capt = myboard[i][j].captured;
            if(capt == 0 + (1-los_player_id)*3 || capt == 2 + (1-los_player_id)*3) { // count of winner
                count_winner_score += 1.0;
            }
            else if(capt == 0 + (los_player_id)*3 || capt == 2 + (los_player_id)*3) {
                count_loser_score += 1.0;
            }
        }
    }

    // Add number of flatstones remaining to the winner's score
    if(player_id == 0)
          count_winner_score += other_player.no_flat;
    else
          count_winner_score += cur_player.no_flat; 
    
    double ans = (LONG_MAX/1000)* (float)count_loser_score/(float)(count_winner_score + count_loser_score) ; 
    return ans;         
}

// This is the search function that searches for the best possible move
double best_move(state myboard[8][8], double alpha, double beta, int depth, string &best_move_chosen, bool minimum) {
    best_called++; // tracks the number of times best_move is called

    // Initialise best_move variables
    double min_val = LONG_MAX, max_val = LONG_MIN, child, ans, val; 

    // To figure out which player was white to aid in move_advantage parameter
    if(!minimum && player_id == 1)
        current_player = -1;
    else if (minimum && player_id == 2)
        current_player = 1;
    else
        current_player = 0;

    // To figure the player calling the best_move function
    int move_player;
    if(minimum) {
        if(player_id == 1) 
            move_player = 2;
        else 
            move_player = 1;
    }
    else  
        move_player = player_id;

    int size = 0; 
    generate_all_moves(move_player,myboard,size);

    vector<pair<double,string> > values; // To store evaluated state values for sorting
    for(int i = 0; i < size; i++) {
        int crushed = 0; // variable which checks if wall was crushed by capstone
        perform_move(all_moves[i], move_player, myboard, crushed, 0); // Move to child state

        if(!minimum)   
            ans = at_endstate(myboard, false, 0); // as it was my move
        else 
            ans = at_endstate(myboard, false, 1); // as it was opponents move

        // Incase of a road_win set val to extremes
        if(ans == 10.0) {
            val = LONG_MAX ;
        }
        else if(ans == -10.0) {
            val = LONG_MIN;
        }
        else if(ans == 0.0) {
            val = get_heuristic(myboard);
        }
        // In case of flatwin
        else {
            if(ans == LONG_MAX * 0.0001) // Flat win tie
                val = 0.0 ;
            else
                val = ans*LONG_MAX;  
        }

        // Sort only if depth is greater than 1
        if(depth != 1)
            values.push_back(std::make_pair(val, all_moves[i])); // Add to vector list before sorting
        // If depth = 1, perform alpha-beta search
        else if(minimum) {
            best_called++;
            child = val;
            beta = min(beta, child);
            min_val = min(child, min_val);
            if(child == min_val) 
                best_move_chosen = all_moves[i];
        }
        else {
            best_called++;
            child = val;
            alpha = max(alpha, child);
            max_val = max(child, max_val);
            if(child == max_val) 
                best_move_chosen = all_moves[i];
        }

        undo_move(all_moves[i], move_player, myboard, crushed); // To return to parent state

        // Pruning Step
        if(alpha > beta)
            return child;
    }
    if(depth == 1) {
        if(minimum)
            return min_val;
        else
            return max_val;
    }
    // If depth > 1, then perform alpha-beta search in the sorted priority queues
    if(minimum) {
        // Contruct max priority queue
        priority_queue<pair<double,string>, vector<pair<double,string> >, Compare_max> maxi_heap(values.begin(),values.end());          
        
        for(int i = 0; i < size; i++) {
            string move_taken = "";
            double heur_val = maxi_heap.top().first;
            move_taken = maxi_heap.top().second;
            maxi_heap.pop();

            int crushed = 0;
            perform_move(move_taken, move_player, myboard, crushed, 0);

            string tmp = "";
            // If Roadwin check check to see better score by checking 'current_score'
            if(heur_val == LONG_MAX) {
                child = LONG_MAX - current_score(myboard,1);
            }
            else if(heur_val == LONG_MIN) {
                child = LONG_MIN  + current_score(myboard,0);
            }
            else if(heur_val > LONG_MAX/100 ) {
                if(get_heuristic(myboard) < 700) // to check if flat win or not
                    child = heur_val;
                else
                    child = best_move(myboard, alpha, beta, (depth-1), tmp, !minimum);
            }
            else if(heur_val < LONG_MIN/100) {
                if(get_heuristic(myboard) > -700) // to check if flat win or not
                    child = heur_val;
                else
                    child = best_move(myboard, alpha, beta, (depth-1), tmp, !minimum);
            }
            else
                child = best_move(myboard, alpha, beta, (depth-1), tmp, !minimum); 
            beta = min(beta, child);
            min_val = min(child, min_val);
            if(child == min_val) 
                best_move_chosen = move_taken; 

            undo_move(move_taken, move_player, myboard, crushed); // To return to parent state
       
            // Pruning Step
            if(alpha > beta)
                return child;
        }
        return min_val;
    }
    else {
        // Contruct min priority queue
        priority_queue<pair<double,string>, vector<pair<double,string> >, Compare_min> mini_heap(values.begin(),values.end());   
        
        for(int i = 0; i < size; i++) { 
            string move_taken = "";
            double heur_val = mini_heap.top().first;
            move_taken = mini_heap.top().second;
            mini_heap.pop();

            int crushed = 0;
            perform_move(move_taken, move_player, myboard, crushed, 0);

            string tmp = "";
            // If Roadwin check check to see better score by checking 'current_score'
            if(heur_val == LONG_MAX)
                child = LONG_MAX - current_score(myboard,1);
            else if(heur_val == LONG_MIN)
                child = LONG_MIN  + current_score(myboard,0);
            else if(heur_val > LONG_MAX/100 ) {
                if(get_heuristic(myboard) < 700) // to check if flat win or not
                    child = heur_val;
                else
                    child = best_move(myboard, alpha, beta, (depth-1), tmp, !minimum);
            }
            else if(heur_val < LONG_MIN/100) {
                if(get_heuristic(myboard) > -700) // to check if flat win or not
                    child = heur_val;
                else
                    child = best_move(myboard, alpha, beta, (depth-1), tmp, !minimum);
            }
            else
                child = best_move(myboard, alpha, beta, (depth-1), tmp, !minimum); 
            alpha = max(alpha, child);
            max_val = max(child, max_val);
            if(child == max_val) 
                best_move_chosen = move_taken;

            undo_move(move_taken, move_player, myboard, crushed); // To return to parent state
       
            // Pruning Step
            if(alpha > beta)
                return child;  
        }
        return max_val;
    }
} 

// Obtain game score in case of GAME OVER
int get_score(state Board[8][8], bool winner) {
    int count1 = 0;
    int count2 = 0;
    for(int i = 0; i < board_size; i++) {
        for(int j = 0; j< board_size; j++) {
            if(Board[i][j].captured == 3 || Board[i][j].captured == 5)
                count2++;
            else if(Board[i][j].captured == 0 || Board[i][j].captured == 2)
                count1++;
        }
    }
    count1 += cur_player.no_flat; // Count unused flatstones if you win
    if(winner)
        return count1;
    else
        return count2;
}

void print_board(state Board[8][8]) {
    for(int i = 0; i < board_size; i++) {
        for(int j = 0; j < board_size; j++) {
            stack<int> temp(Board[i][j].state_stack);
            if(temp.size() == 0) {
                cerr<<"-1       ";
                continue;
            }  
            while(temp.size()!=0) {
                int x = temp.top();
                temp.pop();
                cerr<<x<<":";
            }
            cerr<<"       ";
        }
        cerr<<endl;
    }
}

int main(int argc, char** argv) {
    // Start random seed for random functions
    srand(time(NULL));
    
    int n, time_limit;
    // Get input from server about game specifications
    cin >> player_id >> n >> time_limit;
    begin_time = clock();
    board_size = n;

    // Initialize the two players
    cur_player.assign(board_size);
    other_player.assign(board_size);

    // Initialise board
    state Board [8][8];
    for(int i = 0; i < board_size; i++)
        for(int j = 0; j < board_size; j++)
            Board[i][j].captured = -1;

    if(player_id == 2) {
        // Get other player's move
        string move;
        cin>>move; 
        
        begin_time = clock(); // Start player 2 time
        
        // Perfom other player's move
        int crush = 0;
        perform_move(move, 2, Board, crush, 1);
        
        // Generate possible moves
        int temp_size;
        generate_all_moves(1, Board, temp_size);

        // Choose initial move as random flatstone move
        int randmove = rand()%temp_size;
        while(all_moves[randmove][randmove] != 'F') // Until its a flatstone placing move
            randmove = rand()%temp_size;
        
        // Perform picked move
        perform_move(all_moves[randmove], 1, Board, crush, 1);
        cout<<all_moves[randmove]<<endl;
        
        // End player 2 time and update time remaining
        end_time = clock();
        time_player += float( end_time - begin_time ) /  CLOCKS_PER_SEC;      
        
        int count = 0; // Start move count
        while(true) {
            cin>>move; // Get other player's move

            begin_time = clock(); // Start player 2 time

            // Perfom other player's move
            crush = 0;
            perform_move(move, 1, Board, crush, 1);

            // Check if endstate is reached
            int endstate_val = at_endstate(Board, false, 1);
            if(endstate_val > 0.0) { // Check for win
                cerr<<"You are the winner"<<endl;
                if(endstate_val == 10.0)
                    cerr<<"Scorewin "<<get_score(Board, true) + board_size*board_size<<endl;
                else
                    cerr<<"Scorewin "<<get_score(Board, true)<<endl;
                cerr<<"Scorelose "<<get_score(Board, false)<<endl;
            }
            else if(endstate_val < 0.0) { // Check for loss
                cerr<<"You are the loser"<<endl;
                if(endstate_val == -10.0)
                    cerr<<"Scorewin "<<get_score(Board, false) + board_size*board_size<<endl;
                else
                    cerr<<"Scorewin "<<get_score(Board, false)<<endl;
                cerr<<"Scorelose "<<get_score(Board, true)<<endl;
            }
            current_player = 1;
            
            cerr<<"\nCount is "<<count<<endl;
            
            // Do search for best move using itterative deepening search with time limits
            int limit = 6;
            double val;
            string next_move = "";
            if(time_limit - time_player < 20)
                limit = 4;
            else if(time_limit - time_player < 42)
                limit = 5;
            for(int i = 1; i <= limit; i++) {
                best_called = 0;

                // Measure time taken to find best move
                ids_start = clock();
                val = best_move(Board, LONG_MIN/2, LONG_MAX/2, i, next_move, false);
                ids_end = clock();
                float time = float(ids_end - ids_start) / CLOCKS_PER_SEC;
                
                // Print details
                cerr<<"Depth "<<i<<"="<<next_move;
                cerr<<"\ttime taken = "<<time;
                cerr<<"\teffective branching factor = "<<pow(best_called,(1.0/(i+1)));
                cerr<<"\tnodes = "<<best_called<<endl;
                if(time > time_threshold/pow(best_called,(1.0/(i+1))))
                    break;
            }
            // Perform picked move
            perform_move(next_move, 2, Board, crush, 1);
            print_board(Board);

            // Check if endstate is reached
            endstate_val = at_endstate(Board, false, 0);
            if(endstate_val > 0.0) { // Check for win
                cerr<<"You are the winner"<<endl;
                if(endstate_val == 10.0)
                    cerr<<"Scorewin "<<get_score(Board, true) + board_size*board_size<<endl;
                else
                    cerr<<"Scorewin "<<get_score(Board, true)<<endl;
                cerr<<"Scorelose "<<get_score(Board, false)<<endl;
            }
            else if(endstate_val < 0.0) { // Check for loss
                cerr<<"You are the loser"<<endl;
                if(endstate_val == -10.0)
                    cerr<<"Scorewin "<<get_score(Board, false) + board_size*board_size<<endl;
                else
                    cerr<<"Scorewin "<<get_score(Board, false)<<endl;
                cerr<<"Scorelose "<<get_score(Board, true)<<endl;
            }
            current_player = 0;
            cerr<<"the heuristic value is = "<<get_heuristic(Board)<<","<<val<<endl;
            last_heur_val = get_heuristic(Board);
            cout<<next_move<<endl;
            count++; // Update move count
            
            // End player 2 time and update time remaining
            end_time = clock();
            double time_of_move = 0.0;
            time_of_move = float( end_time - begin_time ) /  CLOCKS_PER_SEC;
            time_player += time_of_move;
        }
    }   
    else if(player_id == 1) {
        // Generate possible moves
        int temp_size;
        generate_all_moves(2, Board, temp_size);

        // Choose initial move as random flatstone move
        int randmove = rand()%temp_size;
        while(all_moves[randmove][0] != 'F') // Until its a flatstone placing move
            randmove = rand()%temp_size;
        
        // Perform picked move
        int crush = 0;
        perform_move(all_moves[randmove], 2, Board, crush, 1);
        cout<<all_moves[randmove]<<endl;
        
        // End player 2 time and update time remaining
        end_time = clock();
        time_player += float( end_time - begin_time ) /  CLOCKS_PER_SEC;
        
        // Get other player's move
        string move;
        cin>>move; 

        begin_time = clock(); // Start player 1 time

        // Perfom other player's move
        crush = 0;
        perform_move(move, 1, Board, crush, 1);

        int count = 0; // Start move count
        while(true) {
            cerr<<"\nCount is "<<count<<endl;

            // Do search for best move using itterative deepening search with time limits
            int limit = 6;
            double val;
            string next_move = "";
            if(time_limit - time_player < 20)
                limit = 4;
            else if(time_limit - time_player < 42)
                limit = 5;
            for(int i = 2; i <= limit; i++) {
                best_called = 0;

                // Measure time taken to find best move
                ids_start = clock();
                val = best_move(Board, LONG_MIN/2, LONG_MAX/2, i, next_move, false);
                ids_end = clock();
                float time = float(ids_end - ids_start) / CLOCKS_PER_SEC;
                
                // Print details
                cerr<<"Depth "<<i<<"="<<next_move;
                cerr<<"\ttime taken = "<<time;
                cerr<<"\teffective branching factor = "<<pow(best_called,(1.0/(i+1)));
                cerr<<"\tnodes = "<<best_called<<endl;
                if(time > time_threshold/pow(best_called,(1.0/(i+1))))
                    break;
            }
            // Perform picked move
            perform_move(next_move, 1, Board, crush, 1);
            print_board(Board);

            // Check if endstate is reached
            int endstate_val = at_endstate(Board, false, 0);
            if(endstate_val > 0.0) { // Check for win
                cerr<<"You are the winner"<<endl;
                if(endstate_val == 10.0)
                    cerr<<"Scorewin "<<get_score(Board, true) + board_size*board_size<<endl;
                else
                    cerr<<"Scorewin "<<get_score(Board, true)<<endl;
                cerr<<"Scorelose "<<get_score(Board, false)<<endl;
            }
            else if(endstate_val < 0.0) { // Check for loss
                cerr<<"You are the loser"<<endl;
                if(endstate_val == -10.0)
                    cerr<<"Scorewin "<<get_score(Board, false) + board_size*board_size<<endl;
                else
                    cerr<<"Scorewin "<<get_score(Board, false)<<endl;
                cerr<<"Scorelose "<<get_score(Board, true)<<endl;
            }
            current_player = -1;
            cerr<<"the heuristic value is = "<<get_heuristic(Board)<<","<<val<<endl;
            last_heur_val = get_heuristic(Board);
            cout<<next_move<<endl;

            // End player 1 time and update time remaining
            end_time = clock();
            double time_of_move = 0.0;
            time_of_move = float( end_time - begin_time ) /  CLOCKS_PER_SEC;
            time_player += time_of_move;  
            
            cin>>move; // Get other player's move

            begin_time = clock(); // Start player 1 time

            // Perfom other player's move
            perform_move(move, 2, Board, crush, 1);

            // Check if endstate is reached
            endstate_val = at_endstate(Board, false, 1);
            if(endstate_val > 0.0){ // Check for win
                cerr<<"You are the winner"<<endl;
                if(endstate_val == 10.0)
                    cerr<<"Scorewin "<<get_score(Board, true) + board_size*board_size<<endl;
                else
                    cerr<<"Scorewin "<<get_score(Board, true)<<endl;
                cerr<<"Scorelose "<<get_score(Board, false)<<endl;
            }
            else if(endstate_val < 0.0){ // Check for loss
                cerr<<"You are the loser"<<endl;
                if(endstate_val == -10.0)
                    cerr<<"Scorewin "<<get_score(Board, false) + board_size*board_size<<endl;
                else
                    cerr<<"Scorewin "<<get_score(Board, false)<<endl;
                cerr<<"Scorelose "<<get_score(Board, true)<<endl;
            }
            current_player = 0;  
            count++; // Update move count   
        }
    }
    return 0;
}
