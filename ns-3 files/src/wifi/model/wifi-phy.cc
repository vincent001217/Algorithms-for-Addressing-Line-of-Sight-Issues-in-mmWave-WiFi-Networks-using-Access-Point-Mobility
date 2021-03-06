/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2005,2006 INRIA
 * Copyright (c) 2015,2016 IMDEA Networks Institute
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
 *          Sébastien Deronne <sebastien.deronne@gmail.com>
 *          Hany Assasa <hany.assasa@gmail.com>
 */

#include "wifi-phy.h"
#include "wifi-phy-state-helper.h"
#include "ns3/simulator.h"
#include "ns3/log.h"
#include "ns3/boolean.h"
#include "ns3/double.h"
#include "ns3/uinteger.h"
#include "ns3/pointer.h"
#include "wifi-phy-tag.h"
#include "ampdu-tag.h"
#include "wifi-utils.h"
#include "wifi-mac.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("WifiPhy");

/****************************************************************
 *       This destructor is needed.
 ****************************************************************/

WifiPhyListener::~WifiPhyListener ()
{
}

/****************************************************************
 *       The actual WifiPhy class
 ****************************************************************/

NS_OBJECT_ENSURE_REGISTERED (WifiPhy);

/**
 * This table maintains the mapping of valid ChannelNumber to
 * Frequency/ChannelWidth pairs.  If you want to make a channel applicable
 * to all standards, then you may use the WIFI_PHY_STANDARD_UNSPECIFIED
 * standard to represent this, as a wildcard.  If you want to limit the
 * configuration of a particular channel/frequency/width to a particular
 * standard(s), then you can specify one or more such bindings.
 */
WifiPhy::ChannelToFrequencyWidthMap WifiPhy::m_channelToFrequencyWidth =
{
  // 802.11b uses width of 22, while OFDM modes use width of 20
  { std::make_pair (1, WIFI_PHY_STANDARD_80211b), std::make_pair (2412, 22) },
  { std::make_pair (1, WIFI_PHY_STANDARD_UNSPECIFIED), std::make_pair (2412, 20) },
  { std::make_pair (2, WIFI_PHY_STANDARD_80211b), std::make_pair (2417, 22) },
  { std::make_pair (2, WIFI_PHY_STANDARD_UNSPECIFIED), std::make_pair (2417, 20) },
  { std::make_pair (3, WIFI_PHY_STANDARD_80211b), std::make_pair (2422, 22) },
  { std::make_pair (3, WIFI_PHY_STANDARD_UNSPECIFIED), std::make_pair (2422, 20) },
  { std::make_pair (4, WIFI_PHY_STANDARD_80211b), std::make_pair (2427, 22) },
  { std::make_pair (4, WIFI_PHY_STANDARD_UNSPECIFIED), std::make_pair (2427, 20) },
  { std::make_pair (5, WIFI_PHY_STANDARD_80211b), std::make_pair (2432, 22) },
  { std::make_pair (5, WIFI_PHY_STANDARD_UNSPECIFIED), std::make_pair (2432, 20) },
  { std::make_pair (6, WIFI_PHY_STANDARD_80211b), std::make_pair (2437, 22) },
  { std::make_pair (6, WIFI_PHY_STANDARD_UNSPECIFIED), std::make_pair (2437, 20) },
  { std::make_pair (7, WIFI_PHY_STANDARD_80211b), std::make_pair (2442, 22) },
  { std::make_pair (7, WIFI_PHY_STANDARD_UNSPECIFIED), std::make_pair (2442, 20) },
  { std::make_pair (8, WIFI_PHY_STANDARD_80211b), std::make_pair (2447, 22) },
  { std::make_pair (8, WIFI_PHY_STANDARD_UNSPECIFIED), std::make_pair (2447, 20) },
  { std::make_pair (9, WIFI_PHY_STANDARD_80211b), std::make_pair (2452, 22) },
  { std::make_pair (9, WIFI_PHY_STANDARD_UNSPECIFIED), std::make_pair (2452, 20) },
  { std::make_pair (10, WIFI_PHY_STANDARD_80211b), std::make_pair (2457, 22) },
  { std::make_pair (10, WIFI_PHY_STANDARD_UNSPECIFIED), std::make_pair (2457, 20) },
  { std::make_pair (11, WIFI_PHY_STANDARD_80211b), std::make_pair (2462, 22) },
  { std::make_pair (11, WIFI_PHY_STANDARD_UNSPECIFIED), std::make_pair (2462, 20) },
  { std::make_pair (12, WIFI_PHY_STANDARD_80211b), std::make_pair (2467, 22) },
  { std::make_pair (12, WIFI_PHY_STANDARD_UNSPECIFIED), std::make_pair (2467, 20) },
  { std::make_pair (13, WIFI_PHY_STANDARD_80211b), std::make_pair (2472, 22) },
  { std::make_pair (13, WIFI_PHY_STANDARD_UNSPECIFIED), std::make_pair (2472, 20) },
  // Only defined for 802.11b
  { std::make_pair (14, WIFI_PHY_STANDARD_80211b), std::make_pair (2484, 22) },

  // Now the 5GHz channels; UNSPECIFIED for 802.11a/n/ac/ax channels
  // 20 MHz channels
  { std::make_pair (36, WIFI_PHY_STANDARD_UNSPECIFIED), std::make_pair (5180, 20) },
  { std::make_pair (40, WIFI_PHY_STANDARD_UNSPECIFIED), std::make_pair (5200, 20) },
  { std::make_pair (44, WIFI_PHY_STANDARD_UNSPECIFIED), std::make_pair (5220, 20) },
  { std::make_pair (48, WIFI_PHY_STANDARD_UNSPECIFIED), std::make_pair (5240, 20) },
  { std::make_pair (52, WIFI_PHY_STANDARD_UNSPECIFIED), std::make_pair (5260, 20) },
  { std::make_pair (56, WIFI_PHY_STANDARD_UNSPECIFIED), std::make_pair (5280, 20) },
  { std::make_pair (60, WIFI_PHY_STANDARD_UNSPECIFIED), std::make_pair (5300, 20) },
  { std::make_pair (64, WIFI_PHY_STANDARD_UNSPECIFIED), std::make_pair (5320, 20) },
  { std::make_pair (100, WIFI_PHY_STANDARD_UNSPECIFIED), std::make_pair (5500, 20) },
  { std::make_pair (104, WIFI_PHY_STANDARD_UNSPECIFIED), std::make_pair (5520, 20) },
  { std::make_pair (108, WIFI_PHY_STANDARD_UNSPECIFIED), std::make_pair (5540, 20) },
  { std::make_pair (112, WIFI_PHY_STANDARD_UNSPECIFIED), std::make_pair (5560, 20) },
  { std::make_pair (116, WIFI_PHY_STANDARD_UNSPECIFIED), std::make_pair (5580, 20) },
  { std::make_pair (120, WIFI_PHY_STANDARD_UNSPECIFIED), std::make_pair (5600, 20) },
  { std::make_pair (124, WIFI_PHY_STANDARD_UNSPECIFIED), std::make_pair (5620, 20) },
  { std::make_pair (128, WIFI_PHY_STANDARD_UNSPECIFIED), std::make_pair (5640, 20) },
  { std::make_pair (132, WIFI_PHY_STANDARD_UNSPECIFIED), std::make_pair (5660, 20) },
  { std::make_pair (136, WIFI_PHY_STANDARD_UNSPECIFIED), std::make_pair (5680, 20) },
  { std::make_pair (140, WIFI_PHY_STANDARD_UNSPECIFIED), std::make_pair (5700, 20) },
  { std::make_pair (144, WIFI_PHY_STANDARD_UNSPECIFIED), std::make_pair (5720, 20) },
  { std::make_pair (149, WIFI_PHY_STANDARD_UNSPECIFIED), std::make_pair (5745, 20) },
  { std::make_pair (153, WIFI_PHY_STANDARD_UNSPECIFIED), std::make_pair (5765, 20) },
  { std::make_pair (157, WIFI_PHY_STANDARD_UNSPECIFIED), std::make_pair (5785, 20) },
  { std::make_pair (161, WIFI_PHY_STANDARD_UNSPECIFIED), std::make_pair (5805, 20) },
  { std::make_pair (165, WIFI_PHY_STANDARD_UNSPECIFIED), std::make_pair (5825, 20) },
  // 40 MHz channels
  { std::make_pair (38, WIFI_PHY_STANDARD_UNSPECIFIED), std::make_pair (5190, 40) },
  { std::make_pair (46, WIFI_PHY_STANDARD_UNSPECIFIED), std::make_pair (5230, 40) },
  { std::make_pair (54, WIFI_PHY_STANDARD_UNSPECIFIED), std::make_pair (5230, 40) },
  { std::make_pair (62, WIFI_PHY_STANDARD_UNSPECIFIED), std::make_pair (5310, 40) },
  { std::make_pair (102, WIFI_PHY_STANDARD_UNSPECIFIED), std::make_pair (5510, 40) },
  { std::make_pair (110, WIFI_PHY_STANDARD_UNSPECIFIED), std::make_pair (5550, 40) },
  { std::make_pair (118, WIFI_PHY_STANDARD_UNSPECIFIED), std::make_pair (5590, 40) },
  { std::make_pair (126, WIFI_PHY_STANDARD_UNSPECIFIED), std::make_pair (5590, 40) },
  { std::make_pair (134, WIFI_PHY_STANDARD_UNSPECIFIED), std::make_pair (5670, 40) },
  { std::make_pair (142, WIFI_PHY_STANDARD_UNSPECIFIED), std::make_pair (5710, 40) },
  { std::make_pair (151, WIFI_PHY_STANDARD_UNSPECIFIED), std::make_pair (5755, 40) },
  { std::make_pair (159, WIFI_PHY_STANDARD_UNSPECIFIED), std::make_pair (5795, 40) },
  // 80 MHz channels
  { std::make_pair (42, WIFI_PHY_STANDARD_UNSPECIFIED), std::make_pair (5210, 80) },
  { std::make_pair (58, WIFI_PHY_STANDARD_UNSPECIFIED), std::make_pair (5290, 80) },
  { std::make_pair (106, WIFI_PHY_STANDARD_UNSPECIFIED), std::make_pair (5530, 80) },
  { std::make_pair (122, WIFI_PHY_STANDARD_UNSPECIFIED), std::make_pair (5610, 80) },
  { std::make_pair (138, WIFI_PHY_STANDARD_UNSPECIFIED), std::make_pair (5690, 80) },
  { std::make_pair (155, WIFI_PHY_STANDARD_UNSPECIFIED), std::make_pair (5775, 80) },
  // 160 MHz channels
  { std::make_pair (50, WIFI_PHY_STANDARD_UNSPECIFIED), std::make_pair (5250, 160) },
  { std::make_pair (114, WIFI_PHY_STANDARD_UNSPECIFIED), std::make_pair (5570, 160) },

  // 802.11p (10 MHz channels at the 5.855-5.925 band
  { std::make_pair (172, WIFI_PHY_STANDARD_80211_10MHZ), std::make_pair (5860, 10) },
  { std::make_pair (174, WIFI_PHY_STANDARD_80211_10MHZ), std::make_pair (5870, 10) },
  { std::make_pair (176, WIFI_PHY_STANDARD_80211_10MHZ), std::make_pair (5880, 10) },
  { std::make_pair (178, WIFI_PHY_STANDARD_80211_10MHZ), std::make_pair (5890, 10) },
  { std::make_pair (180, WIFI_PHY_STANDARD_80211_10MHZ), std::make_pair (5900, 10) },
  { std::make_pair (182, WIFI_PHY_STANDARD_80211_10MHZ), std::make_pair (5910, 10) },
  { std::make_pair (184, WIFI_PHY_STANDARD_80211_10MHZ), std::make_pair (5920, 10) },

  // 802.11ad/WiGig (2.16 GHz channels at the 56.16-64.8) GHz band
  { std::make_pair (1, WIFI_PHY_STANDARD_80211ad), std::make_pair (58320, 2160) },
  { std::make_pair (2, WIFI_PHY_STANDARD_80211ad), std::make_pair (60480, 2160) },
  { std::make_pair (3, WIFI_PHY_STANDARD_80211ad), std::make_pair (62640, 2160) },
  { std::make_pair (4, WIFI_PHY_STANDARD_80211ad), std::make_pair (64800, 2160) }
};

