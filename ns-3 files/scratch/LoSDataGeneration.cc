/*
 * Copyright (c) 2015, 2016 IMDEA Networks Institute
 * Author: Hany Assasa <hany.assasa@gmail.com>
 */
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
#include <fstream>      // std::ofstream
#include <random>

#include <gsl/gsl_rng.h>

#define PI 3.14159265
#define STATIC_AP 0
#define HERMES 1
#define BRUTEFORCE 2

/**
 * Simulation Objective:
 * This script is used to evaluate the performance of Hermes++ and static AP performance 
 * To evaluate CSMA/CA channel access scheme: scheme=1
 * To evaluate SP channel access scheme: scheme=0
 */

NS_LOG_COMPONENT_DEFINE ("CompareAccessSchemes");

using namespace ns3;
using namespace std;

/**  Application Variables **/
uint64_t totalRx = 0;
double throughput = 0;
uint32_t allocationType = 0;               /* The type of channel access scheme during DTI (CBAP is the default) */

/*Ptr<WifiNetDevice> apWifiNetDevice;
Ptr<WifiNetDevice> staWifiNetDevice;

Ptr<DmgApWifiMac> apWifiMac;
Ptr<DmgStaWifiMac> staWifiMac;

void
StationAssoicated (Ptr<DmgStaWifiMac> staWifiMac, Mac48Address address)
{
  if (allocationType == SERVICE_PERIOD_ALLOCATION)
    {
      std::cout << "Allocate DTI as Service Period" << std::endl;
      apWifiMac->AllocateDTIAsServicePeriod (1, staWifiMac->GetAssociationID (), AID_AP);
    }
}*/

