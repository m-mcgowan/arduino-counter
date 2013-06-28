/*
 * counter.h
 *
 * Created: 21/03/2013 22:25:05
 *  Author: mat
 */ 

#pragma once


#include <stddef.h>
#include <stdint.h>
#include <limits.h>
#include <assert.h>
extern uint32_t millis();

/*
   For simple management of all the timers, they are
   stored in a list. The advance method advances all timers
   at once. This is the only operation that is cascaded to
   all timers. 
   
   
   MA -> ST/1000(sec) -> ST/60(min)
   
   to add a milli counter
      - add to end of MA
      - MA -> T -> ST/1000 ...
      
   to add a seconds counter
     - add to the end of ST/1000
     - MA -> ST/1000(sec) -> S -> 
     
     
   MillisAdvance millisTrakcer;
   ScaleTimer seconds(millisTracker, 1000);
   ScaleTimer minutes(seconds, 60);
   
   this sets up the 
 */


/*! A timer type that is advanced.
 * When a period of time has elapsed, the timer is notified by it's advance() method. 
 */
template <class advance_t>
class ChainedTimer
{
public:			
	/**
	 * Advances this timer by the amount given.
	 * \param amount	The amount to advance the counter
	 */
	virtual void advance(advance_t amount) { }
		
};

/**
 * The default "next" ChainedTimer.
 * Even though the type is declared as uint8_t 
 */
extern ChainedTimer<uint8_t> terminator;

template<class advance_t> ChainedTimer<advance_t>* chainTerminator() {
	return (ChainedTimer<advance_t>*)&terminator;
}



/**
 * This is the timer that is previous in the chain.
 * \tparam advance_t the type for the argument to the advance() member function exposed by this timer. 
 * \tparam next_t the advance type for the next timer in the chain. Typically it is the same as advance_t
 * but timers that convert time from one scale to another may use different types. 
 */
template <class next_t> 
class BaseChainedTimer
{
	
public:	
	BaseChainedTimer() : next(chainTerminator<next_t>()) {}
		
	/**
	 * Add a timer to the head of this list. 
	 * The previous head is made the second in line.
	 */
	void chain(BaseChainedTimer<next_t> & head) {
		head.next = next;
		this->next = &head; 
	}
	
	/**
	 * Sets the list. This can only be performed once.
	 */
	void set(ChainedTimer<next_t>& list) {
		//assert(this->next == &chainTerminator<next_t>());
		this->next = &list;
	}		
  
  	/**
	 * The next timer in the chain.
	 */
	ChainedTimer<next_t>* next;		
};

template<class counter_t, class advance_t, class next_t = advance_t> 
class BaseTimer : public ChainedTimer<advance_t>, public BaseChainedTimer<next_t> {
	
public:
  BaseTimer() { this->reset(); }
  
  virtual void advance(advance_t amount) {
    increment(amount);
    this->next->advance(next_t(amount));
  }
  
  void increment(advance_t amount) {
	  counter += amount;
  }
  
  counter_t value() const { return counter; }
  
  void reset() 
  { counter = (counter_t)0; }
  
  bool operator>=(counter_t period) {
	  return elapsed(period);
  }
  
  /* Determines if the given period has elapsed */
  bool elapsed(counter_t period) {
      bool result;
      if ((result=(this->counter>=period))) {
          this->counter -= period;
      }
      return result;
  }
  
protected:
  counter_t counter;
};


typedef uint32_t milli_counter_t;
typedef uint16_t milli_advance_t;

class MillisCounter  : public BaseTimer<milli_counter_t, milli_advance_t>
{
public:
	MillisCounter() {}

  /* Advances the counter based on the time since the
     previous call to this method. */
  void update() {
    milli_counter_t now = millis();
    
    // even with overflow, subtraction of unsigned types
    // is guaranteed to be correct
    milli_advance_t step = milli_advance_t(now-this->value());
    this->advance(step);		
  }
};


template<class counter_t, class advance_t, class next_t> 
class Multiplier : public BaseTimer<counter_t, advance_t, next_t>
{
public:
   Multiplier(uint8_t _scale, BaseChainedTimer<advance_t>& source) 
   : scale(_scale) { source.set(*this); }
   
  virtual void advance(advance_t amount) {
    this->increment(amount);
    if (this->elapsed(scale)) {
      this->next->advance(next_t(1));
    }
  }
  
private:
	uint8_t scale;  
};

