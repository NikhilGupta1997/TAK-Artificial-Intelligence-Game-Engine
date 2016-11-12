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

    #define MAX_BOARD_SIZE 8 
    #define Hash_size 124000000 

    static int max_height = 60;

    // Some cout variables
    static int prune = 0;
    static int best_called = 0;

    // Evaluation function variables
    static double capstone = 90;
    static double flatstone = 75;
    static double standing = 50;
    static double cap_threat = 40;
    static double stand_threat = 40; 

    static double mapping[3];
    static double diff[8];
    static float infl[8][8];
    static int neighbors[1000];
    static int temp_board[8][8];
    static string all_moves[5000];

    // Define some global arrays
    static int player_id;
    static int board_size;
    static int current_player;

    //get_heuristic variables
    static double heuristic_value = 0.0;
    static double captured = 0.0;
    static double threats = 0.0;
    static double piece_val = 0.0;

    static float composition_value = 0.0;
    static int flat_capt_me, wall_capt_me, cap_capt_me, my_capt;
    static int flat_capt_you, wall_capt_you, cap_capt_you, your_capt;
    static int capt_diff;
    static float against_wall = 40, for_wall = 10;
    static float capture_advantage = 0.0;
    static float capture_disadvantage = 0.0;
    static float capture_delta = 0.0;

    static float wall_disadvantage = 0.0;
    static float center_weight = 5;
    static float center_value = 0.0;

    static clock_t func_begin_time;
    static clock_t func_end_time;
    static clock_t ids_start;
    static clock_t ids_end;
    static float time_threshold = 8;

    static float time_get_heuristic = 0.0;
    static float time_influence = 0.0;
    static float time_generate_moves = 0.0;
    static float time_execute_moves = 0.0;
    static float time_undo_moves = 0.0;
    static float time_end_states = 0.0;

    static int number_called_get_heuristic = 0;
    static int number_called_influence = 0;
    static int number_called_generate_moves = 0;
    static int number_called_execute_moves = 0;
    static int number_called_undo_moves = 0;
    static int number_called_end_states = 0;

    static int top1 = 0;
    static int left1 = 0;
    static int bottom1 = 0;
    static int right1 = 0;

    static float myval;
    static int repeated = 0;

    static uint64_t zobrist_table[300000][6];
    static uint64_t global_hash = 0;

    int temp_size;// = gen_board[i][j].state_stack.size();
    int stack_size;// = min(temp_size, board_size);
    int dist_up ;//= i;
    int dist_down ;//= board_size - 1 - i;
    int dist_left ;//= j; 
    int dist_right ;//= board_size - 1 - j;
    vector< vector<int> > part_list;

class storage {
  public:
    float value;
    string best_move;
    int depth;
    storage(string b, float v, int depth) {
        value = v;
        b = best_move;
        this->depth = depth; 
    }
};

unordered_map< uint64_t, storage > Transposition_Table(Hash_size);

struct Compare_min {
    bool operator()(pair<double,string> p1,pair<double,string> p2) {
        return p1.first<p2.first;
    }
};

struct Compare_max {
    bool operator()(pair<double,string> p1,pair<double,string> p2) {
        return p1.first>p2.first;
    }
};

class state {
  public:   
    stack<int> state_stack;
    int captured;
    
    state() {
       captured = -1;
    }
    
    void assign(state Board) {
        stack<int> temp(Board.state_stack);
        swap(this->state_stack,temp);
        this->captured = Board.captured;
    }

    bool check_equal(state &other) {
        if(captured != other.captured)
            return false;
        if(state_stack.size() != other.state_stack.size())
            return false;
        stack<int> temp(other.state_stack);
        stack<int> temp_curr(this->state_stack);
        int a1, a2;
        while(temp.size() != 0) {
            a1 = temp.top();
            a2 = temp_curr.top();
            if(a1 != a2)
                return false;
            temp.pop();
            temp_curr.pop();
        }
        return true;
    }
};

void init_zobrist() {
    int n_square = board_size*board_size;
    for(int i = 0; i < board_size; i++) {
        for(int j = 0; j < board_size; j++) {
            int k1 = (i*board_size+j)*max_height;
            for(int t = 0 ; t < max_height; t++) {
                for(int piece = 0; piece < 6; piece++) {
                    zobrist_table[k1+t][piece] = rand() % 200000000 ;
                }
            }
        }
    }
}

class Player {
  public:
    int no_flat;
    int capstone;
    int id; 

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
} cur_player, other_player;

