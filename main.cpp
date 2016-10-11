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

using namespace std;

#define MAX_BOARD_SIZE 8;

// Some cout variables
int prune=0;
int best_called=0;

// Evaluation function variables
double capstone = 90;
double flatstone = 75;
double standing = 50;
double cap_threat = 40;
double stand_threat = 40; 

double mapping[3];
double diff[8];

// Define some global arrays
int player_id;
int board_size;

struct Compare_min
{
    bool operator()(pair<double,string> p1,pair<double,string> p2)
    {
        return p1.first<p2.first;
    }
};

struct Compare_max
{
    bool operator()(pair<double,string> p1,pair<double,string> p2)
    {
        return p1.first>p2.first;
    }
};

class state{
  public:   
    stack<int> state_stack;
    int captured;
    
    // Constructor
    state(){    
       captured=-1;
    }
    void assign(state Board)
    {
        stack<int> temp(Board.state_stack);
        swap(this->state_stack,temp);
        this->captured=Board.captured;
        
    }
};

class Player{
  public:
    // to store no of pieces remaining
    int no_flat;
    int capstone;
    int id; // 1 or 2e

    void assign (int n){
        if(n == 5){
            this->no_flat=21;
            this->capstone=1;
        }
        else if(n == 6){
            this->no_flat=30;
            this->capstone=1;   
        }
        else if(n == 7){
            this->no_flat=40;
            this->capstone=1;
        }
    }
}cur_player, other_player;

