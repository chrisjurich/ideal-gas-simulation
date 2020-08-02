




#include "GasBox.h"


void
GasBox::initialize() {
    // initializes the GasAtom vector
    for(auto atom_num = 0; atom_num < number_atoms; ++atom_num){
        // time to generate the random (x,y,z) points for the atoms
        // first x 
        const auto x = (static_cast<double>(rand())/RAND_MAX)*(box_dims.x_max-box_dims.x_min) + box_dims.x_min;
        // then y 
        const auto y = (static_cast<double>(rand()) /RAND_MAX)*(box_dims.y_max-box_dims.y_min) + box_dims.y_min;
        // then z
        const auto z = (static_cast<double>(rand())/RAND_MAX)*(box_dims.z_max-box_dims.z_min) + box_dims.z_min;
        // now make the GasAtomObject
        atoms.emplace_back(5.,5.,5.,box_dims,atom_num);
        
        //atoms.emplace_back(x,y,z,box_dims,atom_num);
    }
   
    caluclate_distances();
    //auto max_dist(0.);
    //for(const auto& p : last_distances) {
    //    max_dist=std::max(max_dist, p.second);
    //}
    ////std::cout<<max_dist<<std::endl; 
    ////return;
    //for(const auto& a : atoms)
    //    std::cout<<a;
}

void
GasBox::caluclate_distances() {
    // helper method that finds the cross distances between all the points 
    for(auto first_index = 0; first_index < number_atoms; ++first_index) {
        for(auto second_index = 0; second_index < number_atoms; ++second_index) {
            if(first_index != second_index){
                const auto lower_index = std::min(first_index,second_index);
                const auto upper_index = std::max(first_index,second_index);
                last_distances[
                    {lower_index,upper_index}
                    ] = atoms[lower_index].distance(atoms[upper_index]);
            } 
        }
    }

}

void
GasBox::to_csv(const std::string file_path) {
    
    auto outfile = std::ofstream(file_path);
    auto positions = std::vector<std::vector<Position>>();
    outfile<<"move";
    for(const auto& atom : atoms) {
        outfile<<","<<atom.id();
        positions.push_back(atom.positions());
    }
    outfile<<'\n';
    const auto num_moves = positions[0].size();
    for(auto move_it = 0; move_it < num_moves; ++ move_it) {
        outfile<<move_it;
        for(auto& col : positions) {
            outfile<<','<<col[move_it].to_string(); 
        }
        outfile<<'\n';
    }

    outfile.close();

}

void
GasBox::_move_atoms() {
    
    auto snapshot = BoxSnapShot();

    const auto num_atoms = atoms.size(); 
    auto moves = std::vector<Position>(num_atoms);
    caluclate_distances(); 
    for(auto atom_it = 0; atom_it<num_atoms; ++atom_it){
        // calculate original energy 
        auto energy = double(0.);
        for(const auto& index_pair_distance : last_distances) {
            if (atom_it == index_pair_distance.first.first or
                    atom_it == index_pair_distance.first.second ) {
                energy += lj_potential.energy(
                    atoms[index_pair_distance.first.first],
                    atoms[index_pair_distance.first.second]
                        );
            }
        
        }
        
        const auto atom_move = Position(
                                    move_distance*((static_cast<double>(rand())/RAND_MAX)*2. - 1.),
                                    move_distance*((static_cast<double>(rand())/RAND_MAX)*2. - 1.),
                                    move_distance*((static_cast<double>(rand())/RAND_MAX)*2. - 1.)
                                    );
        auto candidate_energy = double(0.); 
        const auto proposed = atoms[atom_it].proposed_move(atom_move);
        
        for(auto other_it = 0; other_it < num_atoms; ++other_it) {
            if(other_it == atom_it) {
                continue;     
            }
            candidate_energy += lj_potential.energy(
                    proposed.distance(atoms[other_it])
                    );

        }

        if(candidate_energy <= energy) {
            snapshot.accepted_moves += 1;
            moves[atom_it] = atom_move;         
        } else if (mc_selector.accept_move(candidate_energy/energy)){
            snapshot.accepted_moves += 1;
            moves[atom_it] = atom_move;
        } else {
            std::cout<<"NO MOVE"<<std::endl;
            snapshot.rejected_moves += 1;
            moves[atom_it] = Position(0.,0.,0.);
        }
    }

    for(auto atom_it =0; atom_it<num_atoms; ++atom_it) {
        atoms[atom_it] += moves[atom_it];
    }
    
    states.push_back(snapshot);
}

void
GasBox::simulate(int number_steps) {
    for(int step = 0; step<number_steps; ++step) {
        _move_atoms();
    
    }

}

