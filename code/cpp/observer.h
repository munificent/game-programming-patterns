#ifndef observer_h
#define observer_h

namespace ObserverPattern
{
  using namespace std;

  static const int SURFACE_BRIDGE = 0;

  class Entity {
  public:
    bool isHero() const { return false; }
    bool isStandingOn(int surface) const { return false; }
  };

  enum Event
  {
    EVENT_ENTITY_FELL
  };

  enum Achievement
  {
    ACHIEVEMENT_FELL_OFF_BRIDGE
  };

  class PhysicsBody
  {
  public:
    bool isOnSurface() { return true; }
    void accelerate(int force) {}
    void update() {}
  };

  static const int GRAVITY = 1;
  static const int EVENT_START_FALL = 1;

  namespace Motivation
  {
    class Physics
    {
    public:
      void updateBody(PhysicsBody& body);
      void notify(int event, PhysicsBody& body) {}
    };

    //^physics-update
    void Physics::updateBody(PhysicsBody& body)
    {
      bool wasOnSurface = body.isOnSurface();
      body.accelerate(GRAVITY);
      body.update();
      if (wasOnSurface && !body.isOnSurface())
      {
        notify(EVENT_START_FALL, body);
      }
    }
    //^physics-update
  }

  namespace Pattern
  {
    //^observer
    class Observer
    {
    public:
      virtual ~Observer() {}

      virtual void onNotify(const Entity& entity, Event event) = 0;
    };
    //^observer

    //^achievement-observer
    class Achievements : public Observer
    {
    protected:
      void onNotify(const Entity& entity, Event event)
      {
        switch (event)
        {
        case EVENT_ENTITY_FELL:
          if (entity.isHero() && heroIsOnBridge_)
          {
            unlock(ACHIEVEMENT_FELL_OFF_BRIDGE);
          }
          break;

          // Handle other events, and update heroIsOnBridge_...
        }
      }

    private:
      void unlock(Achievement achievement)
      {
        // Unlock if not already unlocked...
      }

      bool heroIsOnBridge_;
    };
    //^achievement-observer

    static const int MAX_OBSERVERS = 10;

    //^subject-list
    //^subject-register
    class Subject
    {
      //^omit subject-list
    public:
      void addObserver(Observer* observer)
      {
        observers_[numObservers_++] = observer;
      }

      void removeObserver(Observer* observer)
      {
        int index;
        for (index = 0; index < MAX_OBSERVERS; index++)
        {
          if (observers_[index] == observer) break;
        }

        if (index < numObservers_)
        {
          // Shift the following ones up.
          for (; index < numObservers_ - 1; index++)
          {
            observers_[index] = observers_[index + 1];
          }

          numObservers_--;
        }
      }

      // Other stuff...
      //^omit subject-list
      //^omit subject-register
    protected:
      void notify(const Entity& entity, Event event);

    private:
      Observer* observers_[MAX_OBSERVERS];
      int numObservers_;
      //^omit subject-register
    };
    //^subject-list
    //^subject-register

    //^subject-notify
    void Subject::notify(const Entity& entity, Event event)
    {
      for (int i = 0; i < numObservers_; i++)
      {
        observers_[i]->onNotify(entity, event);
      }
    }
    //^subject-notify

    //^physics-inherit
    class Physics : public Subject
    {
    public:
      void updateBody(PhysicsBody& body);
    };
    //^physics-inherit

    class PhysicsEvent : public Observer
    {
      Subject entityFell_;
      Subject& entityFell() { return entityFell_; }

      void onNotify(const Entity& entity, Event event) {}

      void physicsEvent()
      {
        PhysicsEvent physics;

        //^physics-event
        physics.entityFell()
          .addObserver(this);
        //^physics-event
      }
    };
  }

  namespace LinkedObservers
  {
    //^linked-observer
    class Observer
    {
      friend class Subject;

    public:
      Observer()
      : next_(NULL)
      {}

      // Other stuff...
      //^omit
      void onNotify(const Entity& entity, Event event) {}
      //^omit
    private:
      Observer* next_;
    };
    //^linked-observer

    //^linked-subject
    class Subject
    {
      // Methods...
      //^omit
      void addObserver(Observer* observer);
      void removeObserver(Observer* observer);
      void notify(const Entity& entity, Event event);
      //^omit
    private:
      Observer* head_;
    };
    //^linked-subject

    //^linked-add
    void Subject::addObserver(Observer* observer)
    {
      observer->next_ = head_;
      head_ = observer;
    }
    //^linked-add

    //^linked-remove
    void Subject::removeObserver(Observer* observer)
    {
      if (head_ == observer)
      {
        head_ = observer->next_;
        observer->next_ = NULL;
        return;
      }

      Observer* current = head_;
      while (current != NULL)
      {
        if (current->next_ == observer)
        {
          current->next_ = observer->next_;
          observer->next_ = NULL;
          return;
        }

        current = current->next_;
      }
    }
    //^linked-remove