void string_to_move_cur(string move, int id, state myBoard[8][8], int &crushed, bool hash) {
    int j = (int)(move[1]) - 96; 
    int i = (int)(move[2]) - 48;   
    if(!isdigit(move[0])) {
        assert(myBoard[i-1][j-1].captured == -1);
        int x;
        if(move[0] == 'F') 
            x = 0;
        else if(move[0] == 'S') 
            x = 1;
        else if(move[0] == 'C') 
            x = 2;
        if(id != player_id) 
            x = x+3;
        // if(hash) {
        //     int entry = ((i-1)*board_size+(j-1))*max_height;
        //     global_hash = global_hash xor zobrist_table[entry][x];
        // }
        myBoard[i-1][j-1].captured = x;
        myBoard[i-1][j-1].state_stack.push(x);
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
    else {
        int no_picked = (int) move[0],top;
        no_picked -= 48;
        stack<int> picked;
        int entry = ((i-1)*board_size+(j-1))*max_height;
        int s =  myBoard[i-1][j-1].state_stack.size();
        for(int l = 0; l < no_picked; l++) {
            top = myBoard[i-1][j-1].state_stack.top();
            if(hash)
                global_hash = global_hash xor zobrist_table[entry+s][top];   
            myBoard[i-1][j-1].state_stack.pop();
            s--;
            picked.push(top);
        }
        if(myBoard[i-1][j-1].state_stack.size() != 0)
            myBoard[i-1][j-1].captured = myBoard[i-1][j-1].state_stack.top();
        else
            myBoard[i-1][j-1].captured = -1; 
        char dir = move[3];
        vector<int> drop;
        for(int l = 4; l < move.length(); l++) {
            char m_ch = move[l];
            int i1 = (int)m_ch - 48;
            drop.push_back(i1);
        }
        int mi, mj;
        if(dir == '+') {
            mi = 1; 
            mj = 0;
        }
        else if(dir == '-') {
            mi = -1; 
            mj = 0;
        }
        else if(dir == '>') {
            mi = 0; 
            mj = 1;
        }
        else {
            mi = 0; 
            mj = -1;
        }
        int w1, w2;  
        for(int k = 1; k <= drop.size(); k++) {
            w1 = i-1+k*mi;
            w2 = j-1+k*mj;
            stack<int> tempo(myBoard[w1][w2].state_stack);
            int s1 = tempo.size();
            int cap = myBoard[w1][w2].captured;
            int x1 = 1;
            int top1, t1;
            t1 = myBoard[w1][w2].captured; 
            int entry1= (w1*board_size+w2)*max_height ;
            while(x1 <= drop[k-1]) {
                top1 = picked.top();
                picked.pop();
                if(drop[k-1] == 1 && t1 % 3 == 1) {
                    int xw = tempo.top();
                    tempo.pop();
                    tempo.push(xw-1);
                    // if(hash) {
                    //     global_hash = global_hash xor zobrist_table[entry1+s1][xw];
                    //     global_hash = global_hash xor zobrist_table[entry1+s1][xw-1]; 
                    // }    
                    crushed=1;      
                }   
                t1 = top1;
                tempo.push(top1);
                s1++;
                // if(hash)
                //     global_hash = global_hash xor zobrist_table[entry1+s1][t1];               
                x1++;
            }
            myBoard[w1][w2].captured = top1;
            swap(tempo, myBoard[w1][w2].state_stack);   
        } 
    }
}

void undo_move(string move, int id,state gen_Board[8][8], int crushed, bool hash) {
    int j = (int)(move[1]) - 96;
    int i = (int)(move[2]) - 48;
    if(!isdigit(move[0])) {
        int x = gen_Board[i-1][j-1].state_stack.top();
        gen_Board[i-1][j-1].captured = -1;
        // if(hash) {
        //     int entry = ((i-1)*board_size+(j-1))*max_height;
        //     global_hash = global_hash xor zobrist_table[entry][x];
        // }
        gen_Board[i-1][j-1].state_stack.pop();
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
    else {   
        int no_picked = (int)move[0], top;
        no_picked -= 48;
        int drops = move.length()-4;
        int dropped[8];
        char dir = move[3];
        for(int l = 4; l < move.length(); l++) {
            char m_ch = move[l];
            int i1 = (int)m_ch -48;
            dropped[l-4] = i1;
        }
        int mi, mj;
        if(dir == '+') {
            mi = 1; 
            mj = 0; 
        }
        else if(dir == '-') {
            mi = -1; 
            mj = 0;
        }
        else if(dir == '>') {
            mi = 0; 
            mj = 1; 
        }
        else {
            mi = 0; 
            mj = -1;
        }
        stack<int> reverse_drop;
        int pick_up, w1, w2;
        for(int k = drops; k > 0; k--) {
            w1 = i-1+k*mi;
            w2 = j-1+k*mj;
            pick_up = dropped[k-1];
            int entry2 = (w1*board_size+w2)*max_height;
            int x2;
            int s2 = gen_Board[w1][w2].state_stack.size();                    
            int captured = gen_Board[w1][w2].captured;
            if(k == drops && gen_Board[w1][w2].state_stack.size() >= 2 && pick_up == 1 && captured % 3 == 2) {
                    reverse_drop.push(captured);
                    x2 = gen_Board[w1][w2].state_stack.top();
                    // if(hash)
                    //     global_hash = global_hash xor zobrist_table[entry2+s2][x2]; 
                    gen_Board[w1][w2].state_stack.pop();
                    s2--;
                    captured = gen_Board[w1][w2].state_stack.top();
                    
                    if(captured % 3 == 0 && crushed == 1) {
                        int wx = captured;
                        gen_Board[w1][w2].state_stack.pop();
                        wx++;
                        gen_Board[w1][w2].state_stack.push(wx);
                        gen_Board[w1][w2].captured = wx;
                        // if(hash) {
                        //     global_hash = global_hash xor zobrist_table[entry2+s2][wx-1];
                        //     global_hash = global_hash xor zobrist_table[entry2+s2][wx];
                        // }
                    }
                    else 
                        gen_Board[w1][w2].captured = captured;   
                }
            else {   
                int x1 = 1;
                while(x1 <= pick_up) {   
                    reverse_drop.push(gen_Board[w1][w2].state_stack.top());
                    x2 = gen_Board[w1][w2].state_stack.top();
                    // if(hash) {
                    //     global_hash = global_hash xor zobrist_table[entry2+s2][x2];
                    // }
                    gen_Board[w1][w2].state_stack.pop();    
                    s2--;
                    x1++;
                }
                if(gen_Board[w1][w2].state_stack.size() == 0)
                    gen_Board[w1][w2].captured = -1;
                else
                    gen_Board[w1][w2].captured = gen_Board[w1][w2].state_stack.top();
            }
        }
        int entry1 = ((i-1)*board_size+(j-1))*max_height; 
        int  s3 = gen_Board[i-1][j-1].state_stack.size();
        while(reverse_drop.size() != 0) {
            int picking = reverse_drop.top();
            reverse_drop.pop();
            gen_Board[i-1][j-1].state_stack.push(picking);
            s3++;
            // if(hash)
            //     global_hash= global_hash xor zobrist_table[entry1+s3][picking];
        }
        gen_Board[i-1][j-1].captured = gen_Board[i-1][j-1].state_stack.top();      
    }
}

vector< vector<int> > partition(int stack_size) {   
    vector< vector<int> > answer;
    if(stack_size <= 0) 
        return answer;    
    if(stack_size == 1) {
        vector<int> t; 
        t.push_back(1);
        answer.push_back(t);
        return answer;
    }       
    for(int i = 1; i < stack_size; i++) {
        int j = stack_size-i;
        vector<int> temp2;
        vector<vector<int> > part = partition(j);
        for(int i1 = 0; i1 < part.size(); i1++) {
            temp2 = part[i1];
            temp2.insert(temp2.begin(),i);
            answer.push_back(temp2);
        }   
    }
    vector<int> t1;
    t1.push_back(stack_size);
    answer.push_back(t1);
    return answer;   
}

int get_neighbors(int i, int j, int index) {
    int count = index;
    if(i != 0 && temp_board[i-1][j] == 0) {
        neighbors[count] = ((i-1)*board_size + j);
        count++;
    }
    if(j != 0 && temp_board[i][j-1] == 0) {
        neighbors[count] = (i*board_size + j-1);
        count++;
    }
    if(i != board_size-1 && temp_board[i+1][j] == 0) {
        neighbors[count] = ((i+1)*board_size + j);
        count++;
    }
    if(j != board_size-1 && temp_board[i][j+1] == 0) {
        neighbors[count] = (i*board_size + j+1);
        count++;
    }
    return count;
}

    

int DFS(int i, int j, int myboard[8][8],  int player_id, state gen_board[8][8], int neigh_index) {
    bool road = false;
    int size = get_neighbors(i, j, neigh_index);
    int curr, curr_i, curr_j;
    for(int i = neigh_index; i < size; i++) {
        curr = neighbors[i];
        curr_i = curr / board_size;
        curr_j = curr % board_size;
        if(myboard[curr_i][curr_j] == 1)
            continue;
        myboard[curr_i][curr_j] = 1;
        if(gen_board[curr_i][curr_j].captured == (0 + player_id * 3) || gen_board[curr_i][curr_j].captured == (2 + player_id * 3)) {
            if(curr_i == 0) 
                top1 = 1;
            if(curr_i == board_size-1) 
                bottom1 = 1;
            if(curr_j == 0) 
                left1 = 1;
            if(curr_j == board_size-1) 
                right1 = 1;            
            if(top1 == 1 && bottom1 == 1)
                return true;
            if(left1 == 1 && right1 == 1)
                return true;    
            road = DFS(curr_i, curr_j, myboard, player_id, gen_board, size);
            if(road)
                return road;
        }
    }
    return road;
}

void reset_visited(int matrix[8][8]) {
    for(int i = 0; i < board_size; i++) {
        for(int j = 0; j < board_size; j++) {
            matrix[i][j] = 0;
        }
    }
}

bool flat_win(state gen_board[8][8], float &value, bool debug) {
    float my_count = 0;
    float your_count = 0;
    int capt;
    bool flag = false;
    int cur_player_count = cur_player.no_flat;// + cur_player.capstone;
    int other_player_count = other_player.no_flat;// + other_player.capstone;
    if(cur_player_count == 0 || other_player_count == 0) {
        if(debug)    cerr<<"Count is zero "<<cur_player_count<<","<<other_player_count<<endl;
        flag = true;
    }
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
    if(!flag && debug ) {
        cerr<<"Flat win due to board being filled"<<endl;
    }
    // Should not we add the remaining ones as well
    // Carefully check this one out
    if(my_count > your_count)
        value += (float)(my_count/(float)(my_count + your_count));
    else if(my_count < your_count)
      // wrong  value -= (float)(your_count/(float)(my_count + your_count));
        value -= (float)(my_count/(float)(my_count + your_count));
    else {
       if(debug) cerr<<"Returning 0.0001 from flat win"<<endl;
        value += 0.0001; 
    }
    return true;
}

void initialise() {
    top1 = 0;
    left1 = 0;
    bottom1 = 0;
    right1 = 0;
}

double at_endstate(state gen_board[8][8],int debug,bool score, int move){
    bool road = false;
    reset_visited(temp_board);
    if(move == 0) {
        for(int i = 0; i < board_size; i++) {
            int capt_i = gen_board[i][0].captured;
            if((capt_i == 0 || capt_i == 2) && (temp_board[i][0] == 0)) {
                temp_board[i][0] = 1;
                if(i == 0) 
                    top1 = 1;
                left1 = 1;
                road = DFS(i, 0, temp_board, 0, gen_board, 0);
                initialise();
                if(road)
                    return 10.0;
            }
        }
        for(int j = 1; j < board_size; j++) {
            int capt_j = gen_board[0][j].captured;
            if((capt_j == 0 || capt_j == 2) && (temp_board[0][j] == 0)) {
                temp_board[0][j] = 1;
                top1 = 1;
                road = DFS(0, j, temp_board, 0, gen_board, 0);
                initialise();
                if(road)
                    return 10.0;
            }
        }
        reset_visited(temp_board);
        for(int i = 0; i < board_size; i++) {
            int capt_i = gen_board[i][0].captured;
            if((capt_i == 3 || capt_i == 5) && (temp_board[i][0] == 0)) {
                temp_board[i][0] = 1;
                if(i == 0) 
                    top1 = 1;
                left1 = 1;
                road = DFS(i, 0, temp_board, 1, gen_board, 0);
                initialise();
                if(road)
                    return -10.0;
            } 
        }
        for(int j = 1; j < board_size; j++) {
            int capt_j = gen_board[0][j].captured;
            if((capt_j == 3 || capt_j == 5) && (temp_board[0][j] == 0)) {
                temp_board[0][j] = 1;
                top1 = 1;
                road = DFS(0, j, temp_board,  1, gen_board, 0);
                initialise();
                if(road)
                    return -10.0;
            }    
        }
    }
    else {
        for(int i = 0; i < board_size; i++) {
            int capt_i = gen_board[i][0].captured;
            if((capt_i == 3 || capt_i == 5) && (temp_board[i][0] == 0)) {
                temp_board[i][0] = 1;
                if(i == 0) 
                    top1 = 1;
                left1 = 1;
                road = DFS(i, 0, temp_board, 1, gen_board, 0);
                initialise();
                if(road)
                    return -10.0;
            } 
        }
        for(int j = 1; j < board_size; j++) {
            int capt_j = gen_board[0][j].captured;
            if((capt_j == 3 || capt_j == 5) && (temp_board[0][j] == 0)) {
                temp_board[0][j] = 1;
                top1 = 1;
                road = DFS(0, j, temp_board,  1, gen_board, 0);
                initialise();
                if(road)
                    return -10.0;
            }    
        }
        reset_visited(temp_board);
        for(int i = 0; i < board_size; i++) {
            int capt_i = gen_board[i][0].captured;
            if((capt_i == 0 || capt_i == 2) && (temp_board[i][0] == 0)) {
                temp_board[i][0] = 1;
                if(i == 0) 
                    top1 = 1;
                left1 = 1;
                road = DFS(i, 0, temp_board, 0, gen_board, 0);
                initialise();
                if(road)
                    return 10.0;
            }
        }
        for(int j = 1; j < board_size; j++) {
            int capt_j = gen_board[0][j].captured;
            if((capt_j == 0 || capt_j == 2) && (temp_board[0][j] == 0)) {
                temp_board[0][j] = 1;
                top1 = 1;
                road = DFS(0, j, temp_board, 0, gen_board, 0);
                initialise();
                if(road)
                    return 10.0;
            }
        }
    }
    float flat_val = 0.0;
    bool flat_win_check = flat_win(gen_board,flat_val,debug);
    if(!flat_win_check)
        return 0.0;
    else 
        return flat_val;
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

void influence(float infl_arr[8][8], state gen_board[8][8], float flat, float wall, float cap) {
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

double get_heuristic(state gen_board[8][8], bool debug) {
    // Define different heuristic values
    heuristic_value = 0.0;
    captured = 0.0;
    threats = 0.0;
    piece_val = 0.0;

    // Initialize our array
    int arr[16][6]; 
    for(int i=0;i<2*board_size;i++)
        for(int j=0;j<6;j++)
            arr[i][j]=0;
    
    // Calculate pieces in each row and column
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
        for(int i = 0; i < board_size; i++) {
            captured += (arr[i][0]-arr[i][3])*50 + (arr[i][2]-arr[i][5])*80;
        }

    // SAME ROW PIECES HAS MORE WEIGHT
        composition_value = 0.0;
        center_value = 0.0;
        for(int i=0;i<2*board_size;i++){
            flat_capt_me = arr[i][0];
            wall_capt_me = arr[i][1];
            cap_capt_me = arr[i][2];
            my_capt = flat_capt_me + cap_capt_me;
            flat_capt_you = arr[i][3];
            wall_capt_you = arr[i][4];
            cap_capt_you = arr[i][5];
            your_capt = flat_capt_you + cap_capt_you;
            capt_diff = my_capt - your_capt;
            capture_advantage = diff[my_capt];
            capture_disadvantage = diff[your_capt];
            capture_delta = 0.0;// + 5.0* (max(your_capt, my_capt)-1)/();
            if(capt_diff > 0){
                capture_delta =  5.0 * (my_capt-1) ;
                wall_disadvantage = wall_capt_me*for_wall + wall_capt_you*against_wall + diff[capt_diff]*(wall_capt_me + wall_capt_you);
            }
            else if(capt_diff < 0){
                 capture_delta =  -5.0 * (your_capt-1) ;
                wall_disadvantage = (wall_capt_me*for_wall + wall_capt_you*against_wall - diff[-1*capt_diff]*(wall_capt_me + wall_capt_you));
            }
            else
                wall_disadvantage = wall_capt_me*for_wall + wall_capt_you*for_wall;
            composition_value += capture_advantage - capture_disadvantage +capture_delta - 0.9*wall_disadvantage;
            
            // CENTER CONTROL
            if(i < board_size)
                center_value += (cap_capt_me - cap_capt_you)*(board_size-i-1)*i*center_weight;
            else
                center_value += (cap_capt_me - cap_capt_you)*(2*board_size-i-1)*(i-board_size)*center_weight;
        }

    // CURRENT PIECE HOLDINGS
        // Check to see if capstone is available
        if(other_player.capstone == 1)
            piece_val -= 60;
        if(cur_player.capstone == 1)
            piece_val += 60;

        // See how many flatstones left
        piece_val -= 24*cur_player.no_flat;
        piece_val += 24*other_player.no_flat;

    // INFLUENCE
        double infl_value = 0;
        float flat = 3;
        float wall = 2.98;
        float cap = 2.99;
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

    // // SIZE OF STACK HEURISTIC
    //     double stack_size_value = 0;
    //     float size_val = 1;
    //     float power_size = 2.0;
    //     for(int i = 0; i < board_size; i++){
    //         for(int j = 0; j < board_size; j++){
    //             int size_stack = gen_board[i][j].state_stack.size();
    //             if(size_stack > 1){
    //                 if(gen_board[i][j].captured >= 3)
    //                     stack_size_value -= size_stack*size_stack*size_val;
    //                 else
    //                     stack_size_value += size_stack*size_stack*size_val;
    //             }
    //         }
    //     }

    // // COMPOSITION OF STACK HEURISTIC
    //     double stack_composition_value = 0;
    //     float comp_val = 1;
    //     float comp_power = 2.0;
    //     for(int i = 0; i < board_size; i++){
    //         for(int j = 0; j < board_size; j++){
    //             int size = gen_board[i][j].state_stack.size();
    //             if(size > 1){
    //                 stack<int> temp(gen_board[i][j].state_stack);
    //                 int count_me = 0;
    //                 for(int k = 0; k < size; k++){
    //                     if(temp.top() < 3)
    //                         count_me++;
    //                     temp.pop();
    //                 }
    //                 if(count_me/size > 0.5)
    //                     stack_composition_value += (count_me)*(1 - count_me/size)*comp_val*comp_val*size;
    //                 else if (count_me/size < 0.5)
    //                     stack_composition_value -= (count_me)*(1 - count_me/size)*comp_val*comp_val*size;
    //                 else if(gen_board[i][j].captured >= 3)
    //                     stack_composition_value -= size*size;
    //                 else
    //                     stack_composition_value += size*size;
    //             }
    //         }
    //     }

    // MOVE ADVANTAGE -- working quite well
        double move_advantage = 0.0;
        if(current_player == -1)
            move_advantage -= 250;
        else if(current_player == 1)
            move_advantage += 250;

    heuristic_value = move_advantage + 1.4*captured + 1.55*composition_value + piece_val + 0.9*infl_value + 1.1*center_value;//*stack_size_value;// + 0.1*stack_composition_value;   
     if(debug)
    {
        cerr<<"The value of heuristic is "<<heuristic_value<<endl;
        cerr<<"The Move Advantage is "<<move_advantage<<endl;
        cerr<<"The Captured is "<<captured<<" , "<<1.4*captured<<endl;
        cerr<<"The Composition Value is "<<composition_value<<" , "<<1.55*composition_value<<endl;
        cerr<<"The Piece value is "<<piece_val<<endl;
        cerr<<"The Influence Value is "<<infl_value<<" , "<<0.9*infl_value<<endl;
        cerr<<"The Centre Value is "<<center_value<<endl;
    }
    return heuristic_value;
}

void generate_all_moves(int id, state gen_board[8][8],int &size) {
    // Initialize variables
    size=0;
    string move;
    bool valid;
    Player myplayer;
    int index, capt, curr_capt;

    if(id == player_id) {
        myplayer = cur_player;
        index = 0;
    }
    else {
        myplayer = other_player;
        index = 1;
    }
    for(int i = 0; i < board_size; i++) {
        for(int j = 0; j < board_size; j++) {
            capt = gen_board[i][j].captured;
            if(capt == -1){
                if(myplayer.no_flat != 0) {                            
                    move = "F" ; 
                    move += (char)(97+j); 
                    move += (char)(49+i);
                    all_moves[size] = move; 
                    size++;
                }
            }
            else if(capt >= 3*index && capt < 3 + 3*index){
                temp_size = gen_board[i][j].state_stack.size();
                stack_size = min(temp_size, board_size);
                dist_up = i;
                dist_down = board_size - 1 - i;
                dist_left = j; 
                dist_right = board_size - 1 - j;
                

                for(int x = 1; x <= stack_size; x++) {
                    part_list = partition(x);
                    for(int y = 0; y < part_list.size(); y++) { 
                        int part_size = part_list[y].size(); 
                        if(part_size <= dist_up) { 
                            valid = true;
                            move = ""; 
                            move += (char)(48+x); 
                            move += (char)(97+j); 
                            move += (char)(49+i); 
                            move += "-";
                            int z1 = -1;
                            for(int z = 0; z < part_size; z++) {
                                curr_capt = gen_board[i-z-1][j].captured;
                                if(curr_capt % 3 == 0 || curr_capt == -1)
                                    move += (char)(48+part_list[y][z]);
                                else {
                                    valid = false;
                                    z1 = z;
                                    break;
                                }
                            }
                            int z = part_size-1;
                            curr_capt = gen_board[i-z-1][j].captured;
                            if(z1 == z)
                                if((part_list[y][z] == 1) && ((curr_capt % 3 == 1) ) && (capt == 2 + index*3)) {
                                        move += "1";
                                        valid = true;
                                }
                            if(valid) {
                                all_moves[size] = move; 
                                size++;
                            }
                        }
                        if(part_size <= dist_down) {
                            valid = true;
                            move = ""; 
                            move += (char)(48+x); 
                            move += (char)(97+j); 
                            move += (char)(49+i); 
                            move += "+";
                            int z1 = -1;
                            for(int z = 0; z < part_size; z++) {
                                curr_capt = gen_board[i+z+1][j].captured;
                                if(curr_capt % 3 == 0 || curr_capt == -1)
                                    move += (char)(48+part_list[y][z]); 
                                else {
                                    valid = false;
                                    z1 = z;
                                    break;
                                }
                            }
                            int z = part_size-1;
                            curr_capt = gen_board[i+z+1][j].captured;
                            if(z1 == z)
                                if((part_list[y][z] == 1) && (curr_capt % 3 == 1) && (capt == 2 + index*3)) {
                                        move += "1";
                                        valid = true;
                                    }
                            if(valid) {
                                all_moves[size] = move; 
                                size++;
                            }
                        }
                        if(part_size <= dist_left) {
                            valid = true;
                            move = ""; 
                            move += (char)(48+x); 
                            move += (char)(97+j); 
                            move += (char)(49+i); 
                            move += "<";
                            int z1 = -1;
                            for(int z = 0; z < part_size; z++) {
                                curr_capt = gen_board[i][j-z-1].captured;
                                if(curr_capt % 3 == 0 || curr_capt == -1)
                                    move += (char)(48+part_list[y][z]);  
                                else {
                                    valid = false;
                                    z1 = z;
                                    break;
                                }
                            }
                            int z = part_size-1;
                            curr_capt = gen_board[i][j-z-1].captured;
                            if(z1 == z)
                                if((part_list[y][z] == 1) && (curr_capt % 3 == 1) && (capt == 2 + index*3)) {
                                        move += "1";
                                        valid = true;
                                    } 
                            if(valid) {
                                all_moves[size] = move; 
                                size++;
                            }
                        }
                        if(part_size <= dist_right) {
                            valid = true;
                            move = ""; 
                            move += (char)(48+x); 
                            move += (char)(97+j); 
                            move += (char)(49+i); 
                            move += ">";
                            int z1 = -1;
                            for(int z = 0; z < part_size; z++){
                                curr_capt = gen_board[i][j+z+1].captured;
                                if(curr_capt % 3 == 0 || curr_capt == -1)
                                    move += (char)(48+part_list[y][z]); 
                                else {
                                    valid = false;
                                    z1 = z;
                                    break;
                                }
                            }
                            int z = part_size-1;
                            curr_capt = gen_board[i][j+z+1].captured;
                            if(z1 == z)
                                if((part_list[y][z] == 1) && (curr_capt % 3 == 1) && (capt == 2 + index*3)) {
                                        move += "1";
                                        valid = true;
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
    for(int i = 0; i < board_size; i++) {
        for(int j = 0; j < board_size; j++) {
            capt = gen_board[i][j].captured;
            if(capt == -1){
                if(myplayer.capstone != 0) {
                    move = "C" ; 
                    move += (char)(97+j); 
                    move += (char)(49+i);
                    all_moves[size] = move; 
                    size++;                            
                }
            }
        }
    }
    for(int i = 0; i < board_size; i++) {
        for(int j = 0; j < board_size; j++) {
            capt = gen_board[i][j].captured;
            if(capt == -1){
                if(myplayer.no_flat != 0) {                            
                    move = "S" ; 
                    move += (char)(97+j); 
                    move += (char)(49+i);
                    all_moves[size] = move; 
                    size++;                           
                }
            }
        }
    }
}

double best_move(state myboard[8][8],double alpha,double beta,int depth,string &best_move_chosen,bool minimum) {
    // Declare Variables
    // if(depth>=3)
    best_called++;
    int move_player;
    vector<pair<double,string> > values;
    double min_val = LONG_MAX, max_val = LONG_MIN, child, ans, val;

    // if(Transposition_Table.find(global_hash) != Transposition_Table.end()) {
    //     storage temp = Transposition_Table.find(global_hash)->second;
    //     if(temp.depth >= depth) {
    //         if(depth >= 3) 
    //             repeated++;
    //         best_move_chosen= temp.best_move;
    //         return temp.value;
    //     }
    // }    

    if(!minimum && player_id == 1)
        current_player = -1;
    else if (minimum && player_id == 2)
        current_player = 1;
    else
        current_player = 0;

    if(minimum) {
        if(player_id == 1) 
            move_player = 2;
        else 
            move_player = 1;
    }
    else  
        move_player = player_id;

    int size = 0;
    // number_called_generate_moves++ ;
    // func_begin_time = clock();   
    generate_all_moves(move_player,myboard,size);
    // func_end_time = clock();
    // time_generate_moves += float( func_end_time - func_begin_time ) /  CLOCKS_PER_SEC ;

    for(int i = 0; i < size; i++) {
        int crushed = 0;
     //    number_called_execute_moves++ ;
        // func_begin_time = clock();   
        string_to_move_cur(all_moves[i], move_player, myboard, crushed, false);
     //    func_end_time = clock();
        // time_execute_moves += float( func_end_time - func_begin_time ) /  CLOCKS_PER_SEC ;

        // number_called_end_states++ ;
        // func_begin_time = clock();   
        ans = at_endstate(myboard,0);
     //    func_end_time = clock();
        // time_end_states += float( func_end_time - func_begin_time ) /  CLOCKS_PER_SEC ;

        if(ans == 10.0) {
            // cerr<<"my road_win found"<<endl;
            val = LONG_MAX;
        }
        else if(ans == -10.0) {
            // cerr<<"your road_win found"<<endl;
            val = LONG_MIN;
        }
        else if(ans == 0.0) {
         //    number_called_get_heuristic++;
            // func_begin_time=clock();
            val = get_heuristic(myboard, false);
            // func_end_time=clock();
            // time_get_heuristic += float( func_end_time - func_begin_time ) /  CLOCKS_PER_SEC ;
        }
        else {
            // cerr<<"Flat win detected"<<endl;
            val = ans*LONG_MAX;  
        }
        if(depth != 1)
            values.push_back(std::make_pair(val, all_moves[i]));
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

     //    number_called_undo_moves++;
        // func_begin_time=clock();
        undo_move(all_moves[i], move_player, myboard, crushed, false);
        // func_end_time = clock();
        // time_undo_moves += float( func_end_time - func_begin_time ) /  CLOCKS_PER_SEC ;

        if(alpha > beta) {
            return child;
        } 
    }
    if(depth == 1) {
        if(minimum)
            return min_val;
        else
            return max_val;
    }

    if(minimum) {
        priority_queue<pair<double,string>, vector<pair<double,string> >, Compare_max> maxi_heap(values.begin(),values.end());          
        for(int i = 0; i < size; i++) {
            string move_taken = "";
            double heur_val = maxi_heap.top().first;
            move_taken = maxi_heap.top().second;
            maxi_heap.pop();
            int crushed = 0;

         //    number_called_execute_moves++ ;
            // func_begin_time = clock();   
            uint64_t a1 = global_hash;
            string_to_move_cur(move_taken, move_player, myboard, crushed, true);
      //       func_end_time = clock();
            // time_execute_moves += float( func_end_time - func_begin_time ) /  CLOCKS_PER_SEC ;

            string tmp = "";
            if(heur_val == LONG_MAX)
                child = LONG_MAX;
            else if(heur_val == LONG_MIN)
                child = LONG_MIN;
            else if(heur_val > LONG_MAX/100 )
            {
                if(get_heuristic(myboard, false) < 700) // to check if flat win or not
                    child = heur_val;
                else
                    child = best_move(myboard, alpha, beta, (depth-1), tmp, !minimum);
            }
            else if(heur_val < LONG_MIN/100)
            {
                if(get_heuristic(myboard, false) > -700) // to check if flat win or not
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

      //       number_called_undo_moves++;
            // func_begin_time=clock();
            undo_move(move_taken, move_player, myboard, crushed,true);
            // func_end_time = clock();
            // time_undo_moves += float( func_end_time - func_begin_time ) /  CLOCKS_PER_SEC ;
       
            if(alpha > beta) {
                // if(depth != 1)
             //     cerr<<"Pruned at "<<i<<" at depth "<<depth<<endl;
                // storage temp(best_move_chosen,child,depth);
                // Transposition_Table.insert(std::make_pair(global_hash,temp));
                return child;
            }    
        }
        // storage temp(best_move_chosen,min_val,depth);
        // Transposition_Table.insert(std::make_pair(global_hash,temp));
        return min_val;
    }
    else {
        priority_queue<pair<double,string>, vector<pair<double,string> >, Compare_min> mini_heap(values.begin(),values.end());   
        for(int i = 0; i < size; i++) { 
            string move_taken = "";
            double heur_val = mini_heap.top().first;
            move_taken = mini_heap.top().second;
            mini_heap.pop();
            int crushed = 0;

   //          number_called_execute_moves++ ;
            // func_begin_time = clock();   
            string_to_move_cur(move_taken, move_player, myboard, crushed,true);
      //       func_end_time = clock();
            // time_execute_moves += float( func_end_time - func_begin_time ) /  CLOCKS_PER_SEC ;

            string tmp = "";
            if(heur_val == LONG_MAX)
                child = LONG_MAX;
            else if(heur_val == LONG_MIN)
                child = LONG_MIN;
            else if(heur_val > LONG_MAX/100 )
            {
                if(get_heuristic(myboard, false) < 700) // to check if flat win or not
                    child = heur_val;
                else
                    child = best_move(myboard, alpha, beta, (depth-1), tmp, !minimum);
            }
            else if(heur_val < LONG_MIN/100)
            {
                if(get_heuristic(myboard, false) > -700) // to check if flat win or not
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

      //       number_called_undo_moves++;
            // func_begin_time=clock();
            undo_move(move_taken, move_player, myboard, crushed,true);
      //       func_end_time = clock();
            // time_undo_moves += float( func_end_time - func_begin_time ) /  CLOCKS_PER_SEC ;
       
           if(alpha > beta){
                //if(depth!=1)
                //cerr<<"Pruned at "<<i<<" at depth "<<depth<<endl;
                // storage temp(best_move_chosen,child,depth);
                // Transposition_Table.insert(std::make_pair(global_hash,temp));
                return child;
            }    
        }
        // storage temp(best_move_chosen,max_val,depth);
        // Transposition_Table.insert(std::make_pair(global_hash,temp));
        return max_val;
    }
} 

void print_data(double total_time) {
    // cerr<<"The data for the move is "<<endl;
    cerr<<"The total time taken : "<< total_time<<endl;
    // cerr<<"Get heuristic Function"<<endl;
    // cerr<<"Times Called : "<<number_called_get_heuristic<<endl;
    // cerr<<"Total time taken : "<<time_get_heuristic<<endl;
    // cerr<<"Influence Function"<<endl;
    // cerr<<"Times Called : "<<number_called_influence<<endl;
    // cerr<<"Total time taken : "<<time_influence<<endl;
    // cerr<<"Generate Moves Function"<<endl;
    // cerr<<"Times Called : "<<number_called_generate_moves<<endl;
    // cerr<<"Total time taken : "<<time_generate_moves<<endl;
    // cerr<<"Execute Moves Function"<<endl;
    // cerr<<"Times Called : "<<number_called_execute_moves<<endl;
    // cerr<<"Total time taken : "<<time_execute_moves<<endl;
    // cerr<<"Undo Move Function"<<endl;
    // cerr<<"Times Called : "<<number_called_undo_moves<<endl;
    // cerr<<"Total time taken : "<<time_undo_moves<<endl;
    // cerr<<"End State Function"<<endl;
    // cerr<<"Times Called : "<<number_called_end_states<<endl;
    // cerr<<"Total time taken : "<<time_end_states<<endl;
 //    cerr<<"Percentage : "<<(float)time_end_states/total_time*100<<endl;

 // time_get_heuristic=0.0;
 // time_influence=0.0;
 // time_generate_moves=0.0;
 // time_execute_moves=0.0;
 // time_undo_moves=0.0;
 // time_end_states=0.0;

 // number_called_get_heuristic=0;
 // number_called_influence=0;
 // number_called_generate_moves=0;
 // number_called_execute_moves=0;
 // number_called_undo_moves=0;
 // number_called_end_states=0;
}

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
    count1 += cur_player.no_flat;
    count2 += other_player.no_flat;
    if(winner)
        return count1;
    else
        return count2;
}

int main(int argc, char** argv) {
    diff[0] = 0;
    diff[1] = 25+10;
    diff[2] = 55+20;
    diff[3] = 85+35;
    diff[4] = 120+50;
    diff[5] = 200+35;
    diff[6] = 285;
    diff[7] = 400;
    mapping[0] = flatstone;
    mapping[1] = standing;
    mapping[2] = capstone;
    
    // myval = strtod(argv[1], NULL);
    myval = 24;
    srand(time(NULL));

    // Clock variables
    float time_player = 0.0;
    clock_t begin_time;
    clock_t end_time;

    float endstate_val;
    state Board [8][8];

    int n, time_limit;
    cerr<<"started"<<endl;
    cin>>player_id>>n>>time_limit;
    begin_time = clock();

    board_size = n;
    // init_zobrist();
    cur_player.assign(board_size);
    other_player.assign(board_size);
    for(int i = 0; i < board_size; i++)
        for(int j = 0; j < board_size; j++)
            Board[i][j].captured = -1;
    int debug = 1;    
    string move;
    int temp_size;
    int crush = 0;        
    bool on = true;
    if(player_id == 2) {
        cin>>move;
        begin_time = clock();
        crush = 0;
        string_to_move_cur(move, 2, Board, crush, false);
        // print_board(Board);
        generate_all_moves(1, Board, temp_size);
        int randmove = rand()%temp_size;
        while(all_moves[randmove][randmove] != 'F')
            randmove = rand()%temp_size;
        // string_to_move_cur(all_moves[randmove], 1, Board, crush, false);
        // cout<<all_moves[randmove]<<endl;
        string_to_move_cur(all_moves[3], 1, Board, crush, false);
        cout<<all_moves[3]<<endl;
        end_time = clock();
        time_player += float( end_time - begin_time ) /  CLOCKS_PER_SEC;
        // print_board(Board);       
        int count = 0;

        while(on) {   
            prune = 0;
            cin>>move;
            begin_time = clock();
            crush = 0;
            string_to_move_cur(move, 1, Board, crush, false);
            crush = 0;
            // print_board(Board);
            cerr<<endl;
            endstate_val = at_endstate(Board,debug);
            if(endstate_val > 0.0) {
                cerr<<"You are the winner"<<endl;
                if(endstate_val == 10.0)
                    cerr<<"Scorewin "<<get_score(Board, true) + board_size*board_size<<endl;
                else
                    cerr<<"Scorewin "<<get_score(Board, true)<<endl;
                cerr<<"Scorelose "<<get_score(Board, false)<<endl;
            }
            else if(endstate_val < 0.0) {
                cerr<<"You are the loser"<<endl;
                if(endstate_val == -10.0)
                    cerr<<"Scorewin "<<get_score(Board, false) + board_size*board_size<<endl;
                else
                    cerr<<"Scorewin "<<get_score(Board, false)<<endl;
                cerr<<"Scorelose "<<get_score(Board, true)<<endl;
            }
            current_player = 1;
            // cerr<<"the heuristic value is = "<<get_heuristic(Board, true)<<endl;
            generate_all_moves(2, Board, temp_size);
            string next_move = "";
            double val;
            int limit = 6;
            cerr<<"\nCount is "<<count<<endl;
            if(time_limit - time_player < 8)
                limit = 4;
            else if(time_limit - time_player < 22)
                limit = 5;
            for(int i = 1; i <= limit; i++) {
                best_called = 0;
                ids_start = clock();
                val = best_move(Board, LONG_MIN/2, LONG_MAX/2, i, next_move, false);
                ids_end = clock();
                float time = float(ids_end - ids_start) / CLOCKS_PER_SEC;
                cerr<<"Depth "<<i<<"="<<next_move;
                cerr<<"\ttime taken = "<<time;
                cerr<<"\teffective branching factor = "<<pow(best_called,(1.0/(i+1)));
                cerr<<"\tnodes = "<<best_called<<endl;
                if(time > time_threshold/pow(best_called,(1.0/(i+1))))
                    break;
            }
            // cerr<<"Repeated "<<repeated<<" out of "<<best_called<<endl;
            repeated = 0;
            //Transposition_Table.clear();
            string_to_move_cur(next_move, 2, Board, crush, false);
            // print_board(Board);
            endstate_val = at_endstate(Board, debug);
            if(endstate_val > 0.0) {
                cerr<<"You are the winner"<<endl;
                if(endstate_val == 10.0)
                    cerr<<"Scorewin "<<get_score(Board, true) + board_size*board_size<<endl;
                else
                    cerr<<"Scorewin "<<get_score(Board, true)<<endl;
                cerr<<"Scorelose "<<get_score(Board, false)<<endl;
            }
            else if(endstate_val < 0.0) {
                cerr<<"You are the loser"<<endl;
                if(endstate_val == -10.0)
                    cerr<<"Scorewin "<<get_score(Board, false) + board_size*board_size<<endl;
                else
                    cerr<<"Scorewin "<<get_score(Board, false)<<endl;
                cerr<<"Scorelose "<<get_score(Board, true)<<endl;
            }
            current_player = 0;
            cerr<<"the heuristic value is = "<<get_heuristic(Board, true)<<","<<val<<endl;
            count++;
            cout<<next_move<<endl;
            end_time = clock();
            double time_of_move = 0.0;
            time_of_move = float( end_time - begin_time ) /  CLOCKS_PER_SEC;
            time_player += time_of_move;
            print_data(time_of_move);
        }
    }   
    else if(player_id == 1) {
        generate_all_moves(2, Board, temp_size);
        int randmove = rand()%temp_size;
        while(all_moves[randmove][0] != 'F')
            randmove = rand()%temp_size;
        // cerr<<all_moves[randmove]<<" \n";
        // string_to_move_cur(all_moves[randmove], 2, Board, crush, false);
        // cout<<all_moves[randmove]<<endl;
        cerr<<all_moves[0]<<" \n";
        string_to_move_cur(all_moves[0], 2, Board, crush, false);
        cout<<all_moves[0]<<endl;
        end_time = clock();
        time_player += float( end_time - begin_time ) /  CLOCKS_PER_SEC;
        crush = 0;
        cerr<<endl;
        cin>>move;
        begin_time = clock();
        string_to_move_cur(move, 1, Board, crush, false);
        int count = 0;
        while(on) {
            crush = 0;
            string next_move;
            double val;
            cerr<<"\nCount is "<<count<<endl;
            count++;
            int limit = 6;
            if(time_limit - time_player < 8)
                limit = 4;
            else if(time_limit - time_player < 42)
                limit = 5;
            for(int i = 2; i <= limit; i++) {
                best_called = 0;
                ids_start = clock();
                val = best_move(Board, LONG_MIN/2, LONG_MAX/2, i, next_move, false);
                ids_end = clock();
                float time = float(ids_end - ids_start) / CLOCKS_PER_SEC;
                cerr<<"Depth "<<i<<"="<<next_move;
                cerr<<"\ttime taken = "<<time;
                cerr<<"\teffective branching factor = "<<pow(best_called,(1.0/(i+1)));
                cerr<<"\tnodes = "<<best_called<<endl;
                if(time > time_threshold/pow(best_called,(1.0/(i+1))))
                    break;
            }
            string_to_move_cur(next_move, 1, Board, crush, false);
            // print_board(Board);
            endstate_val = at_endstate(Board, debug);
            if(endstate_val > 0.0) {
                cerr<<"You are the winner"<<endl;
                if(endstate_val == 10.0)
                    cerr<<"Scorewin "<<get_score(Board, true) + board_size*board_size<<endl;
                else
                    cerr<<"Scorewin "<<get_score(Board, true)<<endl;
                cerr<<"Scorelose "<<get_score(Board, false)<<endl;
            }
            else if(endstate_val < 0.0) {
                cerr<<"You are the loser"<<endl;
                if(endstate_val == -10.0)
                    cerr<<"Scorewin "<<get_score(Board, false) + board_size*board_size<<endl;
                else
                    cerr<<"Scorewin "<<get_score(Board, false)<<endl;
                cerr<<"Scorelose "<<get_score(Board, true)<<endl;
            }
            current_player = -1;
            cerr<<"the heuristic value is = "<<get_heuristic(Board, true)<<","<<val<<endl;
            cout<<next_move<<endl;
            end_time = clock();
            double time_of_move = 0.0;
            time_of_move = float( end_time - begin_time ) /  CLOCKS_PER_SEC;
            time_player += time_of_move;
            print_data(time_of_move);  
            cin>>move;
            begin_time = clock();
            string_to_move_cur(move, 2, Board, crush, false);
            print_board(Board);
            endstate_val = at_endstate(Board,debug);
            if(endstate_val > 0.0){
                cerr<<"You are the winner"<<endl;
                if(endstate_val == 10.0)
                    cerr<<"Scorewin "<<get_score(Board, true) + board_size*board_size<<endl;
                else
                    cerr<<"Scorewin "<<get_score(Board, true)<<endl;
                cerr<<"Scorelose "<<get_score(Board, false)<<endl;
            }
            else if(endstate_val < 0.0){
                cerr<<"You are the loser"<<endl;
                if(endstate_val == -10.0)
                    cerr<<"Scorewin "<<get_score(Board, false) + board_size*board_size<<endl;
                else
                    cerr<<"Scorewin "<<get_score(Board, false)<<endl;
                cerr<<"Scorelose "<<get_score(Board, true)<<endl;
            }
            current_player = 0;
            // cerr<<"the heuristic value is = "<<get_heuristic(Board, true)<<","<<val<<endl;       
        }
    }
    return 0;
}
