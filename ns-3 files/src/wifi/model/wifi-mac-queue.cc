/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2005, 2009 INRIA
 * Copyright (c) 2009 MIRKO BANCHI
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Authors: Mathieu Lacage <mathieu.lacage@sophia.inria.fr>
 *          Mirko Banchi <mk.banchi@gmail.com>
 *          Stefano Avallone <stavallo@unina.it>
 */

#include "ns3/simulator.h"
#include "ns3/uinteger.h"
#include "ns3/enum.h"
#include "ns3/assert.h"
#include "wifi-mac-queue.h"
#include "qos-blocked-destinations.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("WifiMacQueue");

WifiMacQueueItem::WifiMacQueueItem (Ptr<const Packet> p, const WifiMacHeader & header)
  : m_packet (p),
    m_header (header),
    m_tstamp (Simulator::Now ())
{
}

WifiMacQueueItem::~WifiMacQueueItem ()
{
}

Ptr<const Packet>
WifiMacQueueItem::GetPacket (void) const
{
  return m_packet;
}

const WifiMacHeader&
WifiMacQueueItem::GetHeader (void) const
{
  return m_header;
}

Mac48Address
WifiMacQueueItem::GetAddress (enum WifiMacHeader::AddressType type) const
{
  if (type == WifiMacHeader::ADDR1)
    {
      return m_header.GetAddr1 ();
    }
  if (type == WifiMacHeader::ADDR2)
    {
      return m_header.GetAddr2 ();
    }
  if (type == WifiMacHeader::ADDR3)
    {
      return m_header.GetAddr3 ();
    }
  return 0;
}

void
WifiMacQueueItem::SetAddress (enum WifiMacHeader::AddressType type, Mac48Address address)
{
  if (type == WifiMacHeader::ADDR1)
    {
      m_header.SetAddr1 (address);
    }
  if (type == WifiMacHeader::ADDR2)
    {
      m_header.SetAddr2 (address);
    }
  if (type == WifiMacHeader::ADDR3)
    {
      m_header.SetAddr3 (address);
    }
}

Time
WifiMacQueueItem::GetTimeStamp (void) const
{
  return m_tstamp;
}

uint32_t
WifiMacQueueItem::GetSize (void) const
{
  return m_packet->GetSize () + m_header.GetSerializedSize ();
}


NS_OBJECT_TEMPLATE_CLASS_DEFINE (Queue,WifiMacQueueItem);

template<>
TypeId
WifiMacQueue::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::WifiMacQueue")
    .SetParent<Queue<WifiMacQueueItem> > ()
    .SetGroupName ("Wifi")
    .AddConstructor<WifiMacQueue> ()
    .AddAttribute ("MaxDelay", "If a packet stays longer than this delay in the queue, it is dropped.",
                   TimeValue (MilliSeconds (5000.0)),
                   MakeTimeAccessor (&WifiMacQueue::m_maxDelay),
                   MakeTimeChecker ())
    .AddAttribute ("DropPolicy", "Upon enqueue with full queue, drop oldest (DropOldest) or newest (DropNewest) packet",
                   EnumValue (DROP_NEWEST),
                   MakeEnumAccessor (&WifiMacQueue::m_dropPolicy),
                   MakeEnumChecker (WifiMacQueue::DROP_OLDEST, "DropOldest",
                                    WifiMacQueue::DROP_NEWEST, "DropNewest"))
  ;
  return tid;
}

template<>
WifiMacQueue::WifiQueue ()
{
}

template<>
WifiMacQueue::~WifiQueue ()
{
  NS_LOG_FUNCTION_NOARGS ();
}

template<>
void
WifiMacQueue::SetMaxDelay (Time delay)
{
  NS_LOG_FUNCTION (this << delay);

  m_maxDelay = delay;
}

template<>
Time
WifiMacQueue::GetMaxDelay (void) const
{
  NS_LOG_FUNCTION (this);

  return m_maxDelay;
}

template<>
bool
WifiMacQueue::TtlExceeded (ConstIterator &it)
{
  //NS_LOG_FUNCTION (this);

  if (Simulator::Now () > (*it)->GetTimeStamp () + m_maxDelay)
    {
      NS_LOG_DEBUG ("Removing packet that stayed in the queue for too long (" <<
                    Simulator::Now () - (*it)->GetTimeStamp () << ")");
      auto curr = it++;
      DoRemove (curr);
      return true;
    }
  return false;
}

template<>
bool
WifiMacQueue::Enqueue (Ptr<WifiMacQueueItem> item)
{
  NS_LOG_FUNCTION (this << item << item->GetPacket ());

  NS_ASSERT_MSG (GetMode () == QueueBase::QUEUE_MODE_PACKETS, "WifiMacQueues must be in packet mode");

  // if the queue is full, check if the time-to-live of the oldest packet has
  // expired. If so, it can be removed so as to make room for the new packet.
  if (GetNPackets () == GetMaxPackets ())
    {
      NS_LOG_DEBUG ("Queue is full");
      auto it = Head ();
      TtlExceeded (it);
    }

  if (GetNPackets () == GetMaxPackets () && m_dropPolicy == DROP_OLDEST)
    {
      NS_LOG_DEBUG ("Remove the oldest item in the queue");
      DoRemove (Head ());
    }

  return DoEnqueue (Tail (), item);
}

