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
using namespace std;
#define MAX_BOARD_SIZE 8;
//int n;//Stores the Board size

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
};

class Game{ 
  public:
    state Board [8][8];
    int player_id;
    int board_size;
    Player cur_player;
    Player other_player;
    
    Game(int id, int size){
        this->player_id = id;
        this->board_size = size;
        cur_player.assign(board_size);
        other_player.assign(board_size);
        for(int i=0;i<board_size;i++)
            for(int j=0;j<board_size;j++)
                Board[i][j].captured=-1;
    }
    
    void string_to_move_cur(string move,int id,state Board1[8][8]){ // executes a valid move on the game board
       // cout<<"Executing "<<move<<endl;
        int j = (int)(move[1]) - 96; // stores movement across a,b,...
        int i = (int)(move[2]) - 48 ;   // stores from 1,2..
        // if first position is integer then its a Move move else a place
        if(!isdigit(move[0])){
            //assert(Board1[i-1][j-1].captured == -1);
            int x;
            if(move[0]=='F') x = 0;
            else if(move[0]=='S') x = 1;
            else if(move[0]=='C') x = 2;
            if(id != player_id) 
                x = x+3;    // Maps the pices to those of the opponent (3,4,5)
            Board1[i-1][j-1].captured = x;
            Board1[i-1][j-1].state_stack.push(x);
            if(move[0] == 'F' || move[0] == 'S'){
                if(id == player_id) 
                    cur_player.no_flat--;       
                else 
                    other_player.no_flat--;     
            }   
            else{   
                if(id == player_id) 
                    cur_player.capstone--;
                else 
                    other_player.capstone--;
            } 
        }
        else{
            //cout<<"Moving a piece"<<endl;
            int no_picked = (int) move[0],top;
            no_picked-=48;
           // cout<<"number picked="<<no_picked<<endl;
            stack<int> temp(Board1[i-1][j-1].state_stack);
            stack<int> picked;
            //cout<<Board1[i-1][j-1].state_stack.size()<<" is the size"<<endl;
            // for(int l = 0; l < no_picked; l++){
            //     top = Board1[i-1][j-1].state_stack.top();
            //     Board1[i-1][j-1].state_stack.pop();
            //     picked.push(top);
            // }
            for(int l = 0; l < no_picked; l++){
                top = temp.top();
                temp.pop();
                picked.push(top);
            }
           //  cout<<temp.size()<<":"<<Board1[i-1][j-1].state_stack.size()<<endl;
            swap( Board1[i-1][j-1].state_stack , temp );
           //  cout<<temp.size()<<":"<<Board1[i-1][j-1].state_stack.size()<<endl;
           // cout<<"Picked"<<std::endl;
            if(Board1[i-1][j-1].state_stack.size() != 0)
                Board1[i-1][j-1].captured = Board1[i-1][j-1].state_stack.top();
            else
                Board1[i-1][j-1].captured = -1; 
            //cout<<" At top now is "<<Board1[i-1][j-1].captured<<endl;
            char dir = move[3];
            vector<int> drop;
            for(int l = 4; l < move.length(); l++)
            {
                char m_ch=move[l];
                int i1=(int)m_ch -48;
                drop.push_back(i1);
            }
            //cout<<"Drop done "<<drop.size()<<endl;   
            int mi, mj;
            if(dir == '+')
                {mi = 1; mj = 0;}
            else if(dir == '-')
                {mi = -1; mj = 0;}
            else if(dir == '>')
                {mi = 0; mj = 1; }
            else
                {mi = 0; mj = -1; }
            for(int k = 1; k <= drop.size(); k++){
                int x1 = 1;
                int top1;
                stack<int> tempo1(Board1[i-1+k*mi][j-1+k*mj].state_stack);
                while(x1 <= drop[k-1]){
                    //cout<<"Gonna drop "<<drop[k-1]<<endl;
                    top1 = picked.top();
                    picked.pop();
                    int t1=Board1[i-1+k*mi][j-1+k*mj].captured;
                    if(t1 % 3==1)// Implies that there is a standing stone
                    {
                    assert(drop[k-1]==1 && top1 %3==2);
                    int xw=tempo1.top();
                    tempo1.pop();
                    tempo1.push(xw-1);      
                    }   
                    Board1[i-1+k*mi][j-1+k*mj].captured = top1;
                    tempo1.push(top1);
                    swap(tempo1,Board1[i-1+k*mi][j-1+k*mj].state_stack);
                   // cout<<"Pushed "<<top1<<" at "<<i-1+k*mi<<","<<j-1+k*mj<<endl;
                    //cout<<"New size is "<< Board1[i-1+k*mi][j-1+k*mj].state_stack.size()<<endl;
                    x1++;
                }   
            } 
        }
     //   cout<<"Executed"<<endl; 
    }