void string_to_move_cur(string move,int id,state myBoard[8][8],int &crushed) { // executes a valid move on the game board
    int j = (int)(move[1]) - 96;    // stores movement across a,b,...
    int i = (int)(move[2]) - 48;    // stores from 1,2..
    if(!isdigit(move[0])) {
        assert(myBoard[i-1][j-1].captured == -1);
        int x;
        if(move[0]=='F') x = 0;
        else if(move[0]=='S') x = 1;
        else if(move[0]=='C') x = 2;
        if(id != player_id) 
            x = x+3;    // Maps the pices to those of the opponent (3,4,5)
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
        for(int l = 0; l < no_picked; l++) {
            top = myBoard[i-1][j-1].state_stack.top();
            myBoard[i-1][j-1].state_stack.pop();
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
        if(dir == '+'){
            mi = 1; 
            mj = 0;
        }
        else if(dir == '-'){
            mi = -1; 
            mj = 0;
        }
        else if(dir == '>'){
            mi = 0; 
            mj = 1;
        }
        else{
            mi = 0; 
            mj = -1;
        }
            int w1,w2;  
        for(int k = 1; k <= drop.size(); k++) {
            w1 = i-1+k*mi;
            w2 = j-1+k*mj;
            stack<int> tempo(myBoard[w1][w2].state_stack);
            int cap = myBoard[w1][w2].captured;
            int x1 = 1;
            int top1, t1;
            t1 = myBoard[w1][w2].captured; 
            while(x1 <= drop[k-1]) {
                top1 = picked.top();
                picked.pop();
                if(drop[k-1] == 1 && t1 % 3 == 1) {
                    int xw = tempo.top();
                    tempo.pop();
                    tempo.push(xw-1);
                    crushed=1;      
                }   
                t1 = top1;
                tempo.push(top1);
                x1++;
            }
            myBoard[w1][w2].captured = top1;
            swap(tempo, myBoard[w1][w2].state_stack);   
        } 
    }
}

void undo_move(string move,int id,state gen_Board[8][8],int crushed) {
    int j = (int)(move[1]) - 96;    // stores movement across a,b,...
    int i = (int)(move[2]) - 48;    // stores from 1,2..
    // if first position is integer then its a Move move else a place
    if(!isdigit(move[0])){
        int x;
        gen_Board[i-1][j-1].captured = -1;
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
        //dropped stores the amount it drops
        int mi, mj;
        if(dir == '+')
            {mi = 1; mj = 0; }
        else if(dir == '-')
            {mi = -1; mj = 0;}
        else if(dir == '>')
            {mi = 0; mj = 1; }
        else
            {mi = 0; mj = -1;}
        stack<int> reverse_drop;
        int pick_up,w1,w2 ;
        for(int k=drops;k>0;k--) {
            w1 = i-1+k*mi;
            w2 = j-1+k*mj;
            pick_up = dropped[k-1];
            int captured = gen_Board[w1][w2].captured;
            if(k == drops && gen_Board[w1][w2].state_stack.size() >= 2 && pick_up == 1 && captured % 3 == 2) {
                    reverse_drop.push(captured);
                    gen_Board[w1][w2].state_stack.pop();
                    captured = gen_Board[w1][w2].state_stack.top();
                    if(captured % 3 == 0 && crushed == 1) {
                        int wx = captured;
                        gen_Board[w1][w2].state_stack.pop();
                        wx++;
                        gen_Board[w1][w2].state_stack.push(wx);
                        gen_Board[w1][w2].captured = wx;
                    }
                    else {
                        gen_Board[w1][w2].captured = captured;
                    }       
                }
            else {   
                int x1 = 1;
                while(x1 <= pick_up) {   
                    reverse_drop.push(gen_Board[w1][w2].state_stack.top());
                    gen_Board[w1][w2].state_stack.pop();
                    x1++;
                }
                if(gen_Board[w1][w2].state_stack.size() == 0)
                    gen_Board[w1][w2].captured = -1;
                else
                    gen_Board[w1][w2].captured = gen_Board[w1][w2].state_stack.top();
            }
        }
        while(reverse_drop.size() != 0) {
            int picking = reverse_drop.top();
            reverse_drop.pop();
            gen_Board[i-1][j-1].state_stack.push(picking);
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

stack<int> get_neighbors(int i, int j) {
    stack<int> neighbors;
    if(i != 0)
        neighbors.push((i-1)*board_size + j);
    if(j != 0)
        neighbors.push(i*board_size + j-1);
    if(i != board_size-1)
        neighbors.push((i+1)*board_size + j);
    if(j != board_size-1)
        neighbors.push(i*board_size + j+1);
    return neighbors;
}

    void extract_ij(int pos, int &i, int &j) {
        i = pos / board_size;
        j = pos % board_size;
    }

    int DFS(int i, int j, int myboard[8][8], string direction, int player_id, state gen_board[8][8]) {
        bool road = false;
        stack<int> neighbors;
        neighbors = get_neighbors(i, j);
        int curr, curr_i, curr_j;
        while(!neighbors.empty()) {
            curr = neighbors.top();
            neighbors.pop();
            extract_ij(curr, curr_i, curr_j);
            if(myboard[curr_i][curr_j] == 1)
                continue;
            myboard[curr_i][curr_j] = 1;
            if(gen_board[curr_i][curr_j].captured == (0 + player_id * 3) || gen_board[curr_i][curr_j].captured == (2 + player_id * 3)) {
                if(direction == "horizontal") {
                    if(curr_j == board_size - 1)
                        return true;
                    road = DFS(curr_i, curr_j, myboard, "horizontal", player_id, gen_board);
                }
                else if(direction == "vertical")  {
                    if(curr_i == board_size - 1)
                        return true;
                    road = DFS(curr_i, curr_j, myboard, "vertical", player_id, gen_board);
                }
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

    bool flat_win(state gen_board[8][8], float &value){
        int my_count = 0;
        int your_count = 0;
        int capt;
        bool flag = false;
        int cur_player_count=cur_player.no_flat+cur_player.capstone;
        int other_player_count=other_player.no_flat+other_player.capstone;
        if(cur_player_count == 0 || other_player_count == 0)
            flag = true;
        if(!flag){
            for(int i = 0; i < board_size; i++){
                for(int j =0; j < board_size; j++){
                    capt = gen_board[i][j].captured;
                    if(capt == -1){
                        return false;
                    }
                    else if(capt == 0 || capt == 2)
                        my_count++;
                    else if(capt == 3 || capt == 5)
                        your_count++;
                }
            }
        }    
        if(my_count > your_count)
            value = (my_count / (my_count + your_count));
        else if(my_count < your_count)
            value = -1*(your_count / (my_count + your_count)); 
        else
            value = 0;
            return true;
    }

    double at_endstate(state gen_board[8][8]) {
        bool road = false;
        int temp_board[8][8];
        reset_visited(temp_board);
        
        for(int i = 0; i < board_size; i++) {
            int capt_i = gen_board[i][0].captured;
            if((capt_i == 0 || capt_i == 2) && (temp_board[i][0] == 0)) {
                temp_board[i][0] = 1;
                road = DFS(i, 0, temp_board, "horizontal", 0, gen_board);
                if(road){
                    return 1.0;
                }
            }
        }
        reset_visited(temp_board);

        for(int i = 0; i < board_size; i++) {
            int capt_i = gen_board[i][0].captured;
            if((capt_i == 3 || capt_i == 5) && (temp_board[i][0] == 0)) {
                temp_board[i][0] = 1;
                road = DFS(i, 0, temp_board, "horizontal", 1, gen_board);
                if(road){
                    return -1.0;
                }
            } 
        }
        reset_visited(temp_board);
        
        for(int j = 0; j < board_size; j++) {
            int capt_j = gen_board[0][j].captured;
            if((capt_j == 0 || capt_j == 2) && (temp_board[0][j] == 0)) {
                temp_board[0][j] = 1;
                road = DFS(0, j, temp_board, "vertical", 0, gen_board);
                if(road){
                    return 1.0;
                }
            }
        }
        reset_visited(temp_board);

        for(int j = 0; j < board_size; j++) {
            int capt_j = gen_board[0][j].captured;
            if((capt_j == 3 || capt_j == 5) && (temp_board[0][j] == 0)) {
                temp_board[0][j] = 1;
                road = DFS(0, j, temp_board, "vertical", 1, gen_board);
                if(road){
                    return -1.0;
                }
            }    
        }

        float flat_val = 0.0;
        bool flat_win_check = flat_win(gen_board,flat_val);
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
                    cerr<<"-1"<<"\t\t";
                    continue;
                }  
                while(temp.size()!=0) {
                    int x = temp.top();
                    temp.pop();
                    cerr<<x<<":";
                }
                cerr<<"\t\t";
            }
            cerr<<endl<<endl;
        }
    }

    void influence(int infl[8][8], state gen_board[8][8], float flat, float wall, float cap){
        for(int i = 0; i < board_size; i++){
            for(int j = 0; j < board_size; j++){
                infl[i][j] = 0;
            }
        }
        for(int i = 0; i < board_size; i++){
            for(int j = 0; j < board_size; j++){
                // Map the value of k
                int k;
                int capt = gen_board[i][j].captured;
                // stack<int> mystack(gen_board[i][j].state_stack);
                // int size = mystack.size();
                int temp;
                if(capt == -1)
                    k = 0;
                else if(capt < 3){
                    if(capt == 0)
                        k = flat;
                    else if(capt == 1)
                        k = wall;
                    else
                        k = cap;
                }
                else if(capt >= 3){
                    if(capt == 3)
                        k = -flat;
                    else if(capt == 4)
                        k = -wall;
                    else
                        k = -cap;
                }

                if(k != 0){
                    // Change current
                    infl[i][j] += k;
                        // Change up
                        temp = gen_board[i-1][j].captured;
                        if(i-1 >= 0 && (temp == -1 || temp %3 == 0 || (capt%3 == 2 && temp %3 != 2)))
                            infl[i-1][j] += k;
                        // Change down
                        temp = gen_board[i+1][j].captured;
                        if(i+1 < board_size && (temp == -1 || temp % 3 == 0 || (capt%3 == 2 && temp %3 != 2)))
                            infl[i+1][j] += k;
                        // Change left
                        temp = gen_board[i][j-1].captured;
                        if(j-1 >= 0 && (temp == -1 || temp % 3 == 0 || (capt%3 == 2 && temp %3 != 2)))
                            infl[i][j-1] += k;
                        // Change right
                        temp = gen_board[i][j+1].captured;
                        if(j+1 < board_size-1 && (temp == -1 || temp % 3 == 0 || (capt%3 == 2 && temp %3 != 2)))
                            infl[i][j+1] += k;
                }
            }
        }
    }

   double get_heuristic(state gen_board[8][8], bool debug){
        // Define different heuristic values
        double heuristic_value = 0.0;
        double captured = 0.0;
        double threats = 0.0;
        double piece_val = 0.0;

        // Initialize our array
        int arr[16][6];// 0 to board_size -1 are for rows
        for(int i=0;i<2*board_size;i++)
            for(int j=0;j<6;j++)
                arr[i][j]=0;
        
        // Caluculate pieces in each row and column
        for(int i=0;i<board_size;i++){
            for(int j=0;j<board_size;j++){
                int capt=gen_board[i][j].captured;
                if(capt==-1) 
                    continue;
                else
                    arr[i][capt]++;
                    arr[j+board_size][capt]++;
            }
        }

        // NUMBER SQUARES CAPTURED
        for(int i = 0; i < board_size; i++){
            captured += (arr[i][0]-arr[i][3])*50 + (arr[i][2]-arr[i][5])*80;
        }

        // SAME ROW PIECES HAS MORE WEIGHT
        float composition_value = 0.0;
        int flat_capt_me, wall_capt_me, cap_capt_me, my_capt;
        int flat_capt_you, wall_capt_you, cap_capt_you, your_capt;
        int capt_diff;
        float against_wall = 40, for_wall = 10;
        float capture_advantage = 0.0;
        float capture_disadvantage = 0.0;
        float wall_disadvantage = 0.0;
        float Center_weight = 5;
        float center_value = 0.0;
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
            if(capt_diff > 0)
                wall_disadvantage = wall_capt_me*for_wall + wall_capt_you*against_wall + diff[capt_diff]*(wall_capt_me + wall_capt_you);
            else if(capt_diff < 0)
                wall_disadvantage = (wall_capt_me*for_wall + wall_capt_you*against_wall - diff[-1*capt_diff]*(wall_capt_me + wall_capt_you));
            else
                wall_disadvantage = wall_capt_me*for_wall + wall_capt_you*for_wall;
            composition_value += capture_advantage - capture_disadvantage - 0.9*wall_disadvantage;
            // CENTER CONTROL
            if(i < board_size)
                center_value += (cap_capt_me - cap_capt_you)*(board_size-i-1)*i*center_value;
            else
                center_value += (cap_capt_me - cap_capt_you)*(2*board_size-i-1)*(i-board_size)*center_value;
        }


        // CURRENT PIECE HOLDINGS
        // Check to see if capstone is available
        if(other_player.capstone == 1)
            piece_val -= 60;
        if(cur_player.capstone == 1)
            piece_val += 60;

        // See how many flatstones left
        piece_val -= 20*cur_player.no_flat;
        piece_val += 20*other_player.no_flat;

        double infl_value = 0;
        int infl[8][8];
        float flat = 3;
        float wall = 3.1;
        float cap = 3.3;
        influence(infl, gen_board, flat, wall, cap);
        for(int i = 0; i < board_size; i++){
            for(int j = 0; j < board_size; j++){
                if(infl[i][j] > 0)
                    infl_value += pow(infl[i][j],2.0);
                else
                    infl_value -= pow(infl[i][j],2.0);
            }
        }

        heuristic_value = 1.1*captured + 1.5*composition_value + piece_val + 0.95*infl_value + center_value;   

        return heuristic_value;
    }

vector<string> generate_all_moves(int id, state gen_board[8][8]){
    // Vector of moves
    vector<string> all_moves;
    string move;
    bool valid;
    Player myplayer;
    int index;
    int capt;
    int curr_capt;

    if(id == player_id){
        myplayer = cur_player;
        index = 0;
    }
    else{
        myplayer = other_player;
        index = 1;
    }
        for(int i = 0; i < board_size; i++){
            for(int j = 0; j < board_size; j++){
                capt = gen_board[i][j].captured;
                if(capt == -1){
                    if(myplayer.no_flat != 0){                            
                        move = "F" ;move+= (char)(97+j); move+=(char)(49+i);
                        all_moves.push_back(move);
                        move = "S" ;move+= (char)(97+j); move+=(char)(49+i);
                        all_moves.push_back(move);                            
                    }
                    if(myplayer.capstone != 0){
                        move = "C" ;move+= (char)(97+j); move+=(char)(49+i);
                        all_moves.push_back(move);                            
                    }
                }
                else if(capt >= 3*index && capt < 3 + 3*index){
                    int temp_size = gen_board[i][j].state_stack.size();
                    int stack_size = min(temp_size, board_size);
                    int dist_up = i;
                    int dist_down = board_size - 1 - i;
                    int dist_left = j; 
                    int dist_right = board_size - 1 - j;
                    vector< vector<int> > part_list;

                    for(int x = 1; x <= stack_size; x++){
                        part_list = partition(x); // Stores all possible permutations
                        for(int y = 0; y < part_list.size(); y++){
                            int part_size =part_list[y].size(); // Checks to see how many non zero elements
                            if(part_size<=dist_up){
                                
                                valid = true;
                                move =""; move +=(char)(48+x); move +=(char)(97+j); move += (char)(49+i); move += "-";
                                int z1=-1;
                                for(int z = 0; z < part_size; z++){
                                    curr_capt = gen_board[i-z-1][j].captured;
                                    if(curr_capt == 0 || curr_capt == 3 || curr_capt == -1){
                                        move += (char)(48+part_list[y][z]);
                                    }  
                                    else{
                                        valid = false;
                                        z1 = z;
                                        break;
                                    }
                                }
                                int z=part_size-1;
                                curr_capt = gen_board[i-z-1][j].captured;
                                if(z1 == z)
                                    if((part_list[y][z] == 1) && ((curr_capt == 1) || (curr_capt == 4)) && (capt == 2 + index*3)){
                                            move += "1";
                                            valid = true;
                                        }
                                if(valid)
                                    all_moves.push_back(move);
                            }
                            if(part_size<=dist_down){
                                
                                valid = true;
                                move = "";move +=(char)(48+x); move +=(char)(97+j); move += (char)(49+i); move += "+";
                                int z1=-1;
                                for(int z = 0; z < part_size; z++){
                                    curr_capt = gen_board[i+z+1][j].captured;
                                    if(curr_capt == 0 || curr_capt == 3 || curr_capt == -1){
                                        move += (char)(48+part_list[y][z]);
                                    }  
                                    else{
                                        valid = false;
                                        z1 = z;
                                        break;
                                    }
                                }
                                int z=part_size-1;
                                curr_capt = gen_board[i+z+1][j].captured;
                                if(z1 == z)
                                    if((part_list[y][z] == 1) && ((curr_capt == 1) || (curr_capt == 4)) && (capt == 2 + index*3)){
                                            move += "1";
                                            valid = true;
                                        }
                                if(valid)
                                    all_moves.push_back(move);
                            }
                            if(part_size<=dist_left){
                                
                                valid = true;
                                move = "";move+=(char)(48+x); move+=  (char)(97+j); move+= (char)(49+i); move+="<";
                                int z1=-1;
                                for(int z = 0; z < part_size; z++){
                                    curr_capt = gen_board[i][j-z-1].captured;
                                    if(curr_capt == 0 || curr_capt == 3 || curr_capt == -1){
                                        move += (char)(48+part_list[y][z]);
                                    }  
                                    else{
                                        valid = false;
                                        z1 = z;
                                        break;
                                    }
                                }
                                int z=part_size-1;
                                curr_capt = gen_board[i][j-z-1].captured;
                                if(z1 == z)
                                    if((part_list[y][z] == 1) && ((curr_capt == 1) || (curr_capt == 4)) && (capt == 2 + index*3)){
                                            move += "1";
                                            valid = true;
                                        }
                                if(valid)
                                    all_moves.push_back(move);
                            }
                            if(part_size<=dist_right){
                                
                                valid = true;
                                move =""; move+=(char)(48+x); move+= (char)(97+j); move+=(char)(49+i); move+= ">";
                                int z1=-1;
                                for(int z = 0; z < part_size; z++){
                                    curr_capt = gen_board[i][j+z+1].captured;
                                    if(curr_capt == 0 || curr_capt == 3 || curr_capt == -1){
                                        move += (char)(48+part_list[y][z]);
                                    }  
                                    else{
                                        valid = false;
                                        z1 = z;
                                        break;
                                    }
                                }
                                int z=part_size-1;
                                curr_capt = gen_board[i][j+z+1].captured;
                                if(z1 == z)
                                    if((part_list[y][z] == 1) && ((curr_capt == 1) || (curr_capt == 4)) && (capt == 2 + index*3)){
                                            move += "1";
                                            valid = true;
                                        }
                                if(valid)
                                    all_moves.push_back(move);
                            }
                        }
                    }
                }
            }
        }
    return all_moves;
}

double best_move(state myboard[8][8],double alpha,double beta,int depth,string &best_move_chosen,bool minimum){
    // Declare Variables
    vector<string> moves;
    int move_player, crushed;
    vector<pair<double,string> > values;
    double min_val = LONG_MAX, max_val = LONG_MIN, child, ans, val, heur_val;
    string move_taken = "", tmp = "";

    // Operations
    if(depth == 0)
        return get_heuristic(myboard,false);

    if(minimum) {
        if(player_id == 1) 
            move_player = 2;
        else 
            move_player = 1;
    }
    else {   
        move_player = player_id;
    }

    moves = generate_all_moves(move_player, myboard);   

    for(int i = 0; i < moves.size(); i++) {
        crushed = 0;
        string_to_move_cur(neigh[i], move_player, myboard, crushed);
        ans = at_endstate(myboard);
        if(ans == 1.0)
            val = LONG_MAX;
        else if(ans == -1.0)
            val = LONG_MIN;
        else if(ans == 0.0)
            val = get_heuristic(myboard, false);
        else {
            cerr<<"Detected a flat ending"<<endl;   
            val= ans*LONG_MAX;  
        }
        values.push_back(std::make_pair(val, moves[i]));
        undo_move(moves[i], move_player, myboard, crushed);
    }

    if(minimum) {
        priority_queue<pair<double,string>, vector<pair<double,string> >, Compare_max> maxi_heap(values.begin(),values.end());          
        for(int i = 0; i < moves.size(); i++) {
            move_taken = "";
            heur_val = maxi_heap.top().first;
            move_taken = maxi_heap.top().second;
            maxi_heap.pop();
            crushed = 0; 
            string_to_move_cur(move_taken, i11, myboard, crushed);
            tmp = "";
            if(heur_val == LONG_MAX)
                child = LONG_MAX;
            else if(heur_val == LONG_MIN)
                child = LONG_MIN;
            else if(depth == 1)
                child = heur_val; 
            else    
                child = best_move(myboard, alpha, beta, (depth-1), tmp, !minimum);
            beta = min(beta, child);
            min_val = min(child, min_val);
            if(child == min_val) 
                best_move_chosen = move_taken;    
            undo_move(move_taken, move_player, myboard, crushed);
            if(alpha > beta)  
                return child;
        }
        return min_val;
    }
    else {
        priority_queue<pair<double,string>, vector<pair<double,string> >, Compare_min> mini_heap(values.begin(),values.end());   
        for(int i = 0; i < moves.size(); i++) { 
            move_taken = "";
            heur_val = mini_heap.top().first;
            move_taken = mini_heap.top().second;
            mini_heap.pop();
            crushed = 0;
            string_to_move_cur(move_taken, move_player, myboard, crushed);
            tmp = "";
            if(heur_val == LONG_MAX)
                child = LONG_MAX;
            else if(heur_val == LONG_MIN)
                child = LONG_MIN;
            else if(depth == 1)
                child = heur_val; 
            else 
                child = best_move(myboard, alpha, beta, (depth-1), tmp, !minimum);
            alpha = max(alpha, child);
            max_val = max(child, max_val);
            if(child == max_val) 
                best_move_chosen = move_taken;
            undo_move(move_taken, move_player, myboard, crushed);
            if(alpha > beta)   
                return child;
        }
        return max_val;
    }
} 

int main(){
    diff[0]=0;
    diff[1]=25+10;
    diff[2]=55+20;
    diff[3]=85+30;
    diff[4]=120+45;
    diff[5]=200+110;
    diff[6]=155;
    diff[7]=206;
    mapping[0]=flatstone;
    mapping[1]=standing;
    mapping[2]=capstone;
    
    // Clock variables
    float time_player = 0.0;
    clock_t begin_time;
    clock_t end_time;

    state Board [8][8];

    int n, time_limit;
    cerr<<"started"<<endl;
    cin>>player_id>>n>>time_limit;
    begin_time = clock();

    board_size = n;
    cur_player.assign(board_size);
    other_player.assign(board_size);
    for(int i = 0; i < board_size; i++)
        for(int j = 0; j < board_size; j++)
            Board[i][j].captured = -1;

    string move;
    int crush = 0;        
    bool on = true;
    if(player_id == 2)
    {
        cin>>move;
        begin_time = clock();
        crush=0;
        string_to_move_cur(move,2,Board,crush);
        print_board(Board);
        // wait for other persons move
        vector<string> poss=generate_all_moves(1,Board);
        string_to_move_cur(poss[0],1,Board,crush);
        cerr << poss[0]<<" "<<poss[1]<<" \n";
        cout<<poss[0]<<endl;
        end_time = clock();
        time_player += float( end_time - begin_time ) /  CLOCKS_PER_SEC;
        print_board(Board);
        
        int count = 0;

        // play a move
        while(on)
        {   cerr<<"Time left = "<< time_limit - time_player<<endl; 
            prune=0;best_called=0;
            cin>>move;
            begin_time = clock();
            crush=0;
            string_to_move_cur(move,1,Board,crush);
            crush=0;
            // print_board(Board);
            cerr<<endl;
            if(at_endstate(Board)!=0.0){
                cerr<<"You are the winner"<<endl;
            }
            cerr<<"the heuristic value is = "<<get_heuristic(Board,true)<<endl;
            vector<string> poss=generate_all_moves(2,Board);
            string next_move="";
            double val;
            if(time_limit - time_player < 20 || count < 4)
                val=best_move(Board,LONG_MIN/2,LONG_MAX/2,3,next_move,false);
            else 
                val=best_move(Board,LONG_MIN/2,LONG_MAX/2,4,next_move,false);
            cerr<<"Count is "<<count<<endl;
            cerr<<"Finished Generating"<<poss.size()<<endl;
            string_to_move_cur(next_move,2,Board,crush);
            cerr<<"Move played by opponent is "<<move<<endl;
            cout<<next_move<<endl;
            end_time = clock();
            time_player += float( end_time - begin_time ) /  CLOCKS_PER_SEC;
            // print_board(Board);
            cerr<<endl;
            if(at_endstate(Board)!=0.0){
                cerr<<"You are the winner"<<endl;
            }
            cerr<<"the heuristic value is = "<<get_heuristic(Board,true)<<","<<val<<endl;
            count++;
        }
    }   
     else if(player_id == 1)
    {
        vector<string> poss=generate_all_moves(2,Board);
        string_to_move_cur(poss[0],2,Board,crush);
        cout<<poss[0]<<endl;
        end_time = clock();
        time_player += float( end_time - begin_time ) /  CLOCKS_PER_SEC;
        crush=0;
        cin>>move;
        begin_time = clock();
        string_to_move_cur(move,1,Board,crush);
        int count=0;
        while(on)
        {   cerr<<"Time left = "<< time_limit - time_player<<endl; 
            //vector<string> poss=generate_all_moves(1,Board);
            crush=0;
            string next_move;
            double val;
            if(time_limit - time_player < 20 || count < 4)
                val=best_move(Board,LONG_MIN/2,LONG_MAX/2,3,next_move,false);
            else 
                val=best_move(Board,LONG_MIN/2,LONG_MAX/2,4,next_move,false);
            cerr<<"Count is "<<count<<endl;
            count++;
            cout<<next_move<<endl;
            end_time = clock();
            time_player += float( end_time - begin_time ) /  CLOCKS_PER_SEC;
            string_to_move_cur(next_move,1,Board,crush);
            // print_board(Board);
            if(at_endstate(Board)!=0.0){
                cerr<<"You are the winner"<<endl;
            }
            cerr<<"the heuristic value is = "<<get_heuristic(Board,true)<<endl;  
            cin>>move;
            begin_time = clock();
            cerr<<"Move played by opponent is "<<move<<endl;
            string_to_move_cur(move,2,Board,crush);
            // print_board(Board);
            if(at_endstate(Board)!=0.0){
                cerr<<"You are the winner"<<endl;
            }
            cerr<<"the heuristic value is = "<<get_heuristic(Board,true)<<","<<val<<endl;
                    
        }
    }
    return 0;
}
