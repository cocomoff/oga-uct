#include <iostream>
#include <iomanip>
#include <strings.h>

#define DISCOUNT 1
#define ROWS 3
#define COLUMNS 4
#define INIX 3
#define INIY 0
#define FINALX 3
#define FINALY 2

class state_t {
    short x_;
    short y_;

  public:
    state_t(short x = INIX, short y = INIY)
      : x_(x), y_(y) { }
    state_t(const state_t &s)
      : x_(s.x_), y_(s.y_) { }
    ~state_t() { }

    size_t hash() const {
        return (x_ << (8*sizeof(short))) | (y_);
    }

    bool in_grid(short rows, short cols) const {
        return (x_ >= 0) && (x_ < COLUMNS) && (y_ >= 0) && (y_ < ROWS);
    }
    std::pair<int, int> direction(Problem::action_t a) const {
        switch( a ) {
            case 0: return std::make_pair(0, 1);
            case 1: return std::make_pair(1, 0);
            case 2: return std::make_pair(0, -1);
            case 3: return std::make_pair(-1, 0);
      
            //default: return std::make_pair(-1, -1);
        }
    }

    state_t apply(Problem::action_t a) const {
        std::pair<int, int> dir = direction(a);
        return state_t(x_ + dir.first, y_ + dir.second);
    }

    const state_t& operator=( const state_t &s) {
        x_ = s.x_;
        y_ = s.y_;
        return *this;
    }
    bool operator==(const state_t &s) const {
        return (x_ == s.x_) && (y_ == s.y_) ;
    }
    bool operator!=(const state_t &s) const {
        return (x_ != s.x_) || (y_ != s.y_) ;
    }
    bool operator<(const state_t &s) const {
        return (x_ < s.x_) ||
               ((x_ == s.x_) && (y_ < s.y_));
    }
    void print(std::ostream &os) const {
        os << "(" << x_ << "," << y_ << ")";
    }
    friend class problem_t;
};

inline std::ostream& operator<<(std::ostream &os, const state_t &s) {
    s.print(os);
    return os;
}

class problem_t : public Problem::problem_t<state_t> {
    int rows_;
    int cols_;
    float disappearance_prob_[ROWS*COLUMNS];
    state_t init_;
    state_t goal_;

    static const float default_disappearance_prob_[];

  public:
    problem_t(int rows, int cols,
              int init_x = INIX, int init_y = INIY,
              int goal_x = std::numeric_limits<int>::max(),
              int goal_y = std::numeric_limits<int>::max())
      : Problem::problem_t<state_t>(DISCOUNT),
        rows_(rows), cols_(cols), init_(init_x, init_y), goal_(goal_x, goal_y) {

        if( (goal_x == std::numeric_limits<int>::max()) ||
            (goal_y == std::numeric_limits<int>::max()) ) {
            goal_.x_ =FINALX;
            goal_.y_=FINALY; //state_t(rows - 1, cols - 1);
        }
        bcopy(default_disappearance_prob_, disappearance_prob_, ROWS*COLUMNS * sizeof(float));
        
    }
    virtual ~problem_t() { }

    virtual Problem::action_t number_actions(const state_t &s) const { return 4; }
    
    virtual const state_t& init() const { return init_; }
    virtual bool terminal(const state_t &s) const {
        return (s.x_ == goal_.x_) && (s.y_ == goal_.y_);
    }
    virtual bool dead_end(const state_t &s) const { return false; }
    virtual bool applicable(const state_t &s, ::Problem::action_t a) const {
        return s.apply(a).in_grid(ROWS, COLUMNS);
    }
    virtual float cost(const state_t &s, Problem::action_t a) const {
        return terminal(s.apply(a)) ? 0 : 1;
    }
   virtual float calculate_transition(const state_t s1, const state_t s2, Problem::action_t a) const {

        state_t next_s = s1.apply(a);

        float p = disappearance_prob_[next_s.y_*COLUMNS+next_s.x_];

        if(s2 == init_)
        {
            if(next_s == init_)
                return 1.0;
            else
                return p;
        }

        else
        {
            if(s2 == next_s)
                return (1-p);
            else
                return 0;
        }
    }

    virtual void sample_factored( const state_t &s, Problem::action_t a, state_t &next_state) const {
        std::vector<std::pair<state_t, float>> outcomes;
        next(s, a, outcomes);

        float r = Random::real();

        for(unsigned i = 0; i < outcomes.size(); ++i)
        {
            if(r <= outcomes[i].second)
            {
                next_state = outcomes[i].first;
                break;
            }

            else
            {
                r = r - outcomes[i].second;
            }
        }
    }

    virtual void next(const state_t &s, Problem::action_t a, std::vector<std::pair<state_t,float> > &outcomes) const {
        ++expansions_;
        outcomes.clear();
        outcomes.reserve(2);
        state_t next_s = s.apply(a);
        assert(next_s.in_grid(rows_, cols_));
        float p=disappearance_prob_[next_s.y_*COLUMNS+next_s.x_];
        outcomes.push_back(std::make_pair(next_s,1-p));
        outcomes.push_back(std::make_pair(init_,p));
        
    }
    virtual void print(std::ostream &os) const { }
};


const float problem_t::default_disappearance_prob_[] = {
    // 0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,
    // 0.1,0.1,0.2,0.2,0.2,0.3,0.3,0.4,0.4,0.5,0.5,0.5,0.6,0.7,0.7,0.7,0.8,0.3,0.9,0.9,
    // 0.1,0.1,0.1,0.2,0.2,0.3,0.3,0.4,0.4,0.5,0.5,0.6,0.6,0.6,0.7,0.7,0.7,0.3,0.9,0.9,
    // 0.1,0.1,0.1,0.2,0.2,0.3,0.3,0.4,0.4,0.5,0.5,0.6,0.6,0.7,0.7,0.7,0.8,0.3,0.9,0.9,
    // 0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0

    0.0,0.0,0.0,0.0,
    0.04,0.34,0.63,0.91,
    0.0,0.0,0.0,0.0



     };

    //  const float problem_t::default_disappearance_prob_[] = {
    // 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,
    // 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,
    // 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,
    // 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,
    // 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,   




    //  };
// const float problem_t::default_disappearance_prob_[] = {
//     0.0,0.0,0.0,0.0,
//      0.5,0.3,0.6,0.9,
//       0.0,0.0,0.0,0.0


//     };


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


