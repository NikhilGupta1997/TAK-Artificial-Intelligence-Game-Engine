    void string_to_move_cur(string move,int id,state Board1[8][8],int &crushed) { // executes a valid move on the game board
        int j = (int)(move[1]) - 96;    // stores movement across a,b,...
        int i = (int)(move[2]) - 48;    // stores from 1,2..
        if(!isdigit(move[0])) {
            assert(Board1[i-1][j-1].captured == -1);
            int x;
            if(move[0]=='F') x = 0;
            else if(move[0]=='S') x = 1;
            else if(move[0]=='C') x = 2;
            if(id != player_id) 
                x = x+3;    // Maps the pices to those of the opponent (3,4,5)
            Board1[i-1][j-1].captured = x;
            Board1[i-1][j-1].state_stack.push(x);
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
                top = Board1[i-1][j-1].state_stack.top();
                Board1[i-1][j-1].state_stack.pop();
                picked.push(top);
            }
            if(Board1[i-1][j-1].state_stack.size() != 0)
                Board1[i-1][j-1].captured = Board1[i-1][j-1].state_stack.top();
            else
                Board1[i-1][j-1].captured = -1; 
            char dir = move[3];
            vector<int> drop;
            for(int l = 4; l < move.length(); l++) {
                char m_ch = move[l];
                int i1 = (int)m_ch - 48;
                drop.push_back(i1);
            }
            int mi, mj;
            if(dir == '+')
                mi = 1; mj = 0;
            else if(dir == '-')
                mi = -1; mj = 0;
            else if(dir == '>')
                mi = 0; mj = 1;
            else
                mi = 0; mj = -1;
                int w1,w2;  
            for(int k = 1; k <= drop.size(); k++) {
                w1 = i-1+k*mi;
                w2 = j-1+k*mj;
                stack<int> tempo(Board1[w1][w2].state_stack);
                int cap = Board1[w1][w2].captured;
                int x1 = 1;
                int top1, t1;
                t1 = Board1[w1][w2].captured; 
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
                Board1[w1][w2].captured = top1;
                swap(tempo,Board1[w1][w2].state_stack);   
            } 
        }
    }