#include <iostream>
#include <iomanip>
#include <limits.h>
#include <strings.h>

#define DISCOUNT 1
#define NUM_COMPUTERS 10
#define REBOOT_PENALTY 0.75
#define REBOOT_PROB 0.05


//int Decisions_Remaining=100;
class state_t {
    unsigned short computer_state_;   

  public:
    state_t()
    {
       computer_state_ = (1<<NUM_COMPUTERS)-1;
    }
    state_t(const state_t &s)
    {
           computer_state_ =s.computer_state_;
    }

    ~state_t() { }

    size_t hash() const 
    {
        return computer_state_;
    }

  
    const state_t& operator=( const state_t &s) 
    {
        computer_state_=s.computer_state_;
        return *this;
    }

    bool getIthCompStatus(unsigned i) const
    {
        return ((computer_state_ & (1<<i))!=0);
    }
    const state_t& SetIthCompStatus(unsigned i, bool Value) 
    {
        if(Value)
            computer_state_|= (1<<i);
        else
            computer_state_ &= ~(1<<i);
        return *this;

    }

    bool operator==(const state_t &s) const 
    {
        if(computer_state_!=s.computer_state_)
            return 0;
        return  1;
    }

    bool operator!=(const state_t &s) const 
    {
        return (1-operator==(s));
       
    }
    bool operator<(const state_t &s) const 
    {
       if(computer_state_<s.computer_state_)
            return true;
        else
            return false;
       
    }
     void print(std::ostream &os) const {
        os << "(" ;
           for(int i=0;i<NUM_COMPUTERS;i++)
            {
                os<<getIthCompStatus(i)<<",";
            }
            os<<")";
    }
    friend class problem_t;
};

inline std::ostream& operator<<(std::ostream &os, const state_t &s) {
    s.print(os);
    return os;
}

class problem_t : public Problem::problem_t<state_t> {
   
    float costs_[5];
    state_t init_;
    state_t goal_;
    bool connections_[NUM_COMPUTERS*NUM_COMPUTERS];
    static const bool default_connections_[];
    static const float default_wind_transition_[];
    static const float default_costs_[];

  public:
    problem_t(int dim1, int dim2)
      : Problem::problem_t<state_t>(DISCOUNT)
          {

         init_.computer_state_=((1<<NUM_COMPUTERS)-1);
         bcopy(default_connections_, connections_, NUM_COMPUTERS*NUM_COMPUTERS * sizeof(bool));
         for (int i=0;i<NUM_COMPUTERS;i++)  
            for(int j=0;j<NUM_COMPUTERS;j++)
            {
              /* if((i==0)||(i==j))
               {
                   connections_[i*NUM_COMPUTERS+j]=1;
                   connections_[j*NUM_COMPUTERS+i]=1;
                }*/
                if((i==j)||(i==j+1)||(i+1==j))
                {
                    connections_[i*NUM_COMPUTERS+j]=1;
                    connections_[j*NUM_COMPUTERS+i]=1;
                }
                

            } 
            connections_[0*NUM_COMPUTERS+NUM_COMPUTERS-1]=1;
            connections_[(NUM_COMPUTERS-1)*NUM_COMPUTERS+0]=1;
            //connections_[NUM_COMPUTERS-1]=1;
            //connections_[NUM_COMPUTERS*(NUM_COMPUTERS-1)]=1;

      
    }
    virtual ~problem_t() { }

    virtual Problem::action_t number_actions(const state_t &s) const { return 1+NUM_COMPUTERS; }
    
    virtual const state_t& init() const { return init_; }
    virtual bool terminal(const state_t &s) const {

        return (false);
    }
    virtual bool dead_end(const state_t &s) const { return false; }
    virtual bool applicable(const state_t &s, ::Problem::action_t a) const {
        return true;
    }
    virtual float cost(const state_t &s, Problem::action_t a) const {
        float reward=0;

        std::vector<std::pair<state_t,float> > outcomes;
        
        reward=(a==0)?0:-REBOOT_PENALTY;
        reward+=working_comps(s);
        //next(s,a,outcomes);
        /*for(unsigned i=0;i<outcomes.size();i++)
        {
            reward+=outcomes[i].second*working_comps(outcomes[i].first);
        }*/
          
       return -1*reward;
       // return terminal(s) ? 0 : -1*reward;
    }
     virtual unsigned working_comps( const state_t &s) const{
        unsigned count=0;
        for(int i=0;i<NUM_COMPUTERS;i++)
        {
            if(s.getIthCompStatus(i))
                count+=1;
        }    
        return count;

    }

