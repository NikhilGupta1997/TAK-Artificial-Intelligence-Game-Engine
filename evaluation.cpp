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