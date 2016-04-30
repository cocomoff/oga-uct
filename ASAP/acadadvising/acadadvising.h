#include <iostream>
#include <iomanip>
#include <strings.h>

#define DISCOUNT 1
#define NUM_COURSES 10  // NUM_COURSES HAS TO BE LESS THAN equal to 16 !
#define COURSE_COST 1
#define COURSE_RETAKE_COST 2
#define PROGRAM_INCOMPLETE_PENALTY 5
#define PRIOR_PROB_PASS_NO_PREREQ 0.8
#define PRIOR_PROB_PASS 0.2

class state_t {
    unsigned short course_taken_;
    unsigned short course_passed_;

  public:
    state_t()
   {
        course_taken_ = 0;
        course_passed_ = 0;
    }

    state_t(const state_t &s)
     {
        course_taken_ = s.course_taken_;
        course_passed_ = s.course_passed_;
      }

    ~state_t() { }

    size_t hash() const {
        return (course_passed_ << (8*sizeof(unsigned short))) | (course_taken_);
    }

  
    const state_t& operator=( const state_t &s)
    {

        course_taken_ = s.course_taken_;
        course_passed_ = s.course_passed_;
       
        return *this;
    }

    bool operator==(const state_t &s) const {

        return ((course_taken_ == s.course_taken_) && (course_passed_ == s.course_passed_));
    }

    bool operator!=(const state_t &s) const {
        return !(*this==(s));   
    }

    bool operator<(const state_t &s) const {
        
        if(course_passed_ < s.course_passed_)
            return true;

        if(course_passed_ > s.course_passed_)
            return false;

        if(course_taken_ < s.course_taken_)
            return true;

        return false;
    }

    inline bool get(unsigned short bitVector, int i) const
    {
    	return ((bitVector & (1 << i)) != 0);
    }

     void print(std::ostream &os) const {
        os << "[" ;
           for(int i=0;i<NUM_COURSES;i++)
            {
                os<<"("<<get(course_passed_,i)<<","<<get(course_taken_,i)<<"),";
            }
            os<<"]";
    }
    friend class problem_t;
};

inline std::ostream& operator<<(std::ostream &os, const state_t &s) {
    s.print(os);
    return os;
}

class problem_t : public Problem::problem_t<state_t> {
   
    bool prog_req[NUM_COURSES];
    state_t init_;
    state_t goal_;
    bool prereq[NUM_COURSES*NUM_COURSES];
    static const bool default_prereq[NUM_COURSES*NUM_COURSES];
    static const bool default_prog_req[NUM_COURSES];

  public:
    problem_t(int dim1, int dim2)
      : Problem::problem_t<state_t>(DISCOUNT)
        {
        	init_.course_passed_ = 0;
        	init_.course_taken_ = 0;

            bcopy(default_prereq, prereq, NUM_COURSES*NUM_COURSES * sizeof(bool));
            bcopy(default_prog_req,prog_req,NUM_COURSES* sizeof(bool));
            
            goal_.course_passed_ = ((1 << NUM_COURSES) - 1);
            goal_.course_taken_ = ((1 << NUM_COURSES) - 1);

    }
    virtual ~problem_t() { }

    virtual Problem::action_t number_actions(const state_t &s) const { return NUM_COURSES; }
    
    virtual const state_t& init() const { return init_; }
    virtual bool terminal(const state_t &s) const {
        for(int i=0;i<NUM_COURSES;i++)
        {
            if((s.get(s.course_passed_,i)==false)||(s.get(s.course_taken_,i)==false))
                return false;
        }
        return (true);
    }
    virtual bool dead_end(const state_t &s) const { return false; }
    virtual bool applicable(const state_t &s, ::Problem::action_t a) const {
        return true;
    }
    
    virtual bool check_program_incomplete(const state_t &s) const {
        for(int i=0;i<NUM_COURSES;i++)
        {
            if((prog_req[i]) && (!s.get(s.course_passed_,i)))
                return true;
        }
        return false;
    }


