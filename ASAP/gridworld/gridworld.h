#include <iostream>
#include <iomanip>
#include <strings.h>
#include <vector>

#define DISCOUNT .95

using namespace std;

class state_t {
  short x_;
  short y_;

public:
  state_t(short x = 0, short y = 0) : x_(x), y_(y) { }
  state_t(const state_t &s) : x_(s.x_), y_(s.y_) { }
  ~state_t() { }

  size_t hash() const {
    return (x_ << 3) | y_;
  }

  bool in_gridworld(short rows, short cols) const {
    return (x_ >= 0) && (x_ < rows) && (y_ >= 0) && (y_ < cols);
  }
  std::pair<int, int> direction(Problem::action_t a) const {
    switch( a ) {
    case 0: return std::make_pair(0, 1);
    case 1: return std::make_pair(1, 0);
    case 2: return std::make_pair(0, -1);
    case 3: return std::make_pair(-1, 0);
    default: return std::make_pair(0, 0);
    }
  }

  state_t apply(Problem::action_t a) const {
    std::pair<int, int> dir = direction(a);
    return state_t(x_ + dir.first, y_ + dir.second);
  }

  const state_t& operator=(const state_t &s) {
    x_ = s.x_;
    y_ = s.y_;
    return *this;
  }
  bool operator==(const state_t &s) const {
    return (x_ == s.x_) && (y_ == s.y_);
  }
  bool operator!=(const state_t &s) const {
    return (x_ != s.x_) || (y_ != s.y_);
  }
  bool operator<(const state_t &s) const {
    return (x_ < s.x_) || ((x_ == s.x_) && (y_ < s.y_));
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
  state_t init_;
  state_t goal1_; // corresponding to (0, 0)
  state_t goal2_; // corresponding to (rows_-1, cols_-1)

public:
  problem_t(int rows, int cols,
            int init_x = 2, int init_y = 2,
            float *wind_transition = 0, float *costs = 0)
    : Problem::problem_t<state_t>(DISCOUNT),
    rows_(rows), cols_(cols), init_(init_x, init_y), goal1_(0, 0), goal2_(rows-1, cols-1) {

  }
  virtual ~problem_t() { }

  virtual Problem::action_t number_actions(const state_t &s) const { return 4; }

  virtual const state_t& init() const { return init_; }
  
  virtual bool terminal(const state_t &s) const {
    bool g1 = (s.x_ == goal1_.x_) && (s.y_ == goal1_.y_);
    bool g2 = (s.x_ == goal2_.x_) && (s.y_ == goal2_.y_);
    return g1 || g2;
  }
  
  virtual bool dead_end(const state_t &s) const { return false; }
  
  virtual bool applicable(const state_t &s, ::Problem::action_t a) const {
    return s.apply(a).in_gridworld(rows_, cols_);
  }
  
  virtual float min_absolute_cost() const { return 1; }
  
  virtual float max_absolute_cost() const { return 4; }
  
  virtual float cost(const state_t &s, Problem::action_t a) const {
    return terminal(s) ? 0 : 1.0;
  }
  
  virtual int max_action_branching() const {
    return 4;
  }
  virtual int max_state_branching() const {
    return 4;
  }
  
  virtual void next(const state_t &s, Problem::action_t a,
                    vector<std::pair<state_t,float> > &outcomes) const {
    ++expansions_;
    outcomes.clear();
    state_t next_s = s.apply(a);
    assert(next_s.in_gridworld(rows_, cols_));
    outcomes.push_back(std::make_pair(next_s, 1.0));
  }
  virtual void print(std::ostream &os) const { }

  virtual float calculate_transition(const state_t s1,
                                     const state_t s2,
                                     Problem::action_t a) const {
    auto x1 = s1.x_;
    auto y1 = s1.y_;
    auto x2 = s2.x_;
    auto y2 = s2.y_;

    if (a == 0) {
      if (x1 == x2 && y1 + 1 == y2) {
        return 1.0;
      } else {
        return 0.0;
      }
    } else if (a == 1) {
      if (x1 + 1 == x2 && y1 == y2) {
        return 1.0;
      } else {
        return 0.0;
      }
    } else if (a == 2) {
      if (x1 == x2 && y1 - 1 == y2) {
        return 1.0;
      } else {
        return 0.0;
      }
    } else if (a == 3) {
      if (x1 - 1== x2 && y1 == y2) {
        return 1.0;
      } else {
        return 0.0;
      }
    } else {
      return 0.0;
    }
  }

  virtual void sample_factored( const state_t &s, Problem::action_t a,
                                state_t &next_state) const {
    std::vector<std::pair<state_t, float>> outcomes;
    next(s, a, outcomes);

    float r = Random::real();

    for(int i = 0; i < outcomes.size(); ++i) {
      if(r <= outcomes[i].second) {
        next_state = outcomes[i].first;
        break;
      } else {
        r = r - outcomes[i].second;
      }
    }
  }
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