    virtual float calculate_transition(const state_t s1, const state_t s2,Problem::action_t a) const{
        float p=1;
        //return p;
        for(int i=0;i<NUM_COMPUTERS;i++)
        {
            float m=1;
            if(a!=i+1)
            {
                if(s1.getIthCompStatus(i)==1)
                {
                    int nbrs=0;
                    int active_nbrs=0;

                    for(int j=0;j<NUM_COMPUTERS;j++)
                    {
                        if(i!=j)
                        {
                            if(connections_[j*NUM_COMPUTERS+i]==1)
                            {
                                nbrs+=1;
                                if(s1.getIthCompStatus(j)==1)
                                    active_nbrs+=1;
                             }
                        }     
                    }
                    m=0.45+0.5*(1.0+(float)active_nbrs)/(1+nbrs);
                }
                else
                    m=REBOOT_PROB;
            }   
            if(s2.getIthCompStatus(i)==1)
                p*=m;
            else
                p*=(1-m);
        }
       
        return p;

    }
    virtual void sample_factored(const state_t &s, Problem::action_t a, state_t &outcome) const {

        for(int i=0;i<NUM_COMPUTERS;i++)
        {
             float m=1;
             if(a!=i)
             {
                if(s.getIthCompStatus(i)==1)
                {
                    int nbrs=0;
                    int active_nbrs=0;
                    for(int j=0;j<NUM_COMPUTERS;j++)
                    {
                        if(i!=j)
                        {
                            if(connections_[j*NUM_COMPUTERS+i]==1)
                            {
                                nbrs+=1;
                                if(s.getIthCompStatus(j)==1)
                                    active_nbrs+=1;
                             }
                        }     
                    }
                    m=0.45+0.5*(1.0+(float)active_nbrs)/(1+nbrs);
                }
                else
                    m=REBOOT_PROB;
            }
            float r = Random::real();
            //std::cout<<"Random Numer"<<r<<"\n";
            if(r<m)
                outcome = outcome.SetIthCompStatus(i,true);
            else
                outcome = outcome.SetIthCompStatus(i,false);

        }

    }