    vector< vector<int> > partition(int stack_size){   
    vector< vector<int> > answer;
    if(stack_size<=0) 
        return answer;    
    if(stack_size==1){
        vector<int> t; t.push_back(1);
        answer.push_back(t);
        return answer;
    }       
    for(int i=1;i<stack_size;i++){
        int j=stack_size-i;
        vector<int> temp2;
        vector<vector<int> > part = partition(j);
        for(int i1=0;i1<part.size();i1++){
            temp2=part[i1];
            temp2.insert(temp2.begin(),i);
            answer.push_back(temp2);
        }   
    }
    vector<int> t1;
    t1.push_back(stack_size);
    answer.push_back(t1);
    return answer;   
    }

    void state_diff(state board_old[8][8], state board_new[8][8], int diff[8][8], string move){
        reset_visited(diff);
        int get_pivot_i = (int)(move[2] - 1);
        int get_pivot_j = (int)(move[1] - 97);
        if(!isdigit(move[0])){
            if(board_new[get_pivot_i][get_pivot_j].captured < 3)
                diff[get_pivot_i][get_pivot_j] = 1;
            else
                diff[get_pivot_i][get_pivot_j] = -1;
        }
        else{
            int i, j;
            for(int k = 0; k < board_size; i++){
                if(move[3] == '-' || move[3] == '+'){
                    i = k; 
                    j = get_pivot_j;
                }
                else if(move[3] == '>' || move[3] == '<'){
                    i = get_pivot_i; 
                    j = k;
                }
                if(board_old[i][j].captured == board_new[i][j].captured)
                    diff[i][j] = 0;
                else if(board_old[i][j].captured == -1){
                    if(board_new[i][j].captured < 3)
                        diff[i][j] = 1;
                    else
                        diff[i][j] = -1;
                }
                else if(board_old[i][j].captured < 3){
                    if(board_new[i][j].captured >= 3)
                        diff[i][j] = -2;
                    else if (board_new[i][j].captured == -1)
                        diff[i][j] = -1;
                    else
                        diff[i][j] = 0;
                }
                else if(board_old[i][j].captured >= 3){
                    if (board_new[i][j].captured == -1)
                        diff[i][j] = 1;
                    else if(board_new[i][j].captured < 3)
                        diff[i][j] = 2;
                    else
                        diff[i][j] = 0;
                }
            }
        }
    }

    // passed by reference
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
                if(gen_board[i][j].captured == -1)
                    k = 0;
                else if(gen_board[i][j].captured < 3){
                    if(gen_board[i][j].captured == 0)
                        k = flat;
                    else if(gen_board[i][j].captured == 1)
                        k = wall;
                    else
                        k = cap;
                }
                else if(gen_board[i][j].captured >= 3){
                    if(gen_board[i][j].captured == 3)
                        k = -flat;
                    else if(gen_board[i][j].captured == 4)
                        k = -wall;
                    else
                        k = -cap;
                }

