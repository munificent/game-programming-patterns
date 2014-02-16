#ifndef cpp_event_queue_h
#define cpp_event_queue_h

namespace EventQueue
{
  typedef int ResourceId;
  typedef int SoundId;

  static const int SOUND_BLOOP = 1;
  static const int VOL_MAX = 1;

  ResourceId loadSound(SoundId id) { return 0; }
  int findOpenChannel() { return -1; }
  void startSound(ResourceId resource, int channel, int volume) {}

  namespace EventLoop
  {
    typedef int Event;
    Event getNextEvent() { return 0; }

    void eventLoop()
    {
      bool running = true;
      //^event-loop
      while (running)
      {
        Event event = getNextEvent();
        // Handle event...
        //^omit
        use(event);
        //^omit
      }
      //^event-loop
    }
  }

  namespace Unqueued
  {
    //^sync-api
    class Audio
    {
    public:
      static void playSound(SoundId id, int volume);
    };
    //^sync-api

    //^sync-impl
    void Audio::playSound(SoundId id, int volume)
    {
      ResourceId resource = loadSound(id);
      int channel = findOpenChannel();
      if (channel == -1) return;
      startSound(resource, channel, volume);
    }
    //^sync-impl

    class Menu
    {
    public:
      void onSelect(int index);
    };

    //^menu-bloop
    void Menu::onSelect(int index)
    {
      Audio::playSound(SOUND_BLOOP, VOL_MAX);
      // Other stuff...
    }
    //^menu-bloop
  }

  //^play-message
  struct PlayMessage
  {
    SoundId id;
    int volume;
  };
  //^play-message

  namespace DeferList
  {
    //^pending-array
    class Audio
    {
    public:
      static void init()
      {
        numPending_ = 0;
      }

      //^omit
      static void playSound(SoundId id, int volume);
      static void update();
      //^omit
    private:
      static const int MAX_PENDING = 16;

      static PlayMessage pending_[MAX_PENDING];
      static int numPending_;
    };
    //^pending-array

    //^array-play
    void Audio::playSound(SoundId id, int volume)
    {
      assert(numPending_ < MAX_PENDING);

      pending_[numPending_].id = id;
      pending_[numPending_].volume = volume;
      numPending_++;
    }
    //^array-play

    //^array-update
    void Audio::update()
    {
      for (int i = 0; i < numPending_; i++)
      {
        ResourceId resource = loadSound(pending_[i].id);
        int channel = findOpenChannel();
        if (channel == -1) return;
        startSound(resource, channel, pending_[i].volume);
      }

      numPending_ = 0;
    }
    //^array-update

    PlayMessage Audio::pending_[MAX_PENDING];
    int Audio::numPending_;
  }

  namespace HeadTail
  {
    //^head-tail
    class Audio
    {
    public:
      static void init()
      {
        head_ = 0;
        tail_ = 0;
      }

      // Methods...
      //^omit
      static void playSound(SoundId id, int volume);
      static void update();
      //^omit
    private:
      //^omit
      static const int MAX_PENDING = 16;

      static PlayMessage pending_[MAX_PENDING];
      //^omit
      static int head_;
      static int tail_;

      // Array...
    };
    //^head-tail

    //^tail-play
    void Audio::playSound(SoundId id, int volume)
    {
      assert(tail_ < MAX_PENDING);

      // Add to the end of the list.
      pending_[tail_].id = id;
      pending_[tail_].volume = volume;
      tail_++;
    }
    //^tail-play

    //^tail-update
    void Audio::update()
    {
      // If there are no pending requests, do nothing.
      if (head_ == tail_) return;

      ResourceId resource = loadSound(pending_[head_].id);
      int channel = findOpenChannel();
      if (channel == -1) return;
      startSound(resource, channel, pending_[head_].volume);

      head_++;
    }
    //^tail-update

    PlayMessage Audio::pending_[MAX_PENDING];
    int Audio::tail_;
    int Audio::head_;
  }

  namespace Ring
  {
    class Audio
    {
    public:
      static void init()
      {
        head_ = 0;
        tail_ = 0;
      }

      static void playSound(SoundId id, int volume);
      static void update();
    private:
      static const int MAX_PENDING = 16;

      static PlayMessage pending_[MAX_PENDING];
      static int head_;
      static int tail_;
    };

    //^ring-play
    void Audio::playSound(SoundId id, int volume)
    {
      assert((tail_ + 1) % MAX_PENDING != head_);

      // Add to the end of the list.
      pending_[tail_].id = id;
      pending_[tail_].volume = volume;
      tail_ = (tail_ + 1) % MAX_PENDING;
    }
    //^ring-play

    //^ring-update
    void Audio::update()
    {
      // If there are no pending requests, do nothing.
      if (head_ == tail_) return;

      ResourceId resource = loadSound(pending_[head_].id);
      int channel = findOpenChannel();
      if (channel == -1) return;
      startSound(resource, channel, pending_[head_].volume);

      head_ = (head_ + 1) % MAX_PENDING;
    }
    //^ring-update

    PlayMessage Audio::pending_[MAX_PENDING];
    int Audio::tail_;
    int Audio::head_;
  }

  namespace Duplicate
  {
    class Audio
    {
    public:
      static void init()
      {
        head_ = 0;
        tail_ = 0;
      }

      static void playSound(SoundId id, int volume);
      static void update();
    private:
      static const int MAX_PENDING = 16;

      static PlayMessage pending_[MAX_PENDING];
      static int head_;
      static int tail_;
    };

    int max(int a, int b) { return a; }

    //^drop-dupe-play
    void Audio::playSound(SoundId id, int volume)
    {
      // Walk the pending requests.
      for (int i = head_; i != tail_;
           i = (i + 1) % MAX_PENDING)
      {
        if (pending_[i].id == id)
        {
          // Use the larger of the two volumes.
          pending_[i].volume = max(volume, pending_[i].volume);

          // Don't need to enqueue.
          return;
        }
      }

      // Previous code...
    }
    //^drop-dupe-play

    //^ring-update
    void Audio::update()
    {
      // If there are no pending requests, do nothing.
      if (head_ == tail_) return;

      ResourceId resource = loadSound(pending_[head_].id);
      int channel = findOpenChannel();
      if (channel == -1) return;
      startSound(resource, channel, pending_[head_].volume);

      head_ = (head_ + 1) % MAX_PENDING;
    }
    //^ring-update

    PlayMessage Audio::pending_[MAX_PENDING];
    int Audio::tail_;
    int Audio::head_;
  }

  namespace Queued
  {
    //  head (0)
    //  |     tail (3)
    //  v     v
    // +-+-+-+-+-+-+-+-+-+
    // |A|B|C| | | | | | |
    // +-+-+-+-+-+-+-+-+-+

    class Audio
    {
    public:
      Audio()
      : head_(0),
        numMessages_(0)
      {}

      void playSound(SoundId id, int volume)
      {
        assert(numMessages_ < MAX_MESSAGES);

        queue_[head_].id = id;
        queue_[head_].volume = volume;

        head_ = (head_ + 1) % MAX_MESSAGES;
        numMessages_++;
      }

      void update()
      {
        
      }

    private:
      class SoundMessage
      {
      public:
        SoundId id;
        int volume;
      };
      
      static const int MAX_MESSAGES = 16;

      SoundMessage queue_[MAX_MESSAGES];
      int head_;
      int numMessages_;
    };
  }
}
#endif