TypeId
WifiPhy::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::WifiPhy")
    .SetParent<Object> ()
    .SetGroupName ("Wifi")
    .AddAttribute ("Frequency",
                   "The operating center frequency (MHz)",
                   UintegerValue (0),
                   MakeUintegerAccessor (&WifiPhy::GetFrequency,
                                         &WifiPhy::SetFrequency),
                   MakeUintegerChecker<uint16_t> ())
    .AddAttribute ("ChannelWidth",
                   "Whether 5MHz, 10MHz, 20MHz, 22MHz, 40MHz, 80 MHz or 160 MHz or 2160.",
                   UintegerValue (20),
                   MakeUintegerAccessor (&WifiPhy::GetChannelWidth,
                                         &WifiPhy::SetChannelWidth),
                   MakeUintegerChecker<uint16_t> (5, 2160))
    .AddAttribute ("ChannelNumber",
                   "If set to non-zero defined value, will control Frequency and ChannelWidth assignment",
                   UintegerValue (0),
                   MakeUintegerAccessor (&WifiPhy::SetChannelNumber,
                                         &WifiPhy::GetChannelNumber),
                   MakeUintegerChecker<uint8_t> (0, 196))
    .AddAttribute ("EnergyDetectionThreshold",
                   "The energy of a received signal should be higher than "
                   "this threshold (dbm) to allow the PHY layer to detect the signal.",
                   DoubleValue (-96.0),
                   MakeDoubleAccessor (&WifiPhy::SetEdThreshold,
                                       &WifiPhy::GetEdThreshold),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("CcaMode1Threshold",
                   "The energy of a received signal should be higher than "
                   "this threshold (dbm) to allow the PHY layer to declare CCA BUSY state.",
                   DoubleValue (-99.0),
                   MakeDoubleAccessor (&WifiPhy::SetCcaMode1Threshold,
                                       &WifiPhy::GetCcaMode1Threshold),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("TxGain",
                   "Transmission gain (dB).",
                   DoubleValue (0.0),
                   MakeDoubleAccessor (&WifiPhy::SetTxGain,
                                       &WifiPhy::GetTxGain),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("RxGain",
                   "Reception gain (dB).",
                   DoubleValue (0.0),
                   MakeDoubleAccessor (&WifiPhy::SetRxGain,
                                       &WifiPhy::GetRxGain),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("TxPowerLevels",
                   "Number of transmission power levels available between "
                   "TxPowerStart and TxPowerEnd included.",
                   UintegerValue (1),
                   MakeUintegerAccessor (&WifiPhy::m_nTxPower),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("TxPowerEnd",
                   "Maximum available transmission level (dbm).",
                   DoubleValue (16.0206),
                   MakeDoubleAccessor (&WifiPhy::SetTxPowerEnd,
                                       &WifiPhy::GetTxPowerEnd),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("TxPowerStart",
                   "Minimum available transmission level (dbm).",
                   DoubleValue (16.0206),
                   MakeDoubleAccessor (&WifiPhy::SetTxPowerStart,
                                       &WifiPhy::GetTxPowerStart),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("RxNoiseFigure",
                   "Loss (dB) in the Signal-to-Noise-Ratio due to non-idealities in the receiver."
                   " According to Wikipedia (http://en.wikipedia.org/wiki/Noise_figure), this is "
                   "\"the difference in decibels (dB) between"
                   " the noise output of the actual receiver to the noise output of an "
                   " ideal receiver with the same overall gain and bandwidth when the receivers "
                   " are connected to sources at the standard noise temperature T0 (usually 290 K)\".",
                   DoubleValue (7),
                   MakeDoubleAccessor (&WifiPhy::SetRxNoiseFigure,
                                       &WifiPhy::GetRxNoiseFigure),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("State",
                   "The state of the PHY layer.",
                   PointerValue (),
                   MakePointerAccessor (&WifiPhy::m_state),
                   MakePointerChecker<WifiPhyStateHelper> ())
    .AddAttribute ("ChannelSwitchDelay",
                   "Delay between two short frames transmitted on different frequencies.",
                   TimeValue (MicroSeconds (250)),
                   MakeTimeAccessor (&WifiPhy::m_channelSwitchDelay),
                   MakeTimeChecker ())
    .AddAttribute ("TxAntennas",
                   "The number of supported Tx antennas.",
                   UintegerValue (1),
                   MakeUintegerAccessor (&WifiPhy::m_numberOfTransmitters),
                   MakeUintegerChecker<uint8_t> (1, 8),
                   TypeId::DEPRECATED,
                   "Not used anymore.")
    .AddAttribute ("RxAntennas",
                   "The number of supported Rx antennas.",
                   UintegerValue (1),
                   MakeUintegerAccessor (&WifiPhy::m_numberOfReceivers),
                   MakeUintegerChecker<uint8_t> (1, 8),
                   TypeId::DEPRECATED,
                   "Not used anymore.")
    .AddAttribute ("Antennas",
                   "The number of antennas on the device.",
                   UintegerValue (1),
                   MakeUintegerAccessor (&WifiPhy::GetNumberOfAntennas,
                                         &WifiPhy::SetNumberOfAntennas),
                   MakeUintegerChecker<uint8_t> (1, 8))
    .AddAttribute ("MaxSupportedTxSpatialStreams",
                   "The maximum number of supported TX spatial streams."
                   "This parameter is only valuable for 802.11n/ac/ax STAs and APs.",
                   UintegerValue (1),
                   MakeUintegerAccessor (&WifiPhy::GetMaxSupportedTxSpatialStreams,
                                         &WifiPhy::SetMaxSupportedTxSpatialStreams),
                   MakeUintegerChecker<uint8_t> (1, 8))
    .AddAttribute ("MaxSupportedRxSpatialStreams",
                   "The maximum number of supported RX spatial streams."
                   "This parameter is only valuable for 802.11n/ac/ax STAs and APs.",
                   UintegerValue (1),
                   MakeUintegerAccessor (&WifiPhy::GetMaxSupportedRxSpatialStreams,
                                         &WifiPhy::SetMaxSupportedRxSpatialStreams),
                   MakeUintegerChecker<uint8_t> (1, 8))
    .AddAttribute ("ShortGuardEnabled",
                   "Whether or not short guard interval is enabled for HT/VHT transmissions."
                   "This parameter is only valuable for 802.11n/ac/ax STAs and APs.",
                   BooleanValue (false),
                   MakeBooleanAccessor (&WifiPhy::GetShortGuardInterval,
                                        &WifiPhy::SetShortGuardInterval),
                   MakeBooleanChecker ())
    .AddAttribute ("GuardInterval",
                   "Whether 800ns, 1600ns or 3200ns guard interval is used for HE transmissions."
                   "This parameter is only valuable for 802.11ax STAs and APs.",
                   TimeValue (NanoSeconds (3200)),
                   MakeTimeAccessor (&WifiPhy::GetGuardInterval,
                                     &WifiPhy::SetGuardInterval),
                   MakeTimeChecker (NanoSeconds (400), NanoSeconds (3200)))
    .AddAttribute ("LdpcEnabled",
                   "Whether or not LDPC is enabled (not supported yet!).",
                   BooleanValue (false),
                   MakeBooleanAccessor (&WifiPhy::GetLdpc,
                                        &WifiPhy::SetLdpc),
                   MakeBooleanChecker ())
    .AddAttribute ("STBCEnabled",
                   "Whether or not STBC is enabled (not supported yet!).",
                   BooleanValue (false),
                   MakeBooleanAccessor (&WifiPhy::GetStbc,
                                        &WifiPhy::SetStbc),
                   MakeBooleanChecker ())
    .AddAttribute ("GreenfieldEnabled",
                   "Whether or not Greenfield is enabled."
                   "This parameter is only valuable for 802.11n STAs and APs.",
                   BooleanValue (false),
                   MakeBooleanAccessor (&WifiPhy::GetGreenfield,
                                        &WifiPhy::SetGreenfield),
                   MakeBooleanChecker ())
    .AddAttribute ("ShortPlcpPreambleSupported",
                   "Whether or not short PLCP preamble is supported."
                   "This parameter is only valuable for 802.11b STAs and APs."
                   "Note: 802.11g APs and STAs always support short PLCP preamble.",
                   BooleanValue (false),
                   MakeBooleanAccessor (&WifiPhy::GetShortPlcpPreambleSupported,
                                        &WifiPhy::SetShortPlcpPreambleSupported),
                   MakeBooleanChecker ())

    /* IEEE 802.11ad Attributes */
    .AddAttribute ("SupportOfdmPhy", "Whether the DMG STA supports OFDM PHY layer.",
                   BooleanValue (true),
                   MakeBooleanAccessor (&WifiPhy::m_supportOFDM),
                   MakeBooleanChecker ())
    .AddAttribute ("SupportLpScPhy", "Whether the DMG STA supports LP-SC PHY layer.",
                   BooleanValue (false),
                   MakeBooleanAccessor (&WifiPhy::m_supportLpSc),
                   MakeBooleanChecker ())

    .AddTraceSource ("PhyTxBegin",
                     "Trace source indicating a packet "
                     "has begun transmitting over the channel medium",
                     MakeTraceSourceAccessor (&WifiPhy::m_phyTxBeginTrace),
                     "ns3::Packet::TracedCallback")
    .AddTraceSource ("PhyTxEnd",
                     "Trace source indicating a packet "
                     "has been completely transmitted over the channel. "
                     "NOTE: the only official WifiPhy implementation "
                     "available to this date never fires "
                     "this trace source.",
                     MakeTraceSourceAccessor (&WifiPhy::m_phyTxEndTrace),
                     "ns3::Packet::TracedCallback")
    .AddTraceSource ("PhyTxDrop",
                     "Trace source indicating a packet "
                     "has been dropped by the device during transmission",
                     MakeTraceSourceAccessor (&WifiPhy::m_phyTxDropTrace),
                     "ns3::Packet::TracedCallback")
    .AddTraceSource ("PhyRxBegin",
                     "Trace source indicating a packet "
                     "has begun being received from the channel medium "
                     "by the device",
                     MakeTraceSourceAccessor (&WifiPhy::m_phyRxBeginTrace),
                     "ns3::Packet::TracedCallback")
    .AddTraceSource ("PhyRxEnd",
                     "Trace source indicating a packet "
                     "has been completely received from the channel medium "
                     "by the device",
                     MakeTraceSourceAccessor (&WifiPhy::m_phyRxEndTrace),
                     "ns3::Packet::TracedCallback")
    .AddTraceSource ("PhyRxDrop",
                     "Trace source indicating a packet "
                     "has been dropped by the device during reception",
                     MakeTraceSourceAccessor (&WifiPhy::m_phyRxDropTrace),
                     "ns3::Packet::TracedCallback")
    .AddTraceSource ("MonitorSnifferRx",
                     "Trace source simulating a wifi device in monitor mode "
                     "sniffing all received frames",
                     MakeTraceSourceAccessor (&WifiPhy::m_phyMonitorSniffRxTrace),
                     "ns3::WifiPhy::MonitorSnifferRxTracedCallback")
    .AddTraceSource ("MonitorSnifferTx",
                     "Trace source simulating the capability of a wifi device "
                     "in monitor mode to sniff all frames being transmitted",
                     MakeTraceSourceAccessor (&WifiPhy::m_phyMonitorSniffTxTrace),
                     "ns3::WifiPhy::MonitorSnifferTxTracedCallback")
  ;
  return tid;
}

WifiPhy::WifiPhy ()
  : m_mpdusNum (0),
    m_plcpSuccess (false),
    m_txMpduReferenceNumber (0xffffffffffffffff),
    m_rxMpduReferenceNumber (0xffffffffffffffff),
    m_endRxEvent (),
    m_endPlcpRxEvent (),
    m_standard (WIFI_PHY_STANDARD_UNSPECIFIED),
    m_isConstructed (false),
    m_channelCenterFrequency (0),
    m_initialFrequency (0),
    m_frequencyChannelNumberInitialized (false),
    m_channelNumber (0),
    m_initialChannelNumber (0),
    m_totalAmpduSize (0),
    m_totalAmpduNumSymbols (0)
{
  NS_LOG_FUNCTION (this);
  NS_UNUSED (m_numberOfTransmitters);
  NS_UNUSED (m_numberOfReceivers);
  m_random = CreateObject<UniformRandomVariable> ();
  m_state = CreateObject<WifiPhyStateHelper> ();
  m_totalAmpduSize = 0;
  m_totalAmpduNumSymbols = 0;
  m_totalBits = 0;
  //802-11ad parameters
  m_antenna = 0;
  m_rdsActivated = false;
  m_lastTxDuration = NanoSeconds (0.0);
}

WifiPhy::~WifiPhy ()
{
  NS_LOG_FUNCTION (this);
}

void
WifiPhy::DoDispose (void)
{
  NS_LOG_FUNCTION (this);
  m_device = 0;
  m_mobility = 0;
  m_state = 0;
  m_deviceRateSet.clear ();
  m_deviceMcsSet.clear ();
}

void
WifiPhy::DoInitialize (void)
{
  NS_LOG_FUNCTION (this);
  m_isConstructed = true;
  if (m_frequencyChannelNumberInitialized == true)
    {
      NS_LOG_DEBUG ("Frequency already initialized");
      return;
    }
  InitializeFrequencyChannelNumber ();
}

void
WifiPhy::SetReceiveOkCallback (RxOkCallback callback)
{
  m_state->SetReceiveOkCallback (callback);
}

void
WifiPhy::SetReceiveErrorCallback (RxErrorCallback callback)
{
  m_state->SetReceiveErrorCallback (callback);
}

void
WifiPhy::RegisterListener (WifiPhyListener *listener)
{
  m_state->RegisterListener (listener);
}

void
WifiPhy::UnregisterListener (WifiPhyListener *listener)
{
  m_state->UnregisterListener (listener);
}

void
WifiPhy::InitializeFrequencyChannelNumber (void)
{
  NS_LOG_FUNCTION (this);

  NS_ASSERT_MSG (m_frequencyChannelNumberInitialized == false, "Initialization called twice");

  // If frequency has been set to a non-zero value during attribute
  // construction phase, the frequency and channel width will drive the
  // initial configuration.  If frequency has not been set, but both
  // standard and channel number have been set, that pair will instead
  // drive the configuration, and frequency and channel number will be
  // aligned
  if (m_initialFrequency != 0)
    {
      SetFrequency (m_initialFrequency);
    }
  else if (m_initialChannelNumber != 0 && GetStandard () != WIFI_PHY_STANDARD_UNSPECIFIED)
    {
      SetChannelNumber (m_initialChannelNumber);
    }
  else if (m_initialChannelNumber != 0 && GetStandard () == WIFI_PHY_STANDARD_UNSPECIFIED)
    {
      NS_FATAL_ERROR ("Error, ChannelNumber " << (uint16_t)GetChannelNumber () << " was set by user, but neither a standard nor a frequency");
    }
  m_frequencyChannelNumberInitialized = true;
}

void
WifiPhy::SetEdThreshold (double threshold)
{
  NS_LOG_FUNCTION (this << threshold);
  m_edThresholdW = DbmToW (threshold);
}

double
WifiPhy::GetEdThresholdW (void) const
{
  return m_edThresholdW;
}

double
WifiPhy::GetEdThreshold (void) const
{
  return WToDbm (m_edThresholdW);
}

void
WifiPhy::SetCcaMode1Threshold (double threshold)
{
  NS_LOG_FUNCTION (this << threshold);
  m_ccaMode1ThresholdW = DbmToW (threshold);
}

double
WifiPhy::GetCcaMode1Threshold (void) const
{
  return WToDbm (m_ccaMode1ThresholdW);
}

void
WifiPhy::SetRxNoiseFigure (double noiseFigureDb)
{
  NS_LOG_FUNCTION (this << noiseFigureDb);
  m_interference.SetNoiseFigure (DbToRatio (noiseFigureDb));
  m_interference.SetNumberOfReceiveAntennas (GetNumberOfAntennas ());
}

double
WifiPhy::GetRxNoiseFigure (void) const
{
  return RatioToDb (m_interference.GetNoiseFigure ());
}

void
WifiPhy::SetTxPowerStart (double start)
{
  NS_LOG_FUNCTION (this << start);
  m_txPowerBaseDbm = start;
}

double
WifiPhy::GetTxPowerStart (void) const
{
  return m_txPowerBaseDbm;
}

void
WifiPhy::SetTxPowerEnd (double end)
{
  NS_LOG_FUNCTION (this << end);
  m_txPowerEndDbm = end;
}

double
WifiPhy::GetTxPowerEnd (void) const
{
  return m_txPowerEndDbm;
}

void
WifiPhy::SetNTxPower (uint32_t n)
{
  NS_LOG_FUNCTION (this << n);
  m_nTxPower = n;
}

uint32_t
WifiPhy::GetNTxPower (void) const
{
  return m_nTxPower;
}

void
WifiPhy::SetTxGain (double gain)
{
  NS_LOG_FUNCTION (this << gain);
  m_txGainDb = gain;
}

double
WifiPhy::GetTxGain (void) const
{
  return m_txGainDb;
}

void
WifiPhy::SetRxGain (double gain)
{
  NS_LOG_FUNCTION (this << gain);
  m_rxGainDb = gain;
}

double
WifiPhy::GetRxGain (void) const
{
  return m_rxGainDb;
}

void
WifiPhy::SetLdpc (bool ldpc)
{
  NS_LOG_FUNCTION (this << ldpc);
  m_ldpc = ldpc;
}

bool
WifiPhy::GetLdpc (void) const
{
  return m_ldpc;
}

void
WifiPhy::SetStbc (bool stbc)
{
  NS_LOG_FUNCTION (this << stbc);
  m_stbc = stbc;
}

bool
WifiPhy::GetStbc (void) const
{
  return m_stbc;
}

void
WifiPhy::SetGreenfield (bool greenfield)
{
  NS_LOG_FUNCTION (this << greenfield);
  m_greenfield = greenfield;
}

bool
WifiPhy::GetGreenfield (void) const
{
  return m_greenfield;
}

void
WifiPhy::SetShortGuardInterval (bool shortGuardInterval)
{
  NS_LOG_FUNCTION (this << shortGuardInterval);
  m_shortGuardInterval = shortGuardInterval;
}

bool
WifiPhy::GetShortGuardInterval (void) const
{
  return m_shortGuardInterval;
}

void
WifiPhy::SetGuardInterval (Time guardInterval)
{
  NS_LOG_FUNCTION (this << guardInterval);
  NS_ASSERT (guardInterval == NanoSeconds (800) || guardInterval == NanoSeconds (1600) || guardInterval == NanoSeconds (3200));
  m_guardInterval = guardInterval;
}

Time
WifiPhy::GetGuardInterval (void) const
{
  return m_guardInterval;
}

void
WifiPhy::SetShortPlcpPreambleSupported (bool enable)
{
  NS_LOG_FUNCTION (this << enable);
  m_shortPreamble = enable;
}

bool
WifiPhy::GetShortPlcpPreambleSupported (void) const
{
  return m_shortPreamble;
}

void
WifiPhy::SetDevice (Ptr<NetDevice> device)
{
  m_device = device;
}

Ptr<NetDevice>
WifiPhy::GetDevice (void) const
{
  return m_device;
}

void
WifiPhy::SetMobility (Ptr<MobilityModel> mobility)
{
  m_mobility = mobility;
}

Ptr<MobilityModel>
WifiPhy::GetMobility (void) const
{
  if (m_mobility != 0)
    {
      return m_mobility;
    }
  else
    {
      return m_device->GetNode ()->GetObject<MobilityModel> ();
    }
}

void
WifiPhy::SetErrorRateModel (Ptr<ErrorRateModel> rate)
{
  m_interference.SetErrorRateModel (rate);
  m_interference.SetNumberOfReceiveAntennas (GetNumberOfAntennas ());
}

Ptr<ErrorRateModel>
WifiPhy::GetErrorRateModel (void) const
{
  return m_interference.GetErrorRateModel ();
}

double
WifiPhy::GetPowerDbm (uint8_t power) const
{
  NS_ASSERT (m_txPowerBaseDbm <= m_txPowerEndDbm);
  NS_ASSERT (m_nTxPower > 0);
  double dbm;
  if (m_nTxPower > 1)
    {
      dbm = m_txPowerBaseDbm + power * (m_txPowerEndDbm - m_txPowerBaseDbm) / (m_nTxPower - 1);
    }
  else
    {
      NS_ASSERT_MSG (m_txPowerBaseDbm == m_txPowerEndDbm, "cannot have TxPowerEnd != TxPowerStart with TxPowerLevels == 1");
      dbm = m_txPowerBaseDbm;
    }
  return dbm;
}

Time
WifiPhy::GetChannelSwitchDelay (void) const
{
  return m_channelSwitchDelay;
}

double
WifiPhy::CalculateSnr (WifiTxVector txVector, double ber) const
{
  return m_interference.GetErrorRateModel ()->CalculateSnr (txVector, ber);
}

void
WifiPhy::ConfigureDefaultsForStandard (WifiPhyStandard standard)
{
  NS_LOG_FUNCTION (this << standard);
  switch (standard)
    {
    case WIFI_PHY_STANDARD_80211a:
      SetChannelWidth (20);
      SetFrequency (5180);
      // Channel number should be aligned by SetFrequency () to 36
      NS_ASSERT (GetChannelNumber () == 36);
      break;
    case WIFI_PHY_STANDARD_80211b:
      SetChannelWidth (22);
      SetFrequency (2412);
      // Channel number should be aligned by SetFrequency () to 1
      NS_ASSERT (GetChannelNumber () == 1);
      break;
    case WIFI_PHY_STANDARD_80211g:
      SetChannelWidth (20);
      SetFrequency (2412);
      // Channel number should be aligned by SetFrequency () to 1
      NS_ASSERT (GetChannelNumber () == 1);
      break;
    case WIFI_PHY_STANDARD_80211_10MHZ:
      SetChannelWidth (10);
      SetFrequency (5860);
      // Channel number should be aligned by SetFrequency () to 172
      NS_ASSERT (GetChannelNumber () == 172);
      break;
    case WIFI_PHY_STANDARD_80211_5MHZ:
      SetChannelWidth (5);
      SetFrequency (5860);
      // Channel number should be aligned by SetFrequency () to 0
      NS_ASSERT (GetChannelNumber () == 0);
      break;
    case WIFI_PHY_STANDARD_holland:
      SetChannelWidth (20);
      SetFrequency (5180);
      // Channel number should be aligned by SetFrequency () to 36
      NS_ASSERT (GetChannelNumber () == 36);
      break;
    case WIFI_PHY_STANDARD_80211n_2_4GHZ:
      SetChannelWidth (20);
      SetFrequency (2412);
      // Channel number should be aligned by SetFrequency () to 1
      NS_ASSERT (GetChannelNumber () == 1);
      break;
    case WIFI_PHY_STANDARD_80211n_5GHZ:
      SetChannelWidth (20);
      SetFrequency (5180);
      // Channel number should be aligned by SetFrequency () to 36
      NS_ASSERT (GetChannelNumber () == 36);
      break;
    case WIFI_PHY_STANDARD_80211ad:
      SetChannelWidth (2160);
      SetFrequency (58320);
      // Channel number should be aligned by SetFrequency () to 36
      NS_ASSERT (GetChannelNumber () == 1);
      break;
    case WIFI_PHY_STANDARD_80211ac:
      SetChannelWidth (80);
      SetFrequency (5210);
      // Channel number should be aligned by SetFrequency () to 42
      NS_ASSERT (GetChannelNumber () == 42);
      break;
    case WIFI_PHY_STANDARD_80211ax_2_4GHZ:
      SetChannelWidth (20);
      SetFrequency (2412);
      // Channel number should be aligned by SetFrequency () to 1
      NS_ASSERT (GetChannelNumber () == 1);
      break;
    case WIFI_PHY_STANDARD_80211ax_5GHZ:
      SetChannelWidth (80);
      SetFrequency (5210);
      // Channel number should be aligned by SetFrequency () to 42
      NS_ASSERT (GetChannelNumber () == 42);
      break;
    case WIFI_PHY_STANDARD_UNSPECIFIED:
      NS_LOG_WARN ("Configuring unspecified standard; performing no action");
      break;
    default:
      NS_ASSERT (false);
      break;
    }
}

void
WifiPhy::Configure80211a (void)
{
  NS_LOG_FUNCTION (this);

  m_deviceRateSet.push_back (WifiPhy::GetOfdmRate6Mbps ());
  m_deviceRateSet.push_back (WifiPhy::GetOfdmRate9Mbps ());
  m_deviceRateSet.push_back (WifiPhy::GetOfdmRate12Mbps ());
  m_deviceRateSet.push_back (WifiPhy::GetOfdmRate18Mbps ());
  m_deviceRateSet.push_back (WifiPhy::GetOfdmRate24Mbps ());
  m_deviceRateSet.push_back (WifiPhy::GetOfdmRate36Mbps ());
  m_deviceRateSet.push_back (WifiPhy::GetOfdmRate48Mbps ());
  m_deviceRateSet.push_back (WifiPhy::GetOfdmRate54Mbps ());
}

void
WifiPhy::Configure80211b (void)
{
  NS_LOG_FUNCTION (this);

  m_deviceRateSet.push_back (WifiPhy::GetDsssRate1Mbps ());
  m_deviceRateSet.push_back (WifiPhy::GetDsssRate2Mbps ());
  m_deviceRateSet.push_back (WifiPhy::GetDsssRate5_5Mbps ());
  m_deviceRateSet.push_back (WifiPhy::GetDsssRate11Mbps ());
}

void
WifiPhy::Configure80211g (void)
{
  NS_LOG_FUNCTION (this);
  Configure80211b ();

  m_deviceRateSet.push_back (WifiPhy::GetErpOfdmRate6Mbps ());
  m_deviceRateSet.push_back (WifiPhy::GetErpOfdmRate9Mbps ());
  m_deviceRateSet.push_back (WifiPhy::GetErpOfdmRate12Mbps ());
  m_deviceRateSet.push_back (WifiPhy::GetErpOfdmRate18Mbps ());
  m_deviceRateSet.push_back (WifiPhy::GetErpOfdmRate24Mbps ());
  m_deviceRateSet.push_back (WifiPhy::GetErpOfdmRate36Mbps ());
  m_deviceRateSet.push_back (WifiPhy::GetErpOfdmRate48Mbps ());
  m_deviceRateSet.push_back (WifiPhy::GetErpOfdmRate54Mbps ());
}

void
WifiPhy::Configure80211_10Mhz (void)
{
  NS_LOG_FUNCTION (this);

  m_deviceRateSet.push_back (WifiPhy::GetOfdmRate3MbpsBW10MHz ());
  m_deviceRateSet.push_back (WifiPhy::GetOfdmRate4_5MbpsBW10MHz ());
  m_deviceRateSet.push_back (WifiPhy::GetOfdmRate6MbpsBW10MHz ());
  m_deviceRateSet.push_back (WifiPhy::GetOfdmRate9MbpsBW10MHz ());
  m_deviceRateSet.push_back (WifiPhy::GetOfdmRate12MbpsBW10MHz ());
  m_deviceRateSet.push_back (WifiPhy::GetOfdmRate18MbpsBW10MHz ());
  m_deviceRateSet.push_back (WifiPhy::GetOfdmRate24MbpsBW10MHz ());
  m_deviceRateSet.push_back (WifiPhy::GetOfdmRate27MbpsBW10MHz ());
}

void
WifiPhy::Configure80211_5Mhz (void)
{
  NS_LOG_FUNCTION (this);

  m_deviceRateSet.push_back (WifiPhy::GetOfdmRate1_5MbpsBW5MHz ());
  m_deviceRateSet.push_back (WifiPhy::GetOfdmRate2_25MbpsBW5MHz ());
  m_deviceRateSet.push_back (WifiPhy::GetOfdmRate3MbpsBW5MHz ());
  m_deviceRateSet.push_back (WifiPhy::GetOfdmRate4_5MbpsBW5MHz ());
  m_deviceRateSet.push_back (WifiPhy::GetOfdmRate6MbpsBW5MHz ());
  m_deviceRateSet.push_back (WifiPhy::GetOfdmRate9MbpsBW5MHz ());
  m_deviceRateSet.push_back (WifiPhy::GetOfdmRate12MbpsBW5MHz ());
  m_deviceRateSet.push_back (WifiPhy::GetOfdmRate13_5MbpsBW5MHz ());
}

void
WifiPhy::ConfigureHolland (void)
{
  NS_LOG_FUNCTION (this);

  m_deviceRateSet.push_back (WifiPhy::GetOfdmRate6Mbps ());
  m_deviceRateSet.push_back (WifiPhy::GetOfdmRate12Mbps ());
  m_deviceRateSet.push_back (WifiPhy::GetOfdmRate18Mbps ());
  m_deviceRateSet.push_back (WifiPhy::GetOfdmRate36Mbps ());
  m_deviceRateSet.push_back (WifiPhy::GetOfdmRate54Mbps ());
}

void
WifiPhy::ConfigureHtDeviceMcsSet (void)
{
  NS_LOG_FUNCTION (this);

  bool htFound = false;
  for (std::vector<uint32_t>::size_type i = 0; i < m_bssMembershipSelectorSet.size (); i++)
    {
      if (m_bssMembershipSelectorSet[i] == HT_PHY)
        {
          htFound = true;
          break;
        }
    }
  if (htFound)
    {
      // erase all HtMcs modes from deviceMcsSet
      size_t index = m_deviceMcsSet.size () - 1;
      for (std::vector<WifiMode>::reverse_iterator rit = m_deviceMcsSet.rbegin (); rit != m_deviceMcsSet.rend (); ++rit, --index)
        {
          if (m_deviceMcsSet[index].GetModulationClass () == WIFI_MOD_CLASS_HT)
            {
              m_deviceMcsSet.erase (m_deviceMcsSet.begin () + index);
            }
        }
      m_deviceMcsSet.push_back (WifiPhy::GetHtMcs0 ());
      m_deviceMcsSet.push_back (WifiPhy::GetHtMcs1 ());
      m_deviceMcsSet.push_back (WifiPhy::GetHtMcs2 ());
      m_deviceMcsSet.push_back (WifiPhy::GetHtMcs3 ());
      m_deviceMcsSet.push_back (WifiPhy::GetHtMcs4 ());
      m_deviceMcsSet.push_back (WifiPhy::GetHtMcs5 ());
      m_deviceMcsSet.push_back (WifiPhy::GetHtMcs6 ());
      m_deviceMcsSet.push_back (WifiPhy::GetHtMcs7 ());
      if (GetMaxSupportedTxSpatialStreams () > 1)
        {
          m_deviceMcsSet.push_back (WifiPhy::GetHtMcs8 ());
          m_deviceMcsSet.push_back (WifiPhy::GetHtMcs9 ());
          m_deviceMcsSet.push_back (WifiPhy::GetHtMcs10 ());
          m_deviceMcsSet.push_back (WifiPhy::GetHtMcs11 ());
          m_deviceMcsSet.push_back (WifiPhy::GetHtMcs12 ());
          m_deviceMcsSet.push_back (WifiPhy::GetHtMcs13 ());
          m_deviceMcsSet.push_back (WifiPhy::GetHtMcs14 ());
          m_deviceMcsSet.push_back (WifiPhy::GetHtMcs15 ());
        }
      if (GetMaxSupportedTxSpatialStreams () > 2)
        {
          m_deviceMcsSet.push_back (WifiPhy::GetHtMcs16 ());
          m_deviceMcsSet.push_back (WifiPhy::GetHtMcs17 ());
          m_deviceMcsSet.push_back (WifiPhy::GetHtMcs18 ());
          m_deviceMcsSet.push_back (WifiPhy::GetHtMcs19 ());
          m_deviceMcsSet.push_back (WifiPhy::GetHtMcs20 ());
          m_deviceMcsSet.push_back (WifiPhy::GetHtMcs21 ());
          m_deviceMcsSet.push_back (WifiPhy::GetHtMcs22 ());
          m_deviceMcsSet.push_back (WifiPhy::GetHtMcs23 ());
        }
      if (GetMaxSupportedTxSpatialStreams () > 3)
        {
          m_deviceMcsSet.push_back (WifiPhy::GetHtMcs24 ());
          m_deviceMcsSet.push_back (WifiPhy::GetHtMcs25 ());
          m_deviceMcsSet.push_back (WifiPhy::GetHtMcs26 ());
          m_deviceMcsSet.push_back (WifiPhy::GetHtMcs27 ());
          m_deviceMcsSet.push_back (WifiPhy::GetHtMcs28 ());
          m_deviceMcsSet.push_back (WifiPhy::GetHtMcs29 ());
          m_deviceMcsSet.push_back (WifiPhy::GetHtMcs30 ());
          m_deviceMcsSet.push_back (WifiPhy::GetHtMcs31 ());
        }
    }
}

void
WifiPhy::Configure80211n (void)
{
  NS_LOG_FUNCTION (this);
  if (Is2_4Ghz (GetFrequency ()))
    {
      Configure80211b ();
      Configure80211g ();
    }
  if (Is5Ghz (GetFrequency ()))
    {
      Configure80211a ();
    }
  m_bssMembershipSelectorSet.push_back (HT_PHY);
  ConfigureHtDeviceMcsSet ();
}

void
WifiPhy::Configure80211ad (void)
{
  NS_LOG_FUNCTION (this);

  /* CTRL-PHY */
  m_deviceRateSet.push_back (WifiPhy::GetDMG_MCS0 ());

  /* SC-PHY */
  m_deviceRateSet.push_back (WifiPhy::GetDMG_MCS1 ());
  m_deviceRateSet.push_back (WifiPhy::GetDMG_MCS2 ());
  m_deviceRateSet.push_back (WifiPhy::GetDMG_MCS3 ());
  m_deviceRateSet.push_back (WifiPhy::GetDMG_MCS4 ());
  m_deviceRateSet.push_back (WifiPhy::GetDMG_MCS5 ());
  m_deviceRateSet.push_back (WifiPhy::GetDMG_MCS6 ());
  m_deviceRateSet.push_back (WifiPhy::GetDMG_MCS7 ());
  m_deviceRateSet.push_back (WifiPhy::GetDMG_MCS8 ());
  m_deviceRateSet.push_back (WifiPhy::GetDMG_MCS9 ());
  m_deviceRateSet.push_back (WifiPhy::GetDMG_MCS10 ());
  m_deviceRateSet.push_back (WifiPhy::GetDMG_MCS11 ());
  m_deviceRateSet.push_back (WifiPhy::GetDMG_MCS12 ());

  /* OFDM-PHY */
  if (m_supportOFDM)
    {
      m_deviceRateSet.push_back (WifiPhy::GetDMG_MCS13 ());
      m_deviceRateSet.push_back (WifiPhy::GetDMG_MCS14 ());
      m_deviceRateSet.push_back (WifiPhy::GetDMG_MCS15 ());
      m_deviceRateSet.push_back (WifiPhy::GetDMG_MCS16 ());
      m_deviceRateSet.push_back (WifiPhy::GetDMG_MCS17 ());
      m_deviceRateSet.push_back (WifiPhy::GetDMG_MCS18 ());
      m_deviceRateSet.push_back (WifiPhy::GetDMG_MCS19 ());
      m_deviceRateSet.push_back (WifiPhy::GetDMG_MCS20 ());
      m_deviceRateSet.push_back (WifiPhy::GetDMG_MCS21 ());
      m_deviceRateSet.push_back (WifiPhy::GetDMG_MCS22 ());
      m_deviceRateSet.push_back (WifiPhy::GetDMG_MCS23 ());
      m_deviceRateSet.push_back (WifiPhy::GetDMG_MCS24 ());
    }

  /* LP-SC PHY */
  if (m_supportLpSc)
    {
      m_deviceRateSet.push_back (WifiPhy::GetDMG_MCS25 ());
      m_deviceRateSet.push_back (WifiPhy::GetDMG_MCS26 ());
      m_deviceRateSet.push_back (WifiPhy::GetDMG_MCS27 ());
      m_deviceRateSet.push_back (WifiPhy::GetDMG_MCS28 ());
      m_deviceRateSet.push_back (WifiPhy::GetDMG_MCS29 ());
      m_deviceRateSet.push_back (WifiPhy::GetDMG_MCS30 ());
      m_deviceRateSet.push_back (WifiPhy::GetDMG_MCS31 ());
    }
}

void
WifiPhy::Configure80211ac (void)
{
  NS_LOG_FUNCTION (this);
  Configure80211n ();

  m_deviceMcsSet.push_back (WifiPhy::GetVhtMcs0 ());
  m_deviceMcsSet.push_back (WifiPhy::GetVhtMcs1 ());
  m_deviceMcsSet.push_back (WifiPhy::GetVhtMcs2 ());
  m_deviceMcsSet.push_back (WifiPhy::GetVhtMcs3 ());
  m_deviceMcsSet.push_back (WifiPhy::GetVhtMcs4 ());
  m_deviceMcsSet.push_back (WifiPhy::GetVhtMcs5 ());
  m_deviceMcsSet.push_back (WifiPhy::GetVhtMcs6 ());
  m_deviceMcsSet.push_back (WifiPhy::GetVhtMcs7 ());
  m_deviceMcsSet.push_back (WifiPhy::GetVhtMcs8 ());
  m_deviceMcsSet.push_back (WifiPhy::GetVhtMcs9 ());

  m_bssMembershipSelectorSet.push_back (VHT_PHY);
}

void
WifiPhy::Configure80211ax (void)
{
  NS_LOG_FUNCTION (this);
  Configure80211n ();
  if (Is5Ghz (GetFrequency ()))
    {
      Configure80211ac ();
    }
  m_deviceMcsSet.push_back (WifiPhy::GetHeMcs0 ());
  m_deviceMcsSet.push_back (WifiPhy::GetHeMcs1 ());
  m_deviceMcsSet.push_back (WifiPhy::GetHeMcs2 ());
  m_deviceMcsSet.push_back (WifiPhy::GetHeMcs3 ());
  m_deviceMcsSet.push_back (WifiPhy::GetHeMcs4 ());
  m_deviceMcsSet.push_back (WifiPhy::GetHeMcs5 ());
  m_deviceMcsSet.push_back (WifiPhy::GetHeMcs6 ());
  m_deviceMcsSet.push_back (WifiPhy::GetHeMcs7 ());
  m_deviceMcsSet.push_back (WifiPhy::GetHeMcs8 ());
  m_deviceMcsSet.push_back (WifiPhy::GetHeMcs9 ());
  m_deviceMcsSet.push_back (WifiPhy::GetHeMcs10 ());
  m_deviceMcsSet.push_back (WifiPhy::GetHeMcs11 ());

  m_bssMembershipSelectorSet.push_back (HE_PHY);
}

bool
WifiPhy::DefineChannelNumber (uint8_t channelNumber, WifiPhyStandard standard, uint16_t frequency, uint16_t channelWidth)
{
  NS_LOG_FUNCTION (this << (uint16_t)channelNumber << standard << frequency << channelWidth);
  ChannelNumberStandardPair p = std::make_pair (channelNumber, standard);
  ChannelToFrequencyWidthMap::const_iterator it;
  it = m_channelToFrequencyWidth.find (p);
  if (it != m_channelToFrequencyWidth.end ())
    {
      NS_LOG_DEBUG ("channel number/standard already defined; returning false");
      return false;
    }
  FrequencyWidthPair f = std::make_pair (frequency, channelWidth);
  m_channelToFrequencyWidth[p] = f;
  return true;
}

uint8_t
WifiPhy::FindChannelNumberForFrequencyWidth (uint16_t frequency, uint16_t width) const
{
  NS_LOG_FUNCTION (this << frequency << (uint16_t)width);
  bool found = false;
  FrequencyWidthPair f = std::make_pair (frequency, width);
  ChannelToFrequencyWidthMap::const_iterator it = m_channelToFrequencyWidth.begin ();
  while (it != m_channelToFrequencyWidth.end ())
    {
      if (it->second == f)
        {
          found = true;
          break;
        }
      ++it;
    }
  if (found)
    {
      NS_LOG_DEBUG ("Found, returning " << it->first.first);
      return (it->first.first);
    }
  else
    {
      NS_LOG_DEBUG ("Not found, returning 0");
      return 0;
    }
}

void
WifiPhy::ConfigureChannelForStandard (WifiPhyStandard standard)
{
  NS_LOG_FUNCTION (this << standard);
  // If the user has configured both Frequency and ChannelNumber, Frequency
  // takes precedence
  if (GetFrequency () != 0)
    {
      // If Frequency is already set, then see whether a ChannelNumber can
      // be found that matches Frequency and ChannelWidth. If so, configure
      // the ChannelNumber to that channel number. If not, set ChannelNumber to zero.
      NS_LOG_DEBUG ("Frequency set; checking whether a channel number corresponds");
      uint8_t channelNumberSearched = FindChannelNumberForFrequencyWidth (GetFrequency (), GetChannelWidth ());
      if (channelNumberSearched)
        {
          NS_LOG_DEBUG ("Channel number found; setting to " << (uint16_t)channelNumberSearched);
          SetChannelNumber (channelNumberSearched);
        }
      else
        {
          NS_LOG_DEBUG ("Channel number not found; setting to zero");
          SetChannelNumber (0);
        }
    }
  else if (GetChannelNumber () != 0)
    {
      // If the channel number is known for this particular standard or for
      // the unspecified standard, configure using the known values;
      // otherwise, this is a configuration error
      NS_LOG_DEBUG ("Configuring for channel number " << (uint16_t)GetChannelNumber ());
      FrequencyWidthPair f = GetFrequencyWidthForChannelNumberStandard (GetChannelNumber (), standard);
      if (f.first == 0)
        {
          // the specific pair of number/standard is not known
          NS_LOG_DEBUG ("Falling back to check WIFI_PHY_STANDARD_UNSPECIFIED");
          f = GetFrequencyWidthForChannelNumberStandard (GetChannelNumber (), WIFI_PHY_STANDARD_UNSPECIFIED);
        }
      if (f.first == 0)
        {
          NS_FATAL_ERROR ("Error, ChannelNumber " << (uint16_t)GetChannelNumber () << " is unknown for this standard");
        }
      else
        {
          NS_LOG_DEBUG ("Setting frequency to " << f.first << "; width to " << (uint16_t)f.second);
          SetFrequency (f.first);
          SetChannelWidth (f.second);
        }
    }
}

void
WifiPhy::ConfigureStandard (WifiPhyStandard standard)
{
  NS_LOG_FUNCTION (this << standard);
  m_standard = standard;
  m_isConstructed = true;
  if (m_frequencyChannelNumberInitialized == false)
    {
      InitializeFrequencyChannelNumber ();
    }
  if (GetFrequency () == 0 && GetChannelNumber () == 0)
    {
      ConfigureDefaultsForStandard (standard);
    }
  else
    {
      // The user has configured either (or both) Frequency or ChannelNumber
      ConfigureChannelForStandard (standard);
    }
  switch (standard)
    {
    case WIFI_PHY_STANDARD_80211a:
      Configure80211a ();
      break;
    case WIFI_PHY_STANDARD_80211b:
      Configure80211b ();
      break;
    case WIFI_PHY_STANDARD_80211g:
      Configure80211g ();
      break;
    case WIFI_PHY_STANDARD_80211_10MHZ:
      Configure80211_10Mhz ();
      break;
    case WIFI_PHY_STANDARD_80211_5MHZ:
      Configure80211_5Mhz ();
      break;
    case WIFI_PHY_STANDARD_holland:
      ConfigureHolland ();
      break;
    case WIFI_PHY_STANDARD_80211n_2_4GHZ:
      Configure80211n ();
      break;
    case WIFI_PHY_STANDARD_80211n_5GHZ:
      Configure80211n ();
      break;
    case WIFI_PHY_STANDARD_80211ad:
      Configure80211ad ();
      break;
    case WIFI_PHY_STANDARD_80211ac:
      Configure80211ac ();
      break;
    case WIFI_PHY_STANDARD_80211ax_2_4GHZ:
      Configure80211ax ();
      break;
    case WIFI_PHY_STANDARD_80211ax_5GHZ:
      Configure80211ax ();
      break;
    default:
      NS_ASSERT (false);
      break;
    }
}

WifiPhyStandard
WifiPhy::GetStandard (void) const
{
  return m_standard;
}

void
WifiPhy::SetFrequency (uint16_t frequency)
{
  NS_LOG_FUNCTION (this << frequency);
  if (m_isConstructed == false)
    {
      NS_LOG_DEBUG ("Saving frequency configuration for initialization");
      m_initialFrequency = frequency;
      return;
    }
  if (GetFrequency () == frequency)
    {
      NS_LOG_DEBUG ("No frequency change requested");
      return;
    }
  if (frequency == 0)
    {
      DoFrequencySwitch (0);
      NS_LOG_DEBUG ("Setting frequency and channel number to zero");
      m_channelCenterFrequency = 0;
      m_channelNumber = 0;
      return;
    }
  // If the user has configured both Frequency and ChannelNumber, Frequency
  // takes precedence.  Lookup the channel number corresponding to the
  // requested frequency.
  uint8_t nch = FindChannelNumberForFrequencyWidth (frequency, GetChannelWidth ());
  if (nch != 0)
    {
      NS_LOG_DEBUG ("Setting frequency " << frequency << " corresponds to channel " << (uint16_t)nch);
      if (DoFrequencySwitch (frequency))
        {
          NS_LOG_DEBUG ("Channel frequency switched to " << frequency << "; channel number to " << (uint16_t)nch);
          m_channelCenterFrequency = frequency;
          m_channelNumber = nch;
        }
      else
        {
          NS_LOG_DEBUG ("Suppressing reassignment of frequency");
        }
    }
  else
    {
      NS_LOG_DEBUG ("Channel number is unknown for frequency " << frequency);
      if (DoFrequencySwitch (frequency))
        {
          NS_LOG_DEBUG ("Channel frequency switched to " << frequency << "; channel number to " << 0);
          m_channelCenterFrequency = frequency;
          m_channelNumber = 0;
        }
      else
        {
          NS_LOG_DEBUG ("Suppressing reassignment of frequency");
        }
    }
}

uint16_t
WifiPhy::GetFrequency (void) const
{
  return m_channelCenterFrequency;
}

bool
WifiPhy::Is2_4Ghz (double frequency) const
{
  if (frequency >= 2400 && frequency <= 2500)
    {
      return true;
    }
  return false;
}

bool
WifiPhy::Is5Ghz (double frequency) const
{
  if (frequency >= 5000 && frequency <= 6000)
    {
      return true;
    }
  return false;
}

void
WifiPhy::SetChannelWidth (uint16_t channelwidth)
{
  NS_ASSERT_MSG (channelwidth == 5 || channelwidth == 10 || channelwidth == 20 || channelwidth == 22 || channelwidth == 40 || channelwidth == 80 || channelwidth == 160 || channelwidth == 2160, "wrong channel width value");
  m_channelWidth = channelwidth;
  AddSupportedChannelWidth (channelwidth);
}

uint16_t
WifiPhy::GetChannelWidth (void) const
{
  return m_channelWidth;
}

void
WifiPhy::SetNumberOfAntennas (uint8_t antennas)
{
  NS_ASSERT_MSG (antennas > 0 && antennas <= 4, "unsupported number of antennas");
  m_numberOfAntennas = antennas;
  m_interference.SetNumberOfReceiveAntennas (antennas);
}

uint8_t
WifiPhy::GetNumberOfAntennas (void) const
{
  return m_numberOfAntennas;
}

void
WifiPhy::SetMaxSupportedTxSpatialStreams (uint8_t streams)
{
  NS_ASSERT (streams <= GetNumberOfAntennas ());
  m_txSpatialStreams = streams;
  ConfigureHtDeviceMcsSet ();
}

uint8_t
WifiPhy::GetMaxSupportedTxSpatialStreams (void) const
{
  return m_txSpatialStreams;
}

void
WifiPhy::SetMaxSupportedRxSpatialStreams (uint8_t streams)
{
  NS_ASSERT (streams <= GetNumberOfAntennas ());
  m_rxSpatialStreams = streams;
}

uint8_t
WifiPhy::GetMaxSupportedRxSpatialStreams (void) const
{
  return m_rxSpatialStreams;
}

uint32_t
WifiPhy::GetNBssMembershipSelectors (void) const
{
  return m_bssMembershipSelectorSet.size ();
}

uint32_t
WifiPhy::GetBssMembershipSelector (uint32_t selector) const
{
  return m_bssMembershipSelectorSet[selector];
}

WifiModeList
WifiPhy::GetMembershipSelectorModes (uint32_t selector)
{
  uint32_t id = GetBssMembershipSelector (selector);
  WifiModeList supportedmodes;
  if (id == HT_PHY || id == VHT_PHY || id == HE_PHY)
    {
      //mandatory MCS 0 to 7
      supportedmodes.push_back (WifiPhy::GetHtMcs0 ());
      supportedmodes.push_back (WifiPhy::GetHtMcs1 ());
      supportedmodes.push_back (WifiPhy::GetHtMcs2 ());
      supportedmodes.push_back (WifiPhy::GetHtMcs3 ());
      supportedmodes.push_back (WifiPhy::GetHtMcs4 ());
      supportedmodes.push_back (WifiPhy::GetHtMcs5 ());
      supportedmodes.push_back (WifiPhy::GetHtMcs6 ());
      supportedmodes.push_back (WifiPhy::GetHtMcs7 ());
    }
  if (id == VHT_PHY || id == HE_PHY)
    {
      //mandatory MCS 0 to 9
      supportedmodes.push_back (WifiPhy::GetVhtMcs0 ());
      supportedmodes.push_back (WifiPhy::GetVhtMcs1 ());
      supportedmodes.push_back (WifiPhy::GetVhtMcs2 ());
      supportedmodes.push_back (WifiPhy::GetVhtMcs3 ());
      supportedmodes.push_back (WifiPhy::GetVhtMcs4 ());
      supportedmodes.push_back (WifiPhy::GetVhtMcs5 ());
      supportedmodes.push_back (WifiPhy::GetVhtMcs6 ());
      supportedmodes.push_back (WifiPhy::GetVhtMcs7 ());
      supportedmodes.push_back (WifiPhy::GetVhtMcs8 ());
      supportedmodes.push_back (WifiPhy::GetVhtMcs9 ());
    }
  if (id == HE_PHY)
    {
      //mandatory MCS 0 to 11
      supportedmodes.push_back (WifiPhy::GetHeMcs0 ());
      supportedmodes.push_back (WifiPhy::GetHeMcs1 ());
      supportedmodes.push_back (WifiPhy::GetHeMcs2 ());
      supportedmodes.push_back (WifiPhy::GetHeMcs3 ());
      supportedmodes.push_back (WifiPhy::GetHeMcs4 ());
      supportedmodes.push_back (WifiPhy::GetHeMcs5 ());
      supportedmodes.push_back (WifiPhy::GetHeMcs6 ());
      supportedmodes.push_back (WifiPhy::GetHeMcs7 ());
      supportedmodes.push_back (WifiPhy::GetHeMcs8 ());
      supportedmodes.push_back (WifiPhy::GetHeMcs9 ());
      supportedmodes.push_back (WifiPhy::GetHeMcs10 ());
      supportedmodes.push_back (WifiPhy::GetHeMcs11 ());
    }
  return supportedmodes;
}

void
WifiPhy::AddSupportedChannelWidth (uint16_t width)
{
  NS_LOG_FUNCTION (this << width);
  for (std::vector<uint16_t>::size_type i = 0; i != m_supportedChannelWidthSet.size (); i++)
    {
      if (m_supportedChannelWidthSet[i] == width)
        {
          return;
        }
    }
  NS_LOG_FUNCTION ("Adding " << width << " to supported channel width set");
  m_supportedChannelWidthSet.push_back (width);
}

std::vector<uint16_t>
WifiPhy::GetSupportedChannelWidthSet (void) const
{
  return m_supportedChannelWidthSet;
}

WifiPhy::FrequencyWidthPair
WifiPhy::GetFrequencyWidthForChannelNumberStandard (uint8_t channelNumber, WifiPhyStandard standard) const
{
  ChannelNumberStandardPair p = std::make_pair (channelNumber, standard);
  FrequencyWidthPair f = m_channelToFrequencyWidth[p];
  return f;
}

void
WifiPhy::SetChannelNumber (uint8_t nch)
{
  NS_LOG_FUNCTION (this << (uint16_t)nch);
  if (m_isConstructed == false)
    {
      NS_LOG_DEBUG ("Saving channel number configuration for initialization");
      m_initialChannelNumber = nch;
      return;
    }
  if (GetChannelNumber () == nch)
    {
      NS_LOG_DEBUG ("No channel change requested");
      return;
    }
  if (nch == 0)
    {
      // This case corresponds to when there is not a known channel
      // number for the requested frequency.  There is no need to call
      // DoChannelSwitch () because DoFrequencySwitch () should have been
      // called by the client
      NS_LOG_DEBUG ("Setting channel number to zero");
      m_channelNumber = 0;
      return;
    }

  // First make sure that the channel number is defined for the standard
  // in use
  FrequencyWidthPair f = GetFrequencyWidthForChannelNumberStandard (nch, GetStandard ());
  if (f.first == 0)
    {
      f = GetFrequencyWidthForChannelNumberStandard (nch, WIFI_PHY_STANDARD_UNSPECIFIED);
    }
  if (f.first != 0)
    {
      if (DoChannelSwitch (nch))
        {
          NS_LOG_DEBUG ("Setting frequency to " << f.first << "; width to " << (uint16_t)f.second);
          m_channelCenterFrequency = f.first;
          SetChannelWidth (f.second);
          m_channelNumber = nch;
        }
      else
        {
          // Subclass may have suppressed (e.g. waiting for state change)
          NS_LOG_DEBUG ("Channel switch suppressed");
        }
    }
  else
    {
      NS_FATAL_ERROR ("Frequency not found for channel number " << nch);
    }
}

uint8_t
WifiPhy::GetChannelNumber (void) const
{
  return m_channelNumber;
}

bool
WifiPhy::DoChannelSwitch (uint8_t nch)
{
  if (!IsInitialized ())
    {
      //this is not channel switch, this is initialization
      NS_LOG_DEBUG ("initialize to channel " << (uint16_t)nch);
      return true;
    }

  NS_ASSERT (!IsStateSwitching ());
  switch (m_state->GetState ())
    {
    case WifiPhy::RX:
      NS_LOG_DEBUG ("drop packet because of channel switching while reception");
      m_endPlcpRxEvent.Cancel ();
      m_endRxEvent.Cancel ();
      goto switchChannel;
      break;
    case WifiPhy::TX:
      NS_LOG_DEBUG ("channel switching postponed until end of current transmission");
      Simulator::Schedule (GetDelayUntilIdle (), &WifiPhy::SetChannelNumber, this, nch);
      break;
    case WifiPhy::CCA_BUSY:
    case WifiPhy::IDLE:
      goto switchChannel;
      break;
    case WifiPhy::SLEEP:
      NS_LOG_DEBUG ("channel switching ignored in sleep mode");
      break;
    default:
      NS_ASSERT (false);
      break;
    }

  return false;

switchChannel:

  NS_LOG_DEBUG ("switching channel " << (uint16_t)GetChannelNumber () << " -> " << (uint16_t)nch);
  m_state->SwitchToChannelSwitching (GetChannelSwitchDelay ());
  m_interference.EraseEvents ();
  /*
   * Needed here to be able to correctly sensed the medium for the first
   * time after the switching. The actual switching is not performed until
   * after m_channelSwitchDelay. Packets received during the switching
   * state are added to the event list and are employed later to figure
   * out the state of the medium after the switching.
   */
  return true;
}

bool
WifiPhy::DoFrequencySwitch (uint16_t frequency)
{
  if (!IsInitialized ())
    {
      //this is not channel switch, this is initialization
      NS_LOG_DEBUG ("start at frequency " << frequency);
      return true;
    }

  NS_ASSERT (!IsStateSwitching ());
  switch (m_state->GetState ())
    {
    case WifiPhy::RX:
      NS_LOG_DEBUG ("drop packet because of channel/frequency switching while reception");
      m_endPlcpRxEvent.Cancel ();
      m_endRxEvent.Cancel ();
      goto switchFrequency;
      break;
    case WifiPhy::TX:
      NS_LOG_DEBUG ("channel/frequency switching postponed until end of current transmission");
      Simulator::Schedule (GetDelayUntilIdle (), &WifiPhy::SetFrequency, this, frequency);
      break;
    case WifiPhy::CCA_BUSY:
    case WifiPhy::IDLE:
      goto switchFrequency;
      break;
    case WifiPhy::SLEEP:
      NS_LOG_DEBUG ("frequency switching ignored in sleep mode");
      break;
    default:
      NS_ASSERT (false);
      break;
    }

  return false;

switchFrequency:

  NS_LOG_DEBUG ("switching frequency " << GetFrequency () << " -> " << frequency);
  m_state->SwitchToChannelSwitching (GetChannelSwitchDelay ());
  m_interference.EraseEvents ();
  /*
   * Needed here to be able to correctly sensed the medium for the first
   * time after the switching. The actual switching is not performed until
   * after m_channelSwitchDelay. Packets received during the switching
   * state are added to the event list and are employed later to figure
   * out the state of the medium after the switching.
   */
  return true;
}

void
WifiPhy::SetSleepMode (void)
{
  NS_LOG_FUNCTION (this);
  switch (m_state->GetState ())
    {
    case WifiPhy::TX:
      NS_LOG_DEBUG ("setting sleep mode postponed until end of current transmission");
      Simulator::Schedule (GetDelayUntilIdle (), &WifiPhy::SetSleepMode, this);
      break;
    case WifiPhy::RX:
      NS_LOG_DEBUG ("setting sleep mode postponed until end of current reception");
      Simulator::Schedule (GetDelayUntilIdle (), &WifiPhy::SetSleepMode, this);
      break;
    case WifiPhy::SWITCHING:
      NS_LOG_DEBUG ("setting sleep mode postponed until end of channel switching");
      Simulator::Schedule (GetDelayUntilIdle (), &WifiPhy::SetSleepMode, this);
      break;
    case WifiPhy::CCA_BUSY:
    case WifiPhy::IDLE:
      NS_LOG_DEBUG ("setting sleep mode");
      m_state->SwitchToSleep ();
      break;
    case WifiPhy::SLEEP:
      NS_LOG_DEBUG ("already in sleep mode");
      break;
    default:
      NS_ASSERT (false);
      break;
    }
}

void
WifiPhy::ResumeFromSleep (void)
{
  NS_LOG_FUNCTION (this);
  switch (m_state->GetState ())
    {
    case WifiPhy::TX:
    case WifiPhy::RX:
    case WifiPhy::IDLE:
    case WifiPhy::CCA_BUSY:
    case WifiPhy::SWITCHING:
      {
        NS_LOG_DEBUG ("not in sleep mode, there is nothing to resume");
        break;
      }
    case WifiPhy::SLEEP:
      {
        NS_LOG_DEBUG ("resuming from sleep mode");
        Time delayUntilCcaEnd = m_interference.GetEnergyDuration (DbmToW (GetCcaMode1Threshold ()));
        m_state->SwitchFromSleep (delayUntilCcaEnd);
        break;
      }
    default:
      {
        NS_ASSERT (false);
        break;
      }
    }
}

WifiMode
WifiPhy::GetHtPlcpHeaderMode ()
{
  return WifiPhy::GetHtMcs0 ();
}

WifiMode
WifiPhy::GetVhtPlcpHeaderMode ()
{
  return WifiPhy::GetVhtMcs0 ();
}

WifiMode
WifiPhy::GetHePlcpHeaderMode ()
{
  return WifiPhy::GetHeMcs0 ();
}

Time
WifiPhy::GetPlcpTrainingSymbolDuration (WifiTxVector txVector)
{
  uint8_t Ndltf, Neltf;
  //We suppose here that STBC = 0.
  //If STBC > 0, we need a different mapping between Nss and Nltf (IEEE 802.11n-2012 standard, page 1682).
  if (txVector.GetNss () < 3)
    {
      Ndltf = txVector.GetNss ();
    }
  else if (txVector.GetNss () < 5)
    {
      Ndltf = 4;
    }
  else if (txVector.GetNss () < 7)
    {
      Ndltf = 6;
    }
  else
    {
      Ndltf = 8;
    }

  if (txVector.GetNess () < 3)
    {
      Neltf = txVector.GetNess ();
    }
  else
    {
      Neltf = 4;
    }

  switch (txVector.GetPreambleType ())
    {
    case WIFI_PREAMBLE_HT_MF:
      return MicroSeconds (4 + (4 * Ndltf) + (4 * Neltf));
    case WIFI_PREAMBLE_HT_GF:
      return MicroSeconds ((4 * Ndltf) + (4 * Neltf));
    case WIFI_PREAMBLE_VHT:
      return MicroSeconds (4 + (4 * Ndltf));
    case WIFI_PREAMBLE_HE_SU:
      return MicroSeconds (4 + (8 * Ndltf));
    default:
      return MicroSeconds (0);
    }
}

Time
WifiPhy::GetPlcpHtSigHeaderDuration (WifiPreamble preamble)
{
  switch (preamble)
    {
    case WIFI_PREAMBLE_HT_MF:
    case WIFI_PREAMBLE_HT_GF:
      //HT-SIG
      return MicroSeconds (8);
    default:
      //no HT-SIG for non HT
      return MicroSeconds (0);
    }
}

Time
WifiPhy::GetPlcpSigA1Duration (WifiPreamble preamble)
{
  switch (preamble)
    {
    case WIFI_PREAMBLE_VHT:
    case WIFI_PREAMBLE_HE_SU:
      //VHT-SIG-A1 and HE-SIG-A1
      return MicroSeconds (4);
    default:
      // no SIG-A1
      return MicroSeconds (0);
    }
}

Time
WifiPhy::GetPlcpSigA2Duration (WifiPreamble preamble)
{
  switch (preamble)
    {
    case WIFI_PREAMBLE_VHT:
    case WIFI_PREAMBLE_HE_SU:
      //VHT-SIG-A2 and HE-SIG-A2
      return MicroSeconds (4);
    default:
      // no SIG-A2
      return MicroSeconds (0);
    }
}

Time
WifiPhy::GetPlcpSigBDuration (WifiPreamble preamble)
{
  switch (preamble)
    {
    case WIFI_PREAMBLE_VHT:
      //VHT-SIG-B
      return MicroSeconds (4);
    case WIFI_PREAMBLE_HE_SU:
      //HE-SIG-B: MU not supported so HE-SIG-B not used
      return MicroSeconds (0);
    default:
      // no SIG-B
      return MicroSeconds (0);
    }
}

WifiMode
WifiPhy::GetPlcpHeaderMode (WifiTxVector txVector)
{
  switch (txVector.GetMode ().GetModulationClass ())
    {
    case WIFI_MOD_CLASS_OFDM:
    case WIFI_MOD_CLASS_HT:
    case WIFI_MOD_CLASS_VHT:
    case WIFI_MOD_CLASS_HE:
      switch (txVector.GetChannelWidth ())
        {
        case 5:
          return WifiPhy::GetOfdmRate1_5MbpsBW5MHz ();
        case 10:
          return WifiPhy::GetOfdmRate3MbpsBW10MHz ();
        case 20:
        case 40:
        case 80:
        case 160:
        default:
          //(Section 18.3.2 "PLCP frame format"; IEEE Std 802.11-2012)
          //actually this is only the first part of the PlcpHeader,
          //because the last 16 bits of the PlcpHeader are using the
          //same mode of the payload
          return WifiPhy::GetOfdmRate6Mbps ();
        }
    case WIFI_MOD_CLASS_ERP_OFDM:
      return WifiPhy::GetErpOfdmRate6Mbps ();
    case WIFI_MOD_CLASS_DSSS:
    case WIFI_MOD_CLASS_HR_DSSS:
      if (txVector.GetPreambleType () == WIFI_PREAMBLE_LONG || txVector.GetMode () == WifiPhy::GetDsssRate1Mbps ())
        {
          //(Section 16.2.3 "PLCP field definitions" and Section 17.2.2.2 "Long PPDU format"; IEEE Std 802.11-2012)
          return WifiPhy::GetDsssRate1Mbps ();
        }
      else //WIFI_PREAMBLE_SHORT
        {
          //(Section 17.2.2.3 "Short PPDU format"; IEEE Std 802.11-2012)
          return WifiPhy::GetDsssRate2Mbps ();
        }

    case WIFI_MOD_CLASS_DMG_CTRL:
      return WifiPhy::GetDMG_MCS0 ();

    case WIFI_MOD_CLASS_DMG_SC:
      return WifiPhy::GetDMG_MCS1 ();

    case WIFI_MOD_CLASS_DMG_OFDM:
      return WifiPhy::GetDMG_MCS13 ();

    default:
      NS_FATAL_ERROR ("unsupported modulation class");
      return WifiMode ();
    }
}

Time
WifiPhy::GetPlcpHeaderDuration (WifiTxVector txVector)
{
  WifiPreamble preamble = txVector.GetPreambleType ();
  if (preamble == WIFI_PREAMBLE_NONE)
    {
      return MicroSeconds (0);
    }
  switch (txVector.GetMode ().GetModulationClass ())
    {
    case WIFI_MOD_CLASS_OFDM:
      {
        switch (txVector.GetChannelWidth ())
          {
          case 20:
          default:
            //(Section 18.3.3 "PLCP preamble (SYNC))" and Figure 18-4 "OFDM training structure"; IEEE Std 802.11-2012)
            //also (Section 18.3.2.4 "Timing related parameters" Table 18-5 "Timing-related parameters"; IEEE Std 802.11-2012)
            //We return the duration of the SIGNAL field only, since the
            //SERVICE field (which strictly speaking belongs to the PLCP
            //header, see Section 18.3.2 and Figure 18-1) is sent using the
            //payload mode.
            return MicroSeconds (4);
          case 10:
            //(Section 18.3.2.4 "Timing related parameters" Table 18-5 "Timing-related parameters"; IEEE Std 802.11-2012)
            return MicroSeconds (8);
          case 5:
            //(Section 18.3.2.4 "Timing related parameters" Table 18-5 "Timing-related parameters"; IEEE Std 802.11-2012)
            return MicroSeconds (16);
          }
      }
    case WIFI_MOD_CLASS_HT:
      {
        //L-SIG
        //IEEE 802.11n Figure 20.1
        switch (preamble)
          {
          case WIFI_PREAMBLE_HT_MF:
          default:
            return MicroSeconds (4);
          case WIFI_PREAMBLE_HT_GF:
            return MicroSeconds (0);
          }
      }
    case WIFI_MOD_CLASS_ERP_OFDM:
    case WIFI_MOD_CLASS_VHT:
      //L-SIG
      return MicroSeconds (4);
    case WIFI_MOD_CLASS_HE:
      //LSIG + R-LSIG
      return MicroSeconds (8);
    case WIFI_MOD_CLASS_DSSS:
    case WIFI_MOD_CLASS_HR_DSSS:
      if ((preamble == WIFI_PREAMBLE_SHORT) && (txVector.GetMode ().GetDataRate (22) > 1000000))
        {
          //(Section 17.2.2.3 "Short PPDU format" and Figure 17-2 "Short PPDU format"; IEEE Std 802.11-2012)
          return MicroSeconds (24);
        }
      else //WIFI_PREAMBLE_LONG
        {
          //(Section 17.2.2.2 "Long PPDU format" and Figure 17-1 "Short PPDU format"; IEEE Std 802.11-2012)
          return MicroSeconds (48);
        }
    case WIFI_MOD_CLASS_DMG_CTRL:
      /* From Annex L (L.5.2.5) */
      return NanoSeconds (4654);
    case WIFI_MOD_CLASS_DMG_SC:
    case WIFI_MOD_CLASS_DMG_LP_SC:
      /* From Table 21-4 in 802.11ad spec 21.3.4 */
      return NanoSeconds (582);
    case WIFI_MOD_CLASS_DMG_OFDM:
      /* From Table 21-4 in 802.11ad spec 21.3.4 */
      return NanoSeconds (242);
    default:
      NS_FATAL_ERROR ("unsupported modulation class");
      return MicroSeconds (0);
    }
}

Time
WifiPhy::GetPlcpPreambleDuration (WifiTxVector txVector)
{
  WifiPreamble preamble = txVector.GetPreambleType ();
  if (preamble == WIFI_PREAMBLE_NONE)
    {
      return MicroSeconds (0);
    }
  switch (txVector.GetMode ().GetModulationClass ())
    {
    case WIFI_MOD_CLASS_OFDM:
      {
        switch (txVector.GetChannelWidth ())
          {
          case 20:
          default:
            //(Section 18.3.3 "PLCP preamble (SYNC))" Figure 18-4 "OFDM training structure"
            //also Section 18.3.2.3 "Modulation-dependent parameters" Table 18-4 "Modulation-dependent parameters"; IEEE Std 802.11-2012)
            return MicroSeconds (16);
          case 10:
            //(Section 18.3.3 "PLCP preamble (SYNC))" Figure 18-4 "OFDM training structure"
            //also Section 18.3.2.3 "Modulation-dependent parameters" Table 18-4 "Modulation-dependent parameters"; IEEE Std 802.11-2012)
            return MicroSeconds (32);
          case 5:
            //(Section 18.3.3 "PLCP preamble (SYNC))" Figure 18-4 "OFDM training structure"
            //also Section 18.3.2.3 "Modulation-dependent parameters" Table 18-4 "Modulation-dependent parameters"; IEEE Std 802.11-2012)
            return MicroSeconds (64);
          }
      }
    case WIFI_MOD_CLASS_HT:
    case WIFI_MOD_CLASS_VHT:
    case WIFI_MOD_CLASS_HE:
      //L-STF + L-LTF
      return MicroSeconds (16);
    case WIFI_MOD_CLASS_ERP_OFDM:
      return MicroSeconds (16);
    case WIFI_MOD_CLASS_DSSS:
    case WIFI_MOD_CLASS_HR_DSSS:
      if ((preamble == WIFI_PREAMBLE_SHORT) && (txVector.GetMode ().GetDataRate (22) > 1000000))
        {
          //(Section 17.2.2.3 "Short PPDU format)" Figure 17-2 "Short PPDU format"; IEEE Std 802.11-2012)
          return MicroSeconds (72);
        }
      else //WIFI_PREAMBLE_LONG
        {
          //(Section 17.2.2.2 "Long PPDU format)" Figure 17-1 "Long PPDU format"; IEEE Std 802.11-2012)
          return MicroSeconds (144);
        }
		
    case WIFI_MOD_CLASS_DMG_CTRL:
      // CTRL Preamble = (6400 + 1152) Samples * Tc (Chip Time for SC), Tc = Tccp = 0.57ns.
      // CTRL Preamble = 4.291 micro seconds.
      return NanoSeconds (4291);

    case WIFI_MOD_CLASS_DMG_SC:
    case WIFI_MOD_CLASS_DMG_LP_SC:
      // SC Preamble = 3328 Samples * Tc (Chip Time for SC), Tc = 0.57ns.
      // SC Preamble = 1.89 micro seconds.
      return NanoSeconds (1891);

    case WIFI_MOD_CLASS_DMG_OFDM:
      // OFDM Preamble = 4992 Samples * Ts (Chip Time for OFDM), Tc = 0.38ns.
      // OFDM Preamble = 1.89 micro seconds.
      return NanoSeconds (1891);
		
    default:
      NS_FATAL_ERROR ("unsupported modulation class");
      return MicroSeconds (0);
    }
}

Time
WifiPhy::GetPayloadDuration (uint32_t size, WifiTxVector txVector, uint16_t frequency)
{
  return GetPayloadDuration (size, txVector, frequency, NORMAL_MPDU, 0);
}

Time
WifiPhy::GetPayloadDuration (uint32_t size, WifiTxVector txVector, uint16_t frequency, MpduType mpdutype, uint8_t incFlag)
{
  WifiMode payloadMode = txVector.GetMode ();
  WifiPreamble preamble = txVector.GetPreambleType ();
  NS_LOG_FUNCTION (size << payloadMode);

  if (payloadMode.GetModulationClass () == WIFI_MOD_CLASS_DMG_CTRL)
    {
      if (txVector.GetTrainngFieldLength () == 0)
        {
          uint32_t Ncw;                       /* Number of LDPC codewords. */
          uint32_t Ldpcw;                     /* Number of bits in the second and any subsequent codeword except the last. */
          uint32_t Ldplcw;                    /* Number of bits in the last codeword. */
          uint32_t DencodedSymmbols;          /* Number of differentailly encoded payload symbols. */
          uint32_t Chips;                     /* Number of chips (After spreading using Ga32 Golay Sequence). */
          uint32_t Nbits = (size - 8) * 8;    /* Number of bits in the payload part. */

          Ncw = 1 + (uint32_t) ceil ((double (size) - 6) * 8/168);
          Ldpcw = (uint32_t) ceil ((double (size) - 6) * 8/(Ncw - 1));
          Ldplcw = (size - 6) * 8 - (Ncw - 2) * Ldpcw;
          DencodedSymmbols = (672 - (504 - Ldpcw)) * (Ncw - 2) + (672 - (504 - Ldplcw));
          Chips = DencodedSymmbols * 32;

          /* Make sure the result is in nanoseconds. */
          double ret = double (Chips)/1.76;
          NS_LOG_DEBUG("bits " << Nbits << " Diff encoded Symmbols " << DencodedSymmbols << " rate " << payloadMode.GetDataRate() << " Payload Time " << ret << " ns");

          return NanoSeconds (ceil (ret));
        }
      else
        {
          uint32_t Ncw;                       /* Number of LDPC codewords. */
          Ncw = 1 + (uint32_t) ceil ((double (size) - 6) * 8/168);
          return NanoSeconds (ceil (double ((88 + (size - 6) * 8 + Ncw * 168) * 0.57 * 32)));
        }
    }
  else if (payloadMode.GetModulationClass () == WIFI_MOD_CLASS_DMG_LP_SC)
    {
//        uint32_t Nbits = (size * 8);  /* Number of bits in the payload part. */
//        uint32_t Nrsc;                /* The total number of Reed Solomon codewords */
//        uint32_t Nrses;               /* The total number of Reed Solomon encoded symbols */
//        Nrsc = (uint32_t) ceil(Nbits/208);
//        Nrses = Nbits + Nrsc * 16;

//        uint32_t Nsbc;                 /* Short Block code Size */
//        if (payloadMode.GetCodeRate() == WIFI_CODE_RATE_13_28)
//          Nsbc = 16;
//        else if (payloadMode.GetCodeRate() == WIFI_CODE_RATE_13_21)
//          Nsbc = 12;
//        else if (payloadMode.GetCodeRate() == WIFI_CODE_RATE_52_63)
//          Nsbc = 9;
//        else if (payloadMode.GetCodeRate() == WIFI_CODE_RATE_13_14)
//          Nsbc = 8;
//        else
//          NS_FATAL_ERROR("unsupported code rate");

//        uint32_t Ncbps;               /* Ncbps = Number of coded bits per symbol. Check Table 21-21 for different constellations. */
//        if (payloadMode.GetConstellationSize() == 2)
//          Ncbps = 336;
//        else if (payloadMode.GetConstellationSize() == 4)
//          Ncbps = 2 * 336;
//          NS_FATAL_ERROR("unsupported constellation size");

//        uint32_t Neb;                 /* Total number of encoded bits */
//        uint32_t Nblks;               /* Total number of 512 blocks containing 392 data symbols */
//        Neb = Nsbc * Nrses;
//        Nblks = (uint32_t) ceil(neb/());
      return NanoSeconds (0);
    }
  else if (payloadMode.GetModulationClass () == WIFI_MOD_CLASS_DMG_SC)
    {
      /* 21.3.4 Timeing Related Parameters, Table 21-4 TData = (Nblks * 512 + 64) * Tc. */
      /* 21.6.3.2.3.3 (4), Compute Nblks = The number of symbol blocks. */

      uint32_t Ncbpb; // Ncbpb = Number of coded bits per symbol block. Check Table 21-20 for different constellations.
      if (payloadMode.GetConstellationSize () == 2)
        Ncbpb = 448;
      else if (payloadMode.GetConstellationSize () == 4)
        Ncbpb = 2 * 448;
      else if (payloadMode.GetConstellationSize () == 16)
        Ncbpb = 4 * 448;
      else
        NS_FATAL_ERROR("unsupported constellation size");

      uint32_t Nbits = (size * 8); /* Nbits = Number of bits in the payload part. */
      uint32_t Ncbits;             /* Ncbits = Number of coded bits in the payload part. */

      if (payloadMode.GetCodeRate () == WIFI_CODE_RATE_1_4)
        Ncbits = Nbits * 4;
      else if (payloadMode.GetCodeRate () == WIFI_CODE_RATE_1_2)
        Ncbits = Nbits * 2;
      else if (payloadMode.GetCodeRate () == WIFI_CODE_RATE_13_16)
        Ncbits = (uint32_t) ceil (double (Nbits) * 16.0 / 13);
      else if (payloadMode.GetCodeRate () == WIFI_CODE_RATE_3_4)
        Ncbits = (uint32_t) ceil (double (Nbits) * 4.0 / 3);
      else if (payloadMode.GetCodeRate () == WIFI_CODE_RATE_5_8)
        Ncbits = (uint32_t) ceil (double (Nbits) * 8.0 / 5);
      else
        NS_FATAL_ERROR("unsupported code rate");

      /* We have Lcw = 672 which is the LDPC codeword length. */
      uint32_t Ncw = (uint32_t) ceil (double (Ncbits) / 672.0);         /* Ncw = The number of LDPC codewords.  */
      uint32_t Nblks = (uint32_t) ceil (double (Ncw) * 672.0 / Ncbpb);  /* Nblks = The number of symbol blocks. */

      /* Make sure the result is in nanoseconds. */
      uint32_t tData = lrint (ceil ((double (Nblks) * 512 + 64) / 1.76)); /* The duration of the data part */
      NS_LOG_DEBUG ("bits " << Nbits << " cbits " << Ncbits
                    << " Ncw " << Ncw
                    << " Nblks " << Nblks
                    << " rate " << payloadMode.GetDataRate() << " Payload Time " << tData << " ns");

      if (txVector.GetTrainngFieldLength () != 0)
        {
          if (tData < OFDMSCMin)
            tData = OFDMSCMin;
        }
      return NanoSeconds (tData);
    }
  else if (payloadMode.GetModulationClass () == WIFI_MOD_CLASS_DMG_OFDM)
    {
      /* 21.3.4 Timeing Related Parameters, Table 21-4 TData = Nsym * Tsys(OFDM) */
      /* 21.5.3.2.3.3 (5), Compute Nsym = Number of OFDM Symbols */

      uint32_t Ncbps; // Ncbpb = Number of coded bits per symbol. Check Table 21-20 for different constellations.
      if (payloadMode.GetConstellationSize () == 2)
        Ncbps = 336;
      else if (payloadMode.GetConstellationSize () == 4)
        Ncbps = 2 * 336;
      else if (payloadMode.GetConstellationSize () == 16)
        Ncbps = 4 * 336;
      else if (payloadMode.GetConstellationSize () == 64)
        Ncbps = 6 * 336;
      else
        NS_FATAL_ERROR("unsupported constellation size");

      uint32_t Nbits = (size * 8); /* Nbits = Number of bits in the payload part. */
      uint32_t Ncbits;             /* Ncbits = Number of coded bits in the payload part. */

      if (payloadMode.GetCodeRate () == WIFI_CODE_RATE_1_4)
        Ncbits = Nbits * 4;
      else if (payloadMode.GetCodeRate () == WIFI_CODE_RATE_1_2)
        Ncbits = Nbits * 2;
      else if (payloadMode.GetCodeRate () == WIFI_CODE_RATE_13_16)
        Ncbits = (uint32_t) ceil (double (Nbits) * 16.0 / 13);
      else if (payloadMode.GetCodeRate () == WIFI_CODE_RATE_3_4)
        Ncbits = (uint32_t) ceil (double (Nbits) * 4.0 / 3);
      else if (payloadMode.GetCodeRate () == WIFI_CODE_RATE_5_8)
        Ncbits = (uint32_t) ceil (double (Nbits) * 8.0 / 5);
      else
        NS_FATAL_ERROR ("unsupported code rate");

      uint32_t Ncw = (uint32_t) ceil (double (Ncbits) / 672.0);         /* Ncw = The number of LDPC codewords.  */
      uint32_t Nsym = (uint32_t) ceil (double (Ncw * 672.0) / Ncbps);   /* Nsym = Number of OFDM Symbols. */

      /* Make sure the result is in nanoseconds */
      uint32_t tData;       /* The duration of the data part */
      tData = Nsym * 242;   /* Tsys(OFDM) = 242ns */
      NS_LOG_DEBUG ("bits " << Nbits << " cbits " << Ncbits << " rate " << payloadMode.GetDataRate() << " Payload Time " << tData << " ns");

      if (txVector.GetTrainngFieldLength () != 0)
        {
          if (tData < OFDMBRPMin)
            tData = OFDMBRPMin;
        }
      return NanoSeconds (tData);
    }
  
  double stbc = 1;
  if (txVector.IsStbc ()
      && (payloadMode.GetModulationClass () == WIFI_MOD_CLASS_HT
          || payloadMode.GetModulationClass () == WIFI_MOD_CLASS_VHT))
    {
      stbc = 2;
    }

  double Nes = 1;
  //todo: improve logic to reduce the number of if cases
  //todo: extend to NSS > 4 for VHT rates
  if (payloadMode.GetUniqueName () == "HtMcs21"
      || payloadMode.GetUniqueName () == "HtMcs22"
      || payloadMode.GetUniqueName () == "HtMcs23"
      || payloadMode.GetUniqueName () == "HtMcs28"
      || payloadMode.GetUniqueName () == "HtMcs29"
      || payloadMode.GetUniqueName () == "HtMcs30"
      || payloadMode.GetUniqueName () == "HtMcs31")
    {
      Nes = 2;
    }
  if (payloadMode.GetModulationClass () == WIFI_MOD_CLASS_VHT)
    {
      if (txVector.GetChannelWidth () == 40
          && txVector.GetNss () == 3
          && payloadMode.GetMcsValue () >= 8)
        {
          Nes = 2;
        }
      if (txVector.GetChannelWidth () == 80
          && txVector.GetNss () == 2
          && payloadMode.GetMcsValue () >= 7)
        {
          Nes = 2;
        }
      if (txVector.GetChannelWidth () == 80
          && txVector.GetNss () == 3
          && payloadMode.GetMcsValue () >= 7)
        {
          Nes = 2;
        }
      if (txVector.GetChannelWidth () == 80
          && txVector.GetNss () == 3
          && payloadMode.GetMcsValue () == 9)
        {
          Nes = 3;
        }
      if (txVector.GetChannelWidth () == 80
          && txVector.GetNss () == 4
          && payloadMode.GetMcsValue () >= 4)
        {
          Nes = 2;
        }
      if (txVector.GetChannelWidth () == 80
          && txVector.GetNss () == 4
          && payloadMode.GetMcsValue () >= 7)
        {
          Nes = 3;
        }
      if (txVector.GetChannelWidth () == 160
          && payloadMode.GetMcsValue () >= 7)
        {
          Nes = 2;
        }
      if (txVector.GetChannelWidth () == 160
          && txVector.GetNss () == 2
          && payloadMode.GetMcsValue () >= 4)
        {
          Nes = 2;
        }
      if (txVector.GetChannelWidth () == 160
          && txVector.GetNss () == 2
          && payloadMode.GetMcsValue () >= 7)
        {
          Nes = 3;
        }
      if (txVector.GetChannelWidth () == 160
          && txVector.GetNss () == 3
          && payloadMode.GetMcsValue () >= 3)
        {
          Nes = 2;
        }
      if (txVector.GetChannelWidth () == 160
          && txVector.GetNss () == 3
          && payloadMode.GetMcsValue () >= 5)
        {
          Nes = 3;
        }
      if (txVector.GetChannelWidth () == 160
          && txVector.GetNss () == 3
          && payloadMode.GetMcsValue () >= 7)
        {
          Nes = 4;
        }
      if (txVector.GetChannelWidth () == 160
          && txVector.GetNss () == 4
          && payloadMode.GetMcsValue () >= 2)
        {
          Nes = 2;
        }
      if (txVector.GetChannelWidth () == 160
          && txVector.GetNss () == 4
          && payloadMode.GetMcsValue () >= 4)
        {
          Nes = 3;
        }
      if (txVector.GetChannelWidth () == 160
          && txVector.GetNss () == 4
          && payloadMode.GetMcsValue () >= 5)
        {
          Nes = 4;
        }
      if (txVector.GetChannelWidth () == 160
          && txVector.GetNss () == 4
          && payloadMode.GetMcsValue () >= 7)
        {
          Nes = 6;
        }
    }

  Time symbolDuration = Seconds (0);
  switch (payloadMode.GetModulationClass ())
    {
    case WIFI_MOD_CLASS_OFDM:
    case WIFI_MOD_CLASS_ERP_OFDM:
      {
        //(Section 18.3.2.4 "Timing related parameters" Table 18-5 "Timing-related parameters"; IEEE Std 802.11-2012
        //corresponds to T_{SYM} in the table)
        switch (txVector.GetChannelWidth ())
          {
          case 20:
          default:
            symbolDuration = MicroSeconds (4);
            break;
          case 10:
            symbolDuration = MicroSeconds (8);
            break;
          case 5:
            symbolDuration = MicroSeconds (16);
            break;
          }
        break;
      }
    case WIFI_MOD_CLASS_HT:
    case WIFI_MOD_CLASS_VHT:
      {
        //if short GI data rate is used then symbol duration is 3.6us else symbol duration is 4us
        //In the future has to create a stationmanager that only uses these data rates if sender and receiver support GI
        uint16_t gi = txVector.GetGuardInterval ();
        NS_ASSERT (gi == 400 || gi == 800);
        symbolDuration = NanoSeconds (3200 + gi);
      }
      break;
    case WIFI_MOD_CLASS_HE:
      {
        //if short GI data rate is used then symbol duration is 3.6us else symbol duration is 4us
        //In the future has to create a stationmanager that only uses these data rates if sender and receiver support GI
        uint16_t gi = txVector.GetGuardInterval ();
        NS_ASSERT (gi == 800 || gi == 1600 || gi == 3200);
        symbolDuration = NanoSeconds (12800 + gi);
      }
      break;
    default:
      break;
    }

  double numDataBitsPerSymbol = payloadMode.GetDataRate (txVector) * symbolDuration.GetNanoSeconds () / 1e9;

  double numSymbols = 0;
  if (mpdutype == MPDU_IN_AGGREGATE && preamble != WIFI_PREAMBLE_NONE)
    {
      //First packet in an A-MPDU
      numSymbols = (stbc * (16 + size * 8.0 + 6 * Nes) / (stbc * numDataBitsPerSymbol));
      if (incFlag == 1)
        {
          m_totalAmpduSize += size;
          m_totalAmpduNumSymbols += numSymbols;
        }
    }
  else if (mpdutype == MPDU_IN_AGGREGATE && preamble == WIFI_PREAMBLE_NONE)
    {
      //consecutive packets in an A-MPDU
      numSymbols = (stbc * size * 8.0) / (stbc * numDataBitsPerSymbol);
      if (incFlag == 1)
        {
          m_totalAmpduSize += size;
          m_totalAmpduNumSymbols += numSymbols;
        }
    }
  else if (mpdutype == LAST_MPDU_IN_AGGREGATE && preamble == WIFI_PREAMBLE_NONE)
    {
      //last packet in an A-MPDU
      uint32_t totalAmpduSize = m_totalAmpduSize + size;
      numSymbols = lrint (stbc * ceil ((16 + totalAmpduSize * 8.0 + 6 * Nes) / (stbc * numDataBitsPerSymbol)));
      NS_ASSERT (m_totalAmpduNumSymbols <= numSymbols);
      numSymbols -= m_totalAmpduNumSymbols;
      if (incFlag == 1)
        {
          m_totalAmpduSize = 0;
          m_totalAmpduNumSymbols = 0;
        }
    }
  else if (mpdutype == NORMAL_MPDU && preamble != WIFI_PREAMBLE_NONE)
    {
      //Not an A-MPDU
      numSymbols = lrint (stbc * ceil ((16 + size * 8.0 + 6.0 * Nes) / (stbc * numDataBitsPerSymbol)));
    }
  else
    {
      NS_FATAL_ERROR ("Wrong combination of preamble and packet type");
    }

  switch (payloadMode.GetModulationClass ())
    {
    case WIFI_MOD_CLASS_OFDM:
    case WIFI_MOD_CLASS_ERP_OFDM:
      {
        //Add signal extension for ERP PHY
        if (payloadMode.GetModulationClass () == WIFI_MOD_CLASS_ERP_OFDM)
          {
            return FemtoSeconds (numSymbols * symbolDuration.GetFemtoSeconds ()) + MicroSeconds (6);
          }
        else
          {
            return FemtoSeconds (numSymbols * symbolDuration.GetFemtoSeconds ());
          }
      }
    case WIFI_MOD_CLASS_HT:
    case WIFI_MOD_CLASS_VHT:
      {
        if (payloadMode.GetModulationClass () == WIFI_MOD_CLASS_HT && Is2_4Ghz (frequency)
            && ((mpdutype == NORMAL_MPDU && preamble != WIFI_PREAMBLE_NONE)
                || (mpdutype == LAST_MPDU_IN_AGGREGATE && preamble == WIFI_PREAMBLE_NONE))) //at 2.4 GHz
          {
            return FemtoSeconds (numSymbols * symbolDuration.GetFemtoSeconds ()) + MicroSeconds (6);
          }
        else //at 5 GHz
          {
            return FemtoSeconds (numSymbols * symbolDuration.GetFemtoSeconds ());
          }
      }
    case WIFI_MOD_CLASS_HE:
      {
        if (Is2_4Ghz (frequency)
            && ((mpdutype == NORMAL_MPDU && preamble != WIFI_PREAMBLE_NONE)
                || (mpdutype == LAST_MPDU_IN_AGGREGATE && preamble == WIFI_PREAMBLE_NONE))) //at 2.4 GHz
          {
            return FemtoSeconds (numSymbols * symbolDuration.GetFemtoSeconds ()) + MicroSeconds (6);
          }
        else //at 5 GHz
          {
            return FemtoSeconds (numSymbols * symbolDuration.GetFemtoSeconds ());
          }
      }
    case WIFI_MOD_CLASS_DSSS:
    case WIFI_MOD_CLASS_HR_DSSS:
      return MicroSeconds (lrint (ceil ((size * 8.0) / (payloadMode.GetDataRate (22) / 1.0e6))));
    default:
      NS_FATAL_ERROR ("unsupported modulation class");
      return MicroSeconds (0);
  }
}

uint64_t
WifiPhy::GetTotalTransmittedBits () const
{
  return m_totalBits;
}

Time
WifiPhy::GetLastRxDuration (void) const
{
  return m_rxDuration;
}

Time
WifiPhy::CalculatePlcpPreambleAndHeaderDuration (WifiTxVector txVector)
{
  WifiPreamble preamble = txVector.GetPreambleType ();
  Time duration = GetPlcpPreambleDuration (txVector)
    + GetPlcpHeaderDuration (txVector)
    + GetPlcpHtSigHeaderDuration (preamble)
    + GetPlcpSigA1Duration (preamble)
    + GetPlcpSigA2Duration (preamble)
    + GetPlcpTrainingSymbolDuration (txVector)
    + GetPlcpSigBDuration (preamble);
  return duration;
}

Time
WifiPhy::CalculateTxDuration (uint32_t size, WifiTxVector txVector, uint16_t frequency, MpduType mpdutype, uint8_t incFlag)
{
  Time duration = CalculatePlcpPreambleAndHeaderDuration (txVector)
    + GetPayloadDuration (size, txVector, frequency, mpdutype, incFlag);
  return duration;
}

Time
WifiPhy::CalculateTxDuration (uint32_t size, WifiTxVector txVector, uint16_t frequency)
{
  return CalculateTxDuration (size, txVector, frequency, NORMAL_MPDU, 0);
}

void
WifiPhy::NotifyTxBegin (Ptr<const Packet> packet)
{
  m_phyTxBeginTrace (packet);
}

void
WifiPhy::NotifyTxEnd (Ptr<const Packet> packet)
{
  m_phyTxEndTrace (packet);
}

void
WifiPhy::NotifyTxDrop (Ptr<const Packet> packet)
{
  m_phyTxDropTrace (packet);
}

void
WifiPhy::NotifyRxBegin (Ptr<const Packet> packet)
{
  m_phyRxBeginTrace (packet);
}

void
WifiPhy::NotifyRxEnd (Ptr<const Packet> packet)
{
  m_phyRxEndTrace (packet);
}

void
WifiPhy::NotifyRxDrop (Ptr<const Packet> packet)
{
  m_phyRxDropTrace (packet);
}

void
WifiPhy::NotifyMonitorSniffRx (Ptr<const Packet> packet, uint16_t channelFreqMhz, WifiTxVector txVector, MpduInfo aMpdu, SignalNoiseDbm signalNoise)
{
  m_phyMonitorSniffRxTrace (packet, channelFreqMhz, txVector, aMpdu, signalNoise);
}

void
WifiPhy::NotifyMonitorSniffTx (Ptr<const Packet> packet, uint16_t channelFreqMhz, WifiTxVector txVector, MpduInfo aMpdu)
{
  m_phyMonitorSniffTxTrace (packet, channelFreqMhz, txVector, aMpdu);
}

void
WifiPhy::SetAntenna (Ptr<AbstractAntenna> antenna)
{
  m_antenna = antenna;
}

Ptr<AbstractAntenna>
WifiPhy::GetAntenna (void) const
{
  return m_antenna;
}

void
WifiPhy::SetDirectionalAntenna (Ptr<DirectionalAntenna> antenna)
{
  m_directionalAntenna = antenna;
}

Ptr<DirectionalAntenna>
WifiPhy::GetDirectionalAntenna (void) const
{
  return m_directionalAntenna;
}

void
WifiPhy::SendPacket (Ptr<const Packet> packet, WifiTxVector txVector, Time frameDuration, MpduType mpdutype)
{
  NS_LOG_FUNCTION (this << packet << txVector.GetMode ()
                        << txVector.GetMode ().GetDataRate (txVector)
                        << txVector.GetPreambleType ()
                        << (uint16_t)txVector.GetTxPowerLevel ()
                        << frameDuration
                        << (uint16_t)mpdutype);

  WifiMacHeader firsthdr;
  packet->PeekHeader(firsthdr);
  NS_LOG_FUNCTION (firsthdr.GetAddr1 () << firsthdr.GetAddr2 () << Simulator::Now ());

  /* Transmission can happen if:
   *  - we are syncing on a packet. It is the responsability of the
   *    MAC layer to avoid doing this but the PHY does nothing to
   *    prevent it.
   *  - we are idle
   */
  NS_ASSERT (!m_state->IsStateTx () && !m_state->IsStateSwitching ());

  if (txVector.GetNss () > GetMaxSupportedTxSpatialStreams ())
    {
      NS_FATAL_ERROR ("Unsupported number of spatial streams!");
    }

  bool sendTrnFields = false;

  if (m_state->IsStateSleep ())
    {
      NS_LOG_DEBUG ("Dropping packet because in sleep mode");
      NotifyTxDrop (packet);
      return;
    }

  Time txDuration = frameDuration;
  WifiPreamble preamble = txVector.GetPreambleType ();
  NS_ASSERT (txDuration > NanoSeconds (0));

  /* Check if the MPDU is single or last aggregate MPDU */
  if (((mpdutype == NORMAL_MPDU && preamble != WIFI_PREAMBLE_NONE) ||
       (mpdutype == LAST_MPDU_IN_AGGREGATE && preamble == WIFI_PREAMBLE_NONE)) && (txVector.GetTrainngFieldLength () > 0))
    {
      NS_LOG_DEBUG ("Send TRN Fields:" << txVector.GetTrainngFieldLength ());
      txDuration += txVector.GetTrainngFieldLength () * TRNUnit;
      sendTrnFields = true;
    }

  if (m_state->IsStateRx ())
    {
      NS_LOG_DEBUG ("Cancel current reception");
      m_endPlcpRxEvent.Cancel ();
      m_endRxEvent.Cancel ();
      m_interference.NotifyRxEnd ();
    }
  NotifyTxBegin (packet);
  if ((mpdutype == MPDU_IN_AGGREGATE) && (preamble != WIFI_PREAMBLE_NONE))
    {
      //send the first MPDU in an MPDU
      m_txMpduReferenceNumber++;
    }
  NS_LOG_FUNCTION (GetPowerDbm (txVector.GetTxPowerLevel ()) << "dbm");
  MpduInfo aMpdu;
  aMpdu.type = mpdutype;
  aMpdu.mpduRefNumber = m_txMpduReferenceNumber;
  NotifyMonitorSniffTx (packet, GetFrequency (), txVector, aMpdu);
  m_state->SwitchToTx (txDuration, packet, GetPowerDbm (txVector.GetTxPowerLevel ()), txVector);

  Ptr<Packet> newPacket = packet->Copy (); // obtain non-const Packet
  WifiPhyTag oldtag;
  newPacket->RemovePacketTag (oldtag);
  WifiPhyTag tag (txVector, mpdutype);
  newPacket->AddPacketTag (tag);

  StartTx (newPacket, txVector, frameDuration);

  /* Send TRN Fields if beam refinement or tracking is required */
  if (sendTrnFields)
    {
      /* Prepare transmission of the first TRN Packet */
      Simulator::Schedule (frameDuration, &WifiPhy::SendTrnField, this, txVector, txVector.GetTrainngFieldLength ());
    }
  /* Record duration of the current transmission */
  m_lastTxDuration = txDuration;
  /* New Trace for Tx End */
  Simulator::Schedule (txDuration, &WifiPhy::NotifyTxEnd, this, packet);
}

void
WifiPhy::StartReceivePreambleAndHeader (Ptr<Packet> packet, double rxPowerW, Time rxDuration)
{
  //This function should be later split to check separately whether plcp preamble and plcp header can be successfully received.
  //Note: plcp preamble reception is not yet modeled.
  NS_LOG_FUNCTION (this << packet << WToDbm (rxPowerW) << rxDuration << Simulator::Now ());
  WifiPhyTag tag;
  AmpduTag ampduTag;
  m_rxPowerDbm = WToDbm (rxPowerW);

  bool found = packet->RemovePacketTag (tag);
  if (!found)
    {
      NS_FATAL_ERROR ("Received Wi-Fi Signal with no WifiPhyTag");
      return;
    }

  WifiTxVector txVector = tag.GetWifiTxVector ();
  Time totalDuration = rxDuration + txVector.GetTrainngFieldLength () * TRNUnit;
  m_rxDuration = totalDuration; // Duraion of the last frame
  Time endRx = Simulator::Now () + totalDuration;

  if (txVector.GetMode ().GetModulationClass () == WIFI_MOD_CLASS_HT
      && (txVector.GetNss () != (1 + (txVector.GetMode ().GetMcsValue () / 8))))
    {
      NS_FATAL_ERROR ("MCS value does not match NSS value: MCS = " << (uint16_t)txVector.GetMode ().GetMcsValue () << ", NSS = " << (uint16_t)txVector.GetNss ());
    }

  if (txVector.GetNss () > GetMaxSupportedRxSpatialStreams ())
    {
      NS_FATAL_ERROR ("Reception ends in failure because of an unsupported number of spatial streams");
    }

  WifiPreamble preamble = txVector.GetPreambleType ();
  MpduType mpdutype = tag.GetMpduType ();
  Time preambleAndHeaderDuration = CalculatePlcpPreambleAndHeaderDuration (txVector);

  Ptr<InterferenceHelper::Event> event;
  event = m_interference.Add (packet->GetSize (),
                              txVector,
                              totalDuration,
                              rxPowerW);

  switch (m_state->GetState ())
    {
    case WifiPhy::SWITCHING:
      NS_LOG_DEBUG ("drop packet because of channel switching");
      NotifyRxDrop (packet);
      m_plcpSuccess = false;
      /*
       * Packets received on the upcoming channel are added to the event list
       * during the switching state. This way the medium can be correctly sensed
       * when the device listens to the channel for the first time after the
       * switching e.g. after channel switching, the channel may be sensed as
       * busy due to other devices' tramissions started before the end of
       * the switching.
       */
      if (endRx > Simulator::Now () + m_state->GetDelayUntilIdle ())
        {
          //that packet will be noise _after_ the completion of the
          //channel switching.
          goto maybeCcaBusy;
        }
      break;
    case WifiPhy::RX:
      NS_LOG_DEBUG ("drop packet " << packet << " because already in Rx (power=" <<
                    rxPowerW << "W)");
      NotifyRxDrop (packet);
      if (endRx > Simulator::Now () + m_state->GetDelayUntilIdle ())
        {
          //that packet will be noise _after_ the reception of the
          //currently-received packet.
          goto maybeCcaBusy;
        }
      break;
    case WifiPhy::TX:
      NS_LOG_DEBUG ("drop packet because already in Tx (power=" <<
                    rxPowerW << "W)");
      NotifyRxDrop (packet);
      if (endRx > Simulator::Now () + m_state->GetDelayUntilIdle ())
        {
          //that packet will be noise _after_ the transmission of the
          //currently-transmitted packet.
          goto maybeCcaBusy;
        }
      break;
    case WifiPhy::CCA_BUSY:
    case WifiPhy::IDLE:
      if (rxPowerW > GetEdThresholdW ()) //checked here, no need to check in the payload reception (current implementation assumes constant rx power over the packet duration)
        {
          if (m_rdsActivated)
            {
              NS_LOG_DEBUG ("Receiving as RDS in FD-AF Mode");
              /**
               * We are working in Full Duplex-Amplify and Forward. So we receive the packet without decoding it
               * or checking its header. Then we amplify it and redirect it to the the destination.
               * We take a simple approach to model full duplex communication by swapping current steering sector.
               * Another approach would by adding new PHY layer which adds more complexity to the solution.
               */

              if ((mpdutype == NORMAL_MPDU) || (mpdutype == LAST_MPDU_IN_AGGREGATE))
                {
                  if ((m_rdsSector == m_srcSector) && (m_rdsAntenna == m_srcAntenna))
                    {
                      m_rdsSector = m_dstSector;
                      m_rdsAntenna = m_dstAntenna;
                    }
                  else
                    {
                      m_rdsSector = m_srcSector;
                      m_rdsAntenna = m_srcAntenna;
                    }

                  m_directionalAntenna->SetCurrentTxSectorID (m_rdsSector);
                  m_directionalAntenna->SetCurrentTxAntennaID (m_rdsAntenna);
                }

              /* We simply transmit the frame on the channel without passing it to the upper layers (We amplify the power) */
              StartTx (packet, txVector, rxDuration);
            }
          else
            {
              if (preamble == WIFI_PREAMBLE_NONE && (m_mpdusNum == 0 || m_plcpSuccess == false))
                {
                  m_plcpSuccess = false;
                  m_mpdusNum = 0;
                  NS_LOG_DEBUG ("drop packet because no PLCP preamble/header has been received");
                  NotifyRxDrop (packet);
                  goto maybeCcaBusy;
                }
              else if (preamble != WIFI_PREAMBLE_NONE && packet->PeekPacketTag (ampduTag) && m_mpdusNum == 0)
                {
                  //received the first MPDU in an A-MPDU
                  m_mpdusNum = ampduTag.GetRemainingNbOfMpdus ();
                  m_rxMpduReferenceNumber++;
                }
              else if (preamble == WIFI_PREAMBLE_NONE && packet->PeekPacketTag (ampduTag) && m_mpdusNum > 0)
                {
                  //received the other MPDUs that are part of the A-MPDU
                  if (ampduTag.GetRemainingNbOfMpdus () < (m_mpdusNum - 1))
                    {
                      NS_LOG_DEBUG ("Missing MPDU from the A-MPDU " << m_mpdusNum - ampduTag.GetRemainingNbOfMpdus ());
                      m_mpdusNum = ampduTag.GetRemainingNbOfMpdus ();
                    }
                  else
                    {
                      m_mpdusNum--;
                    }
                }
              else if (preamble != WIFI_PREAMBLE_NONE && packet->PeekPacketTag (ampduTag) && m_mpdusNum > 0)
                {
                  NS_LOG_DEBUG ("New A-MPDU started while " << m_mpdusNum << " MPDUs from previous are lost");
                  m_mpdusNum = ampduTag.GetRemainingNbOfMpdus ();
                }
              else if (preamble != WIFI_PREAMBLE_NONE && m_mpdusNum > 0 )
                {
                  NS_LOG_DEBUG ("Didn't receive the last MPDUs from an A-MPDU " << m_mpdusNum);
                  m_mpdusNum = 0;
                }

              NS_LOG_DEBUG ("sync to signal (power=" << rxPowerW << "W)");
              //sync to signal
              m_state->SwitchToRx (totalDuration);
              NS_ASSERT (m_endPlcpRxEvent.IsExpired ());
              NotifyRxBegin (packet);
              m_interference.NotifyRxStart ();

              if (preamble != WIFI_PREAMBLE_NONE)
                {
                  NS_ASSERT (m_endPlcpRxEvent.IsExpired ());
                  m_endPlcpRxEvent = Simulator::Schedule (preambleAndHeaderDuration, &WifiPhy::StartReceivePacket, this,
                                                          packet, txVector, mpdutype, event);
                }

              NS_ASSERT (m_endRxEvent.IsExpired ());
              if (txVector.GetTrainngFieldLength () == 0)
                {
                  m_endRxEvent = Simulator::Schedule (rxDuration, &WifiPhy::EndPsduReceive, this,
                                                      packet, preamble, mpdutype, event);
                }
              else
                {
                  m_endRxEvent = Simulator::Schedule (rxDuration, &WifiPhy::EndPsduOnlyReceive, this,
                                                      packet, txVector.GetPacketType (), preamble, mpdutype, event);
                }
            }
        }
      else
        {
          NS_LOG_DEBUG ("drop packet because signal power too Small (" <<
                        WToDbm (rxPowerW) << "<" << WToDbm (GetEdThresholdW ()) << ")");
          NotifyRxDrop (packet);
          m_plcpSuccess = false;
          goto maybeCcaBusy;
        }
      break;
    case WifiPhy::SLEEP:
      NS_LOG_DEBUG ("drop packet because in sleep mode");
      NotifyRxDrop (packet);
      m_plcpSuccess = false;
      break;
    }

  return;

maybeCcaBusy:
  //We are here because we have received the first bit of a packet and we are
  //not going to be able to synchronize on it
  //In this model, CCA becomes busy when the aggregation of all signals as
  //tracked by the InterferenceHelper class is higher than the CcaBusyThreshold

  Time delayUntilCcaEnd = m_interference.GetEnergyDuration (DbmToW (GetCcaMode1Threshold ()));
  if (!delayUntilCcaEnd.IsZero ())
    {
      NS_LOG_DEBUG ("In CCA Busy State for " << delayUntilCcaEnd);
      m_state->SwitchMaybeToCcaBusy (delayUntilCcaEnd);
    }
  else
    {
      NS_LOG_DEBUG ("Not in CCA Busy State");
    }
}

void
WifiPhy::StartReceivePacket (Ptr<Packet> packet,
                             WifiTxVector txVector,
                             MpduType mpdutype,
                             Ptr<InterferenceHelper::Event> event)
{
  NS_LOG_FUNCTION (this << packet << packet->GetSize() << txVector.GetMode () << txVector.GetPreambleType () << (uint16_t)mpdutype);
  for (uint16_t i=0; i<MCSHistory.size(); i++)
    NS_LOG_FUNCTION ("this" << MCSHistory.at(i).first << MCSHistory.at(i).second);
 
  /*if (packet->GetSize()>100 && firsthdr.GetAddr1 ()==m_self){
    std::pair <std::string,uint16_t> temp = {txVector.GetMode ().GetUniqueName (), 0};
    if (MCSHistory.size() == 0) MCSHistory.push_back(temp);
    for (uint16_t i=0; i<MCSHistory.size(); i++)
      {
        if (txVector.GetMode ()==MCSHistory.at(i).first)
          {
            MCSHistory.at(i).second++;
            break;
          }
        if (i==MCSHistory.size()-1)
          MCSHistory.push_back(temp);
      }
    }*/

  WifiMacHeader firsthdr;
  packet->PeekHeader(firsthdr);
  NS_LOG_FUNCTION (firsthdr.GetAddr1 () << firsthdr.GetAddr2 ());
  m_rxSensitivityMap = {{firsthdr.GetAddr2 (), firsthdr.GetAddr1 ()}, m_rxPowerDbm}; 

  NS_ASSERT (IsStateRx ());
  NS_ASSERT (m_endPlcpRxEvent.IsExpired ());
  WifiMode txMode = txVector.GetMode ();

  InterferenceHelper::SnrPer snrPer;
  snrPer = m_interference.CalculatePlcpHeaderSnrPer (event);

  NS_LOG_DEBUG ("snr(dB)=" << RatioToDb (snrPer.snr) << ", per=" << snrPer.per);

  if (m_random->GetValue () > snrPer.per) //plcp reception succeeded
    {
      if (IsModeSupported (txMode) || IsMcsSupported (txMode))
        {
          NS_LOG_DEBUG ("receiving plcp payload"); //endReceive is already scheduled
          m_plcpSuccess = true;
        }
      else //mode is not allowed
        {
          NS_LOG_DEBUG ("drop packet because it was sent using an unsupported mode (" << txMode << ")");
          NotifyRxDrop (packet);
          m_plcpSuccess = false;
        }
    }
  else //plcp reception failed
    {
      NS_LOG_DEBUG ("drop packet because plcp preamble/header reception failed");
      NotifyRxDrop (packet);
      m_plcpSuccess = false;
    }
}

void
WifiPhy::EndPsduReceive (Ptr<Packet> packet, WifiPreamble preamble, MpduType mpdutype, Ptr<InterferenceHelper::Event> event)
{
  NS_LOG_FUNCTION (this << packet << event << Simulator::Now () << RatioToDb (event->GetRxPowerW ()) + 30 << event->GetTxVector ().GetMode () << event->GetRxPowerW ());
  NS_ASSERT (IsStateRx ());
  NS_ASSERT (event->GetEndTime () == Simulator::Now ());

  WifiMacHeader firsthdr;
  packet->PeekHeader(firsthdr);
  NS_LOG_FUNCTION (firsthdr.GetAddr1 () << firsthdr.GetAddr2 ());

  InterferenceHelper::SnrPer snrPer;
  snrPer = m_interference.CalculatePlcpPayloadSnrPer (event);
  m_interference.NotifyRxEnd ();


  if (m_plcpSuccess == true)
    {
      NS_LOG_DEBUG ("mode=" << (event->GetPayloadMode ().GetDataRate (event->GetTxVector ())) <<
                    ", snr(dB)=" << RatioToDb (snrPer.snr) << ", per=" << snrPer.per << ", size=" << packet->GetSize ());
      double rnd = m_random->GetValue ();
      if (rnd > snrPer.per)
        {
          NotifyRxEnd (packet);
          SignalNoiseDbm signalNoise;
          signalNoise.signal = RatioToDb (event->GetRxPowerW ()) + 30;
          signalNoise.noise = RatioToDb (event->GetRxPowerW () / snrPer.snr) - GetRxNoiseFigure () + 30;
          MpduInfo aMpdu;
          aMpdu.type = mpdutype;
          aMpdu.mpduRefNumber = m_rxMpduReferenceNumber;
          NotifyMonitorSniffRx (packet, GetFrequency (), event->GetTxVector (), aMpdu, signalNoise);
          m_state->SwitchFromRxEndOk (packet, snrPer.snr, event->GetTxVector ());
        }
      else
        {
          /* failure. */
          NS_LOG_DEBUG ("drop packet because the probability to receive it = " << rnd << " is lower than " << snrPer.per);
          NotifyRxDrop (packet);
          m_state->SwitchFromRxEndError (packet, snrPer.snr);
        }
    }
  else
    {
      m_state->SwitchFromRxEndError (packet, snrPer.snr);
    }

  if (preamble == WIFI_PREAMBLE_NONE && mpdutype == LAST_MPDU_IN_AGGREGATE)
    {
      m_plcpSuccess = false;
    }
}

void
WifiPhy::EndPsduOnlyReceive (Ptr<Packet> packet, PacketType packetType, WifiPreamble preamble, MpduType mpdutype, Ptr<InterferenceHelper::Event> event)
{
  NS_LOG_FUNCTION (this << packet << event);
  NS_ASSERT (IsStateRx ());

  WifiMacHeader firsthdr;
  packet->PeekHeader(firsthdr);
  NS_LOG_FUNCTION (firsthdr.GetAddr1 () << firsthdr.GetAddr2 ());

  bool isEndOfFrame = ((mpdutype == NORMAL_MPDU && preamble != WIFI_PREAMBLE_NONE) || (mpdutype == LAST_MPDU_IN_AGGREGATE && preamble == WIFI_PREAMBLE_NONE));
  struct InterferenceHelper::SnrPer snrPer;
  snrPer = m_interference.CalculatePlcpPayloadSnrPer (event);

  if (m_plcpSuccess == true)
    {
      NS_LOG_DEBUG ("mode=" << (event->GetPayloadMode ().GetDataRate ()) <<
                    ", snr(dB)=" << RatioToDb(snrPer.snr) << ", per=" << snrPer.per << ", size=" << packet->GetSize ());
      double rnd = m_random->GetValue ();
      m_psduSuccess = (rnd > snrPer.per);
      if (m_psduSuccess)
        {
          NotifyRxEnd (packet);
          SignalNoiseDbm signalNoise;
          signalNoise.signal = RatioToDb (event->GetRxPowerW ()) + 30;
          signalNoise.noise = RatioToDb (event->GetRxPowerW () / snrPer.snr) - GetRxNoiseFigure () + 30;
          MpduInfo aMpdu;
          aMpdu.type = mpdutype;
          aMpdu.mpduRefNumber = m_rxMpduReferenceNumber;
          NotifyMonitorSniffRx (packet, GetFrequency (), event->GetTxVector (), aMpdu, signalNoise);
          m_state->ReportPsduEndOk (packet, snrPer.snr, event->GetTxVector ());
        }
      else
        {
          /* failure. */
          NS_LOG_DEBUG ("drop packet because the probability to receive it = " << rnd << " is lower than " << snrPer.per);
          NotifyRxDrop (packet);
          m_state->ReportPsduEndError (packet, snrPer.snr);
        }
    }
  else
    {
      m_state->ReportPsduEndError (packet, snrPer.snr);
    }

  if (preamble == WIFI_PREAMBLE_NONE && mpdutype == LAST_MPDU_IN_AGGREGATE)
    {
      m_plcpSuccess = false;
    }

  if (isEndOfFrame && (packetType == TRN_R))
    {
      /* If the received frame has TRN-R Fields, we should sweep antenna configuration at the beginning of each field */
      m_directionalAntenna->SetInDirectionalReceivingMode ();
      m_directionalAntenna->SetCurrentRxSectorID (1);
      m_directionalAntenna->SetCurrentRxAntennaID (1);
    }
}

void
WifiPhy::PacketTxEnd (Ptr<Packet> packet)
{
  m_lastTxDuration = NanoSeconds (0);
  NotifyTxEnd (packet);
}

Time
WifiPhy::GetLastTxDuration (void) const
{
  return m_lastTxDuration;
}

void
WifiPhy::SendTrnField (WifiTxVector txVector, uint8_t fieldsRemaining)
{
  NS_LOG_FUNCTION (this << txVector.GetMode () << uint (fieldsRemaining));
  if (txVector.GetPacketType () == TRN_T)
    {
      /* Change Sector ID at the begining of each TRN-T field */
      m_directionalAntenna->SetCurrentTxSectorID (fieldsRemaining);
    }

  fieldsRemaining--;
  if (fieldsRemaining != 0)
    {
      Simulator::Schedule (TRNUnit, &WifiPhy::SendTrnField, this, txVector, fieldsRemaining);
    }

  StartTrnTx (txVector, fieldsRemaining);
}

void
WifiPhy::StartReceiveTrnField (WifiTxVector txVector, double rxPowerDbm, uint8_t fieldsRemaining)
{
  NS_LOG_FUNCTION (this << txVector.GetMode () << rxPowerDbm << fieldsRemaining);
  double rxPowerW = DbmToW (rxPowerDbm);
  if (m_plcpSuccess && m_state->IsStateRx ())
    {
      /* Add Interference event for TRN field */
      Ptr<InterferenceHelper::Event> event;
      event = m_interference.Add (txVector,
                                  TRNUnit,
                                  rxPowerW);

      /* Schedule an event for the complete reception of this TRN Field */
      Simulator::Schedule (TRNUnit, &WifiPhy::EndReceiveTrnField, this,
                           m_directionalAntenna->GetCurrentRxSectorID (), m_directionalAntenna->GetCurrentRxAntennaID (),
                           txVector, fieldsRemaining, event);

      if (txVector.GetPacketType () == TRN_R)
        {
          /* Change Rx Sector for the next TRN Field */
          m_directionalAntenna->SetCurrentRxSectorID (m_directionalAntenna->GetNextRxSectorID ());
        }
    }
  else
    {
      NS_LOG_DEBUG ("Drop TRN Field because signal power too Small (" << rxPowerW << "<" << GetEdThresholdW ());
    }
}

void
WifiPhy::EndReceiveTrnField (uint8_t sectorId, uint8_t antennaId,
                                 WifiTxVector txVector, uint8_t fieldsRemaining,
                                 Ptr<InterferenceHelper::Event> event)
{
  NS_LOG_FUNCTION (this << sectorId << antennaId << txVector.GetMode () << fieldsRemaining << event);

  /* Calculate SNR and report it to the upper layer */
  double snr;
  snr = m_interference.CalculatePlcpTrnSnr (event);
  m_reportSnrCallback (sectorId, antennaId, fieldsRemaining, snr, (txVector.GetPacketType () == TRN_T));

  /* Check if this is the last TRN field in the current transmission */
  if (fieldsRemaining == 0)
    {
      EndReceiveTrnFields ();
    }
}

void
WifiPhy::EndReceiveTrnFields (void)
{
  NS_LOG_FUNCTION (this);
  NS_ASSERT (IsStateRx ());
  m_interference.NotifyRxEnd ();

  if (m_plcpSuccess && m_psduSuccess)
    {
      m_state->SwitchFromRxEndOk ();
    }
  else
    {
      m_state->SwitchFromRxEndError ();
    }
}

void
WifiPhy::RegisterReportSnrCallback (ReportSnrCallback callback)
{
  NS_LOG_FUNCTION (this);
  m_reportSnrCallback = callback;
}

void
WifiPhy::ActivateRdsOpereation (uint8_t srcSector, uint8_t srcAntenna,
                                    uint8_t dstSector, uint8_t dstAntenna)
{
  NS_LOG_FUNCTION (this << srcSector << srcAntenna << dstSector << dstAntenna);
  m_rdsActivated = true;
  m_rdsSector = m_srcSector = srcSector;
  m_rdsAntenna = m_srcAntenna = srcAntenna;
  m_dstSector = dstSector;
  m_dstAntenna = dstAntenna;
}

void
WifiPhy::ResumeRdsOperation (void)
{
  NS_LOG_FUNCTION (this);
  m_rdsActivated = true;
  m_rdsSector = m_srcSector;
  m_rdsAntenna = m_srcAntenna;
}

void
WifiPhy::SuspendRdsOperation (void)
{
  NS_LOG_FUNCTION (this);
  m_rdsActivated = false;
}

/* Channel Measurement Functions for 802.11ad-2012 */

void
WifiPhy::StartMeasurement (uint16_t measurementDuration, uint8_t blocks)
{
  m_measurementBlocks = blocks;
  m_measurementUnit = floor (measurementDuration/blocks);
  Simulator::Schedule (MicroSeconds (measurementDuration), &WifiPhy::EndMeasurement, this);
}

void
WifiPhy::MeasurementUnitEnded (void)
{
  /* Add measurement to the list of measurements */
  m_measurementList.push_back (m_measurementBlocks);
  m_measurementBlocks--;
  if (m_measurementBlocks > 0)
    {
      /* Schedule new measurement Unit */
      Simulator::Schedule (MicroSeconds (m_measurementUnit), &WifiPhy::EndMeasurement, this);
    }
}

void
WifiPhy::EndMeasurement (void)
{
  m_reportMeasurementCallback (m_measurementList);
}

void
WifiPhy::RegisterMeasurementResultsReady (ReportMeasurementCallback callback)
{
  m_reportMeasurementCallback = callback;
}

// Clause 15 rates (DSSS)

WifiMode
WifiPhy::GetDsssRate1Mbps ()
{
  static WifiMode mode =
    WifiModeFactory::CreateWifiMode ("DsssRate1Mbps",
                                     WIFI_MOD_CLASS_DSSS,
                                     true,
                                     WIFI_CODE_RATE_UNDEFINED,
                                     2);
  return mode;
}

WifiMode
WifiPhy::GetDsssRate2Mbps ()
{
  static WifiMode mode =
    WifiModeFactory::CreateWifiMode ("DsssRate2Mbps",
                                     WIFI_MOD_CLASS_DSSS,
                                     true,
                                     WIFI_CODE_RATE_UNDEFINED,
                                     4);
  return mode;
}


// Clause 18 rates (HR/DSSS)

WifiMode
WifiPhy::GetDsssRate5_5Mbps ()
{
  static WifiMode mode =
    WifiModeFactory::CreateWifiMode ("DsssRate5_5Mbps",
                                     WIFI_MOD_CLASS_HR_DSSS,
                                     true,
                                     WIFI_CODE_RATE_UNDEFINED,
                                     16);
  return mode;
}

WifiMode
WifiPhy::GetDsssRate11Mbps ()
{
  static WifiMode mode =
    WifiModeFactory::CreateWifiMode ("DsssRate11Mbps",
                                     WIFI_MOD_CLASS_HR_DSSS,
                                     true,
                                     WIFI_CODE_RATE_UNDEFINED,
                                     256);
  return mode;
}


// Clause 19.5 rates (ERP-OFDM)

WifiMode
WifiPhy::GetErpOfdmRate6Mbps ()
{
  static WifiMode mode =
    WifiModeFactory::CreateWifiMode ("ErpOfdmRate6Mbps",
                                     WIFI_MOD_CLASS_ERP_OFDM,
                                     true,
                                     WIFI_CODE_RATE_1_2,
                                     2);
  return mode;
}

WifiMode
WifiPhy::GetErpOfdmRate9Mbps ()
{
  static WifiMode mode =
    WifiModeFactory::CreateWifiMode ("ErpOfdmRate9Mbps",
                                     WIFI_MOD_CLASS_ERP_OFDM,
                                     false,
                                     WIFI_CODE_RATE_3_4,
                                     2);
  return mode;
}

WifiMode
WifiPhy::GetErpOfdmRate12Mbps ()
{
  static WifiMode mode =
    WifiModeFactory::CreateWifiMode ("ErpOfdmRate12Mbps",
                                     WIFI_MOD_CLASS_ERP_OFDM,
                                     true,
                                     WIFI_CODE_RATE_1_2,
                                     4);
  return mode;
}

WifiMode
WifiPhy::GetErpOfdmRate18Mbps ()
{
  static WifiMode mode =
    WifiModeFactory::CreateWifiMode ("ErpOfdmRate18Mbps",
                                     WIFI_MOD_CLASS_ERP_OFDM,
                                     false,
                                     WIFI_CODE_RATE_3_4,
                                     4);
  return mode;
}

WifiMode
WifiPhy::GetErpOfdmRate24Mbps ()
{
  static WifiMode mode =
    WifiModeFactory::CreateWifiMode ("ErpOfdmRate24Mbps",
                                     WIFI_MOD_CLASS_ERP_OFDM,
                                     true,
                                     WIFI_CODE_RATE_1_2,
                                     16);
  return mode;
}

WifiMode
WifiPhy::GetErpOfdmRate36Mbps ()
{
  static WifiMode mode =
    WifiModeFactory::CreateWifiMode ("ErpOfdmRate36Mbps",
                                     WIFI_MOD_CLASS_ERP_OFDM,
                                     false,
                                     WIFI_CODE_RATE_3_4,
                                     16);
  return mode;
}

WifiMode
WifiPhy::GetErpOfdmRate48Mbps ()
{
  static WifiMode mode =
    WifiModeFactory::CreateWifiMode ("ErpOfdmRate48Mbps",
                                     WIFI_MOD_CLASS_ERP_OFDM,
                                     false,
                                     WIFI_CODE_RATE_2_3,
                                     64);
  return mode;
}

WifiMode
WifiPhy::GetErpOfdmRate54Mbps ()
{
  static WifiMode mode =
    WifiModeFactory::CreateWifiMode ("ErpOfdmRate54Mbps",
                                     WIFI_MOD_CLASS_ERP_OFDM,
                                     false,
                                     WIFI_CODE_RATE_3_4,
                                     64);
  return mode;
}


// Clause 17 rates (OFDM)

WifiMode
WifiPhy::GetOfdmRate6Mbps ()
{
  static WifiMode mode =
    WifiModeFactory::CreateWifiMode ("OfdmRate6Mbps",
                                     WIFI_MOD_CLASS_OFDM,
                                     true,
                                     WIFI_CODE_RATE_1_2,
                                     2);
  return mode;
}

WifiMode
WifiPhy::GetOfdmRate9Mbps ()
{
  static WifiMode mode =
    WifiModeFactory::CreateWifiMode ("OfdmRate9Mbps",
                                     WIFI_MOD_CLASS_OFDM,
                                     false,
                                     WIFI_CODE_RATE_3_4,
                                     2);
  return mode;
}

WifiMode
WifiPhy::GetOfdmRate12Mbps ()
{
  static WifiMode mode =
    WifiModeFactory::CreateWifiMode ("OfdmRate12Mbps",
                                     WIFI_MOD_CLASS_OFDM,
                                     true,
                                     WIFI_CODE_RATE_1_2,
                                     4);
  return mode;
}

WifiMode
WifiPhy::GetOfdmRate18Mbps ()
{
  static WifiMode mode =
    WifiModeFactory::CreateWifiMode ("OfdmRate18Mbps",
                                     WIFI_MOD_CLASS_OFDM,
                                     false,
                                     WIFI_CODE_RATE_3_4,
                                     4);
  return mode;
}

WifiMode
WifiPhy::GetOfdmRate24Mbps ()
{
  static WifiMode mode =
    WifiModeFactory::CreateWifiMode ("OfdmRate24Mbps",
                                     WIFI_MOD_CLASS_OFDM,
                                     true,
                                     WIFI_CODE_RATE_1_2,
                                     16);
  return mode;
}

WifiMode
WifiPhy::GetOfdmRate36Mbps ()
{
  static WifiMode mode =
    WifiModeFactory::CreateWifiMode ("OfdmRate36Mbps",
                                     WIFI_MOD_CLASS_OFDM,
                                     false,
                                     WIFI_CODE_RATE_3_4,
                                     16);
  return mode;
}

WifiMode
WifiPhy::GetOfdmRate48Mbps ()
{
  static WifiMode mode =
    WifiModeFactory::CreateWifiMode ("OfdmRate48Mbps",
                                     WIFI_MOD_CLASS_OFDM,
                                     false,
                                     WIFI_CODE_RATE_2_3,
                                     64);
  return mode;
}

WifiMode
WifiPhy::GetOfdmRate54Mbps ()
{
  static WifiMode mode =
    WifiModeFactory::CreateWifiMode ("OfdmRate54Mbps",
                                     WIFI_MOD_CLASS_OFDM,
                                     false,
                                     WIFI_CODE_RATE_3_4,
                                     64);
  return mode;
}


// 10 MHz channel rates

WifiMode
WifiPhy::GetOfdmRate3MbpsBW10MHz ()
{
  static WifiMode mode =
    WifiModeFactory::CreateWifiMode ("OfdmRate3MbpsBW10MHz",
                                     WIFI_MOD_CLASS_OFDM,
                                     true,
                                     WIFI_CODE_RATE_1_2,
                                     2);
  return mode;
}

WifiMode
WifiPhy::GetOfdmRate4_5MbpsBW10MHz ()
{
  static WifiMode mode =
    WifiModeFactory::CreateWifiMode ("OfdmRate4_5MbpsBW10MHz",
                                     WIFI_MOD_CLASS_OFDM,
                                     false,
                                     WIFI_CODE_RATE_3_4,
                                     2);
  return mode;
}

WifiMode
WifiPhy::GetOfdmRate6MbpsBW10MHz ()
{
  static WifiMode mode =
    WifiModeFactory::CreateWifiMode ("OfdmRate6MbpsBW10MHz",
                                     WIFI_MOD_CLASS_OFDM,
                                     true,
                                     WIFI_CODE_RATE_1_2,
                                     4);
  return mode;
}

WifiMode
WifiPhy::GetOfdmRate9MbpsBW10MHz ()
{
  static WifiMode mode =
    WifiModeFactory::CreateWifiMode ("OfdmRate9MbpsBW10MHz",
                                     WIFI_MOD_CLASS_OFDM,
                                     false,
                                     WIFI_CODE_RATE_3_4,
                                     4);
  return mode;
}

WifiMode
WifiPhy::GetOfdmRate12MbpsBW10MHz ()
{
  static WifiMode mode =
    WifiModeFactory::CreateWifiMode ("OfdmRate12MbpsBW10MHz",
                                     WIFI_MOD_CLASS_OFDM,
                                     true,
                                     WIFI_CODE_RATE_1_2,
                                     16);
  return mode;
}

WifiMode
WifiPhy::GetOfdmRate18MbpsBW10MHz ()
{
  static WifiMode mode =
    WifiModeFactory::CreateWifiMode ("OfdmRate18MbpsBW10MHz",
                                     WIFI_MOD_CLASS_OFDM,
                                     false,
                                     WIFI_CODE_RATE_3_4,
                                     16);
  return mode;
}

WifiMode
WifiPhy::GetOfdmRate24MbpsBW10MHz ()
{
  static WifiMode mode =
    WifiModeFactory::CreateWifiMode ("OfdmRate24MbpsBW10MHz",
                                     WIFI_MOD_CLASS_OFDM,
                                     false,
                                     WIFI_CODE_RATE_2_3,
                                     64);
  return mode;
}

WifiMode
WifiPhy::GetOfdmRate27MbpsBW10MHz ()
{
  static WifiMode mode =
    WifiModeFactory::CreateWifiMode ("OfdmRate27MbpsBW10MHz",
                                     WIFI_MOD_CLASS_OFDM,
                                     false,
                                     WIFI_CODE_RATE_3_4,
                                     64);
  return mode;
}


// 5 MHz channel rates

WifiMode
WifiPhy::GetOfdmRate1_5MbpsBW5MHz ()
{
  static WifiMode mode =
    WifiModeFactory::CreateWifiMode ("OfdmRate1_5MbpsBW5MHz",
                                     WIFI_MOD_CLASS_OFDM,
                                     true,
                                     WIFI_CODE_RATE_1_2,
                                     2);
  return mode;
}

WifiMode
WifiPhy::GetOfdmRate2_25MbpsBW5MHz ()
{
  static WifiMode mode =
    WifiModeFactory::CreateWifiMode ("OfdmRate2_25MbpsBW5MHz",
                                     WIFI_MOD_CLASS_OFDM,
                                     false,
                                     WIFI_CODE_RATE_3_4,
                                     2);
  return mode;
}

WifiMode
WifiPhy::GetOfdmRate3MbpsBW5MHz ()
{
  static WifiMode mode =
    WifiModeFactory::CreateWifiMode ("OfdmRate3MbpsBW5MHz",
                                     WIFI_MOD_CLASS_OFDM,
                                     true,
                                     WIFI_CODE_RATE_1_2,
                                     4);
  return mode;
}

WifiMode
WifiPhy::GetOfdmRate4_5MbpsBW5MHz ()
{
  static WifiMode mode =
    WifiModeFactory::CreateWifiMode ("OfdmRate4_5MbpsBW5MHz",
                                     WIFI_MOD_CLASS_OFDM,
                                     false,
                                     WIFI_CODE_RATE_3_4,
                                     4);
  return mode;
}

WifiMode
WifiPhy::GetOfdmRate6MbpsBW5MHz ()
{
  static WifiMode mode =
    WifiModeFactory::CreateWifiMode ("OfdmRate6MbpsBW5MHz",
                                     WIFI_MOD_CLASS_OFDM,
                                     true,
                                     WIFI_CODE_RATE_1_2,
                                     16);
  return mode;
}

WifiMode
WifiPhy::GetOfdmRate9MbpsBW5MHz ()
{
  static WifiMode mode =
    WifiModeFactory::CreateWifiMode ("OfdmRate9MbpsBW5MHz",
                                     WIFI_MOD_CLASS_OFDM,
                                     false,
                                     WIFI_CODE_RATE_3_4,
                                     16);
  return mode;
}

WifiMode
WifiPhy::GetOfdmRate12MbpsBW5MHz ()
{
  static WifiMode mode =
    WifiModeFactory::CreateWifiMode ("OfdmRate12MbpsBW5MHz",
                                     WIFI_MOD_CLASS_OFDM,
                                     false,
                                     WIFI_CODE_RATE_2_3,
                                     64);
  return mode;
}

WifiMode
WifiPhy::GetOfdmRate13_5MbpsBW5MHz ()
{
  static WifiMode mode =
    WifiModeFactory::CreateWifiMode ("OfdmRate13_5MbpsBW5MHz",
                                     WIFI_MOD_CLASS_OFDM,
                                     false,
                                     WIFI_CODE_RATE_3_4,
                                     64);
  return mode;
}


// Clause 20

WifiMode
WifiPhy::GetHtMcs0 ()
{
  static WifiMode mcs =
    WifiModeFactory::CreateWifiMcs ("HtMcs0", 0, WIFI_MOD_CLASS_HT);
  return mcs;
}

WifiMode
WifiPhy::GetHtMcs1 ()
{
  static WifiMode mcs =
    WifiModeFactory::CreateWifiMcs ("HtMcs1", 1, WIFI_MOD_CLASS_HT);
  return mcs;
}

WifiMode
WifiPhy::GetHtMcs2 ()
{
  static WifiMode mcs =
    WifiModeFactory::CreateWifiMcs ("HtMcs2", 2, WIFI_MOD_CLASS_HT);
  return mcs;
}

WifiMode
WifiPhy::GetHtMcs3 ()
{
  static WifiMode mcs =
    WifiModeFactory::CreateWifiMcs ("HtMcs3", 3, WIFI_MOD_CLASS_HT);
  return mcs;
}

WifiMode
WifiPhy::GetHtMcs4 ()
{
  static WifiMode mcs =
    WifiModeFactory::CreateWifiMcs ("HtMcs4", 4, WIFI_MOD_CLASS_HT);
  return mcs;
}

WifiMode
WifiPhy::GetHtMcs5 ()
{
  static WifiMode mcs =
    WifiModeFactory::CreateWifiMcs ("HtMcs5", 5, WIFI_MOD_CLASS_HT);
  return mcs;
}

WifiMode
WifiPhy::GetHtMcs6 ()
{
  static WifiMode mcs =
    WifiModeFactory::CreateWifiMcs ("HtMcs6", 6, WIFI_MOD_CLASS_HT);
  return mcs;
}

WifiMode
WifiPhy::GetHtMcs7 ()
{
  static WifiMode mcs =
    WifiModeFactory::CreateWifiMcs ("HtMcs7", 7, WIFI_MOD_CLASS_HT);
  return mcs;
}

WifiMode
WifiPhy::GetHtMcs8 ()
{
  static WifiMode mcs =
    WifiModeFactory::CreateWifiMcs ("HtMcs8", 8, WIFI_MOD_CLASS_HT);
  return mcs;
}

WifiMode
WifiPhy::GetHtMcs9 ()
{
  static WifiMode mcs =
    WifiModeFactory::CreateWifiMcs ("HtMcs9", 9, WIFI_MOD_CLASS_HT);
  return mcs;
}

WifiMode
WifiPhy::GetHtMcs10 ()
{
  static WifiMode mcs =
    WifiModeFactory::CreateWifiMcs ("HtMcs10", 10, WIFI_MOD_CLASS_HT);
  return mcs;
}

WifiMode
WifiPhy::GetHtMcs11 ()
{
  static WifiMode mcs =
    WifiModeFactory::CreateWifiMcs ("HtMcs11", 11, WIFI_MOD_CLASS_HT);
  return mcs;
}

WifiMode
WifiPhy::GetHtMcs12 ()
{
  static WifiMode mcs =
    WifiModeFactory::CreateWifiMcs ("HtMcs12", 12, WIFI_MOD_CLASS_HT);
  return mcs;
}

WifiMode
WifiPhy::GetHtMcs13 ()
{
  static WifiMode mcs =
    WifiModeFactory::CreateWifiMcs ("HtMcs13", 13, WIFI_MOD_CLASS_HT);
  return mcs;
}

WifiMode
WifiPhy::GetHtMcs14 ()
{
  static WifiMode mcs =
    WifiModeFactory::CreateWifiMcs ("HtMcs14", 14, WIFI_MOD_CLASS_HT);
  return mcs;
}

WifiMode
WifiPhy::GetHtMcs15 ()
{
  static WifiMode mcs =
    WifiModeFactory::CreateWifiMcs ("HtMcs15", 15, WIFI_MOD_CLASS_HT);
  return mcs;
}

WifiMode
WifiPhy::GetHtMcs16 ()
{
  static WifiMode mcs =
    WifiModeFactory::CreateWifiMcs ("HtMcs16", 16, WIFI_MOD_CLASS_HT);
  return mcs;
}

WifiMode
WifiPhy::GetHtMcs17 ()
{
  static WifiMode mcs =
    WifiModeFactory::CreateWifiMcs ("HtMcs17", 17, WIFI_MOD_CLASS_HT);
  return mcs;
}

WifiMode
WifiPhy::GetHtMcs18 ()
{
  static WifiMode mcs =
    WifiModeFactory::CreateWifiMcs ("HtMcs18", 18, WIFI_MOD_CLASS_HT);
  return mcs;
}

WifiMode
WifiPhy::GetHtMcs19 ()
{
  static WifiMode mcs =
    WifiModeFactory::CreateWifiMcs ("HtMcs19", 19, WIFI_MOD_CLASS_HT);
  return mcs;
}

WifiMode
WifiPhy::GetHtMcs20 ()
{
  static WifiMode mcs =
    WifiModeFactory::CreateWifiMcs ("HtMcs20", 20, WIFI_MOD_CLASS_HT);
  return mcs;
}

WifiMode
WifiPhy::GetHtMcs21 ()
{
  static WifiMode mcs =
    WifiModeFactory::CreateWifiMcs ("HtMcs21", 21, WIFI_MOD_CLASS_HT);
  return mcs;
}

WifiMode
WifiPhy::GetHtMcs22 ()
{
  static WifiMode mcs =
    WifiModeFactory::CreateWifiMcs ("HtMcs22", 22, WIFI_MOD_CLASS_HT);
  return mcs;
}

WifiMode
WifiPhy::GetHtMcs23 ()
{
  static WifiMode mcs =
    WifiModeFactory::CreateWifiMcs ("HtMcs23", 23, WIFI_MOD_CLASS_HT);
  return mcs;
}

WifiMode
WifiPhy::GetHtMcs24 ()
{
  static WifiMode mcs =
    WifiModeFactory::CreateWifiMcs ("HtMcs24", 24, WIFI_MOD_CLASS_HT);
  return mcs;
}

WifiMode
WifiPhy::GetHtMcs25 ()
{
  static WifiMode mcs =
    WifiModeFactory::CreateWifiMcs ("HtMcs25", 25, WIFI_MOD_CLASS_HT);
  return mcs;
}

WifiMode
WifiPhy::GetHtMcs26 ()
{
  static WifiMode mcs =
    WifiModeFactory::CreateWifiMcs ("HtMcs26", 26, WIFI_MOD_CLASS_HT);
  return mcs;
}

WifiMode
WifiPhy::GetHtMcs27 ()
{
  static WifiMode mcs =
    WifiModeFactory::CreateWifiMcs ("HtMcs27", 27, WIFI_MOD_CLASS_HT);
  return mcs;
}

WifiMode
WifiPhy::GetHtMcs28 ()
{
  static WifiMode mcs =
    WifiModeFactory::CreateWifiMcs ("HtMcs28", 28, WIFI_MOD_CLASS_HT);
  return mcs;
}

WifiMode
WifiPhy::GetHtMcs29 ()
{
  static WifiMode mcs =
    WifiModeFactory::CreateWifiMcs ("HtMcs29", 29, WIFI_MOD_CLASS_HT);
  return mcs;
}

WifiMode
WifiPhy::GetHtMcs30 ()
{
  static WifiMode mcs =
    WifiModeFactory::CreateWifiMcs ("HtMcs30", 30, WIFI_MOD_CLASS_HT);
  return mcs;
}

WifiMode
WifiPhy::GetHtMcs31 ()
{
  static WifiMode mcs =
    WifiModeFactory::CreateWifiMcs ("HtMcs31", 31, WIFI_MOD_CLASS_HT);
  return mcs;
}


// Clause 22

WifiMode
WifiPhy::GetVhtMcs0 ()
{
  static WifiMode mcs =
    WifiModeFactory::CreateWifiMcs ("VhtMcs0", 0, WIFI_MOD_CLASS_VHT);
  return mcs;
}

WifiMode
WifiPhy::GetVhtMcs1 ()
{
  static WifiMode mcs =
    WifiModeFactory::CreateWifiMcs ("VhtMcs1", 1, WIFI_MOD_CLASS_VHT);
  return mcs;
}

WifiMode
WifiPhy::GetVhtMcs2 ()
{
  static WifiMode mcs =
    WifiModeFactory::CreateWifiMcs ("VhtMcs2", 2, WIFI_MOD_CLASS_VHT);
  return mcs;
}

WifiMode
WifiPhy::GetVhtMcs3 ()
{
  static WifiMode mcs =
    WifiModeFactory::CreateWifiMcs ("VhtMcs3", 3, WIFI_MOD_CLASS_VHT);
  return mcs;
}

WifiMode
WifiPhy::GetVhtMcs4 ()
{
  static WifiMode mcs =
    WifiModeFactory::CreateWifiMcs ("VhtMcs4", 4, WIFI_MOD_CLASS_VHT);
  return mcs;
}

WifiMode
WifiPhy::GetVhtMcs5 ()
{
  static WifiMode mcs =
    WifiModeFactory::CreateWifiMcs ("VhtMcs5", 5, WIFI_MOD_CLASS_VHT);
  return mcs;
}

WifiMode
WifiPhy::GetVhtMcs6 ()
{
  static WifiMode mcs =
    WifiModeFactory::CreateWifiMcs ("VhtMcs6", 6, WIFI_MOD_CLASS_VHT);
  return mcs;
}

WifiMode
WifiPhy::GetVhtMcs7 ()
{
  static WifiMode mcs =
    WifiModeFactory::CreateWifiMcs ("VhtMcs7", 7, WIFI_MOD_CLASS_VHT);
  return mcs;
}

WifiMode
WifiPhy::GetVhtMcs8 ()
{
  static WifiMode mcs =
    WifiModeFactory::CreateWifiMcs ("VhtMcs8", 8, WIFI_MOD_CLASS_VHT);
  return mcs;
}

WifiMode
WifiPhy::GetVhtMcs9 ()
{
  static WifiMode mcs =
    WifiModeFactory::CreateWifiMcs ("VhtMcs9", 9, WIFI_MOD_CLASS_VHT);
  return mcs;
}

// Clause 26

WifiMode
WifiPhy::GetHeMcs0 ()
{
  static WifiMode mcs =
    WifiModeFactory::CreateWifiMcs ("HeMcs0", 0, WIFI_MOD_CLASS_HE);
  return mcs;
}

WifiMode
WifiPhy::GetHeMcs1 ()
{
  static WifiMode mcs =
    WifiModeFactory::CreateWifiMcs ("HeMcs1", 1, WIFI_MOD_CLASS_HE);
  return mcs;
}

WifiMode
WifiPhy::GetHeMcs2 ()
{
  static WifiMode mcs =
    WifiModeFactory::CreateWifiMcs ("HeMcs2", 2, WIFI_MOD_CLASS_HE);
  return mcs;
}

WifiMode
WifiPhy::GetHeMcs3 ()
{
  static WifiMode mcs =
    WifiModeFactory::CreateWifiMcs ("HeMcs3", 3, WIFI_MOD_CLASS_HE);
  return mcs;
}

WifiMode
WifiPhy::GetHeMcs4 ()
{
  static WifiMode mcs =
    WifiModeFactory::CreateWifiMcs ("HeMcs4", 4, WIFI_MOD_CLASS_HE);
  return mcs;
}

WifiMode
WifiPhy::GetHeMcs5 ()
{
  static WifiMode mcs =
    WifiModeFactory::CreateWifiMcs ("HeMcs5", 5, WIFI_MOD_CLASS_HE);
  return mcs;
}

WifiMode
WifiPhy::GetHeMcs6 ()
{
  static WifiMode mcs =
    WifiModeFactory::CreateWifiMcs ("HeMcs6", 6, WIFI_MOD_CLASS_HE);
  return mcs;
}

WifiMode
WifiPhy::GetHeMcs7 ()
{
  static WifiMode mcs =
    WifiModeFactory::CreateWifiMcs ("HeMcs7", 7, WIFI_MOD_CLASS_HE);
  return mcs;
}

WifiMode
WifiPhy::GetHeMcs8 ()
{
  static WifiMode mcs =
    WifiModeFactory::CreateWifiMcs ("HeMcs8", 8, WIFI_MOD_CLASS_HE);
  return mcs;
}

WifiMode
WifiPhy::GetHeMcs9 ()
{
  static WifiMode mcs =
    WifiModeFactory::CreateWifiMcs ("HeMcs9", 9, WIFI_MOD_CLASS_HE);
  return mcs;
}

WifiMode
WifiPhy::GetHeMcs10 ()
{
  static WifiMode mcs =
    WifiModeFactory::CreateWifiMcs ("HeMcs10", 10, WIFI_MOD_CLASS_HE);
  return mcs;
}

WifiMode
WifiPhy::GetHeMcs11 ()
{
  static WifiMode mcs =
    WifiModeFactory::CreateWifiMcs ("HeMcs11", 11, WIFI_MOD_CLASS_HE);
  return mcs;
}

bool
WifiPhy::IsValidTxVector (WifiTxVector txVector)
{
  uint16_t chWidth = txVector.GetChannelWidth ();
  uint8_t nss = txVector.GetNss ();
  std::string modeName = txVector.GetMode ().GetUniqueName ();

  if (chWidth == 20)
    {
      if (nss != 3 && nss != 6)
        {
          return (modeName != "VhtMcs9");
        }
    }
  else if (chWidth == 80)
    {
      if (nss == 3 || nss == 7)
        {
          return (modeName != "VhtMcs6");
        }
      else if (nss == 6)
        {
          return (modeName != "VhtMcs9");
        }
    }
  else if (chWidth == 160)
    {
      if (nss == 3)
        {
          return (modeName != "VhtMcs9");
        }
    }

  return true;
}

bool
WifiPhy::IsModeSupported (WifiMode mode) const
{
  for (uint32_t i = 0; i < GetNModes (); i++)
    {
      if (mode == GetMode (i))
        {
          return true;
        }
    }
  return false;
}

bool
WifiPhy::IsMcsSupported (WifiMode mcs) const
{
  for (uint32_t i = 0; i < GetNMcs (); i++)
    {
      if (mcs == GetMcs (i))
        {
          return true;
        }
    }
  return false;
}

uint32_t
WifiPhy::GetNModes (void) const
{
  return m_deviceRateSet.size ();
}

WifiMode
WifiPhy::GetMode (uint32_t mode) const
{
  return m_deviceRateSet[mode];
}

uint8_t
WifiPhy::GetNMcs (void) const
{
  return m_deviceMcsSet.size ();
}

WifiMode
WifiPhy::GetMcs (uint8_t mcs) const
{
  return m_deviceMcsSet[mcs];
}

bool
WifiPhy::IsStateCcaBusy (void)
{
  return m_state->IsStateCcaBusy ();
}

bool
WifiPhy::IsStateIdle (void)
{
  return m_state->IsStateIdle ();
}

bool
WifiPhy::IsStateBusy (void)
{
  return m_state->IsStateBusy ();
}

bool
WifiPhy::IsStateRx (void)
{
  return m_state->IsStateRx ();
}

bool
WifiPhy::IsStateTx (void)
{
  return m_state->IsStateTx ();
}

bool
WifiPhy::IsStateSwitching (void)
{
  return m_state->IsStateSwitching ();
}

bool
WifiPhy::IsStateSleep (void)
{
  return m_state->IsStateSleep ();
}

WifiPhy::State
WifiPhy::GetPhyState (void) const
{
  return m_state->GetState ();
}

Time
WifiPhy::GetStateDuration (void)
{
  return m_state->GetStateDuration ();
}

Time
WifiPhy::GetDelayUntilIdle (void)
{
  return m_state->GetDelayUntilIdle ();
}

Time
WifiPhy::GetLastRxStartTime (void) const
{
  return m_state->GetLastRxStartTime ();
}

void
WifiPhy::SwitchMaybeToCcaBusy (void)
{
  NS_LOG_FUNCTION (this);
  //We are here because we have received the first bit of a packet and we are
  //not going to be able to synchronize on it
  //In this model, CCA becomes busy when the aggregation of all signals as
  //tracked by the InterferenceHelper class is higher than the CcaBusyThreshold

  Time delayUntilCcaEnd = m_interference.GetEnergyDuration (DbmToW (GetCcaMode1Threshold ()));
  if (!delayUntilCcaEnd.IsZero ())
    {
      NS_LOG_DEBUG ("Calling SwitchMaybeToCcaBusy for " << delayUntilCcaEnd.As (Time::S));
      m_state->SwitchMaybeToCcaBusy (delayUntilCcaEnd);
    }
}

int64_t
WifiPhy::AssignStreams (int64_t stream)
{
  NS_LOG_FUNCTION (this << stream);
  m_random->SetStream (stream);
  return 1;
}

/*
 * 802.11ad PHY Layer Rates (Clause 21 Rates)
 */

/* DMG Control PHY */
WifiMode
WifiPhy::GetDMG_MCS0 ()
{
  static WifiMode mode =
    WifiModeFactory::CreateWifiMode ("DMG_MCS0", 0,
                                     WIFI_MOD_CLASS_DMG_CTRL,
                                     true,
                                     2160000000, 27500000,
                                     WIFI_CODE_RATE_1_2,
                                     2);
  return mode;
}

/* DMG SC PHY */
WifiMode
WifiPhy::GetDMG_MCS1 ()
{
  static WifiMode mode =
    WifiModeFactory::CreateWifiMode ("DMG_MCS1", 1,
                                     WIFI_MOD_CLASS_DMG_SC,
                                     true,
                                     2160000000, 385000000,
                                     WIFI_CODE_RATE_1_4, /* 2 repetition */
                                     2);
  return mode;
}

WifiMode
WifiPhy::GetDMG_MCS2 ()
{
  static WifiMode mode =
    WifiModeFactory::CreateWifiMode ("DMG_MCS2", 2,
                                     WIFI_MOD_CLASS_DMG_SC,
                                     true,
                                     2160000000, 770000000,
                                     WIFI_CODE_RATE_1_2,
                                     2);
  return mode;
}

WifiMode
WifiPhy::GetDMG_MCS3 ()
{
  static WifiMode mode =
    WifiModeFactory::CreateWifiMode ("DMG_MCS3", 3,
                                     WIFI_MOD_CLASS_DMG_SC,
                                     true,
                                     2160000000, 962500000,
                                     WIFI_CODE_RATE_5_8,
                                     2);
  return mode;
}

WifiMode
WifiPhy::GetDMG_MCS4 ()
{
  static WifiMode mode =
    WifiModeFactory::CreateWifiMode ("DMG_MCS4", 4,
                                     WIFI_MOD_CLASS_DMG_SC,
                                     true, /* VHT SC MCS1-4 mandatory*/
                                     2160000000, 1155000000,
                                     WIFI_CODE_RATE_3_4,
                                     2);
  return mode;
}

WifiMode
WifiPhy::GetDMG_MCS5 ()
{
  static WifiMode mode =
    WifiModeFactory::CreateWifiMode ("DMG_MCS5", 5,
                                     WIFI_MOD_CLASS_DMG_SC,
                                     false,
                                     2160000000, 1251250000,
                                     WIFI_CODE_RATE_13_16,
                                     2);
  return mode;
}

WifiMode
WifiPhy::GetDMG_MCS6 ()
{
  static WifiMode mode =
    WifiModeFactory::CreateWifiMode ("DMG_MCS6", 6,
                                     WIFI_MOD_CLASS_DMG_SC,
                                     false,
                                     2160000000, 1540000000,
                                     WIFI_CODE_RATE_1_2,
                                     4);
  return mode;
}

WifiMode
WifiPhy::GetDMG_MCS7 ()
{
  static WifiMode mode =
    WifiModeFactory::CreateWifiMode ("DMG_MCS7", 7,
                                     WIFI_MOD_CLASS_DMG_SC,
                                     false,
                                     2160000000, 1925000000,
                                     WIFI_CODE_RATE_5_8,
                                     4);
  return mode;
}

WifiMode
WifiPhy::GetDMG_MCS8 ()
{
  static WifiMode mode =
    WifiModeFactory::CreateWifiMode ("DMG_MCS8", 8,
                                     WIFI_MOD_CLASS_DMG_SC,
                                     false,
                                     2160000000, 2310000000ULL,
                                     WIFI_CODE_RATE_3_4,
                                     4);
  return mode;
}

WifiMode
WifiPhy::GetDMG_MCS9 ()
{
  static WifiMode mode =
    WifiModeFactory::CreateWifiMode ("DMG_MCS9", 9,
                                     WIFI_MOD_CLASS_DMG_SC,
                                     false,
                                     2160000000, 2502500000ULL,
                                     WIFI_CODE_RATE_13_16,
                                     4);
  return mode;
}

WifiMode
WifiPhy::GetDMG_MCS10 ()
{
  static WifiMode mode =
    WifiModeFactory::CreateWifiMode ("DMG_MCS10", 10,
                                     WIFI_MOD_CLASS_DMG_SC,
                                     false,
                                     2160000000, 3080000000ULL,
                                     WIFI_CODE_RATE_1_2,
                                     16);
  return mode;
}

WifiMode
WifiPhy::GetDMG_MCS11 ()
{
  static WifiMode mode =
    WifiModeFactory::CreateWifiMode ("DMG_MCS11", 11,
                                     WIFI_MOD_CLASS_DMG_SC,
                                     false,
                                     2160000000, 3850000000ULL,
                                     WIFI_CODE_RATE_5_8,
                                     16);
  return mode;
}

WifiMode
WifiPhy::GetDMG_MCS12 ()
{
  static WifiMode mode =
    WifiModeFactory::CreateWifiMode ("DMG_MCS12", 12,
                                     WIFI_MOD_CLASS_DMG_SC,
                                     false,
                                     2160000000, 4620000000ULL,
                                     WIFI_CODE_RATE_3_4,
                                     16);
  return mode;
}

/**** OFDM MCSs BELOW ****/
WifiMode
WifiPhy::GetDMG_MCS13 ()
{
  static WifiMode mode =
    WifiModeFactory::CreateWifiMode ("DMG_MCS13", 13,
                                     WIFI_MOD_CLASS_DMG_OFDM,
                                     true,
                                     2160000000, 693000000ULL,
                                     WIFI_CODE_RATE_1_2,
                                     2);
  return mode;
}

WifiMode
WifiPhy::GetDMG_MCS14 ()
{
  static WifiMode mode =
    WifiModeFactory::CreateWifiMode ("DMG_MCS14", 14,
                                     WIFI_MOD_CLASS_DMG_OFDM,
                                     false,
                                     2160000000, 866250000ULL,
                                     WIFI_CODE_RATE_5_8,
                                     2);
  return mode;
}

WifiMode
WifiPhy::GetDMG_MCS15 ()
{
  static WifiMode mode =
    WifiModeFactory::CreateWifiMode ("DMG_MCS15", 15,
                                     WIFI_MOD_CLASS_DMG_OFDM,
                                     false,
                                     2160000000, 1386000000ULL,
                                     WIFI_CODE_RATE_1_2,
                                     4);
  return mode;
}

WifiMode
WifiPhy::GetDMG_MCS16 ()
{
  static WifiMode mode =
    WifiModeFactory::CreateWifiMode ("DMG_MCS16", 16,
                                     WIFI_MOD_CLASS_DMG_OFDM,
                                     false,
                                     2160000000, 1732500000ULL,
                                     WIFI_CODE_RATE_5_8,
                                     4);
  return mode;
}

WifiMode
WifiPhy::GetDMG_MCS17 ()
{
  static WifiMode mode =
  WifiModeFactory::CreateWifiMode ("DMG_MCS17", 17,
                                   WIFI_MOD_CLASS_DMG_OFDM,
                                   false,
                                   2160000000, 2079000000ULL,
                                   WIFI_CODE_RATE_3_4,
                                   4);
  return mode;
}

WifiMode
WifiPhy::GetDMG_MCS18 ()
{
  static WifiMode mode =
  WifiModeFactory::CreateWifiMode ("DMG_MCS18", 18,
                                   WIFI_MOD_CLASS_DMG_OFDM,
                                   false,
                                   2160000000, 2772000000ULL,
                                   WIFI_CODE_RATE_1_2,
                                   16);
  return mode;
}

WifiMode
WifiPhy::GetDMG_MCS19 ()
{
  static WifiMode mode =
    WifiModeFactory::CreateWifiMode ("DMG_MCS19", 19,
                                     WIFI_MOD_CLASS_DMG_OFDM,
                                     false,
                                     2160000000, 3465000000ULL,
                                     WIFI_CODE_RATE_5_8,
                                     16);
  return mode;
}

WifiMode
WifiPhy::GetDMG_MCS20 ()
{
  static WifiMode mode =
    WifiModeFactory::CreateWifiMode ("DMG_MCS20", 20,
                                     WIFI_MOD_CLASS_DMG_OFDM,
                                     false,
                                     2160000000, 4158000000ULL,
                                     WIFI_CODE_RATE_3_4,
                                     16);
  return mode;
}

WifiMode
WifiPhy::GetDMG_MCS21 ()
{
  static WifiMode mode =
    WifiModeFactory::CreateWifiMode ("DMG_MCS21", 21,
                                     WIFI_MOD_CLASS_DMG_OFDM,
                                     false,
                                     2160000000, 4504500000ULL,
                                     WIFI_CODE_RATE_13_16,
                                     16);
  return mode;
}

WifiMode
WifiPhy::GetDMG_MCS22 ()
{
  static WifiMode mode =
    WifiModeFactory::CreateWifiMode ("DMG_MCS22", 22,
                                     WIFI_MOD_CLASS_DMG_OFDM,
                                     false,
                                     2160000000, 5197500000ULL,
                                     WIFI_CODE_RATE_5_8,
                                     64);
  return mode;
}

WifiMode
WifiPhy::GetDMG_MCS23 ()
{
  static WifiMode mode =
    WifiModeFactory::CreateWifiMode ("DMG_MCS23", 23,
                                     WIFI_MOD_CLASS_DMG_OFDM,
                                     false,
                                     2160000000, 6237000000ULL,
                                     WIFI_CODE_RATE_3_4,
                                     64);
  return mode;
}

WifiMode
WifiPhy::GetDMG_MCS24 ()
{
  static WifiMode mode =
    WifiModeFactory::CreateWifiMode ("DMG_MCS24", 24,
                                     WIFI_MOD_CLASS_DMG_OFDM,
                                     false,
                                     2160000000, 6756750000ULL,
                                     WIFI_CODE_RATE_13_16,
                                     64);
  return mode;
}

/**** Low Power SC MCSs ****/
WifiMode
WifiPhy::GetDMG_MCS25 ()
{
  static WifiMode mode =
    WifiModeFactory::CreateWifiMode ("DMG_MCS25", 25,
                                     WIFI_MOD_CLASS_DMG_LP_SC,
                                     false,
                                     2160000000, 626000000,
                                     WIFI_CODE_RATE_13_28,
                                     2);
  return mode;
}

WifiMode
WifiPhy::GetDMG_MCS26 ()
{
  static WifiMode mode =
    WifiModeFactory::CreateWifiMode ("DMG_MCS26", 26,
                                     WIFI_MOD_CLASS_DMG_LP_SC,
                                     false,
                                     2160000000, 834000000,
                                     WIFI_CODE_RATE_13_21,
                                     2);
  return mode;
}

WifiMode
WifiPhy::GetDMG_MCS27 ()
{
  static WifiMode mode =
    WifiModeFactory::CreateWifiMode ("DMG_MCS27", 27,
                                     WIFI_MOD_CLASS_DMG_LP_SC,
                                     false,
                                     2160000000, 1112000000ULL,
                                     WIFI_CODE_RATE_52_63,
                                     2);
  return mode;
}

WifiMode
WifiPhy::GetDMG_MCS28 ()
{
  static WifiMode mode =
    WifiModeFactory::CreateWifiMode ("DMG_MCS28", 28,
                                     WIFI_MOD_CLASS_DMG_LP_SC,
                                     false,
                                     2160000000, 1251000000ULL,
                                     WIFI_CODE_RATE_13_28,
                                     2);
  return mode;
}

WifiMode
WifiPhy::GetDMG_MCS29 ()
{
  static WifiMode mode =
    WifiModeFactory::CreateWifiMode ("DMG_MCS29", 29,
                                     WIFI_MOD_CLASS_DMG_LP_SC,
                                     false,
                                     2160000000, 1668000000ULL,
                                     WIFI_CODE_RATE_13_21,
                                     4);
  return mode;
}


WifiMode
WifiPhy::GetDMG_MCS30 ()
{
  static WifiMode mode =
    WifiModeFactory::CreateWifiMode ("DMG_MCS30", 30,
                                     WIFI_MOD_CLASS_DMG_LP_SC,
                                     false,
                                     2160000000, 2224000000ULL,
                                     WIFI_CODE_RATE_52_63,
                                     4);
  return mode;
}


WifiMode
WifiPhy::GetDMG_MCS31 ()
{
  static WifiMode mode =
    WifiModeFactory::CreateWifiMode ("DMG_MCS31", 31,
                                     WIFI_MOD_CLASS_DMG_LP_SC,
                                     false,
                                     2160000000, 2503000000ULL,
                                     WIFI_CODE_RATE_13_14,
                                     4);
  return mode;
}

//Yubing
std::string 
WifiPhy::GetMostMcs (void)
{
  std::string results;
  uint16_t temp = 0;
  for (uint16_t i=0; i<MCSHistory.size(); i++)
    {
      if (MCSHistory.at(i).second > temp){
        results = MCSHistory.at(i).first;
        temp = MCSHistory.at(i).second;
      }
    }
  return results;
}

double 
WifiPhy::GetRss (void)
{
  return m_rssStas.first/m_rssStas.second;
}

void 
WifiPhy::SetRSSAndMCS (std::vector<std::pair<std::string, uint16_t>> MCS, std::pair<double,double> rss)
{
  MCSHistory = MCS;
  m_rssStas = rss; 
}

std::pair<std::pair<Mac48Address, Mac48Address>, double>
WifiPhy::GetSensitivityMap (void) {
  return m_rxSensitivityMap;
}

std::ostream& operator<< (std::ostream& os, enum WifiPhy::State state)
{
  switch (state)
    {
    case WifiPhy::IDLE:
      return (os << "IDLE");
    case WifiPhy::CCA_BUSY:
      return (os << "CCA_BUSY");
    case WifiPhy::TX:
      return (os << "TX");
    case WifiPhy::RX:
      return (os << "RX");
    case WifiPhy::SWITCHING:
      return (os << "SWITCHING");
    case WifiPhy::SLEEP:
      return (os << "SLEEP");
    default:
      NS_FATAL_ERROR ("Invalid WifiPhy state");
      return (os << "INVALID");
    }
}

} //namespace ns3

namespace {

/**
 * Constructor class
 */
static class Constructor
{
public:
  Constructor ()
  {
    ns3::WifiPhy::GetDsssRate1Mbps ();
    ns3::WifiPhy::GetDsssRate2Mbps ();
    ns3::WifiPhy::GetDsssRate5_5Mbps ();
    ns3::WifiPhy::GetDsssRate11Mbps ();
    ns3::WifiPhy::GetErpOfdmRate6Mbps ();
    ns3::WifiPhy::GetErpOfdmRate9Mbps ();
    ns3::WifiPhy::GetErpOfdmRate12Mbps ();
    ns3::WifiPhy::GetErpOfdmRate18Mbps ();
    ns3::WifiPhy::GetErpOfdmRate24Mbps ();
    ns3::WifiPhy::GetErpOfdmRate36Mbps ();
    ns3::WifiPhy::GetErpOfdmRate48Mbps ();
    ns3::WifiPhy::GetErpOfdmRate54Mbps ();
    ns3::WifiPhy::GetOfdmRate6Mbps ();
    ns3::WifiPhy::GetOfdmRate9Mbps ();
    ns3::WifiPhy::GetOfdmRate12Mbps ();
    ns3::WifiPhy::GetOfdmRate18Mbps ();
    ns3::WifiPhy::GetOfdmRate24Mbps ();
    ns3::WifiPhy::GetOfdmRate36Mbps ();
    ns3::WifiPhy::GetOfdmRate48Mbps ();
    ns3::WifiPhy::GetOfdmRate54Mbps ();
    ns3::WifiPhy::GetOfdmRate3MbpsBW10MHz ();
    ns3::WifiPhy::GetOfdmRate4_5MbpsBW10MHz ();
    ns3::WifiPhy::GetOfdmRate6MbpsBW10MHz ();
    ns3::WifiPhy::GetOfdmRate9MbpsBW10MHz ();
    ns3::WifiPhy::GetOfdmRate12MbpsBW10MHz ();
    ns3::WifiPhy::GetOfdmRate18MbpsBW10MHz ();
    ns3::WifiPhy::GetOfdmRate24MbpsBW10MHz ();
    ns3::WifiPhy::GetOfdmRate27MbpsBW10MHz ();
    ns3::WifiPhy::GetOfdmRate1_5MbpsBW5MHz ();
    ns3::WifiPhy::GetOfdmRate2_25MbpsBW5MHz ();
    ns3::WifiPhy::GetOfdmRate3MbpsBW5MHz ();
    ns3::WifiPhy::GetOfdmRate4_5MbpsBW5MHz ();
    ns3::WifiPhy::GetOfdmRate6MbpsBW5MHz ();
    ns3::WifiPhy::GetOfdmRate9MbpsBW5MHz ();
    ns3::WifiPhy::GetOfdmRate12MbpsBW5MHz ();
    ns3::WifiPhy::GetOfdmRate13_5MbpsBW5MHz ();
    ns3::WifiPhy::GetHtMcs0 ();
    ns3::WifiPhy::GetHtMcs1 ();
    ns3::WifiPhy::GetHtMcs2 ();
    ns3::WifiPhy::GetHtMcs3 ();
    ns3::WifiPhy::GetHtMcs4 ();
    ns3::WifiPhy::GetHtMcs5 ();
    ns3::WifiPhy::GetHtMcs6 ();
    ns3::WifiPhy::GetHtMcs7 ();
    ns3::WifiPhy::GetHtMcs8 ();
    ns3::WifiPhy::GetHtMcs9 ();
    ns3::WifiPhy::GetHtMcs10 ();
    ns3::WifiPhy::GetHtMcs11 ();
    ns3::WifiPhy::GetHtMcs12 ();
    ns3::WifiPhy::GetHtMcs13 ();
    ns3::WifiPhy::GetHtMcs14 ();
    ns3::WifiPhy::GetHtMcs15 ();
    ns3::WifiPhy::GetHtMcs16 ();
    ns3::WifiPhy::GetHtMcs17 ();
    ns3::WifiPhy::GetHtMcs18 ();
    ns3::WifiPhy::GetHtMcs19 ();
    ns3::WifiPhy::GetHtMcs20 ();
    ns3::WifiPhy::GetHtMcs21 ();
    ns3::WifiPhy::GetHtMcs22 ();
    ns3::WifiPhy::GetHtMcs23 ();
    ns3::WifiPhy::GetHtMcs24 ();
    ns3::WifiPhy::GetHtMcs25 ();
    ns3::WifiPhy::GetHtMcs26 ();
    ns3::WifiPhy::GetHtMcs27 ();
    ns3::WifiPhy::GetHtMcs28 ();
    ns3::WifiPhy::GetHtMcs29 ();
    ns3::WifiPhy::GetHtMcs30 ();
    ns3::WifiPhy::GetHtMcs31 ();
    ns3::WifiPhy::GetVhtMcs0 ();
    ns3::WifiPhy::GetVhtMcs1 ();
    ns3::WifiPhy::GetVhtMcs2 ();
    ns3::WifiPhy::GetVhtMcs3 ();
    ns3::WifiPhy::GetVhtMcs4 ();
    ns3::WifiPhy::GetVhtMcs5 ();
    ns3::WifiPhy::GetVhtMcs6 ();
    ns3::WifiPhy::GetVhtMcs7 ();
    ns3::WifiPhy::GetVhtMcs8 ();
    ns3::WifiPhy::GetVhtMcs9 ();
    ns3::WifiPhy::GetHeMcs0 ();
    ns3::WifiPhy::GetHeMcs1 ();
    ns3::WifiPhy::GetHeMcs2 ();
    ns3::WifiPhy::GetHeMcs3 ();
    ns3::WifiPhy::GetHeMcs4 ();
    ns3::WifiPhy::GetHeMcs5 ();
    ns3::WifiPhy::GetHeMcs6 ();
    ns3::WifiPhy::GetHeMcs7 ();
    ns3::WifiPhy::GetHeMcs8 ();
    ns3::WifiPhy::GetHeMcs9 ();
    ns3::WifiPhy::GetHeMcs10 ();
    ns3::WifiPhy::GetHeMcs11 ();
    /* Data Rates for 802.11ad PHY*/
    ns3::WifiPhy::GetDMG_MCS0 ();
    ns3::WifiPhy::GetDMG_MCS1 ();
    ns3::WifiPhy::GetDMG_MCS2 ();
    ns3::WifiPhy::GetDMG_MCS3 ();
    ns3::WifiPhy::GetDMG_MCS4 ();
    ns3::WifiPhy::GetDMG_MCS5 ();
    ns3::WifiPhy::GetDMG_MCS6 ();
    ns3::WifiPhy::GetDMG_MCS7 ();
    ns3::WifiPhy::GetDMG_MCS8 ();
    ns3::WifiPhy::GetDMG_MCS9 ();
    ns3::WifiPhy::GetDMG_MCS10 ();
    ns3::WifiPhy::GetDMG_MCS11 ();
    ns3::WifiPhy::GetDMG_MCS12 ();
    ns3::WifiPhy::GetDMG_MCS13 ();
    ns3::WifiPhy::GetDMG_MCS14 ();
    ns3::WifiPhy::GetDMG_MCS15 ();
    ns3::WifiPhy::GetDMG_MCS16 ();
    ns3::WifiPhy::GetDMG_MCS17 ();
    ns3::WifiPhy::GetDMG_MCS18 ();
    ns3::WifiPhy::GetDMG_MCS19 ();
    ns3::WifiPhy::GetDMG_MCS20 ();
    ns3::WifiPhy::GetDMG_MCS21 ();
    ns3::WifiPhy::GetDMG_MCS22 ();
    ns3::WifiPhy::GetDMG_MCS23 ();
    ns3::WifiPhy::GetDMG_MCS24 ();
  }
} g_constructor; ///< the constructor

}