                if(k != 0){
                    // Change current
                    infl[i][j] += k;
                    // Change up
                    if(i != 0)
                        infl[i-1][j] += k;
                    // Change down
                    if(i != board_size-1)
                        infl[i+1][j] += k;
                    // Change left
                    if(j != 0)
                        infl[i][j-1] += k;
                    // Change right
                    if(j != board_size-1)
                        infl[i][j+1] += k;
                }
            }
        }
    }

    stack<int> get_neighbors(int i, int j){
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

    void extract_ij(int pos, int &i, int &j){
        i = pos / board_size;
        j = pos % board_size;
    }

    int DFS(int i, int j, int myboard[8][8], string direction, int player_id, state gen_board[8][8]){
        bool road = false;
        stack<int> neighbors;
        neighbors = get_neighbors(i,j);
        int curr, curr_i, curr_j;
        while(!neighbors.empty()){
            curr = neighbors.top();
            neighbors.pop();
            extract_ij(curr, curr_i, curr_j);
            if(myboard[curr_i][curr_j] == 1)
                continue;
            myboard[curr_i][curr_j] = 1;
            if(gen_board[curr_i][curr_j].captured == (0 + player_id * 3) || gen_board[curr_i][curr_j].captured == (2 + player_id * 3)){
                if(direction == "horizontal"){
                    if(curr_j == board_size - 1){
                        return true;
                    }
                    road = DFS(curr_i, curr_j, myboard, "horizontal", player_id, gen_board);
                }
                else if(direction == "vertical"){
                    if(curr_i == board_size - 1){
                        return true;
                    }
                    road = DFS(curr_i, curr_j, myboard, "vertical", player_id, gen_board);
                }
                if(road){
                    return road;
                }
            }
        }
        return road;  
    }

    void reset_visited(int matrix[8][8]){
        for(int i = 0; i < board_size; i++){
            for(int j = 0; j < board_size; j++){
                matrix[i][j] = 0;
            }
        }
    }

    bool at_endstate(state gen_board[8][8]){
        bool road = false;
        int temp_board[8][8];
        reset_visited(temp_board);
        for(int i = 0; i < board_size; i++){
            if((gen_board[i][0].captured == 0 || gen_board[i][0].captured == 2) && (temp_board[i][0] == 0)){
                temp_board[i][0] = 1;
                road = DFS(i, 0, temp_board, "horizontal", 0, gen_board);
                if(road){
                    return true;
                }
            }
        }
        reset_visited(temp_board);

        for(int i = 0; i < board_size; i++){
            if((gen_board[i][0].captured == 3 || gen_board[i][0].captured == 5) && (temp_board[i][0] == 0)){
                temp_board[i][0] = 1;
                road = DFS(i, 0, temp_board, "horizontal", 1, gen_board);
                if(road){
                    return true;
                }
            } 
        }
        reset_visited(temp_board);
        
        for(int j = 0; j < board_size; j++){
            if((gen_board[0][j].captured == 0 || gen_board[0][j].captured == 2) && (temp_board[0][j] == 0)){
                temp_board[0][j] = 1;
                road = DFS(0, j, temp_board, "vertical", 0, gen_board);
                if(road){
                    return true;
                }
            }
        }
        reset_visited(temp_board);

        for(int j = 0; j < board_size; j++){
            if((gen_board[0][j].captured == 3 || gen_board[0][j].captured == 5) && (temp_board[0][j] == 0)){
                temp_board[0][j] = 1;
                road = DFS(0, j, temp_board, "vertical", 1, gen_board);
                if(road){
                    return true;
                }
            }    
        }
        reset_visited(temp_board);
        return road;
    }

    void print_board(){
        for(int i = 0; i < board_size; i++){
            for(int j = 0; j < board_size; j++){
               // cout<<Board[i][j].captured<<"\t";
                stack<int> temp(Board[i][j].state_stack);
                if(temp.size()==0)
                { cout<<"-1"<<"\t";
                continue;}  
                while(temp.size()!=0){
                    int x=temp.top();
                    temp.pop();
                    cout<<x<<":";
                }
                cout<<"\t";
            }
            cout<<endl<<endl;
        }
    }

    void print_infl(){
        int infl[8][8];
        influence(infl, Board, 1, 1, 1);
        for(int i = 0; i < board_size; i++){
            for(int j = 0; j < board_size; j++){
                cout<<infl[i][j]<<"\t";
            }
            cout<<endl<<endl;
        }
    }

    double heuristic(state gen_board[8][8]){
        double value = 0;
        
        // INFLUENCE HEURISTIC
        double infl_value = 0;
        int infl[8][8];
        float flat = 1;
        float wall = 1;
        float cap = 1;
        influence(infl, gen_board, flat, wall, cap);
        for(int i = 0; i < board_size; i++){
            for(int j = 0; j < board_size; j++){
                if(infl[i][j] > 0)
                    infl_value += pow(infl[i][j],2.0);
                else
                    infl_value -= pow(infl[i][j],2.0);
            }
        }

        // CAPSTONE ON STACK HEURISTIC
        double cap_on_stack_value = 0;
        float cap_on_stack = 1;
        for(int i = 0; i < board_size; i++){
            for(int j = 0; j < board_size; j++){
                if(gen_board[i][j].captured == 2 && gen_board[i][j].state_stack.size() > 1){
                    int first = gen_board[i][j].state_stack.top();
                    gen_board[i][j].state_stack.pop();
                    int second = gen_board[i][j].state_stack.top();
                    gen_board[i][j].state_stack.push(first);
                    if(second >= 3)
                        cap_on_stack_value -= cap_on_stack;
                }
                if(gen_board[i][j].captured == 5 && gen_board[i][j].state_stack.size() > 1){
                    int first = gen_board[i][j].state_stack.top();
                    gen_board[i][j].state_stack.pop();
                    int second = gen_board[i][j].state_stack.top();
                    gen_board[i][j].state_stack.push(first);
                    if(second < 3)
                        cap_on_stack_value += cap_on_stack;
                }
            }
        }

        // SIZE OF STACK HEURISTIC

        // COMPOSITION OF STACK HEURISTIC

        value = infl_value + cap_on_stack_value;
        return value;
    }

    vector<string> generate_all_moves(int id, state gen_board[8][8]){
        // Vector of moves
        vector<string> all_moves;
        string move;
        bool valid;
        Player myplayer;
        int index;

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
                    if(gen_board[i][j].captured == -1){
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
                    // correct
                    else if(gen_board[i][j].captured >= 3*index && gen_board[i][j].captured < 3 + 3*index){
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
                                        if(gen_board[i-z-1][j].captured == 0 || gen_board[i-z-1][j].captured == 3 || gen_board[i-z-1][j].captured == -1){
                                            move += (char)(48+part_list[y][z]);
                                        }  
                                        else{
                                            valid = false;
                                            z1 = z;
                                            break;
                                        }
                                    }
                                    int z=part_size-1;
                                    if(z1 == z)
                                        if((part_list[y][z] == 1) && ((gen_board[i-z-1][j].captured == 1) || (gen_board[i-z-1][j].captured == 4)) && (gen_board[i][j].captured == 2 + index*3)){
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
                                        if(gen_board[i+z+1][j].captured == 0 || gen_board[i+z+1][j].captured == 3 || gen_board[i+z+1][j].captured == -1){
                                            move += (char)(48+part_list[y][z]);
                                        }  
                                        else{
                                            valid = false;
                                            z1 = z;
                                            break;
                                        }
                                    }
                                    int z=part_size-1;
                                    if(z1 == z)
                                        if((part_list[y][z] == 1) && ((gen_board[i+z+1][j].captured == 1) || (gen_board[i+z+1][j].captured == 4)) && (gen_board[i][j].captured == 2 + index*3)){
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
                                        if(gen_board[i][j-z-1].captured == 0 || gen_board[i][j-z-1].captured == 3 || gen_board[i][j-z-1].captured == -1){
                                            move += (char)(48+part_list[y][z]);
                                        }  
                                        else{
                                            valid = false;
                                            z1 = z;
                                            break;
                                        }
                                    }
                                    int z=part_size-1;
                                    if(z1 == z)
                                        if((part_list[y][z] == 1) && ((gen_board[i][j-z-1].captured == 1) || (gen_board[i][j-z-1].captured == 4)) && (gen_board[i][j].captured == 2 + index*3)){
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
                                        if(gen_board[i][j+z+1].captured == 0 || gen_board[i][j+z+1].captured == 3 || gen_board[i][j+z+1].captured == -1){
                                            move += (char)(48+part_list[y][z]);
                                        }  
                                        else{
                                            valid = false;
                                            z1 = z;
                                            break;
                                        }
                                    }
                                    int z=part_size-1;
                                    if(z1 == z)
                                        if((part_list[y][z] == 1) && ((gen_board[i][j+z+1].captured == 1) || (gen_board[i][j+z+1].captured == 4)) && (gen_board[i][j].captured == 2 + index*3)){
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

// Write my function
    double best_move(state Board1[8][8],double alpha,double beta,int depth,string &best_move_chosen,bool minimum){
        vector<string> neigh;
        if(minimum)
        {
            if(at_endstate(Board1)) return LONG_MAX;
        }
        else
        if(at_endstate(Board1)) return LONG_MIN;

        if(depth==0) return heuristic(Board1);
        //if(depth==0) return 0;    
        int i11;
        if(minimum)
        {
        if(player_id==1) i11=2;
        else i11=1;
        neigh=generate_all_moves(i11,Board1);
        }
        else
        {   
        neigh=generate_all_moves(player_id,Board1);
        i11=player_id;
        }
        state cur_Board[8][8];
        
        int mini=-1;
        double min_val=LONG_MAX,max_val=LONG_MIN,child;
        for(int i=0;i<neigh.size();i++)
        {
            // save the 
            // for(int i1 = 0; i1 < board_size; i1++){
      //       for(int j1 = 0; j1 < board_size; j1++){
      //           cur_Board[i1][j1].assign(Board1[i1][j1]);
      //       }
      //   }   
        // Save count for each player and save it back
            int p1_flat_count=cur_player.no_flat;
            int p1_cap_count=cur_player.capstone;
            int p2_flat_count=other_player.no_flat;
            int p2_cap_count=other_player.capstone; 
            string_to_move_cur(neigh[i],i11,cur_Board);
            //if(depth!=1)
            string tmp="";
            child=best_move(cur_Board,alpha,beta,(depth-1),tmp,!minimum);
            if(minimum)
            {
                beta=min(beta,child);
                min_val=min(child,min_val);
                if(child==min_val) best_move_chosen=neigh[i];
            }
            else
            {
                alpha=max(alpha,child);
                max_val=max(child,max_val);
                if(child==max_val) best_move_chosen=neigh[i];
            }
            cur_player.no_flat=p1_flat_count;
            cur_player.capstone=p1_cap_count;
            other_player.no_flat=p2_flat_count;
            other_player.capstone=p2_cap_count; 
            //cout<<alpha<<" :: "<<beta<<endl;
            if(alpha>=beta) 
            {//cout<<"Pruning"<<endl;
             break;
            } 
        }
    if(minimum) 
    return min_val;
    else 
    return max_val;     
    } 

};

//Functions
//1.) String to move and updae the board position
//2.) Generate all possible moves
//3.) make a evaluation function
int main(){
    cout<<"Let's get started Ayush\n";
    //Assign n here
    //What player is my player
    //srand(time(0));
    string data;
    int n,player_id,time_limit;
    cin>>player_id>>n>>time_limit;
    Game game(player_id,n);
    string move;
    bool on=true;
    if(player_id==2)
    {
        cin>>move;
        game.string_to_move_cur(move,2,game.Board);
        game.print_board();
        // wait for other persons move
        vector<string> poss=game.generate_all_moves(1,game.Board);
        game.string_to_move_cur(poss[0],1,game.Board);
        game.print_board();
        
        // play a move
        while(on)
        {
            cin>>move;
            game.string_to_move_cur(move,1,game.Board);
            game.print_board();
            cout<<endl;
            game.print_infl();
            if(game.at_endstate(game.Board)){
                cout<<"You are the winner"<<endl;
            }
            cout<<"the heuristic value is = "<<game.heuristic(game.Board)<<endl;
            vector<string> poss=game.generate_all_moves(2,game.Board);
            //int chose=rand() % poss.size();
            string next_move="";
            double val=game.best_move(game.Board,LONG_MIN,LONG_MAX,4,next_move,false);
            cout<<"Finished Generating"<<poss.size()<<endl;
            game.string_to_move_cur(next_move,2,game.Board);
            cout<<"Move played is "<<next_move<<endl;
            game.print_board();
            cout<<endl;
            game.print_infl();
            if(game.at_endstate(game.Board)){
                cout<<"You are the winner"<<endl;
            }
            cout<<"the heuristic value is = "<<game.heuristic(game.Board)<<endl;
        }
        // continue the game
    }   
    else if(player_id==1)
    {
        vector<string> poss=game.generate_all_moves(2,game.Board);
        game.string_to_move_cur(poss[0],2,game.Board);
        cout<<poss[0]<<endl;
        cin>>move;
        game.string_to_move_cur(move,1,game.Board);
        while(on)
        {
            vector<string> poss=game.generate_all_moves(1,game.Board);
            game.string_to_move_cur(poss[0],1,game.Board);
            int chose=rand() % poss.size();
            cout<<poss[chose]<<endl;
            cin>>move;
            game.string_to_move_cur(move,2,game.Board);     
        }
    }
    
    return 0;
}
