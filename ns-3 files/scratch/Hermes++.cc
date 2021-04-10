/*
 * Copyright (c) 2019 GNAN
 * Author: Yubing */

#include "ns3/applications-module.h"
#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/mobility-module.h"
#include "ns3/network-module.h"
#include "ns3/wifi-module.h"
#include "ns3/obstacle.h"
#include "common-functions.h"
#include <string>
#include <math.h>

#include <gsl/gsl_rng.h>

#define PI 3.14159265

// define shape of platform
#define STRAIGHTLINE 0
#define CIRCLE 1
#define COMPRESSEDSHAPE 2
#define CROSS 3

// Define location of platform
#define LS 0
#define CS 1
#define RS 2
#define FL 3
#define CL 4
#define RL 5

// Define WiMove Activation
#define STATICAPS 0
#define WIMOVE 1
#define MULTIWIMOVE 2

/**
 * Simulation Objective:
 * This script is used to evaluate the performance of Hermes++ and static AP performance 
 * To evaluate CSMA/CA channel access scheme: scheme=1, To evaluate SP channel access scheme: scheme=0
 */

NS_LOG_COMPONENT_DEFINE ("WiMove");

using namespace ns3;
using namespace std;

/**  Application Variables **/
uint64_t totalRx = 0;
double throughput = 0;
uint32_t allocationType = 0;               /* The type of channel access scheme during DTI (CBAP is the default) */