template<>
bool
WifiMacQueue::PushFront (Ptr<WifiMacQueueItem> item)
{
  NS_LOG_FUNCTION (this << item);

  NS_ASSERT_MSG (GetMode () == QueueBase::QUEUE_MODE_PACKETS, "WifiMacQueues must be in packet mode");

  // if the queue is full, check if the time-to-live of the oldest packet has
  // expired. If so, it can be removed so as to make room for the new packet.
  if (GetNPackets () == GetMaxPackets ())
    {
      auto it = Head ();
      TtlExceeded (it);
    }

  if (GetNPackets () == GetMaxPackets () && m_dropPolicy == DROP_OLDEST)
    {
      NS_LOG_DEBUG ("Remove the oldest item in the queue");
      DoRemove (Head ());
    }

  return DoEnqueue (Head (), item);
}

template<>
Ptr<WifiMacQueueItem>
WifiMacQueue::Dequeue (void)
{
  NS_LOG_FUNCTION (this);

  for (auto it = Head (); it != Tail (); )
    {
      if (!TtlExceeded (it))
        {
          return DoDequeue (it);
        }
    }
  NS_LOG_DEBUG ("The queue is empty");
  return 0;
}

template<>
Ptr<WifiMacQueueItem>
WifiMacQueue::DequeueByTidAndAddress (uint8_t tid,
                                      WifiMacHeader::AddressType type, Mac48Address dest)
{
  NS_LOG_FUNCTION (this << dest);

  for (auto it = Head (); it != Tail (); )
    {
      if (!TtlExceeded (it))
        {
          if ((*it)->GetHeader ().IsQosData () && (*it)->GetAddress (type) == dest &&
              (*it)->GetHeader ().GetQosTid () == tid)
            {
              return DoDequeue (it);
            }

          it++;
        }
    }
  NS_LOG_DEBUG ("The queue is empty");
  return 0;
}

template<>
Ptr<WifiMacQueueItem>
WifiMacQueue::DequeueByAddress (WifiMacHeader::AddressType type, Mac48Address dest, const QosBlockedDestinations *blockedPackets)
{
  NS_LOG_FUNCTION (this << dest);

  for (auto it = Head (); it != Tail (); )
    {
      if (!TtlExceeded (it))
        {
          if ((*it)->GetHeader ().IsQosData () && ((*it)->GetAddress (type) == dest) &&
              !blockedPackets->IsBlocked ((*it)->GetHeader ().GetAddr1 (), (*it)->GetHeader ().GetQosTid ()))
            {
              return DoDequeue (it);
            }

          it++;
        }
    }
  NS_LOG_DEBUG ("The queue is empty");
  return 0;
}

template<>
Ptr<WifiMacQueueItem>
WifiMacQueue::DequeueFirstAvailable (const QosBlockedDestinations *blockedPackets)
{
  NS_LOG_FUNCTION (this);

  for (auto it = Head (); it != Tail (); )
    {
      if (!TtlExceeded (it))
        {
          if (!(*it)->GetHeader ().IsQosData ()
              || !blockedPackets->IsBlocked ((*it)->GetHeader ().GetAddr1 (), (*it)->GetHeader ().GetQosTid ()))
            {
              return DoDequeue (it);
            }

          it++;
        }
    }
  NS_LOG_DEBUG ("The queue is empty");
  return 0;
}

template<>
Ptr<const WifiMacQueueItem>
WifiMacQueue::Peek (void) const
{
  NS_LOG_FUNCTION (this);
  return DoPeek (Head ());
}

template<>
Ptr<const WifiMacQueueItem>
WifiMacQueue::PeekByTidAndAddress (uint8_t tid,
                                   WifiMacHeader::AddressType type, Mac48Address dest)
{
  NS_LOG_FUNCTION (this << dest);

  for (auto it = Head (); it != Tail (); )
    {
      if (!TtlExceeded (it))
        {
          if ((*it)->GetHeader ().IsQosData () && (*it)->GetAddress (type) == dest &&
              (*it)->GetHeader ().GetQosTid () == tid)
            {
              return DoPeek (it);
            }

          it++;
        }
    }
  NS_LOG_DEBUG ("The queue is empty");
  return 0;
}

template<>
Ptr<const WifiMacQueueItem>
WifiMacQueue::PeekFirstAvailable (const QosBlockedDestinations *blockedPackets)
{
  NS_LOG_FUNCTION (this);

  for (auto it = Head (); it != Tail (); )
    {
      if (!TtlExceeded (it))
        {
          if (!(*it)->GetHeader ().IsQosData ()
              || !blockedPackets->IsBlocked ((*it)->GetHeader ().GetAddr1 (), (*it)->GetHeader ().GetQosTid ()))
            {
              return DoPeek (it);
            }

          it++;
        }
    }
  NS_LOG_DEBUG ("The queue is empty");
  return 0;
}