    virtual float cost(const state_t &s, Problem::action_t a) const {
        float total_cost=0;
        if(check_program_incomplete(s))
            total_cost=total_cost+PROGRAM_INCOMPLETE_PENALTY;
        if(s.get(s.course_taken_,a)==1)
            total_cost=total_cost+COURSE_RETAKE_COST;
        else
            total_cost=total_cost+COURSE_COST;
        return total_cost;
   
    }
  

    virtual float calculate_transition(const state_t s1, const state_t s2, Problem::action_t a) const
    {
        for(int i=0;i<NUM_COURSES;i++)
        {
            if(a==i)
                continue;
            if((s1.get(s1.course_passed_,i)!=s2.get(s2.course_passed_,i))||(s1.get(s1.course_taken_,i)!=s2.get(s2.course_taken_,i)))
                return 0.0;
        }

        if(!s2.get(s2.course_taken_,a))
            return 0;

        if(s1.get(s1.course_passed_,a))
        {    
            if(s2.get(s2.course_passed_,a))
                return 1;
            else
                return 0;
        }

        bool PreReq_Exists=false;
        unsigned num_prereqs=0;
        unsigned num_passed_prereqs=0;

        for(int i=0;i<NUM_COURSES;i++)
        {
            if(prereq[a*NUM_COURSES+i]==1)
            {
                PreReq_Exists=true;
                num_prereqs+=1;
                if(s1.get(s1.course_passed_,i)==true)
                    num_passed_prereqs+=1;
            }
        }
        if((!PreReq_Exists)&&(s2.get(s2.course_passed_,a)==true))
            return PRIOR_PROB_PASS_NO_PREREQ;
        else if ((!PreReq_Exists)&&(s2.get(s2.course_passed_,a)==false))
            return (1- PRIOR_PROB_PASS_NO_PREREQ);

        float p=PRIOR_PROB_PASS+(1- PRIOR_PROB_PASS)*(num_passed_prereqs/(1+num_prereqs));
        if(s2.get(s2.course_passed_,a)==true)
            return (p);
        else
            return 1-p;

    }

    virtual void sample_factored(const state_t &s, Problem::action_t a, state_t &outcome) const 
    {

        outcome.course_taken_ = s.course_taken_;
        outcome.course_passed_ = s.course_passed_;

        outcome.course_taken_ = outcome.course_taken_ | (1 << a);	// setting the a_th bit of course_taken_

       outcome.course_passed_ = outcome.course_passed_ | (1 << a);	// setting the a_th bit of course_passed_

       float p = calculate_transition(s,outcome,a);


        float r = Random::real();
        if(r<p)
			outcome.course_passed_ = outcome.course_passed_ | (1 << a);	// setting the a_th bit of course_passed_        	
        else
			outcome.course_passed_ = outcome.course_passed_ & (~(1 << a));	// clearing the a_th bit of course_passed_        	
    }





    virtual void next(const state_t &s, Problem::action_t a, std::vector<std::pair<state_t,float> > &outcomes) const {
       
         
    }
    virtual void print(std::ostream &os) const { }
};
const bool problem_t::default_prereq[] = {
 0,0,0,0,0,0,0,0,0,0,
 0,0,0,0,0,0,0,0,0,0,
 1,1,0,0,0,0,0,0,0,0,
 0,1,1,0,0,0,0,0,0,0,
 1,0,1,0,0,0,0,0,0,0,
 1,0,0,1,0,0,0,0,0,0,
 1,0,0,1,0,0,0,0,0,0,
 1,0,0,0,0,0,0,0,0,0,
 0,1,0,0,0,0,1,0,0,0,
 0,0,0,1,1,0,0,0,0,0

};


const bool problem_t::default_prog_req[] = {
0,0,1,1,0,0,1,0,0,0
    
};

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