int
main(int argc, char *argv[])
{
  LogComponentEnable ("WiMove", LOG_LEVEL_ALL);
  //LogComponentEnable ("MacLow", LOG_LEVEL_ALL);
  //LogComponentEnable ("EdcaTxopN", LOG_LEVEL_ALL);
  //LogComponentEnable ("Obstacle", LOG_LEVEL_ALL);
  //LogComponentEnable ("YansWifiChannel", LOG_LEVEL_ALL);
  //LogComponentEnable ("TruncatedNormalDistribution", LOG_LEVEL_ALL);
  //LogComponentEnable ("DmgApWifiMac", LOG_LEVEL_ALL);
  LogComponentEnable ("WifiPhy", LOG_LEVEL_ALL);
  LogComponentEnable ("SensitivityModel60GHz", LOG_LEVEL_ALL);
  //LogComponentEnable ("InterferenceHelper", LOG_LEVEL_ALL);

  uint32_t payloadSize = 1472;                  /* Application payload size in bytes. */
  string dataRate = "4000Mbps";                  /* Application data rate. */
  uint32_t msduAggregationSize = 7935;          /* The maximum aggregation size for A-MSDU in Bytes. */
  uint32_t mpduAggregationSize = 262143;        /* The maximum aggregation size for A-MSPU in Bytes. */
  uint32_t queueSize = 1000;                    /* Wifi MAC Queue Size. */
  string phyMode = "DMG_MCS12";                 /* Type of the Physical Layer. */
  bool verbose = false;                         /* Print Logging Information. */
  double simulationTime = 1.01;                   /* Simulation time in seconds. */
  bool pcapTracing = false;                     /* PCAP Tracing is enabled. */
  double roomX = 9;
  double roomY = 6;
  double x = 4.5;
  double y = 3;
  double z = 3;
  uint16_t clientRS = 1;
  uint16_t distRS = 1;
  uint16_t i = 1;
  uint16_t apIter = 1;
  uint16_t shapeCategary = STRAIGHTLINE;
  uint16_t centerLocation = CS;
  double platformSize = 30;
  Vector trackSize = Vector(0, 0.065, 0.047); // x dimension is a parameter to be changed
  double moveStep = 0.1;
  //Vector apDimension = Vector (0.23, 0.23, 0.12);
  Vector apDimension = Vector (0.001, 0.001, 0.001);
  double depSD = 1;
  uint32_t clientNo = 1;
  uint16_t hermesFlag = WIMOVE;
  uint16_t apNumber = 2; 
  uint16_t obsNumber = 172;
  uint16_t clientDistType = 3;

  /* Command line argument parser setup. */
  CommandLine cmd;
  cmd.AddValue ("payloadSize", "Application payload size in bytes", payloadSize);
  cmd.AddValue ("dataRate", "Application data rate", dataRate);
  cmd.AddValue ("msduAggregation", "The maximum aggregation size for A-MSDU in Bytes", msduAggregationSize);
  cmd.AddValue ("mpduAggregation", "The maximum aggregation size for A-MPDU in Bytes", mpduAggregationSize);
  cmd.AddValue ("queueSize", "The maximum size of the Wifi MAC Queue", queueSize);
  cmd.AddValue ("scheme", "The access scheme used for channel access (0=SP,1=CBAP)", allocationType);
  cmd.AddValue ("phyMode", "802.11ad PHY Mode", phyMode);
  cmd.AddValue ("verbose", "Turn on all WifiNetDevice log components", verbose);
  cmd.AddValue ("simulationTime", "Simulation time in seconds", simulationTime);
  cmd.AddValue ("pcap", "Enable PCAP Tracing", pcapTracing);
  cmd.AddValue ("roomX", "room x dimension", roomX);
  cmd.AddValue ("roomY", "room y dimension", roomY);
  cmd.AddValue ("x", "ap x", x);
  cmd.AddValue ("y", "ap y", y);
  cmd.AddValue ("i", "simulation iteration", i);
  cmd.AddValue ("apNumber", "apNumber", apNumber);
  cmd.AddValue ("apIter", "simulation iteration apNumber", apIter);
  cmd.AddValue ("z", "ap z", z);
  cmd.AddValue ("centerLocation", "center Location Type", centerLocation);
  cmd.AddValue ("platformSize", "platform Size", platformSize); 
  cmd.AddValue ("clientRS", "random seed for client", clientRS);
  cmd.AddValue ("clientDistType", "distribution type for client", clientDistType);
  cmd.AddValue ("distRS", "random seed for truncated normal distribution", distRS);
  cmd.AddValue ("depSD", "random seed for dependent distribution", depSD);
  cmd.AddValue ("clientNo", "Number of client", clientNo);
  cmd.AddValue ("hermesFlag", "0 means static AP scenario, 1 means hermes scenario", hermesFlag);
  cmd.AddValue ("shapeCategary", "shape Categary", shapeCategary);
  cmd.AddValue ("obsNumber", "obstacle Number", obsNumber);  
  cmd.Parse (argc, argv);

  // scale parameter due to shell script without float value
  platformSize = platformSize*0.1;
  depSD = depSD*0.1;
  Vector roomSize = Vector (roomX, roomY, 3.0);
  x = roomX*0.5;
  y = roomY*0.5;

  /* Global params: no fragmentation, no RTS/CTS, fixed rate for all packets */
  Config::SetDefault ("ns3::WifiRemoteStationManager::FragmentationThreshold", StringValue ("999999"));
  Config::SetDefault ("ns3::WifiRemoteStationManager::RtsCtsThreshold", StringValue ("999999"));

  /**** WifiHelper is a meta-helper: it helps creates helpers ****/
  WifiHelper wifi;
  wifi.SetStandard (WIFI_PHY_STANDARD_80211ad);

  /* Turn on logging */
  if (verbose)
    {
      wifi.EnableLogComponents ();
      LogComponentEnable ("CompareAccessSchemes", LOG_LEVEL_ALL);
    }

  /**** Set up Channel ****/
  YansWifiChannelHelper wifiChannelHelper;
  /* Simple propagation delay model */
  wifiChannelHelper.SetPropagationDelay ("ns3::ConstantSpeedPropagationDelayModel");
  /* Friis model with standard-specific wavelength */
  wifiChannelHelper.AddPropagationLoss ("ns3::FriisPropagationLossModel", "Frequency", DoubleValue (60.48e9));
  Ptr<YansWifiChannel> wifiChannel = wifiChannelHelper.Create ();

  /**** Setup physical layer ****/
  YansWifiPhyHelper wifiPhy = YansWifiPhyHelper::Default ();
  /* Nodes will be added to the channel we set up earlier */
  wifiPhy.SetChannel (wifiChannel);
  /* All nodes transmit at 10 dBm == 10 mW, no adaptation */
  wifiPhy.Set ("TxPowerStart", DoubleValue (0.0));
  wifiPhy.Set ("TxPowerEnd", DoubleValue (0.0));
  wifiPhy.Set ("TxPowerLevels", UintegerValue (1));
  wifiPhy.Set ("TxGain", DoubleValue (0));
  wifiPhy.Set ("RxGain", DoubleValue (0));
  /* Sensitivity model includes implementation loss and noise figure */
  wifiPhy.Set ("RxNoiseFigure", DoubleValue (10));
  wifiPhy.Set ("CcaMode1Threshold", DoubleValue (-79));
  wifiPhy.Set ("EnergyDetectionThreshold", DoubleValue (-79 + 3));
  /* Set the phy layer error model */
  wifiPhy.SetErrorRateModel ("ns3::SensitivityModel60GHz");
  /* Set default algorithm for all nodes to be constant rate */
  wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager", "ControlMode", StringValue (phyMode),
                                                                "DataMode", StringValue (phyMode));
  /* Give all nodes directional antenna */
  wifiPhy.EnableAntenna (true, true);
  wifiPhy.SetAntenna ("ns3::Directional60GhzAntenna",
                      "Sectors", UintegerValue (8),
                      "Antennas", UintegerValue (1));

  /* Make two nodes and set them up with the PHY and the MAC */
  NodeContainer staWifiNode;
  staWifiNode.Create (clientNo);
  NodeContainer apWifiNode;
  apWifiNode.Create (1);

  /* Add a DMG upper mac */
  DmgWifiMacHelper wifiMac = DmgWifiMacHelper::Default ();

  Ssid ssid = Ssid ("Compare");
  wifiMac.SetType ("ns3::DmgApWifiMac",
                   "Ssid", SsidValue(ssid),
                   "BE_MaxAmpduSize", UintegerValue (mpduAggregationSize),
                   "BE_MaxAmsduSize", UintegerValue (msduAggregationSize),
                   "SSSlotsPerABFT", UintegerValue (8), "SSFramesPerSlot", UintegerValue (8),
                   "BeaconInterval", TimeValue (MicroSeconds (102400)),
                   "BeaconTransmissionInterval", TimeValue (MicroSeconds (600)),
                   "ATIPresent", BooleanValue (false));

  NetDeviceContainer apDevice;
  apDevice = wifi.Install (wifiPhy, wifiMac, apWifiNode.Get (0));

  wifiMac.SetType ("ns3::DmgStaWifiMac",
                   "Ssid", SsidValue (ssid), "ActiveProbing", BooleanValue (false),
                   "BE_MaxAmpduSize", UintegerValue (mpduAggregationSize),
                   "BE_MaxAmsduSize", UintegerValue (msduAggregationSize));

  NetDeviceContainer staDevice;
  staDevice = wifi.Install (wifiPhy, wifiMac, staWifiNode);

  // Set Obstacles and track location
  Ptr<Obstacle> labScenarios = CreateObject<Obstacle> ();
  labScenarios->SetObstacleNumber(obsNumber);

  // Define the AP location
  /*if (z == 0)
    z = z+apDimension.z*0.5+trackSize.z;
  else
    z = z-apDimension.z*0.5-trackSize.z;*/

  if (hermesFlag == WIMOVE)
    {
      if (shapeCategary == STRAIGHTLINE) 
        {
          double centerPos[10][3] = {{trackSize.y/2, roomSize.y*0.5, z}, {roomSize.x*0.5, roomSize.y*0.5, z}, {roomSize.x-trackSize.y/2, roomSize.y*0.5, z}, {roomSize.x*0.5, trackSize.y/2, z}, {roomSize.x*0.5, roomSize.y*0.5, z}, {roomSize.x*0.5, roomSize.y-trackSize.y/2, z}, {0, roomSize.y*0.5, 1.5}, {roomSize.x, roomSize.y*0.5, 1.5}, {4.5, 0, 1.5}, {4.5, roomSize.y, 1.5}};
          if (centerLocation == LS || centerLocation == CS || centerLocation == RS)
	    {
	      x = centerPos[centerLocation][0];
	      y = centerPos[centerLocation][1]+(-platformSize*0.5+i*moveStep);
              labScenarios->AllocateObstacle(Box (x-trackSize.y/2, x+trackSize.y/2, 3, 6, z, z+trackSize.z/2), roomSize, clientRS); 
	    }
          else if (centerLocation == FL || centerLocation == CL || centerLocation == RL)
	    {
    	      x = centerPos[centerLocation][0]+(-platformSize*0.5+i*moveStep);
	      y = centerPos[centerLocation][1];
	      labScenarios->AllocateObstacle(Box (x-trackSize.y/2, x+trackSize.y/2, 3, 6, z, z+trackSize.z/2), roomSize, clientRS);
    	    }
	  else
	    {
	      x = centerPos[centerLocation][0];
	      y = centerPos[centerLocation][1]; 
	      z = centerPos[centerLocation][2]+(-platformSize*0.5+i*moveStep); 
	      labScenarios->AllocateObstacle(Box (x-trackSize.y/2, x+trackSize.y/2, y-trackSize.y/2, y+trackSize.y/2, 0, 3), roomSize, clientRS); 
	    }
        }
      else if (shapeCategary == CIRCLE) 
        {
          double r = platformSize/(2*PI);
          double centerPos[5][3] = {{r+trackSize.y/2, roomSize.y*0.5, z}, {roomSize.x*0.5, roomSize.y*0.5, z}, {roomSize.x-r-trackSize.y/2, roomSize.y*0.5, z}, {roomSize.x*0.5, r+trackSize.y/2, z}, {roomSize.x*0.5, roomSize.y-r-trackSize.y/2, z}};
          x = centerPos[centerLocation][0] + r*cos(i*12*PI/180);
          y = centerPos[centerLocation][1] + r*sin(i*12*PI/180);
	  labScenarios->AllocateObstacle(Box (centerPos[centerLocation][0]-trackSize.y/2, centerPos[centerLocation][0]+trackSize.y/2, 3, 6, z, z+trackSize.z/2), roomSize, clientRS);
        }
      else if (shapeCategary == COMPRESSEDSHAPE) // square like
        {
          double horizTrackLength = trackSize.y*6;
          double vertTrackLength = (3-horizTrackLength)/7.0;
          double xDim = horizTrackLength + trackSize.y;
          double yDim = vertTrackLength + trackSize.y;
          double centerPos[5][3] = {{xDim*0.5, roomSize.y*0.5, z}, {roomSize.x*0.5, roomSize.y*0.5, z}, {roomSize.x-xDim*0.5, roomSize.y*0.5, z}, {roomSize.x*0.5, yDim*0.5, z}, {roomSize.x*0.5, roomSize.y-yDim*0.5, z}};
          if (fmod(i*moveStep,(vertTrackLength+trackSize.y)) <= vertTrackLength)
	    {
    	      x = centerPos[centerLocation][0] - horizTrackLength*0.5 + floor (i*moveStep/yDim)*trackSize.y;
  	      if ((int)floor(i*moveStep/(vertTrackLength+trackSize.y))%2 == 0) 
                y = centerPos[centerLocation][1] + vertTrackLength*0.5 - fmod(i*moveStep, (vertTrackLength+trackSize.y));	
	      else
	        y = centerPos[centerLocation][1] - vertTrackLength*0.5 + fmod(i*moveStep, (vertTrackLength+trackSize.y));	
   	    }
          else 
	    {
	      x = centerPos[centerLocation][0] - horizTrackLength*0.5 + floor (i*moveStep/(vertTrackLength+trackSize.y))*trackSize.y + fmod (i*moveStep, (vertTrackLength+trackSize.y)) - vertTrackLength;
	      if ((int)floor(i*moveStep/(vertTrackLength+trackSize.y))%2 == 0)
                y = centerPos[centerLocation][1] + vertTrackLength*0.5 - vertTrackLength;
	      else
	        y = centerPos[centerLocation][1] + vertTrackLength*0.5;
	    }
	  labScenarios->AllocateObstacle(Box (4.5-trackSize.y/2, 4.5+trackSize.y/2, 3, 6, z, z+trackSize.z/2), roomSize, clientRS);
        }
      else if (shapeCategary == CROSS) 
        {
          double centerPos[3] = {roomSize.x*0.5, roomSize.y*0.5, z};
	  if (i<15)
 	    {
  	      x = centerPos[0];
	      y = centerPos[1]+(-platformSize*0.25+i*moveStep);
	    }
	  else
 	    {
  	      x = centerPos[0]+(-platformSize*0.25+(i-15)*moveStep);
	      y = centerPos[1];
	    }
	  labScenarios->AllocateObstacle(Box (centerPos[0]-trackSize.y/2, centerPos[0]+trackSize.y/2, 3, 6, z, z+trackSize.z/2), roomSize, clientRS);
        }
    }
  else if (hermesFlag == MULTIWIMOVE) 
    {
	x = (double)(roomX/(apNumber*2)) + (apIter-1)*roomX/apNumber;
	y = roomY*0.5+(-platformSize*0.5+i*moveStep);
        labScenarios->AllocateObstacle(Box (x-trackSize.y/2, x+trackSize.y/2, 3, 6, z, z+trackSize.z/2), roomSize, clientRS); 	
    }
  else
    {
      x = roomX/(i+1.0) * apIter;
      labScenarios->AllocateObstacle(Box (x-apDimension.x/2, x+apDimension.x/2, y-apDimension.y/2, y+apDimension.y/2, z-apDimension.z, z+apDimension.z), roomSize, clientRS); 
    } 

  /* Setting mobility model */
  MobilityHelper mobility;
  Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator> ();
  Vector apPos = Vector (x, y, z);
  std::vector<Vector> clientPos = labScenarios->IdentifyCLientLocation (clientRS, distRS, depSD, clientNo, clientDistType);
  positionAlloc->Add (apPos);	/* PCP/AP */
  std::vector<bool> losFlag;
  for (uint16_t clientId = 0; clientId < clientNo; clientId++)
    positionAlloc->Add (clientPos.at(clientId));  /* DMG STA */
  labScenarios->LoSAnalysis (apPos, clientPos, apDimension);
  wifiChannel->SetScenarioModel(labScenarios);

  for (uint16_t clientId = 0; clientId < clientNo; clientId++)
    {
      std::pair<double, double> fadingInfo = labScenarios->GetFadingInfo(apPos, clientPos.at(clientId));
      losFlag.push_back(fadingInfo.first);
    }

  mobility.SetPositionAllocator (positionAlloc);
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.Install (apWifiNode);
  mobility.Install (staWifiNode);

  /* Internet stack*/
  InternetStackHelper stack;
  stack.Install (apWifiNode);
  stack.Install (staWifiNode);

  Ipv4AddressHelper address;
  address.SetBase ("10.0.0.0", "255.255.255.0");
  Ipv4InterfaceContainer apInterface;
  apInterface = address.Assign (apDevice);
  Ipv4InterfaceContainer staInterface;
  staInterface = address.Assign (staDevice);

  /* Populate routing table */
  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

  /* We do not want any ARP packets */
  PopulateArpCache ();

  ApplicationContainer sourceApplications, sinkApplications;
  uint32_t portNumber = 9;
  for (uint8_t index = 0; index < clientNo; ++index)
    {
      auto ipv4 = staWifiNode.Get (index)->GetObject<Ipv4> ();
      const auto address = ipv4->GetAddress (1, 0).GetLocal ();
      InetSocketAddress sinkSocket (address, portNumber++);
      OnOffHelper src ("ns3::UdpSocketFactory", sinkSocket); 
      src.SetAttribute ("MaxBytes", UintegerValue (0));
      src.SetAttribute ("PacketSize", UintegerValue (payloadSize));
      src.SetAttribute ("OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=1e6]"));
      src.SetAttribute ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0]"));
      src.SetAttribute ("DataRate", DataRateValue (DataRate (dataRate)));
      sourceApplications.Add (src.Install (apWifiNode.Get (0)));
      PacketSinkHelper packetSinkHelper ("ns3::UdpSocketFactory", sinkSocket);
      sinkApplications.Add (packetSinkHelper.Install (staWifiNode.Get (index)));
   }

  sinkApplications.Start (Seconds (0.0));
  sinkApplications.Stop (Seconds (simulationTime));
  sourceApplications.Start (Seconds (1.0));
  sourceApplications.Stop (Seconds (simulationTime));

  /* Print Traces */
  if (pcapTracing)
    {
      wifiPhy.SetPcapDataLinkType (YansWifiPhyHelper::DLT_IEEE802_11_RADIO);
      wifiPhy.EnablePcap ("Traces/AccessPoint", apDevice, false);
      wifiPhy.EnablePcap ("Traces/Station", staDevice, false);
    }

  /*apWifiNetDevice = StaticCast<WifiNetDevice> (apDevice.Get (0));
  staWifiNetDevice = StaticCast<WifiNetDevice> (staDevice.Get (0));
  apWifiMac = StaticCast<DmgApWifiMac> (apWifiNetDevice->GetMac ());
  staWifiMac = StaticCast<DmgStaWifiMac> (staWifiNetDevice->GetMac ());
  staWifiMac->TraceConnectWithoutContext ("Assoc", MakeBoundCallback (&StationAssoicated, staWifiMac));*/

  Simulator::Stop (Seconds (simulationTime + 1));
  Simulator::Run ();
  Simulator::Destroy ();

  /* Print Results Summary */
  std::cerr << i << " " << centerLocation << " "  << clientRS << " "  << apPos << " ";
  std::copy(clientPos.begin(), clientPos.end(), std::ostream_iterator<Vector>(std::cerr, " "));
  std::copy(losFlag.begin(), losFlag.end(), std::ostream_iterator<bool>(std::cerr, " "));
  for (unsigned index = 0; index < sinkApplications.GetN (); ++index)
    {
      uint64_t totalPacketsThrough = StaticCast<PacketSink> (sinkApplications.Get (index))->GetTotalRx ();
      throughput += ((totalPacketsThrough * 8) / ((simulationTime-1) * 1000000.0)); //Mbit/s
      std::cerr << ((totalPacketsThrough * 8) / ((simulationTime-1) * 1000000.0)) << " ";
    }
  std::cerr << std::endl;
  return 0;
}