template<>
Ptr<const WifiMacQueueItem>
WifiMacQueue::PeekFirstAvailableByAddress (WifiMacHeader::AddressType type,
                                           Mac48Address dest, const QosBlockedDestinations *blockedPackets)
{
  NS_LOG_FUNCTION (this);

  for (auto it = Head (); it != Tail (); )
    {
      if (!TtlExceeded (it))
        {
          if ((*it)->GetHeader ().IsQosData () && ((*it)->GetAddress (type) == dest) &&
              !blockedPackets->IsBlocked ((*it)->GetHeader ().GetAddr1 (), (*it)->GetHeader ().GetQosTid ()))
            {
              return DoPeek (it);
            }

          it++;
        }
    }
  NS_LOG_DEBUG ("The queue is empty");
  return 0;
}

template<>
Ptr<WifiMacQueueItem>
WifiMacQueue::Remove (void)
{
  NS_LOG_FUNCTION (this);

  for (auto it = Head (); it != Tail (); )
    {
      if (!TtlExceeded (it))
        {
          return DoRemove (it);
        }
    }
  NS_LOG_DEBUG ("The queue is empty");
  return 0;
}

template<>
bool
WifiMacQueue::Remove (Ptr<const Packet> packet)
{
  NS_LOG_FUNCTION (this << packet);

  for (auto it = Head (); it != Tail (); )
    {
      if (!TtlExceeded (it))
        {
          if ((*it)->GetPacket () == packet)
            {
              DoRemove (it);
              return true;
            }

          it++;
        }
    }
  NS_LOG_DEBUG ("Packet " << packet << " not found in the queue");
  return false;
}

template<>
uint32_t
WifiMacQueue::GetNPacketsByTidAndAddress (uint8_t tid, WifiMacHeader::AddressType type,
                                          Mac48Address addr)
{
  NS_LOG_FUNCTION (this << addr);

  uint32_t nPackets = 0;

  for (auto it = Head (); it != Tail (); )
    {
      if (!TtlExceeded (it))
        {
          if ((*it)->GetHeader ().IsQosData () && (*it)->GetAddress (type) == addr &&
              (*it)->GetHeader ().GetQosTid () == tid)
            {
              nPackets++;
            }

          it++;
        }
    }
  NS_LOG_DEBUG ("returns " << nPackets);
  return nPackets;
}

template<>
bool
WifiMacQueue::HasPackets (void)
{
  NS_LOG_FUNCTION (this);

  for (auto it = Head (); it != Tail (); )
    {
      if (!TtlExceeded (it))
        {
          NS_LOG_DEBUG ("returns true");
          return true;
        }
    }
  NS_LOG_DEBUG ("returns false");
  return false;
}

template<>
void
WifiMacQueue::TransferPacketsByAddress (Mac48Address addr, Ptr<WifiQueue<WifiMacQueueItem> > destQueue)
{
  for (auto it = Head (); it != Tail (); )
    {
      if (!TtlExceeded (it))
        {
          if (((*it)->GetHeader ().IsData ()) && ((*it)->GetHeader ().GetAddr1 () == addr))
            {
              /* Copy the item to the new Queue */
              Ptr<WifiMacQueueItem> item = Create<WifiMacQueueItem> ((*it)->GetPacket (), (*it)->GetHeader ());
              destQueue->Enqueue (item);
              it = m_packets.erase (it);
              m_nBytes -= item->GetSize ();
              m_nPackets--;
            }
          else
            {
              it++;
            }
        }
    }
}

template<>
void
WifiMacQueue::QuickTransfer (Ptr<WifiQueue<WifiMacQueueItem> > destQueue)
{
  for (auto it = Head (); it != Tail (); )
    {
      if (!TtlExceeded (it))
        {
          /* Copy the item to the new Queue */
          Ptr<WifiMacQueueItem> item = Create<WifiMacQueueItem> ((*it)->GetPacket (), (*it)->GetHeader ());
          destQueue->Enqueue (item);
          it = m_packets.erase (it);
          m_nBytes -= item->GetSize ();
          m_nPackets--;
        }
    }
}

template<>
bool
WifiMacQueue::HasPacketsForReceiver (Mac48Address addr)
{
  for (auto it = Head (); it != Tail (); )
    {
      if (!TtlExceeded (it))
        {
          if ((*it)->GetHeader ().GetAddr1 () == addr)
            {
              return true;
            }
          it++;
        }
    }
  return false;
}

template<>
void
WifiMacQueue::ChangePacketsReceiverAddress (Mac48Address OriginalAddress, Mac48Address newAddress)
{
  for (auto it = Head (); it != Tail (); )
    {
      if (!TtlExceeded (it))
        {
          if (((*it)->GetHeader ().IsData ()) && ((*it)->GetHeader ().GetAddr1 () == OriginalAddress))
            {
              (*it)->SetAddress (WifiMacHeader::ADDR1, newAddress);
            }
          it++;
        }
    }
}

NS_OBJECT_TEMPLATE_CLASS_DEFINE (WifiQueue,WifiMacQueueItem);

} //namespace ns3