    virtual void next(const state_t &s, Problem::action_t a, std::vector<std::pair<state_t,float> > &outcomes) const {
      /*  ++expansions_;
       outcomes.clear();
       outcomes.reserve(NUM_COMPUTERS);
       state_t next_s;
       float p;
       if(a!=i)

       for(int i=0;i<NUM_COMPUTERS;i++)
       {

       }*/


       /*
         outcomes.clear();
        outcomes.reserve(pow(2,NUM_COMPUTERS));
        state_t next_s;
        float p;
        for(int i=0;i<(pow(2,NUM_COMPUTERS));i++)
        {
            for(int j=0;j<NUM_COMPUTERS;j++)
            {
                next_s.computer_state_[j]=(((1<<j)&i)&&1);
            }
            p=calculate_transition(s,next_s,a);
            if(p>1)
            {
                std::cout<<"error here";
                exit(0);
            }
            if(p>0)
            {
                outcomes.push_back(std::make_pair(next_s, p));
            }
        }*/
         
    }
    virtual void print(std::ostream &os) const { }
};
const bool problem_t::default_connections_[] = {
0

   /* 1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1

*/

   /* 1,1,1,1,1,1,1,1,1,1,
    1,1,0,0,0,0,0,0,0,0,
    1,0,1,0,0,0,0,0,0,0,
    1,0,0,1,0,0,0,0,0,0,
    1,0,0,0,1,0,0,0,0,0,
    1,0,0,0,0,1,0,0,0,0,
    1,0,0,0,0,0,1,0,0,0,
    1,0,0,0,0,0,0,1,0,0,
    1,0,0,0,0,0,0,0,1,0,
    1,0,0,0,0,0,0,0,0,1*/

/*    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0*/
 /*   1,0,0,0,1,0,0,0,0,1,
    1,1,0,0,0,0,1,1,0,0,
    0,0,1,1,0,0,1,0,0,0,
    0,0,1,1,1,1,0,0,0,0,
    1,1,0,0,1,0,0,1,0,0,
    0,0,1,0,0,1,1,0,0,1,
    0,1,0,0,0,0,1,1,1,0,
    1,0,0,1,1,0,0,1,0,0,
    0,1,0,0,1,1,0,0,1,0,
    0,1,0,1,0,0,0,1,0,1

*/
   /* 1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1*/

// 1,1,1,1,1,
// 1,1,1,1,1,
// 1,1,1,1,1,
// 1,1,1,1,1,
// 1,1,1,1,1
   /*
   1,1,0,0,0,
   1,1,0,0,0,
   0,0,1,0,0,
   0,0,0,1,0,
   0,0,0,0,1*/


/*1,0,0,1,0,
0,1,0,0,0,
0,0,1,1,0,
0,0,0,1,1,
0,0,0,0,1
  */
/*  	1,0,0,1,0,0,0,0,
    0,1,0,0,0,0,0,1,
    0,0,1,1,0,0,0,0,
    0,0,0,1,1,0,0,0,
    0,0,0,0,1,0,1,0,
    0,0,0,1,0,1,0,1,
    0,0,0,0,0,0,1,0,
    0,0,0,0,0,1,0,1 */
/*
    1,1,0,0,0,0,0,0,0,0,
    0,1,1,0,0,0,0,0,0,0,
    0,0,1,1,0,0,0,0,0,0,
    0,0,0,1,1,0,0,0,0,0,
    0,0,0,0,1,1,0,0,0,0,
    0,0,0,0,0,1,1,0,0,0,
    0,0,0,0,0,0,1,1,0,0,
    0,0,0,0,0,0,0,1,1,0,
    0,0,0,0,0,0,0,0,1,1,
    1,0,0,0,0,0,0,0,0,1,

*/
  /*  1,0,0,1,0,0,0,0,1,0,
    0,1,0,0,0,0,0,1,0,0,
    0,0,1,1,0,0,0,0,1,0,
    0,0,0,1,1,0,0,0,0,0,
    0,0,0,0,1,0,1,0,0,0,
    0,0,0,1,0,1,0,1,0,0,
    0,0,0,0,0,0,1,0,1,0,
    0,0,0,0,0,1,0,1,0,1,
    0,0,0,0,0,1,0,0,1,0,
    0,1,0,0,0,0,0,0,0,1*/

};


/*const float problem_t::default_costs_[] = {
    1, 2, 3, 4, std::numeric_limits<float>::max()
};*/

inline std::ostream& operator<<(std::ostream &os, const problem_t &p) {
    p.print(os);
    return os;
}

class scaled_heuristic_t : public Heuristic::heuristic_t<state_t> {
    const Heuristic::heuristic_t<state_t> *h_;
    float multiplier_;
  public:
    scaled_heuristic_t(const Heuristic::heuristic_t<state_t> *h, float multiplier = 1.0)
      : h_(h), multiplier_(multiplier) { }
    virtual ~scaled_heuristic_t() { }
    virtual float value(const state_t &s) const {
        return h_->value(s) * multiplier_;
    }
    virtual void reset_stats() const { }
    virtual float setup_time() const { return 0; }
    virtual float eval_time() const { return 0; }
    virtual size_t size() const { return 0; }
    virtual void dump(std::ostream &os) const { }
    float operator()(const state_t &s) const { return value(s); }
};

class zero_heuristic_t: public Heuristic::heuristic_t<state_t> {
  public:
    zero_heuristic_t() { }
    virtual ~zero_heuristic_t() { }
    virtual float value(const state_t &s) const { return 0; }
    virtual void reset_stats() const { }
    virtual float setup_time() const { return 0; }
    virtual float eval_time() const { return 0; }
    virtual size_t size() const { return 0; }
    virtual void dump(std::ostream &os) const { }
    float operator()(const state_t &s) const { return value(s); }
};


