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