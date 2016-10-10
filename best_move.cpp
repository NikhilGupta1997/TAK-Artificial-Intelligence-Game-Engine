
    double best_move(state Board1[8][8],double alpha,double beta,int depth,string &best_move_chosen,bool minimum){
        vector<string> neigh;
        if(depth == 0)
            return get_heuristic(Board1,false);
        int i11;
        if(minimum) {
            if(player_id==1) 
                i11=2;
            else 
                i11=1;
            neigh=generate_all_moves(i11,Board1);
        }
        else {   
            neigh=generate_all_moves(player_id,Board1);
            i11=player_id;
        }
        vector<pair<double,string> > values;
        double min_val=LONG_MAX,max_val=LONG_MIN,child;
        for(int i=0;i<neigh.size();i++) {
            int crushed=0;
            string_to_move_cur(neigh[i],i11,Board1,crushed);
            double ans=at_endstate(Board1);
            double val;
            if(ans==1.0)
                val = LONG_MAX;
            else if(ans==-1.0)
                val = LONG_MIN;
            else if(ans==0.0)
                val=get_heuristic(Board1,false);
            else {
                cerr<<"Detected a flat ending"<<endl;	
                val= ans*LONG_MAX;	
            }
            values.push_back(std::make_pair(val,neigh[i]));
            undo_move(neigh[i],i11,Board1,crushed);
        }

        if(minimum) {
            priority_queue<pair<double,string>, vector<pair<double,string> >, Compare_max> maxi_heap(values.begin(),values.end());          
            for(int i=0;i<neigh.size();i++) {
                string move_taken="";
                double heur_val=maxi_heap.top().first;
                move_taken=maxi_heap.top().second;
                maxi_heap.pop();
                int crushed=0; 
                string_to_move_cur(move_taken,i11,Board1,crushed);
                string tmp="";
                if(heur_val == LONG_MAX)
                    child = LONG_MAX;
                else if(heur_val == LONG_MIN)
                    child = LONG_MIN;
                else if(depth == 1)
                    child =heur_val; 
                else    
                    child=best_move(Board1,alpha,beta,(depth-1),tmp,!minimum);
                beta=min(beta,child);
                min_val=min(child,min_val);
                if(child==min_val) best_move_chosen=move_taken;    
                undo_move(move_taken,i11,Board1,crushed);
                if(alpha>beta) {   
                    return child;
                }   
            }
            return min_val;
        }
        else {
            priority_queue<pair<double,string>, vector<pair<double,string> >, Compare_min> mini_heap(values.begin(),values.end());   
            for(int i=0;i<neigh.size();i++) { 
                string move_taken="";
                double heur_val=mini_heap.top().first;
                move_taken=mini_heap.top().second;
                mini_heap.pop();
                int crushed=0;
                string_to_move_cur(move_taken,i11,Board1,crushed);
                string tmp="";
                if(heur_val == LONG_MAX)
                    child = LONG_MAX;
                else if(heur_val == LONG_MIN)
                    child = LONG_MIN;
                else if(depth == 1)
                    child =heur_val; 
                else 
                    child=best_move(Board1,alpha,beta,(depth-1),tmp,!minimum);
                alpha=max(alpha,child);
                max_val=max(child,max_val);
                if(child==max_val) 
                    best_move_chosen=move_taken;
                undo_move(move_taken,i11,Board1,crushed);
                if(alpha>beta) {   
                    return child;
                }   
            }
            return max_val;
        }
    } 