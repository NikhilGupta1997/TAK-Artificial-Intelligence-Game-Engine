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