    //^linked-notify
    void Subject::notify(const Entity& entity, Event event)
    {
      Observer* observer = head_;
      while (observer != NULL)
      {
        observer->onNotify(entity, event);
        observer = observer->next_;
      }
    }
    //^linked-notify
  }

  namespace One
  {
    class Observable;

    class Observer
    {
      friend class Observable;

    public:
      bool isObserving() const { return observable_ != NULL; }

      void observe(Observable& observable);
      void detach();

    protected:
      Observer()
      : prev_(this),
        next_(this)
      {}

      virtual ~Observer()
      {
        detach();
      }

      virtual void onNotify(Observable& observable) = 0;

    private:
      // The Observable this Observer is watching.
      Observable* observable_ = NULL;

      // The next and previous nodes in the circular linked list
      // observers of observable_.
      Observer* prev_;
      Observer* next_;
    };

    class Observable
    {
      friend class Observer;

    public:
      bool hasObserver() const { return observer_ != NULL; }

    protected:
      Observable()
      : observer_(NULL)
      {}

      virtual ~Observable()
      {
        // Detach all of the observers.
        while (observer_ != NULL) observer_->detach();
      }

      void notify()
      {
        if (observer_ == NULL) return;

        Observer* observer = observer_;
        do
        {
          observer->onNotify(*this);
          observer = observer->next_;
        }
        while (observer != observer_);
      }

      // The first in the linked list of observers of this.
      Observer* observer_;
    };

    void Observer::observe(Observable& observable)
    {
      // Stop observing what it was previously observing.
      detach();

      if (observable.observer_ == NULL)
      {
        // The first observer.
        observable.observer_ = this;
      }
      else
      {
        // Already have other observers, so link it in at the end of the
        // list.
        prev_ = observable.observer_->prev_;
        next_ = observable.observer_;

        observable.observer_->prev_->next_ = this;
        observable.observer_->prev_ = this;
      }

      observable_ = &observable;
    }

    void Observer::detach()
    {
      if (observable_ == NULL) return;

      // Make sure the observable itself isn't pointing at this node.
      if (observable_->observer_ == this)
      {
        if (next_ == this)
        {
          // This is the only observer, so just clear it.
          observable_->observer_ = NULL;
        }
        else
        {
          // Advance the next node.
          observable_->observer_ = next_;
        }
      }

      // Unlink this observer from the list.
      prev_->next_ = next_;
      next_->prev_ = prev_;

      prev_ = this;
      next_ = this;
      observable_ = NULL;
    }

    class Noise : public Observable
    {
    public:
      Noise(const char* name)
      : name_(name)
      {}

      void sound()
      {
        cout << name_ << "!" << endl;
        notify();
      }

    private:
      const char* name_;
    };
    
    class Ear : public Observer
    {
    public:
      Ear(const char* name)
      : name_(name)
      {}

      int numObserved = 0;

    protected:
      void onNotify(Observable & observable)
      {
        numObserved++;
        cout << name_ << " heard it!" << endl;
      }

    private:
      const char* name_;
    };

    void destructSoloObserverTest()
    {
      Ear* ear = new Ear("ear");
      Noise noise("beep");
      ear->observe(noise);

      delete ear;
      ASSERT(!noise.hasObserver());

      noise.sound();
    }

    void destructMultipleObserverTest()
    {
      Ear* ear1 = new Ear("ear1");
      Ear* ear2 = new Ear("ear2");
      Noise noise("beep");
      ear1->observe(noise);
      ear2->observe(noise);

      delete ear2;
      ASSERT(noise.hasObserver());

      delete ear1;
      ASSERT(!noise.hasObserver());

      noise.sound();
    }

    void destructObservableTest()
    {
      Ear ear1("ear1");
      Ear ear2("ear2");
      Noise* noise = new Noise("beep");
      ear1.observe(*noise);
      ear2.observe(*noise);

      delete noise;
      ASSERT(!ear1.isObserving());
      ASSERT(!ear2.isObserving());
    }

    void notifyTest()
    {
      Noise noise1("beep");
      Ear ear1("one");
      Ear ear2("two");
      Ear ear3("three");

      noise1.sound();
      ASSERT(ear1.numObserved == 0);
      ASSERT(ear2.numObserved == 0);
      ASSERT(ear3.numObserved == 0);

      ear1.observe(noise1);
      noise1.sound();
      ASSERT(ear1.numObserved == 1);
      ASSERT(ear2.numObserved == 0);
      ASSERT(ear3.numObserved == 0);

      ear2.observe(noise1);
      noise1.sound();
      ASSERT(ear1.numObserved == 2);
      ASSERT(ear2.numObserved == 1);
      ASSERT(ear3.numObserved == 0);

      ear3.observe(noise1);
      noise1.sound();
      ASSERT(ear1.numObserved == 3);
      ASSERT(ear2.numObserved == 2);
      ASSERT(ear3.numObserved == 1);

      ear2.detach();
      noise1.sound();
      ASSERT(ear1.numObserved == 4);
      ASSERT(ear2.numObserved == 2);
      ASSERT(ear3.numObserved == 2);

      ear1.detach();
      noise1.sound();
      ASSERT(ear1.numObserved == 4);
      ASSERT(ear2.numObserved == 2);
      ASSERT(ear3.numObserved == 3);

      ear3.detach();
      noise1.sound();
      ASSERT(ear1.numObserved == 4);
      ASSERT(ear2.numObserved == 2);
      ASSERT(ear3.numObserved == 3);
    }

