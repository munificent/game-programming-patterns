//
//  event-queue.h
//  cpp
//
//  Created by Bob Nystrom on 1/16/14.
//  Copyright (c) 2014 Bob Nystrom. All rights reserved.
//

#ifndef cpp_event_queue_h
#define cpp_event_queue_h

namespace EventQueue
{
  typedef int ResourceId;
  typedef int SoundId;

  ResourceId loadSound(SoundId id) { return 0; }
  int findOpenChannel() { return -1; }
  void startSound(ResourceId resource, int channel, int volume) {}

  namespace Unqueued
  {
    class Audio
    {
    public:
      void playSound(SoundId id, int volume)
      {
        ResourceId resource = loadSound(id);
        int channel = findOpenChannel();
        if (channel == -1) return;
        startSound(resource, channel, volume);
      }

    };
    // Problems:
    // Loading resource could be slow.
    // Not thread safe.
    // Fails if no open channel.
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
