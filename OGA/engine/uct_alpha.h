/*
 *  Copyright (C) 2011 Universidad Simon Bolivar
 * 
 *  Permission is hereby granted to distribute this software for
 *  non-commercial research purposes, provided that this copyright
 *  notice is included with any such distribution.
 *  
 *  THIS SOFTWARE IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND,
 *  EITHER EXPRESSED OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 *  PURPOSE.  THE ENTIRE RISK AS TO THE QUALITY AND PERFORMANCE OF THE
 *  SOFTWARE IS WITH YOU.  SHOULD THE PROGRAM PROVE DEFECTIVE, YOU
 *  ASSUME THE COST OF ALL NECESSARY SERVICING, REPAIR OR CORRECTION.
 *  
 *  Blai Bonet, bonet@ldc.usb.ve
 *
 */

#ifndef UCT_H
#define UCT_H

#include "policy.h"

#include <iostream>
#include <sstream>
#include <iomanip>
#include <cassert>
#include <limits>
#include <vector>
#include <math.h>
#include <map>
#include <set>
 #include "utils.h"
#include <algorithm>

#include <unordered_map>

#include <list>

//#define DEBUG
#define ALPHA 0.1
namespace Online {

namespace Policy {

namespace UCT {

////////////////////////////////////////////////
//
// Tree
//

template<typename T> struct node_t {
    mutable std::vector<unsigned> counts_;
    mutable std::vector<float> values_;
    node_t(int num_actions)
      : counts_(1+num_actions, 0),
        values_(1+num_actions, 0) {
    } 
    ~node_t() { }
};

////////////////////////////////////////////////
//
// Hash Table
//

template<typename T> struct map_functions_t {
    size_t operator()(const std::pair<unsigned, T> &p) const {
        return p.second.hash();
    }
};

template<typename T> struct state_hash_fn {
    size_t operator()(const T &s) const {
        return s.hash();
    }
};

template<typename T> struct SA_hash_fn {
    size_t operator()(const std::pair<T, Problem::action_t> &sa) const {
        return (sa.first.hash()) ^ (sa.second);
    }
};


struct data_t {
   // std::vector<float> values_;
   // std::vector<int> counts_;
    std::vector<int> visits_;	// visit count of each SA node after it's abstraction has been recomputed
    //// Is this constructor correct ?
/*    data_t(unsigned num_actions)
    {
    	values_.assign(num_actions+1,0);
    	counts_.assign(num_actions+1,0);
    	visits_.assign(num_actions+1,0);
    }*/
    data_t(const std::vector<int> & visits)
    :visits_(visits){}    
    //data_t(const std::vector<float> &values, const std::vector<int> &counts, const std::vector<int> &visits)
     // : values_(values), counts_(counts), visits_(visits) { }
    // data_t(const std::vector<float> &values, const std::vector<int> &counts)
    //   : values_(values), counts_(counts) { }
    //data_t(const data_t &data)
    //  : values_(data.values_), counts_(data.counts_), visits_(data.visits_) { }
    data_t(const data_t &data)
      :visits_(data.visits_) { }
#if 0
    data_t(data_t &&data)
      : values_(std::move(data.values_)), counts_(std::move(data.counts_)) { }
#endif
};

template<typename T> class hash_t :
  public Hash::generic_hash_map_t<std::pair<unsigned, T>,
                                  data_t,
                                  map_functions_t<T> > {
  public:
    typedef typename Hash::generic_hash_map_t<std::pair<unsigned, T>,
                                              data_t,
                                              map_functions_t<T> >
            base_type;
    typedef typename base_type::const_iterator const_iterator;
    const_iterator begin() const { return base_type::begin(); }
    const_iterator end() const { return base_type::end(); }

  public:
    hash_t() { }
    virtual ~hash_t() { }
    void print(std::ostream &os) const {
        for( const_iterator it = begin(); it != end(); ++it ) {
            os << "(" << it->first.first << "," << it->first.second << ")" << std::endl;
        }
    }
};

template<typename T,typename TVal> class state_hash_t :
  public Hash::generic_hash_map_t<T,
                                  TVal,
                                  state_hash_fn<T> > {
  public:
    typedef typename Hash::generic_hash_map_t<T,
                                              TVal,
                                              state_hash_fn<T> >
            base_type;
    typedef typename base_type::const_iterator const_iterator;
    const_iterator begin() const { return base_type::begin(); }
    const_iterator end() const { return base_type::end(); }

  public:
    state_hash_t() { }
    virtual ~state_hash_t() { }
    void print(std::ostream &os) const {
    }
};

template<typename T,typename TVal> class SA_hash_t :
  public Hash::generic_hash_map_t<std::pair<T, Problem::action_t>,
                                  TVal,
                                  SA_hash_fn<T> > {
  public:
    typedef typename Hash::generic_hash_map_t<std::pair<T, Problem::action_t>,
                                              TVal,
                                              SA_hash_fn<T> >
            base_type;
    typedef typename base_type::const_iterator const_iterator;
    const_iterator begin() const { return base_type::begin(); }
    const_iterator end() const { return base_type::end(); }

  public:
    SA_hash_t() { }
    virtual ~SA_hash_t() { }
    void print(std::ostream &os) const {
    }
};





////////////////////////////////////////////////
//
// Policy
//




template<typename T> class uct_t : public improvement_t<T> {
  using policy_t<T>::problem;

  protected:
    unsigned width_;
    unsigned horizon_;
    float parameter_;
    bool random_ties_;
    mutable hash_t<T> table_;

    mutable int K;	
    // The parameter stating the number of times an SA node has to be visited to recompute it's abstraction
    mutable unsigned timer;
   

    typedef std::pair<unsigned, T> state_node;					// A state node is given by (d,s)
    typedef std::pair<T, Problem::action_t> SA_pair;			// SA pair is simply (s,a)
    // typedef std::pair<state_node, Problem::action_t> SA_node;	// SA node is given by ((d,s), a) -> action a taken from node (d,s)

    mutable std::vector< state_hash_t<T, unsigned> > gnd_to_abstr;					// For each depth, we have a map that gives the abstract state corresponding to ground state
    mutable std::vector< std::unordered_map<unsigned, unsigned> > abstr_to_gnd;		// For each depth, we have a map that gives the vector of ground states correspdonding to abstract state

	mutable std::vector< SA_hash_t<T,unsigned> > SAgnd_to_abstr;				// For each depth, we have a map that gives the abstract SA pair corresponding to ground state action pair
	//mutable std::vector< std::map<unsigned, std::vector<SA_pair>> > SAabstr_to_gnd;	// For each depth, we have a map that gives the vector of ground state action pairs corresponding to abstract SA pair
    mutable std::vector<std::unordered_map<unsigned, unsigned>> SAabstr_to_gnd;
	mutable std::vector< std::unordered_map<unsigned, std::pair<int, float>> > SAabstr_data;	// For each depth, we have the data (count and value) corresponding to each abstract pair

    typedef std::map<unsigned, float> transition_map;	// The transition prob distribution for sa node to children ABSTRACT states
    typedef std::set<unsigned> action_set;	// Set of ABSTRACT state action children of a state node

    // mutable std::vector< std::map<action_set, unsigned> > SuperMap;						// For each depth, we have a map from set of abstract SA pairs to corresponding abstract state
    // mutable std::vector< std::map<transition_map, std::vector<unsigned>> > SuperSAMap;	// For each depth, we have a map from transition table to the vector of abstract SA pairs having that table
    
    mutable std::vector<SA_hash_t<T, std::pair<unsigned, float>>> max_prob_info;
    mutable std::vector<std::map<unsigned, T>> state_birth_time;
    mutable std::vector<SA_hash_t<T,std::vector<std::pair<T,float>>>> active_set_map;

    mutable std::vector<std::unordered_map<unsigned, std::pair<transition_map,float>>> trans_tables;	// For each depth, 

    mutable std::vector<std::unordered_map<unsigned, action_set>> action_tables;	// For each depth, 


    mutable std::vector<unsigned> max_absSA;	// For each depth, we have the maximum index used for abstract SA nodes
    mutable std::vector<unsigned> max_absSt;

    mutable std::vector< state_hash_t<T, std::set<SA_pair>> > parents;
    // mutable std::vector< SA_hash_t<T,std::set<T>> > children;


  public:
    uct_t(const policy_t<T> &base_policy,
          unsigned width,
          unsigned horizon,
          float parameter,
          bool random_ties)
      : improvement_t<T>(base_policy),
        width_(width),
        horizon_(horizon),
        parameter_(parameter),
        random_ties_(random_ties) {
        std::stringstream name_stream;
        name_stream << "uct("
                    << "width=" << width_
                    << ",horizon=" << horizon_
                    << ",par=" << parameter_
                    << ",random-ties=" << (random_ties_ ? "true" : "false")
                    << ")";
        policy_t<T>::set_name(name_stream.str());
    }
    virtual ~uct_t() { }


    virtual Problem::action_t operator()(const T &s) const {
        ++policy_t<T>::decisions_;
        table_.clear();

        K = parameter_;
        timer=0;

        //// MEM_LEAK!: SHOULD WE EVEN CLEAR THE MAPS BEFORE THIS ?????

        gnd_to_abstr.assign(horizon_+1, state_hash_t<T, unsigned>());
        abstr_to_gnd.assign(horizon_+1, std::unordered_map<unsigned, unsigned>());

        SAgnd_to_abstr.assign(horizon_, SA_hash_t<T,unsigned>());
        SAabstr_to_gnd.assign(horizon_, std::unordered_map<unsigned, unsigned>());
        SAabstr_data.assign(horizon_, std::unordered_map<unsigned, std::pair<int, float>>());

		trans_tables.assign(horizon_, std::unordered_map<unsigned, std::pair<transition_map,float>>());
		
        action_tables.assign(horizon_+1, std::unordered_map<unsigned, action_set>());
        max_prob_info.assign(horizon_+1, SA_hash_t<T, std::pair<unsigned, float>>());
        state_birth_time.assign(horizon_+1,std::map<unsigned,T>());
        active_set_map.assign(horizon_+1,SA_hash_t<T, std::vector<std::pair<T,float>>>());
        // SuperMap.assign(horizon_+1, std::map<action_set, unsigned>());
        // SuperSAMap.assign(horizon_, std::map<transition_map, std::vector<unsigned>>());

        max_absSA.assign(horizon_, 0);
        max_absSt.assign(horizon_+1, 0);

        parents.assign(horizon_+1, state_hash_t<T, std::set<SA_pair>>());
        // children.assign(horizon_, SA_hash_t<T,std::set<T>>());

        std::set<SA_pair> empty_set;
        parents[0].insert(std::make_pair(s, empty_set));

        //// How is the count of the horizon state node computed ?

        // ALL MAPS INITIALIZED HERE !

// std::cout << "\n\nStarted at root: " << s << std::endl << std::flush;
    	
// std::cout << "\n\nStarting new decision here !\n";
        
       //  double start_time,end_time;
        //start_time= Utils::my_read_time_in_milli_seconds();
        for( unsigned i = 0; i < width_; ++i )
        {
            search_tree(s, 0);
        }
        //end_time=Utils::my_read_time_in_milli_seconds();
        //std::cout<<end_time-start_time<<"\n";
        typename hash_t<T>::iterator it = table_.find(std::make_pair(0, s));
       // assert(it != table_.end());
        Problem::action_t action = select_action(s, it->second, 0, false, random_ties_);
        //assert(problem().applicable(s, action));

    // std::cout << "STATE:" << s << " ACTION:" << action << std::endl;

// std::cout << "UCT TREE :-" << std::endl;
/*for(int d = 0; d <= horizon_; ++d)
{
    std::cout << d << ":\n";
    for(typename hash_t<T>::const_iterator it = table_.begin(); it != table_.end(); ++it )
    {
        if(it->first.first == d)
            std::cout << it->first.second << std::endl;
    }
    std::cout << std::endl;
}

std::cout << "\n********************************************************\n\n";
*/
/*// Printing equivalent state action pairs
for(unsigned d = 0; d < horizon_; ++d)
{
    std::cout << "\n\nDepth: " << d << '\n';
    for(auto saga_it = SAgnd_to_abstr[d].begin(); saga_it != SAgnd_to_abstr[d].end(); saga_it++)
    {
    	if(SAabstr_to_gnd[d][saga_it->second] > 1)
    	{
		 std::cout << "Equivalence class: " << saga_it->second << '\n';
		 std::cout << saga_it->first.first << "\nAction: " << saga_it->first.second << "\n\n";
        }
    }
}

std::cout << "\n############################################################\n\n";

// Printing equivalent states
for(unsigned d = 0; d <= horizon_; ++d)
{
    std::cout << "\n\nDepth: " << d << '\n';
    for(auto ga_it = gnd_to_abstr[d].begin(); ga_it != gnd_to_abstr[d].end(); ga_it++)
    {
        if(abstr_to_gnd[d][ga_it->second] > 1)
        {
            std::cout << "Equivalence class: " << ga_it->second << '\n';
            std::cout << ga_it->first << "\n\n";
        }
    }
}
*/

/*        for (unsigned d = 0; d <= 3; ++d)
        {
            std::cout << "\n\n\n\nDepth: " << d << '\n';
            for(auto ch_it = children[d].begin(); ch_it != children[d].end(); ch_it++)
            {
                std::cout << "\n\n\n" << ch_it->first.first << "\nAction: " << ch_it->first.second << "\n";

                std::cout << "Set of children: " << "\n\n";
                typename std::set<T>::iterator set_it;
                for(set_it = ch_it->second.begin(); set_it != ch_it->second.end(); set_it++)
                     std::cout << *set_it << "\n\n";
            }
        }


        std::cout << "\nTree finished !\n============================================================================\n\n";

*/        return action;
    }



    virtual const policy_t<T>* clone() const {
        return new uct_t(improvement_t<T>::base_policy_, width_, horizon_, parameter_, random_ties_);
    }
    virtual void print_stats(std::ostream &os) const {
        os << "stats: policy=" << policy_t<T>::name() << std::endl;
        os << "stats: decisions=" << policy_t<T>::decisions_ << std::endl;
        improvement_t<T>::base_policy_.print_stats(os);
    }

    // float value(const T &s, Problem::action_t a) const {
    //     typename hash_t<T>::const_iterator it = table_.find(std::make_pair(0, s));
    //     assert(it != table_.end());
    //     return it->second.values_[1+a];
    // }
    // unsigned count(const T &s, Problem::action_t a) const {
    //     typename hash_t<T>::const_iterator it = table_.find(std::make_pair(0, s));
    //     assert(it != table_.end());
    //     return it->second.counts_[1+a];
    // }
    size_t size() const { return table_.size(); }
    void print_table(std::ostream &os) const {
        table_.print(os);
    }

    // Initialize the abstraction of partially observed state nodes
    inline void init_abstr(T s, unsigned depth) const
    {
    	// gnd_to_abstr[depth].insert(std::pair<T,unsigned>(s, 0));
    	// abstr_to_gnd[depth][0].push_back(s);
        if(true)//depth > 5)//depth < 1)
    	{
        	max_absSt[depth]++;
    		unsigned this_new_abs = max_absSt[depth];    		
    		gnd_to_abstr[depth].insert(std::pair<T,unsigned>(s, this_new_abs));
    		abstr_to_gnd[depth].insert(std::make_pair(this_new_abs, 1));
        }
        // else
        // {
        //     gnd_to_abstr[depth].insert(std::pair<T,unsigned>(s, 0));
        //     typename std::map<unsigned,std::vector<T>>::iterator atg_it=abstr_to_gnd[depth].find(0);
        //     if(atg_it == abstr_to_gnd[depth].end())
        //         abstr_to_gnd[depth].insert(std::make_pair(0,std::vector<T>(1,s)));
        //     else
        //         atg_it->second.push_back(s);
        // }

		// NOT inserting any entry into the action_tables and rev_action_tables map

		// action_set this_set;	// Empty set (because no actions have been explored)
		// action_tables[depth].insert(std::make_pair(this_new_abs, this_set));		
    }    


    // Initialize the abstraction of newly created state action nodes
    inline void init_SA_abstr(T s, Problem::action_t a, unsigned depth) const
    {
    	// SA_pair this_pair = std::make_pair(s,a);
    	
    	// SAgnd_to_abstr[depth].insert(std::pair<SA_pair, unsigned>(this_pair,0));
    	// SAabstr_to_gnd[depth][0].push_back(this_pair);

    	// if(SAabstr_data[depth].find(0) == SAabstr_data[depth].end())
    	// 	SAabstr_data[depth].insert(std::make_pair(0, std::make_pair(0,0)));

    	
        if(true)
        {
            max_absSA[depth]++;
            unsigned this_new_abs = max_absSA[depth];
        	SAabstr_to_gnd[depth].insert(std::make_pair(this_new_abs, 1));
        	SAgnd_to_abstr[depth].insert(std::make_pair(std::make_pair(s,a), this_new_abs));
        	SAabstr_data[depth].insert(std::make_pair(this_new_abs, std::make_pair(0,0)));
        }
        // else
        // {
        //     SA_pair this_pair = std::make_pair(s,a);
        //     SAgnd_to_abstr[depth].insert(std::pair<SA_pair, unsigned>(this_pair,0));
        //     if(SAabstr_to_gnd[depth].find(0) != SAabstr_to_gnd[depth].end())
        //         SAabstr_to_gnd[depth][0].push_back(this_pair);
        //     else
        //         SAabstr_to_gnd[depth].insert(std::make_pair(0, std::vector<SA_pair>(1, this_pair)));

        //     if(SAabstr_data[depth].find(0) == SAabstr_data[depth].end())
        //         SAabstr_data[depth].insert(std::make_pair(0, std::make_pair(0,0)));

        // }

    	// NOT inserting any entry into trans_tables and rev_trans_tables
	    	
	   	// trans_tables[depth].insert(std::make_pair(this_new_abs, std::make_pair(this_map, this_cost)));
        // rev_trans_tables[depth].insert(std::make_pair(std::make_pair(this_map, this_cost), this_new_abs));
    }

    bool recomp_SA_abstr(T s, Problem::action_t a, unsigned depth, bool goUp) const
    {
// std::cout << "s: " << s << " a: " << a << " d: " << depth << std::endl << std::flush;
         //if(isPartiallyUnexplored(s,depth))
           //  return false;
        typename hash_t<T>::iterator it = table_.find(std::make_pair(depth, s));
        // reseting the vis count back to zero
        it->second.visits_[1+a] = 0;

    	transition_map this_map;
    	float this_cost = problem().cost(s,a);
        SA_pair curr_SA_Pair=std::make_pair(s,a);
    	// typename std::set<T>::iterator child_it = children[depth][std::make_pair(s,a)].begin();
    	// for(; child_it != children[depth][std::make_pair(s,a)].end(); ++child_it)	//// can be optimized ?
    	// {
    	// 	float prob = problem().trans_prob(s, a, *child_it);
    	// 	if(prob == 0)
    	// 		continue;

    	// 	unsigned child_abs = gnd_to_abstr[depth+1][*child_it];

    	// 	std::map<unsigned, float>::iterator abs_it = this_map.find(child_abs);
    	// 	if(abs_it == this_map.end())
	    // 		this_map.insert(std::make_pair(child_abs, prob));
    	// 	else
    	// 		abs_it->second += prob;
	   	// }
        std::vector<std::pair<T,float>> old_active_set;
        if(active_set_map[depth].find(curr_SA_Pair)!=active_set_map[depth].end())
            old_active_set=active_set_map[depth][curr_SA_Pair];
       
        
        std::vector<std::pair<T,float>> new_active_set;
        auto max_prob_info_it=max_prob_info[depth].find(curr_SA_Pair);
        std::pair<unsigned, float> max_prob_data;
        if(max_prob_info_it!=max_prob_info[depth].end())
        {
            max_prob_data=max_prob_info_it->second;
           
        }
        else 
        {
            max_prob_data=std::make_pair(0,0);
            
        }
       
        typename std::map<unsigned,T>::reverse_iterator rev_state_birth_it= state_birth_time[depth+1].rbegin();
        //  float prob_max = -1;
        //  for(auto ga_it = gnd_to_abstr[depth+1].begin(); ga_it != gnd_to_abstr[depth+1].end(); ga_it++)
        // {
        //     float prob = problem().trans_prob(s, a, ga_it->first);
        //      if(prob > prob_max)
        //     {
        //         prob_max = prob;
               
        //     }
        // }
       
       for(;rev_state_birth_it!=state_birth_time[depth+1].rend();++rev_state_birth_it)
        {
             
             if(rev_state_birth_it->first >max_prob_data.first)
            {
                float prob = problem().trans_prob(s, a, rev_state_birth_it->second);
                
                if(prob > max_prob_data.second)
                    max_prob_data.second= prob;                               
            }
            else
                break;


       }
      
       
        for(rev_state_birth_it= state_birth_time[depth+1].rbegin();rev_state_birth_it!=state_birth_time[depth+1].rend();++rev_state_birth_it)
        {
             
            if(rev_state_birth_it->first >max_prob_data.first)
            {
                float prob = problem().trans_prob(s, a, rev_state_birth_it->second);
                if(prob < ALPHA * max_prob_data.second)
                    continue;
                //if(prob > max_prob_data.second)
                 //   max_prob_data.second= prob;
               
                new_active_set.push_back(std::make_pair(rev_state_birth_it->second,prob));
                unsigned child_abs=gnd_to_abstr[depth+1][rev_state_birth_it->second];
                std::map<unsigned, float>::iterator abs_it = this_map.find(child_abs);
                if(abs_it == this_map.end())
                    this_map.insert(std::make_pair(child_abs, prob));
                else
                     abs_it->second += prob;
                
            }
            else
                break;


       }
     

       for(unsigned i=0;i<old_active_set.size();i++)
       {
            float prob=old_active_set[i].second;
            //float prob = problem().trans_prob(s, a, old_active_set[i].first);
             if( prob < ALPHA * max_prob_data.second)
                continue;
           //if (prob > max_prob_data.second)
            //    max_prob_data.second= prob;
            unsigned child_abs=gnd_to_abstr[depth+1][old_active_set[i].first];
            
            new_active_set.push_back(old_active_set[i]);
            std::map<unsigned, float>::iterator abs_it = this_map.find(child_abs);
            if(abs_it == this_map.end())
                this_map.insert(std::make_pair(child_abs, prob));
            else
                abs_it->second += prob;
       }
       
       if(old_active_set.empty())
            active_set_map[depth].insert(std::make_pair(curr_SA_Pair,new_active_set));
       else 
            active_set_map[depth][curr_SA_Pair]=new_active_set;
       
      



	  //   for(auto ga_it = gnd_to_abstr[depth+1].begin(); ga_it != gnd_to_abstr[depth+1].end(); ga_it++)
	  //   {
	  //       float prob = problem().trans_prob(s, a, ga_it->first);

	  //       if(prob < ALPHA * max_prob_data.second)
	  //           continue;
   //          if (prob > max_prob_data.second)
   //              max_prob_data.second= prob;

	  //       unsigned child_abs = ga_it->second;

	  //       std::map<unsigned, float>::iterator abs_it = this_map.find(child_abs);
			// if(abs_it == this_map.end())
			// 	this_map.insert(std::make_pair(child_abs, prob));
			// else
			// 	abs_it->second += prob;
	  //   }
        max_prob_data.first=timer;
        if(max_prob_info_it==max_prob_info[depth].end())
            max_prob_info[depth].insert(std::make_pair(curr_SA_Pair,max_prob_data));
        else
            max_prob_info_it->second=max_prob_data;
        
         //std::cout<<"Here3"<<(max_prob_info[depth][].second<<"\n";
    	// typename std::map<T, unsigned>::iterator ga_it = gnd_to_abstr[depth+1].begin();

    	// for(; ga_it != gnd_to_abstr[depth+1].end(); ga_it++)
    	// {
    	// 	float prob = problem().trans_prob(s, a, ga_it->first);

    	// 	if(prob == 0)
    	// 		continue;

    	// 	std::map<unsigned, float>::iterator abs_it = this_map.find(ga_it->second);
    	// 	if(abs_it == this_map.end())
	    // 		this_map.insert(std::make_pair(ga_it->second, prob));
    	// 	else
    	// 		abs_it->second += prob;
    	// }

    	typename SA_hash_t<T,unsigned>::iterator old_abs_it = SAgnd_to_abstr[depth].find(std::make_pair(s,a));
        if(old_abs_it == SAgnd_to_abstr[depth].end())
            std::cout << "ERROR !!!" << std::endl;

        // int this_old_abs = SAgnd_to_abstr[depth][std::make_pair(s,a)];
        int this_old_abs = old_abs_it->second;

    	int this_new_abs = -1;

        // std::map<std::pair<transition_map,float>, unsigned>::iterator SAabs_it = rev_trans_tables[depth].find(std::make_pair(this_map, this_cost));
        // if(SAabs_it != rev_trans_tables[depth].end())
        //     this_new_abs = SAabs_it->second;

// int this_new_abs2 = -1;
        for(std::unordered_map<unsigned, std::pair<transition_map,float>>::iterator SAabs_it = trans_tables[depth].begin(); SAabs_it != trans_tables[depth].end(); SAabs_it++)
        {
        	if((this_cost == (SAabs_it->second).second) && (this_map == SAabs_it->second.first))
        	{
        		this_new_abs = SAabs_it->first;
        		break;
        	}
        }
// if(this_new_abs != this_new_abs2)
//     std::cout << "ERROR";

    	if(this_old_abs == this_new_abs)
    		return false;

        if((this_new_abs == -1) && (SAabstr_to_gnd[depth][this_old_abs] == 1))
        {
            trans_tables[depth][this_old_abs] = std::make_pair(this_map, this_cost);

            return false;
        }

    	if(this_new_abs == -1)
    	{
    		// New abstract state node
    		max_absSA[depth]++;
    		this_new_abs = max_absSA[depth];

	    	SAabstr_to_gnd[depth].insert(std::make_pair(this_new_abs, 1));
	    	
	    	int n = SAabstr_to_gnd[depth][this_old_abs];
	    	
	    	std::pair<int,float> old_data = SAabstr_data[depth][this_old_abs];
	    	
	    	std::pair<int, float> updated_data;
	    	updated_data.first = old_data.first/n;
	    	updated_data.second = old_data.second;
	    	
	    	SAabstr_data[depth].insert(std::make_pair(this_new_abs, updated_data));
	    	//// ENSURE THIS NOT ALIASED !
	    	
	    	old_data.first = old_data.first * (n-1) / n;
	    	SAabstr_data[depth][this_old_abs] = old_data;

	    	// ALL NEW COMPONENTS OF THIS NEW SA PAIR
	    	trans_tables[depth].insert(std::make_pair(this_new_abs, std::make_pair(this_map, this_cost)));
    	}
    	else
    	{
    		unsigned n1 = SAabstr_to_gnd[depth][this_old_abs];
    		unsigned n2 = SAabstr_to_gnd[depth][this_new_abs];
	    	SAabstr_to_gnd[depth][this_new_abs]++;

	    	// Data

            std::unordered_map<unsigned, std::pair<int, float>>::iterator old_data_it = SAabstr_data[depth].find(this_old_abs);
            std::unordered_map<unsigned, std::pair<int, float>>::iterator new_data_it = SAabstr_data[depth].find(this_new_abs);

            if(old_data_it == SAabstr_data[depth].end())
                std::cout << "ERROR in OLD DATA !" << std::endl << std::flush;

            if(new_data_it == SAabstr_data[depth].end())
                std::cout << "ERROR in NEW DATA !" << std::endl << std::flush;


            std::pair<int, float> old_data = old_data_it->second;
            std::pair<int, float> new_data = new_data_it->second;

	    	// std::pair<int, float> old_data = SAabstr_data[depth][this_old_abs];
	    	// std::pair<int, float> new_data = SAabstr_data[depth][this_new_abs];

	    	int c1 = old_data.first; float v1 = old_data.second;
	    	int c2 = new_data.first; float v2 = new_data.second;
	    	
	    	int new_new_count = c2 + (c1/n1);

// if((c1 + n2*c2) == 0)
// {
// std::cout << "new_new_value:" << (c1 + n2*c2) << " c1:" << c1 << " n2:" << n2 << " c2:" << c2 << std::endl << std::flush;
// std::cout << "depth: " << depth << " state: " << s << " action: " << a << std::endl << std::flush;

// std::cout << "OLD ABS: ";
// for(typename std::vector<SA_pair>::iterator gnd_it = SAabstr_to_gnd[depth][this_old_abs].begin(); gnd_it != SAabstr_to_gnd[depth][this_old_abs].end(); ++gnd_it)
//     std::cout << "(" << gnd_it->first << "," << gnd_it->second << ") ";

// std::cout << "\nNEW ABS: ";
// for(typename std::vector<SA_pair>::iterator gnd_it = SAabstr_to_gnd[depth][this_new_abs].begin(); gnd_it != SAabstr_to_gnd[depth][this_new_abs].end(); ++gnd_it)
//     std::cout << "(" << gnd_it->first << "," << gnd_it->second << ") ";
// std::cout << std::endl << std::flush;
// }

		float new_new_value = (v2*c2 + (v1*c1/n1))/(c2 + (c1/n1));
		
		int new_old_count = c1*(n1-1)/n1;


	    	SAabstr_data[depth][this_new_abs] = std::make_pair(new_new_count, new_new_value);
	    	SAabstr_data[depth][this_old_abs] = std::make_pair(new_old_count, v1);
    	}

    	SAgnd_to_abstr[depth][std::make_pair(s,a)] = this_new_abs;

    	//typename std::vector<SA_pair>::iterator old_it = std::find(SAabstr_to_gnd[depth][this_old_abs].begin(), SAabstr_to_gnd[depth][this_old_abs].end(), std::make_pair(s,a));

    	SAabstr_to_gnd[depth][this_old_abs]--;
    	if(SAabstr_to_gnd[depth][this_old_abs]==0)
    	{
    		SAabstr_to_gnd[depth].erase(this_old_abs);

            SAabstr_data[depth].erase(this_old_abs);

    		//// MEM_LEAK: SHOULD WE DELETE TRANS_MAP before erase

    		std::unordered_map<unsigned, std::pair<transition_map,float>>::iterator tt_it = trans_tables[depth].find(this_old_abs);
    		if(tt_it != trans_tables[depth].end())
    		{
	    		trans_tables[depth].erase(this_old_abs);
	    	}
    	}

/*    	// Updating the (abstract) action counts of parent state nodes
    	std::map<unsigned, action_count>::iterator parentSt_it = action_tables[depth].begin();
    	for(; parentSt_it != action_tables[depth].end(); parentSt_it++)
    	{
    		parentSt_it->second[this_old_abs] -= 1;
    		if(parentSt_it->second[this_old_abs] == 0)
    			parentSt_it->second.erase(this_old_abs);

    		if(parentSt_it->second.find(this_new_abs) == parentSt_it->second.end())
    			parentSt_it->second.insert(std::make_pair(this_new_abs, 1));
    		else
    			parentSt_it->second[this_new_abs] += 1;
    	}*/

    	if(goUp)
        {
            std::list<SA_pair> saQueue;
            std::list<T> sQueue;

          
            sQueue.push_back(s);
            // saQueue2.push_back(std::make_pair(std::make_pair(s,a), depth));  // I should NOT add this node to the queue2 because it's abstraction has ALREADY been computed !

            unsigned cur_depth=depth;

            while(true)
            {
                while(!saQueue.empty())
                {
                    SA_pair this_saPair = saQueue.front();
                    saQueue.pop_front();
                    bool abstr_changed=recomp_SA_abstr(this_saPair.first, this_saPair.second, cur_depth, false);
                    if(abstr_changed)
                        sQueue.push_back(this_saPair.first);
                    
                }
                //  Removing Duplicates
                sQueue.sort();
                sQueue.unique();
                while(!sQueue.empty())
                {
                    T this_state = sQueue.front();
                    sQueue.pop_front();
                    bool abstr_changed=recomp_abstr(this_state,cur_depth);
                    if(abstr_changed)
                    {    
                        std::set<SA_pair> &this_parents = parents[cur_depth][this_state];
                        typename std::set<SA_pair>::iterator par_it = this_parents.begin();
                        for(; par_it != this_parents.end(); par_it++)
                        {
                            saQueue.push_back(*par_it);                            
                        }
                    }
                }
                //Removing Duplicates
                saQueue.sort();
                saQueue.unique();

                --cur_depth;

                if(saQueue.empty())
                    break;
            }

        }
        return true;
       // recomp_abstr(s, depth);
    }



    bool isPartiallyUnexplored(T s, unsigned depth) const
    {
	    int nactions = problem().number_actions(s);

		for(Problem::action_t a = 0; a < nactions; ++a)
		{
			if(problem().applicable(s, a))
			{
	            typename SA_hash_t<T,unsigned>::iterator saga_it = SAgnd_to_abstr[depth].find(std::make_pair(s,a));
	            if(saga_it == SAgnd_to_abstr[depth].end())
	            	return true;
			}
		}

		return false;
    }

    
    bool recomp_abstr(T s, unsigned depth) const
    {
    	action_set this_set;
    	int nactions=problem().number_actions(s);

    	for(Problem::action_t a = 0; a < nactions; a++)
    	{
    		if(problem().applicable(s,a))
    		{
    			typename SA_hash_t<T,unsigned>::iterator saga_it = SAgnd_to_abstr[depth].find(std::make_pair(s,a));
    			if(saga_it == SAgnd_to_abstr[depth].end())
    				continue;

    			this_set.insert(saga_it->second);
    		}
    	}


    	int this_old_abs = gnd_to_abstr[depth][s];
    	int this_new_abs = -1;

    	if(false/*isPartiallyUnexplored(s,depth)*/)
    		this_new_abs = 0;

    	else
    	{
	    	// std::map<action_set, unsigned>::iterator abs_it = rev_action_tables[depth].find(this_set);
      //       if(abs_it != rev_action_tables[depth].end())
      //           this_new_abs = abs_it->second;


// int this_new_abs2 = -1;
            for(std::unordered_map<unsigned, action_set>::iterator abs_it = action_tables[depth].begin(); abs_it != action_tables[depth].end(); abs_it++)
            {
            	if(this_set == abs_it->second)
            	{
                	this_new_abs = abs_it->first;
                	break;
                }
            }
	    }

    	if(this_old_abs == this_new_abs)
    		return false;


        if((this_new_abs == -1) && (abstr_to_gnd[depth][this_old_abs] == 1))
        {
            action_tables[depth][this_old_abs] = this_set;

            return false;
        }


    	if(this_new_abs == -1)
    	{
    		max_absSt[depth]++;
    		this_new_abs = max_absSt[depth];

    		abstr_to_gnd[depth].insert(std::make_pair(this_new_abs, 1));
    		action_tables[depth].insert(std::make_pair(this_new_abs, this_set));
    	}
    	else
    	{
    		abstr_to_gnd[depth][this_new_abs]++;
    	}
    	gnd_to_abstr[depth][s] = this_new_abs;

    	//typename std::vector<T>::iterator old_it = std::find(abstr_to_gnd[depth][this_old_abs].begin(), abstr_to_gnd[depth][this_old_abs].end(), s);
    	abstr_to_gnd[depth][this_old_abs]--;
    	if(abstr_to_gnd[depth][this_old_abs]==0)
    	{
    		abstr_to_gnd[depth].erase(this_old_abs);
    		//// MEM_LEAK: SHOULD WE DELETE ACTION SET before erase

    		std::unordered_map<unsigned, action_set>::iterator at_it = action_tables[depth].find(this_old_abs);
    		if(at_it != action_tables[depth].end())
    		{
	    		action_tables[depth].erase(this_old_abs);
	    	}
    	}

/*    	std::map<unsigned,transition_map>::iterator parentSA_it = trans_tables[depth-1].begin();
    	for(;parentSA_it!=trans_tables[depth-1].end();parentSA_it++)
    	{
    		float prob  = trans_prob((parentSA_it->first).first, (parentSA_it->first).second, s);

    		if(prob > 0)
    		{
	    		parentSA_it->second[this_old_abs] -= prob;
	    		if(parentSA_it->second[this_old_abs]==0)
	    			parentSA_it->second.erase(this_old_abs);

	    		if(parentSA_it->second.find(this_new_abs) == parentSA_it->second.end())
	    			parentSA_it->second.insert(std::make_pair(this_new_abs, prob));
	    		else
	    			parentSA_it->second[this_new_abs] += prob;
	    	}
    	}
*/

        return true;
    	// typename std::set<SA_pair>::iterator par_it = parents[depth][s].begin();
    	// for(; par_it != parents[depth][s].end(); par_it++)
    	// 	recomp_SA_abstr(par_it->first, par_it->second, depth-1);
    
    }

    float search_tree(const T &s, unsigned depth) const {


        state_node curr_node =std::make_pair(depth,s);

#ifdef DEBUG
        std::cout << std::setw(2*depth) << "" << "search_tree(" << s << "):";
#endif

        if( (depth == horizon_) || problem().terminal(s) ) {
#ifdef DEBUG
            std::cout << " end" << std::endl;
#endif
            
            if(gnd_to_abstr[depth].find(s) == gnd_to_abstr[depth].end())
            	init_abstr(s, depth);


            //// SHOULD SOMETHING ELSE BE DONE HERE ?
            //// Should we add it to the tree/table_ ?
            return 0;
        }

        if( problem().dead_end(s) ) {
#ifdef DEBUG
            std::cout << " dead-end" << std::endl;
#endif
            //// SHOULD SOMETHING BE DONE HERE ?
            return problem().dead_end_value();
        }

        typename hash_t<T>::iterator it = table_.find(curr_node);

        if( it == table_.end() )	// If this node is new (not present in tree)
        {
            //std::vector<float> values(1 + problem().number_actions(s), 0);
            //std::vector<int> counts(1 + problem().number_actions(s), 0);
            std::vector<int> visits(1 + problem().number_actions(s), 0);
            //table_.insert(std::make_pair(std::make_pair(depth, s), data_t(values, counts, visits)));
            table_.insert(std::make_pair(curr_node, data_t( visits)));
            
            //max_prob_info[depth].insert(std::make_pair(s,std::make_pair(0,0)));;
            timer++;
            state_birth_time[depth].insert(std::make_pair(timer,s));
           
            
            //// CHECK EVALUATE
            float value = evaluate(s, depth);	// doing a rollout from this node
#ifdef DEBUG
            std::cout << " insert in tree w/ value=" << value << std::endl;
#endif
            // Include this node as abstract with all unexplored nodes at this depth
           	init_abstr(s, depth);

//             if(depth > 0)
//             {
//                 // std::set<SA_pair> parents_set;
                            
//                 for(auto saga_it = SAgnd_to_abstr[depth-1].begin(); saga_it != SAgnd_to_abstr[depth-1].end(); saga_it++)
//                 {
//                     float prob = problem().trans_prob(saga_it->first.first, saga_it->first.second, s);

//                     if(prob == 0)
//                     {
// std::cout << "ZERO PROB!\n" << saga_it->first.first << "\nAction: " << saga_it->first.second << '\n' << s << std::endl;
//                         continue;
//                     }

//                     // parents_set.insert(saga_it->first);

//                     auto found_par = children[depth-1].find(saga_it->first);
//                     found_par->second.insert(s);
//                 }

//                 // parents[depth].insert(std::make_pair(s, parents_set));
//             }

            return value;
        }

        else
        {
            // select action for this node and increase counts
            //// CHECK SELECT_ACTION
            Problem::action_t a = select_action(s, it->second, depth, true, random_ties_);

            // sample next state
            // std::pair<const T, bool> p = problem().sample(s, a);

            std::pair<T, bool> p;
            problem().sample_factored(s, a, p.first);
            SA_pair curr_SA_Pair =std::make_pair(s,a);

            typename state_hash_t<T, std::set<SA_pair>>::iterator child_it = parents[depth+1].find(p.first);
            if(child_it == parents[depth+1].end())
            {
            	std::set<SA_pair> empty_set;
            	empty_set.insert(curr_SA_Pair);
            	parents[depth+1].insert(std::make_pair(p.first, empty_set));
            }

            else
            {
            	child_it->second.insert(curr_SA_Pair);
            }

            // typename SA_hash_t<T,std::set<T>>::iterator par_it = children[depth].find(curr_SA_Pair);
            // if(par_it == children[depth].end())
            // {
            // 	std::set<T> empty_set;
            // 	empty_set.insert(p.first);
            // 	children[depth].insert(std::make_pair(curr_SA_Pair, empty_set));
            // }
            // else
            // {
            // 	par_it->second.insert(p.first);
            // }


            // ++it->second.visits_[0];
            ++it->second.visits_[1+a];

            typename SA_hash_t<T,unsigned>::iterator abs_it = SAgnd_to_abstr[depth].find(curr_SA_Pair);

            if(abs_it == SAgnd_to_abstr[depth].end())   // This is a new SA node and therefore should be made singleton abstraction
            {
                init_SA_abstr(s, a, depth);
                // abs_it = SAgnd_to_abstr[depth].find(std::make_pair(s,a));   // update it's abstract node

                // Add all children of this new SA node
//                 std::set<T> children_set;
                
//     for(auto ga_it = gnd_to_abstr[depth+1].begin(); ga_it != gnd_to_abstr[depth+1].end(); ga_it++)
//     {
//         float prob = problem().trans_prob(s, a, ga_it->first);

//         if(prob == 0)
//         {
// std::cout << "ZERO PROB!\n" << s << "\nAction: " << a << '\n' << ga_it->first << std::endl;
//             continue;
//         }

//         children_set.insert(ga_it->first);

//         // auto found_child = parents[depth+1].find(ga_it->first);
//         // found_child->second.insert(curr_SA_Pair);
//     }

//                 children[depth].insert(std::make_pair(curr_SA_Pair, children_set));
            }


            float cost = problem().cost(s, a);

#ifdef DEBUG
            // std::cout << " count=" << it->second.counts_[0]-1
            //           << " fetch " << std::setprecision(5) << it->second.values_[1+a]
            //           << " a=" << a
            //           << " next=" << p.first
            //           << std::endl;
#endif

            float new_value = cost +
              					problem().discount() * search_tree(p.first, 1 + depth);	// New Sample for this state action pair

            
            abs_it = SAgnd_to_abstr[depth].find(curr_SA_Pair);

            if(it->second.visits_[1+a] == K)	// if SA node has been explored K times, recompute it's abstraction
            {
                // recomputing abstractions for the SA node and state node

                recomp_SA_abstr(s, a, depth, true);
                // recomp_abstr(s, depth);

                // reseting the vis count back to zero
                // it->second.visits_[1+a] = 0;

            	abs_it = SAgnd_to_abstr[depth].find(curr_SA_Pair);	// update it's abstract node
            }
            // Abstraction has been recomputed if necessary

            //// THIS ASSERT CAN BE REMOVED !
            //assert(abs_it != SAgnd_to_abstr[depth].end());
            
			std::unordered_map<unsigned, std::pair<int, float>>::iterator data_it = SAabstr_data[depth].find(abs_it->second);

			++(data_it->second).first;

// std::cout << "ABS in ((" << s << "," << a << ")," << depth << "): ";
// for(typename std::vector<SA_pair>::iterator gnd_it = SAabstr_to_gnd[depth][abs_it->second].begin(); gnd_it != SAabstr_to_gnd[depth][abs_it->second].end(); ++gnd_it)
//     std::cout << "(" << gnd_it->first << "," << gnd_it->second << ") ";
// std::cout << std::endl << std::flush;


			float &old_value = (data_it->second).second;
			int n = (data_it->second).first;
			old_value += (new_value - old_value) / n;

            //// SHOULD WE RETURN OLD_VALUE, NEW_VALUE or THE AVERAGED ABSTRACT VALUE ?

            // std::cout << "New Value: " << new_value << "\tOld Value: " << old_value << std::endl;
    		return new_value; //old_value;
        }
    }


    Problem::action_t select_action(const T &state,
                                    const data_t &data,
                                    int depth,
                                    bool add_bonus,
                                    bool random_ties) const {
        // float log_ns = logf(data.counts_[0]);

	    int nactions = problem().number_actions(state);

		// Compute the total count of this state node
		int action_counts[nactions];
		float action_values[nactions];
		int state_count = 0;

        std::vector<Problem::action_t> unexplored_actions;
        bool exists_unexplored = false;

		for(Problem::action_t a = 0; a < nactions; ++a)
		{
			if(problem().applicable(state, a))
			{
	            typename SA_hash_t<T,unsigned>::iterator saga_it = SAgnd_to_abstr[depth].find(std::make_pair(state,a));
	            if(saga_it == SAgnd_to_abstr[depth].end())
                {
                    unexplored_actions.push_back(a);
                    exists_unexplored = true;
	            	continue;
                }

                if(!exists_unexplored)
                {
    	            std::pair<int,float> this_data = SAabstr_data[depth][saga_it->second];
    	            action_values[a] = this_data.second;
    	            action_counts[a] = this_data.first;
    	            state_count += this_data.first;
                }
			}
		}

        if(!unexplored_actions.empty())
        {
            return unexplored_actions[Random::uniform(unexplored_actions.size())];
        }

		float log_ns = logf(state_count);

       std::vector<Problem::action_t> best_actions;
       float best_value = std::numeric_limits<float>::max();

       best_actions.reserve(random_ties ? nactions : 1);

        for( Problem::action_t a = 0; a < nactions; ++a )
        {
            if( problem().applicable(state, a) )
            {

                // compute score of action adding bonus (if applicable)
                float par = (/*parameter_*/0 == 0) ? -fabs(action_values[a]) : parameter_;
                float bonus = add_bonus ? par * sqrtf(2 * log_ns / action_counts[a]) : 0;
                float value = action_values[a] + bonus;

                // update best action so far
                if( value <= best_value ) {
                    if( value < best_value )
                    {
                        best_value = value;
                        best_actions.clear();
                    }
                    /*if( random_ties || best_actions.empty() )*/
                        best_actions.push_back(a);
                }
            }
        }

        //assert(!best_actions.empty());
        return best_actions[Random::uniform(best_actions.size())];
    }

    float evaluate(const T &s, unsigned depth) const {
        return Evaluation::evaluation(improvement_t<T>::base_policy_,
                                      s, 1, horizon_ - depth);	// 2nd parameter in this line is the number of rollouts
    }
};

}; // namespace UCT

template<typename T>
inline const policy_t<T>* make_uct(const policy_t<T> &base_policy,
                                   unsigned width,
                                   unsigned horizon,
                                   float parameter,
                                   bool random_ties) {
    return new UCT::uct_t<T>(base_policy, width, horizon, parameter, random_ties);
}

}; // namespace Policy

}; // namespace Online

#undef DEBUG

#endif