int
main(int argc, char *argv[])
{
  LogComponentEnable ("CompareAccessSchemes", LOG_LEVEL_ALL);
  //LogComponentEnable ("MacLow", LOG_LEVEL_ALL);
  //LogComponentEnable ("EdcaTxopN", LOG_LEVEL_ALL);
  //LogComponentEnable ("Obstacle", LOG_LEVEL_ALL);
  //LogComponentEnable ("YansWifiChannel", LOG_LEVEL_ALL);
  //LogComponentEnable ("TruncatedNormalDistribution", LOG_LEVEL_ALL);
  //LogComponentEnable ("WifiPhy", LOG_LEVEL_ALL);
  //LogComponentEnable ("Directional60GhzAntenna", LOG_LEVEL_ALL);
  //LogComponentEnable ("WifiRemoteStationManager", LOG_LEVEL_ALL);

  uint32_t payloadSize = 1472;                  /* Application payload size in bytes. */
  string dataRate = "4000Mbps";                  /* Application data rate. */
  uint32_t msduAggregationSize = 7935;          /* The maximum aggregation size for A-MSDU in Bytes. */
  uint32_t mpduAggregationSize = 262143;        /* The maximum aggregation size for A-MSPU in Bytes. */
  uint32_t queueSize = 1000;                    /* Wifi MAC Queue Size. */
  string phyMode = "DMG_MCS12";                 /* Type of the Physical Layer. */
  bool verbose = false;                         /* Print Logging Information. */
  double simulationTime = 1.8;                   /* Simulation time in seconds. */
  bool pcapTracing = false;                     /* PCAP Tracing is enabled. */
  double x = 9;
  double y = 6;
  uint16_t clientRS = 1;
  uint16_t distRS = 1;
  uint16_t i = 0;
  uint16_t ii = 1;
  double platformSize = 30;
  Vector roomSize = Vector (9.0, 6.0, 3.0);
  Vector trackSize = Vector(0, 0.065, 0); // x dimension is a parameter to be changed
  Vector apDimension = Vector (0.23, 0.23, 0.12);
  double depSD = 1;
  uint32_t clientNo = 10;
  uint32_t hermesFlag = BRUTEFORCE; 
  uint16_t clientDistType = 3;
  uint16_t obsNumber = 43;
  uint16_t timeStep = 1;  
  double movingSpeed = 0.04;
  uint32_t optTimeStep = 0;
  uint32_t optSum = 0;
  uint32_t edgeTraversed = 0;
  double z = 3-apDimension.z*0.5-trackSize.z;
  bool edgeTraversedFlag = 0;
  float smallEdge = 1.5;
  float largeEdge = 4.5;
  double moveStep = 0.1;
  uint16_t cycleLength = 180;

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
  cmd.AddValue ("x", "ap x", x);
  cmd.AddValue ("y", "ap y", y);
  cmd.AddValue ("i", "simulation iteration", i);
  cmd.AddValue ("ii", "simulation iteration ii", ii);
  cmd.AddValue ("z", "ap z", z);
  cmd.AddValue ("platformSize", "platform Size", platformSize); 
  cmd.AddValue ("clientRS", "random seed for client", clientRS);
  cmd.AddValue ("distRS", "random seed for truncated normal distribution", distRS);
  cmd.AddValue ("depSD", "random seed for dependent distribution", depSD);
  cmd.AddValue ("clientNo", "Number of client", clientNo);
  cmd.AddValue ("hermesFlag", "0 means static AP scenario, 1 means hermes scenario", hermesFlag);
  cmd.AddValue ("clientDistType", "distribution type for client", clientDistType);
  cmd.AddValue ("obsNumber", "obstacle Number", obsNumber);  
  cmd.AddValue ("timeStep", "time Step", timeStep);  
  cmd.Parse (argc, argv);

  // scale parameter due to shell without float value
  platformSize = platformSize*0.1;
  depSD = depSD*0.1;
  x = roomSize.x*0.5;
  y = 3.02;

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
  wifiPhy.Set ("TxPowerStart", DoubleValue (10.0));
  wifiPhy.Set ("TxPowerEnd", DoubleValue (10.0));
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

  // Identify current client number and activate join or leave case
  std::default_random_engine generator;
  std::poisson_distribution<int> distribution(1);
  for (uint16_t i=0; i<floor((timeStep+cycleLength)/cycleLength); i++)
    {
      // Join event or dropping out event
      uint16_t poissonFlag = distribution(generator);
      for (uint16_t j=0; j<poissonFlag; j++)
	{
          RngSeedManager::SetSeed (clientRS);	  
          Ptr<UniformRandomVariable> eventUV = CreateObject<UniformRandomVariable> ();
          eventUV->SetAttribute ("Min", DoubleValue (0.0));
          eventUV->SetAttribute ("Max", DoubleValue (1.99));
	  uint16_t eventType = floor(eventUV->GetValue ());
	  if (eventType == 0)
	    clientNo=clientNo+1;
	  else if (clientNo > 1)
	    clientNo=clientNo-1;	  
	}  
	edgeTraversed = 0;	
    }


  /* Make nodes and set them up with the PHY and the MAC */
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
  RngSeedManager::SetStream (0);
  Ptr<Obstacle> labScenarios = CreateObject<Obstacle> ();
  labScenarios->SetObstacleNumber(obsNumber);

  // Define the AP inital location 
  Vector Destination, optDestination;
  std::ifstream ifs;
  labScenarios->AllocateObstacle(Box (4.5-trackSize.y/2, 4.5+trackSize.y/2, 3, 6, z, z+trackSize.y/2), roomSize, clientRS);

  if (hermesFlag == STATIC_AP)
    {
      x = roomSize.x*0.5;
      y = roomSize.y*0.5+(-platformSize*0.5+i*moveStep);
    }
  else if (timeStep == 0) 
    {
      y = 3.02;
      std::ofstream ofs;
      ofs.open ("/home/yubing/Projects/WiMove/Final_version_v1/Results/apCurrentPos.txt", std::ofstream::out);
      ofs << x << " " << y << " " << z << " " << 0 << " " << 0 << std::endl;  
      ofs.close();
    }
  else if (timeStep != 0)
    {
      // Identify current AP location
      ifs.open("/home/yubing/Projects/WiMove/Final_version_v1/Results/apCurrentPos.txt");
      ifs >> x >> y >> z >> edgeTraversed >> edgeTraversedFlag;
      if (timeStep%cycleLength == 0)
       edgeTraversed = 0;
      ifs.close();

     // Identify destination
     if (hermesFlag == HERMES)
	{
	  //apLoS = input;
	  //losflag + new los matrix
          //identify LoS location for new client with minimum distance
          //Destination.y = location with maximum los
	}
     else if (hermesFlag==BRUTEFORCE && edgeTraversed!=2)
        {
 	  //std::cerr << edgeTraversed << " "  << x << " " << y << " "  << z << " ";
	  if (y == smallEdge)
	    {
	      edgeTraversed += 1;
	      Destination.y = largeEdge;
	      edgeTraversedFlag = 0;
	    } 
	  else if (y == largeEdge)
	    {
	      edgeTraversed += 1;
	      Destination.y = smallEdge;
	      edgeTraversedFlag = 1;
	    } 
	  else if (edgeTraversed == 0)
	    {
	      if (largeEdge-y >= y-smallEdge)
		Destination.y = smallEdge;
	      else
		Destination.y = largeEdge;
	    }
	  else if (edgeTraversed == 1)
	    {
	      if (edgeTraversedFlag == 0)
		Destination.y = largeEdge;
	      else
		Destination.y = smallEdge;
	    }
	} else {
	      uint32_t optSum=0, optTimeStep=0;
	      std::ifstream ifs;
              ifs.open("Results/OptimalLoc.txt");
	      ifs >> optTimeStep >> optSum >> Destination.x >> Destination.y >> Destination.z;
	      ifs.close();	 
	}

	// calculate AP's next location
	if (timeStep%cycleLength != 0)
	  {               
	    if (Destination.y - y > movingSpeed)
		y = y + movingSpeed;
	    else if (y - Destination.y > movingSpeed)
		y = y - movingSpeed;
	  }

      // Save current AP location
      std::ofstream ofs;
      ofs.open ("/home/yubing/Projects/WiMove/Final_version_v1/Results/apCurrentPos.txt", std::ofstream::out);
      ofs << x << " " << y << " " << z << " " << edgeTraversed << " " << edgeTraversedFlag << std::endl;  
      ofs.close();
    }

  /* Setting mobility model */
  MobilityHelper mobility;
  Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator> ();
  Vector apPos = Vector (x, y, z);
  // Generate first k client
  std::vector<Vector> clientPos = labScenarios->IdentifyCLientLocation (clientRS, distRS, depSD, clientNo, clientDistType);
  positionAlloc->Add (apPos);	/* PCP/AP */
  std::vector<bool> losFlag;
  for (uint16_t clientId = 0; clientId < clientNo; clientId++)
    positionAlloc->Add (clientPos.at(clientId));  /* DMG STA */

  // Collect los infomation between all clients
  std::ofstream ofs;
  if (i == 0)
    {
      for (uint16_t i = 0; i < clientNo; i++)
        {  
          std::vector<bool> losClientFlag;
          labScenarios->LoSAnalysis (clientPos.at(i), clientPos, Vector (0, 0, 0));

          for (uint16_t clientId = 0; clientId < clientNo; clientId++)
            {
              std::pair<double, double> fadingInfo = labScenarios->GetFadingInfo(clientPos.at(i), clientPos.at(clientId));
              losClientFlag.push_back(fadingInfo.first);
            }
          std::ofstream ofs;
          ofs.open ("/home/yubing/Projects/WiMove/Final_version_v1/Results/ClientLoS.txt", std::ofstream::out | std::ofstream::app);
          for (uint8_t i=0; i<clientNo; i++)
            ofs << 1-losClientFlag.at(i) << " ";
          ofs << std::endl;
          ofs.close();
        }

      ofs.open ("/home/yubing/Projects/WiMove/Final_version_v1/Results/ClientDistance.txt", std::ofstream::out | std::ofstream::app);
      for (uint16_t i = 0; i < clientNo; i++)
        {
          for (uint16_t j = 0; j < clientNo; j++)
            {
               ofs << CalculateDistance (clientPos.at(i), clientPos.at(j)) << " "; 
            }	
	  ofs << std::endl;
        }
      ofs.close();

      ofs.open ("/home/yubing/Projects/WiMove/Final_version_v1/Results/ClientPos.txt", std::ofstream::out | std::ofstream::app);
      for (uint16_t i = 0; i < clientNo; i++)
        ofs << clientPos.at(i).x << " " << clientPos.at(i).y << " " << clientPos.at(i).z << " "; 
      ofs << std::endl;
      ofs.close();
   }

  // Collect los infomation between AP and all clients
  labScenarios->LoSAnalysis (apPos, clientPos, apDimension);
  wifiChannel->SetScenarioModel(labScenarios);

  for (uint16_t clientId = 0; clientId < clientNo; clientId++)
    {
      std::pair<double, double> fadingInfo = labScenarios->GetFadingInfo(apPos, clientPos.at(clientId));
      losFlag.push_back(1-fadingInfo.first);
    }

  ofs.open ("/home/yubing/Projects/WiMove/Final_version_v1/Results/APLoS.txt", std::ofstream::out | std::ofstream::app);
  for (uint8_t i=0; i<clientNo; i++)
    ofs << losFlag.at(i) << " ";
  ofs << std::endl;
  ofs.close();

  ofs.open ("/home/yubing/Projects/WiMove/Final_version_v1/Results/APDistance.txt", std::ofstream::out | std::ofstream::app);
  for (uint8_t i=0; i<clientNo; i++)
    ofs << CalculateDistance (clientPos.at(i), apPos) << " ";
  ofs << std::endl;
  ofs.close();

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

  Simulator::Stop (Seconds (simulationTime + 1));
  Simulator::Run ();

  ofs.open ("/home/yubing/Projects/WiMove/Final_version_v1/Results/MCS.txt", std::ofstream::out | std::ofstream::app);
  for (uint8_t index = 0; index < clientNo; ++index)
    {
      Ptr<WifiNetDevice> staWifiNetDevice = StaticCast<WifiNetDevice> (staDevice.Get (index));
      ofs << staWifiNetDevice->GetPhy()->GetMostMcs() << " "; 
    }
  ofs << std::endl;
  ofs.close();

  ofs.open ("/home/yubing/Projects/WiMove/Final_version_v1/Results/RSS.txt", std::ofstream::out | std::ofstream::app);
  for (uint8_t index = 0; index < clientNo; ++index)
    {
      Ptr<WifiNetDevice> staWifiNetDevice = StaticCast<WifiNetDevice> (staDevice.Get (index));
      ofs << staWifiNetDevice->GetPhy()->GetRss() << " ";       
    }
  ofs << std::endl;
  ofs.close();

  /* Print Results Summary */
  std::cerr << timeStep << " "  << Destination.y << " " << clientNo << " "  << apPos << " ";
  std::copy(clientPos.begin(), clientPos.end(), std::ostream_iterator<Vector>(std::cerr, " "));
  std::copy(losFlag.begin(), losFlag.end(), std::ostream_iterator<bool>(std::cerr, " "));

  ofs.open ("/home/yubing/Projects/WiMove/Final_version_v1/Results/Throughput.txt", std::ofstream::out | std::ofstream::app);
  for (unsigned index = 0; index < sinkApplications.GetN (); ++index)
    {
      uint64_t totalPacketsThrough = StaticCast<PacketSink> (sinkApplications.Get (index))->GetTotalRx ();
      throughput += ((totalPacketsThrough * 8) / ((simulationTime-1) * 1000000.0)); //Mbit/s
      std::cerr << ((totalPacketsThrough * 8) / ((simulationTime-1) * 1000000.0)) << " ";
      ofs << ((totalPacketsThrough * 8) / ((simulationTime-1) * 1000000.0)) << " ";
    }
  std::cerr << std::endl;
  ofs << std::endl;
  ofs.close();

  // Identify location with maximum performance
  uint32_t curSum = std::accumulate(losFlag.begin(), losFlag.end(), 0); 
  if (hermesFlag == BRUTEFORCE)
    {
      if (timeStep%cycleLength==0)
	{
	  ofs.open ("Results/OptimalLoc.txt", std::ofstream::out);
	  ofs << timeStep << " " << curSum << " " << x << " " << y << " " << z << std::endl;
          ofs.close();
	}
      else
	{
          ifs.open("Results/OptimalLoc.txt");
	  ifs >> optTimeStep >> optSum >> Destination.x >> Destination.y >> Destination.z;
	  ifs.close();	 
	  if (curSum >= optSum)
            {
	      ofs.open ("Results/OptimalLoc.txt", std::ofstream::out);
	      ofs << timeStep << " " << curSum << " " << x << " " << y << " " << z << std::endl;
              ofs.close();
    	   }
	}	
    }

  Simulator::Destroy ();

  //std::cerr << clientNo << " " << timeStep << " " << optTimeStep << " " << curSum << " " << optSum << " " << x << " "  << y << " "  << z  << " " << edgeTraversed << std::endl;
  return 0;
}

