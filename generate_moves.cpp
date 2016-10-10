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