    void observeTest()
    {
      Ear ear("ear");
      Noise beep("beep");
      Noise boop("boop");

      ear.observe(beep);
      beep.sound();
      ASSERT(ear.numObserved == 1);
      boop.sound();
      ASSERT(ear.numObserved == 1);

      // Should stop listening to beep.
      ear.observe(boop);
      beep.sound();
      ASSERT(ear.numObserved == 1);
      boop.sound();
      ASSERT(ear.numObserved == 2);
    }

    void test()
    {
      destructSoloObserverTest();
      destructMultipleObserverTest();
      destructObservableTest();
      notifyTest();
      observeTest();
    }
  }

  namespace Pool
  {
    class Binding;
    class BindingPool;
    class Listener;

    class Event
    {
    public:
      Event(BindingPool& pool)
      : pool_(pool),
        binding_(NULL)
      {}

      void addListener(Listener& listener);
      void removeListener(Listener& listener);

      void send(const char* data);

    private:
      BindingPool& pool_;
      Binding* binding_;
    };

    class Listener
    {
    public:
      Listener(const char* name)
      : name_(name)
      {}

      int numEvents() const;

      void receive(const char* data)
      {
        cout << name_ << " received " << data << endl;
      }

    private:
      const char* name_;
    };

    class Binding
    {
      friend class BindingPool;
      friend class Event;

    public:
      Binding()
      : listener_(NULL),
        next_(NULL)
      {}

    private:
      Listener* listener_;

      // If the binding is in use, this will point to the binding for the
      // next listener. Otherwise, it will point to the next free binding.
      Binding* next_;
    };

    class BindingPool
    {
    public:
      BindingPool()
      {
        // Build the free list.
        free_ = &bindings_[0];
        for (int i = 0; i < POOL_SIZE - 1; i++)
        {
          bindings_[i].next_ = &bindings_[i + 1];
        }
      }

      Binding* newBinding()
      {
        // TODO(bob): Make sure there is a free one.
        // Remove the head of the free list.
        Binding* binding = free_;
        free_ = free_->next_;
        return binding;
      }

    private:
      static const int POOL_SIZE = 100;

      Binding bindings_[POOL_SIZE];

      // Pointer to the first binding in the list of free bindings.
      Binding* free_;
    };


    void Event::addListener(Listener& listener)
    {
      // TODO(bob): Add to end of list.
      Binding* binding = pool_.newBinding();
      binding->listener_ = &listener;
      binding->next_ = binding_;
      binding_ = binding;
    }

    void Event::removeListener(Listener& listener)
    {
      // TODO(bob): Implement me!
    }

    void Event::send(const char* data)
    {
      Binding* binding = binding_;
      while (binding != NULL)
      {
        binding->listener_->receive(data);
        binding = binding->next_;
      }
    }

    int Listener::numEvents() const
    {
      return 0; // ?
    }

    // TODO(bob): Destructors for all of these types.

    void destructEventTest()
    {
      BindingPool pool;

      Event* event1 = new Event(pool);
      Event* event2 = new Event(pool);

      Listener listener1("listener 1");
      Listener listener2("listener 2");

      event1->addListener(listener1);
      event1->addListener(listener2);
      event2->addListener(listener1);
      event2->addListener(listener2);

      ASSERT(listener1.numEvents() == 2);
      ASSERT(listener2.numEvents() == 2);

      delete event1;

      ASSERT(listener1.numEvents() == 1);
      ASSERT(listener2.numEvents() == 1);

      delete event2;

      ASSERT(listener1.numEvents() == 0);
      ASSERT(listener2.numEvents() == 0);
    }

    void test()
    {
      destructEventTest();

      BindingPool pool;

      Event event1(pool);
      Event event2(pool);

      Listener listener1("listener 1");
      Listener listener2("listener 2");
      Listener listener3("listener 3");

      event1.addListener(listener1);
      event1.addListener(listener2);
      event2.addListener(listener2);
      event2.addListener(listener3);

      event1.send("first");
      event2.send("second");
    }
  }

  void test()
  {
    //One::test();
    Pool::test();
  }
}

#endif
