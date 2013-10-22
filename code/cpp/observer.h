#ifndef observer_h
#define observer_h

namespace ObserverPattern
{
  using namespace std;

  class Observable;

  class Observer
  {
    friend class Observable;

  protected:
    Observer()
    : next_(NULL)
    {}

    virtual ~Observer() {}
    virtual void observe(Observable & observable) = 0;

  private:
    Observer* next_;
  };
  
  class Observable
  {
  public:
    void addObserver(Observer & observer)
    {
      // Observer can only observe one object.
      // TODO(bob): Can we loosen this?
      assert(observer.next_ == NULL);

      observer.next_ = observer_;
      observer_ = &observer;
    }

  protected:
    Observable()
    : observer_(NULL)
    {}

    void notify()
    {
      cout << "notify" << endl;
      Observer* observer = observer_;
      while (observer != NULL)
      {
        observer->observe(*this);
        observer = observer->next_;
      }
    }

    Observer* observer_;
  };

  class PhysicsBody : public Observable
  {
  public:
    void collide()
    {
      cout << "collide" << endl;
      notify();
    }
  };

  class AudioEngine : public Observer
  {
  protected:
    void observe(Observable & observable)
    {
      cout << "clang!" << endl;
    }
  };

  void test()
  {
    PhysicsBody body;
    AudioEngine audio;

    body.addObserver(audio);

    body.collide();
  }
}

#endif
