/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2018 Yubing
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
 */

#include "obstacle.h"
#include <ns3/log.h>
#include <random>
#include <cmath>
#include <vector>
#include <ns3/random-variable-stream.h>
#include "ns3/simulator.h"
#include "ns3/core-module.h"
#include <fstream>      // std::ofstream

#define PI 3.14159265

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("Obstacle");

NS_OBJECT_ENSURE_REGISTERED (Obstacle);

Obstacle::Obstacle ()
{
  NS_LOG_FUNCTION (this);  
  // Obstacle configuration based on practical scenario
  m_obstalceNumber = 172;
  m_obstaclePenetrationLoss = {1130, 1470, 1700, 9, 60, 9, 860, 430, 130, 112};  
  /*m_xObsTNDistConf = {0.5, 1.75, 1.08, 0.18};
  m_yObsTNDistConf = {0.25, 1.25, 0.56, 0.08};
  m_zObsTNDistConf = {0.15, 1.75, 0.61, 0.21}; */
  m_xObsTNDistConf = {0.01, 1.75, 0.54, 0.18};
  m_yObsTNDistConf = {0.01, 1.25, 0.23, 0.08};
  m_zObsTNDistConf = {0.15, 1.75, 0.61, 0.21};  

  // Create TN distribution object
  m_tNDist = CreateObject<TruncatedNormalDistribution> ();
}

Obstacle::~Obstacle ()
{
  NS_LOG_FUNCTION (this);
}

void 
Obstacle::SetObstacleNumber (uint16_t obsNumber)
{
  m_obstalceNumber = obsNumber;
}

std::pair<double, double> 
Obstacle::GetFadingInfo (Vector txPos, Vector rxPos)
{
  NS_LOG_FUNCTION (this << txPos << rxPos << m_channelInfo.size());
  std::pair<double, double> fadingInfo;
  if (txPos.x == rxPos.x && txPos.y == rxPos.y && txPos.z == rxPos.z)
    {
      fadingInfo.first = 0;
      fadingInfo.second = 0;
      return fadingInfo;
    }

  for (uint16_t i=0; i<m_channelInfo.size(); i++)
    {
      if ((txPos.x == m_channelInfo.at(i).apPos.x || txPos.x == m_channelInfo.at(i).clientPos.x) && (rxPos.x == m_channelInfo.at(i).apPos.x || rxPos.x == m_channelInfo.at(i).clientPos.x) && (txPos.y == m_channelInfo.at(i).apPos.y || txPos.y == m_channelInfo.at(i).clientPos.y) && (rxPos.y == m_channelInfo.at(i).apPos.y || rxPos.y == m_channelInfo.at(i).clientPos.y) && (txPos.z == m_channelInfo.at(i).apPos.z || txPos.z == m_channelInfo.at(i).clientPos.z) && (rxPos.z == m_channelInfo.at(i).apPos.z || rxPos.z == m_channelInfo.at(i).clientPos.z))
        {
          fadingInfo.first = m_channelInfo.at(i).losFlag;
          fadingInfo.second = m_channelInfo.at(i).fadingLoss; 
          break;         
        }
    }  
  return fadingInfo;
}

void
Obstacle::AllocateObstacle(Box railLocation, Vector roomSize, uint16_t clientRS)
{
  NS_LOG_FUNCTION (this);
  m_roomSize = roomSize;
  std::vector<Box> m_obstacleDimensionComparison;  

  //--- GSL random init ---
  gsl_rng_env_setup();                          // Read variable environnement
  const gsl_rng_type* type = gsl_rng_default;   // Default algorithm 'twister'
  gsl_rng *gen = gsl_rng_alloc (type);          // Rand generator allocation
  gsl_rng_set(gen, 3);

  // Poisson point process obstacle number identification
  std::default_random_engine generator;
  std::poisson_distribution<int> distribution(m_obstalceNumber);
  m_obstalceNumber = distribution(generator);
  NS_LOG_FUNCTION (this << m_obstalceNumber);

  for (uint16_t t=0; t<m_obstalceNumber; t++)
     {
       // Place obstacle based on Poisson point process 
       Ptr<UniformRandomVariable> obstacleNoUV = CreateObject<UniformRandomVariable> ();
       obstacleNoUV->SetAttribute ("Min", DoubleValue (0.0));
       obstacleNoUV->SetAttribute ("Max", DoubleValue (m_roomSize.x));
       double x = obstacleNoUV->GetValue ();
       obstacleNoUV->SetAttribute ("Min", DoubleValue (0.0));
       obstacleNoUV->SetAttribute ("Max", DoubleValue (m_roomSize.y));     
       double y = obstacleNoUV->GetValue ();  

       // Define size of obstacle
       std::pair<double, double> obstacleXRange, obstacleYRange, obstacleZRange;
       std::vector<double> tempXDistConf = m_xObsTNDistConf;
       std::vector<double> tempYDistConf = m_yObsTNDistConf;
       if (m_roomSize.x - x < tempXDistConf.at(1) * 0.5 || x < tempXDistConf.at(1) * 0.5)
         tempXDistConf.at(1) = (m_roomSize.x - x > x)?x*2:2*(m_roomSize.x-x);
       if (m_roomSize.y - y < tempYDistConf.at(1) * 0.5 || y < tempYDistConf.at(1) * 0.5)
        tempYDistConf.at(1) = (m_roomSize.y - y > y)?y*2:2*(m_roomSize.y-y);
       if (tempXDistConf.at(1) <= tempXDistConf.at(0) || tempYDistConf.at(1) <= tempYDistConf.at(0))
         {
           t--;
           continue;
         }
       obstacleXRange = m_tNDist->rtnorm(gen,tempXDistConf.at(0),tempXDistConf.at(1),tempXDistConf.at(2),tempXDistConf.at(3));
       obstacleYRange = m_tNDist->rtnorm(gen,tempYDistConf.at(0),tempYDistConf.at(1),tempYDistConf.at(2),tempYDistConf.at(3));
       obstacleZRange = m_tNDist->rtnorm(gen,m_zObsTNDistConf.at(0),m_zObsTNDistConf.at(1),m_zObsTNDistConf.at(2),m_zObsTNDistConf.at(3));

       NS_LOG_FUNCTION (this << obstacleXRange.first << obstacleYRange.first << m_obstalceNumber << x << y);
       m_obstacleDimension.push_back(Box (x-obstacleXRange.first*0.5, x+obstacleXRange.first*0.5, y-obstacleYRange.first*0.5, y+obstacleYRange.first*0.5, 0, obstacleZRange.first));

       // collision detection
       m_obstacleDimensionComparison.push_back(railLocation);
       m_obstacleDimensionComparison.push_back(m_obstacleDimension.at(t));
     }

  // Change obstacle location based on Poisson distribution
  /*for (uint16_t i=0; i<m_obstalceNumber*0.1; i++)
    {
      Ptr<UniformRandomVariable> obstacleIdUV = CreateObject<UniformRandomVariable> ();
      obstacleIdUV->SetAttribute ("Min", DoubleValue (0.0));
      obstacleIdUV->SetAttribute ("Max", DoubleValue (m_obstalceNumber-0.01));
      uint16_t obstacleId = floor(obstacleIdUV->GetValue ());  

      std::default_random_engine generator;
      std::poisson_distribution<int> distribution(0.1);
      for (uint16_t j=0; j<clientRS/10; j++)
        {
          uint16_t movingFlag = distribution(generator);
          //std::cout << movingFlag << std::endl;
          Ptr<UniformRandomVariable> movingAxisUV = CreateObject<UniformRandomVariable> ();
          movingAxisUV->SetAttribute ("Min", DoubleValue (0.0));
          movingAxisUV->SetAttribute ("Max", DoubleValue (PI*2));
          uint16_t movingAngle = movingAxisUV->GetValue ();

          if (movingFlag > 0)
            {
              if (m_obstacleDimension.at(obstacleId).xMax+0.1*cos(movingAngle)<m_roomSize.x && m_obstacleDimension.at(obstacleId).xMin+0.1*cos(movingAngle)>0)
                {
                  m_obstacleDimension.at(obstacleId).xMin += 0.1*cos(movingAngle);
                  m_obstacleDimension.at(obstacleId).xMax += 0.1*cos(movingAngle);
                }
              if (m_obstacleDimension.at(obstacleId).yMax+0.1*sin(movingAngle)<m_roomSize.y && m_obstacleDimension.at(obstacleId).yMin+0.1*cos(movingAngle)>0)
                {
                  m_obstacleDimension.at(obstacleId).yMin += 0.1*sin(movingAngle);
                  m_obstacleDimension.at(obstacleId).yMax += 0.1*sin(movingAngle);               
                } 
            }
        }
    }*/

  std::ofstream ofs;
  ofs.open ("obstacleList.txt", std::ofstream::out);
  ofs << "obs| " << m_obstalceNumber << std::endl;

  // Allocate obstacle type for penetration analysis
  for(uint16_t j=0; j<m_obstalceNumber; j++)
    { 
       ofs << m_obstacleDimension.at(j) << std::endl;
       NS_LOG_FUNCTION (this << m_obstalceNumber << m_obstacleDimension.at(j));
       Ptr<UniformRandomVariable> obstacleIndex = CreateObject<UniformRandomVariable> ();
       obstacleIndex->SetAttribute ("Min", DoubleValue (0.0));
       obstacleIndex->SetAttribute ("Max", DoubleValue (9.99));
       int index = floor(obstacleIndex->GetValue ());
       m_obstaclePenetrationLoss.push_back (m_obstaclePenetrationLoss.at(index));
    }
  ofs.close();
}
   
void
Obstacle::LoSAnalysis (Vector apLocation, std::vector<Vector> clientLocation, Vector apDimension)
{
  NS_LOG_FUNCTION (this << m_obstalceNumber);
  //std::pair<std::vector<bool>, std::vector<double>> channelStats;
  m_channelInfo.clear();
  for (uint16_t clientId = 0; clientId < clientLocation.size(); clientId++)
    {
      bool channelStatus = LINE_OF_SIGHT;
      double fadingLoss = 1e7;

      // Identify antenna model dimension
      Box antennaSize = Box {apLocation.x-apDimension.x*0.5, apLocation.x+apDimension.x*0.5, apLocation.y-apDimension.y*0.5, apLocation.y+apDimension.y*0.5, apLocation.z-apDimension.z*0.5, apLocation.z+apDimension.z*0.5};

      std::vector<Vector> apAntennaEdge = {{antennaSize.xMin, antennaSize.yMin, antennaSize.zMin}, {antennaSize.xMin, antennaSize.yMin, antennaSize.zMax}, {antennaSize.xMin, antennaSize.yMax, antennaSize.zMin}, {antennaSize.xMin, antennaSize.yMax, antennaSize.zMax}, {antennaSize.xMax, antennaSize.yMin, antennaSize.zMin}, {antennaSize.xMax, antennaSize.yMin, antennaSize.zMax}, {antennaSize.xMax, antennaSize.yMax, antennaSize.zMin}, {antennaSize.xMax, antennaSize.yMax, antennaSize.zMax}};

      Vector clientAntenna = clientLocation.at(clientId);

      // LoS analysis
      for (uint16_t i=0; i<1; i++)
        {
          double tempFadingLoss = 0;
          bool tempChannelStatus = LINE_OF_SIGHT;
          for (uint16_t j=0; j<m_obstalceNumber; j++) 
            {
              bool hitFlag = false;
              m_hitList.clear();
              Vector Hit = Vector (0, 0, 0);
              Vector B1 = Vector (m_obstacleDimension.at(j).xMin, m_obstacleDimension.at(j).yMin, m_obstacleDimension.at(j).zMin);
              Vector B2 = Vector (m_obstacleDimension.at(j).xMax, m_obstacleDimension.at(j).yMax, m_obstacleDimension.at(j).zMax);
              if (clientAntenna.x < m_obstacleDimension.at(j).xMin && apAntennaEdge.at(i).x < m_obstacleDimension.at(j).xMin) continue;
              if (clientAntenna.x > m_obstacleDimension.at(j).xMax && apAntennaEdge.at(i).x > m_obstacleDimension.at(j).xMax) continue;
              if (clientAntenna.y < m_obstacleDimension.at(j).yMin && apAntennaEdge.at(i).y < m_obstacleDimension.at(j).yMin) continue;
              if (clientAntenna.y > m_obstacleDimension.at(j).yMax && apAntennaEdge.at(i).y > m_obstacleDimension.at(j).yMax) continue;
              if (clientAntenna.z < m_obstacleDimension.at(j).zMin && apAntennaEdge.at(i).z < m_obstacleDimension.at(j).zMin) continue;
              if (clientAntenna.z > m_obstacleDimension.at(j).zMax && apAntennaEdge.at(i).z > m_obstacleDimension.at(j).zMax) continue;
              bool insideFlag = 0;
              if (clientAntenna.x >= m_obstacleDimension.at(j).xMin && clientAntenna.x <= m_obstacleDimension.at(j).xMax && clientAntenna.y >= m_obstacleDimension.at(j).yMin && clientAntenna.y <= m_obstacleDimension.at(j).yMax && clientAntenna.z >= m_obstacleDimension.at(j).zMin && clientAntenna.z <= m_obstacleDimension.at(j).zMax) 
                {
                  insideFlag = true;
                  m_hitList.push_back (clientAntenna);
                }              

              bool x1 = GetIntersection(apAntennaEdge.at(i).x-B1.x, clientAntenna.x-B1.x, apAntennaEdge.at(i), clientAntenna, Hit) && InBox(Hit, B1, B2, 1);
              bool x2 = GetIntersection(apAntennaEdge.at(i).x-B2.x, clientAntenna.x-B2.x, apAntennaEdge.at(i), clientAntenna, Hit) && InBox(Hit, B1, B2, 1);
              bool y1 = GetIntersection(apAntennaEdge.at(i).y-B1.y, clientAntenna.y-B1.y, apAntennaEdge.at(i), clientAntenna, Hit) && InBox(Hit, B1, B2, 2);
              bool y2 = GetIntersection(apAntennaEdge.at(i).y-B2.y, clientAntenna.y-B2.y, apAntennaEdge.at(i), clientAntenna, Hit) && InBox(Hit, B1, B2, 2);
              bool z1 = GetIntersection(apAntennaEdge.at(i).z-B1.z, clientAntenna.z-B1.z, apAntennaEdge.at(i), clientAntenna, Hit) && InBox(Hit, B1, B2, 3);
              bool z2 = GetIntersection(apAntennaEdge.at(i).z-B2.z, clientAntenna.z-B2.z, apAntennaEdge.at(i), clientAntenna, Hit) && InBox(Hit, B1, B2, 3);
              NS_LOG_FUNCTION (clientAntenna << m_obstacleDimension.at(j) << j << x1 << x2 << y1 << y2 << z1 << z2);

              if (x1 + y1 + z1 + x2 + y2 + z2 + insideFlag > 1)
                {
                  hitFlag = true;
	          tempChannelStatus = NON_LINE_OF_SIGHT;
                }

              if (hitFlag == true)
                {
                  tempFadingLoss += CalculateDistance(m_hitList.at(0), m_hitList.at(1))*m_obstaclePenetrationLoss.at(j);
                } 
            }
          fadingLoss = (fadingLoss <= tempFadingLoss)?fadingLoss:tempFadingLoss;
          if (tempChannelStatus == LINE_OF_SIGHT) 
            channelStatus = LINE_OF_SIGHT;
        }

      if (fadingLoss > 0)
        channelStatus = NON_LINE_OF_SIGHT;

      bool multipathFlag = true;
      double multipathFading;
      if (multipathFlag)
        {
          std::default_random_engine generator (m_clientRS);
          std::normal_distribution<double> multipathDistribution(0,2.24);
          multipathFading = multipathDistribution(generator);
          fadingLoss = fadingLoss - multipathFading;
        } 

      m_channelInfo.push_back(ChannelInfo());
      m_channelInfo.at(clientId).losFlag = channelStatus;
      m_channelInfo.at(clientId).fadingLoss = -fadingLoss;
      m_channelInfo.at(clientId).apPos = apLocation;
      m_channelInfo.at(clientId).clientPos = clientLocation.at(clientId);
        
      //channelStats.first.push_back (channelStatus);
      //channelStats.second.push_back(-fadingLoss);

      //std::cout<< channelStatus << " " << fadingLoss << " " << apLocation << " " << m_channelInfo.at(clientId).clientPos << " " << clientId << std::endl; 
      NS_LOG_FUNCTION ("los nlos status" << channelStatus << fadingLoss);
    }
}

int 
Obstacle::GetIntersection(float fDst1, float fDst2, Vector P1, Vector P2, Vector &Hit) 
{
  //NS_LOG_FUNCTION (this << fDst1 << fDst2);
  if ( (fDst1 * fDst2) >= 0.0f) return 0;
  if ( fDst1 == fDst2) return 0; 
  Hit = P1 + Vector (-fDst1/(fDst2-fDst1)*(P2.x-P1.x), -fDst1/(fDst2-fDst1)*(P2.y-P1.y), -fDst1/(fDst2-fDst1)*(P2.z-P1.z));
  return 1;
}

int 
Obstacle::InBox(Vector Hit, Vector B1, Vector B2, const int Axis) 
{
  //NS_LOG_FUNCTION (this << Hit << B1 << B2);
  if ( Axis==1 && Hit.z > B1.z && Hit.z < B2.z && Hit.y > B1.y && Hit.y < B2.y) 
    {
      m_hitList.push_back (Hit);
      return 1;
    }
  if ( Axis==2 && Hit.z > B1.z && Hit.z < B2.z && Hit.x > B1.x && Hit.x < B2.x)
    {
      m_hitList.push_back (Hit);
      return 1;
    }
  if ( Axis==3 && Hit.x > B1.x && Hit.x < B2.x && Hit.y > B1.y && Hit.y < B2.y)
    {
      m_hitList.push_back (Hit);
      return 1;
    }
  return 0;
}


std::vector<Vector>
Obstacle::IdentifyCLientLocation (uint16_t clientRS, uint16_t distRS, double variance, uint32_t clientNo, uint16_t clientDistType)
{
  NS_LOG_FUNCTION (this);
  // Set random seed
  m_clientRS = clientRS;
  gsl_rng_env_setup();                          // Read variable environnement
  const gsl_rng_type* type = gsl_rng_default;   // Default algorithm 'twister'
  gsl_rng *gen = gsl_rng_alloc (type);          // Rand generator allocation
  gsl_rng_set(gen, clientRS);

  std::vector<Vector> clientPos;
  bool heightDefined = false;
  for (uint16_t clientId = 0; clientId < clientNo; clientId++)
    {
      double x = 0, y = 0, z = 0;
      uint8_t distributionType = clientDistType;

      // Client size configuration
      RngSeedManager::SetSeed (clientRS*distRS);
      Ptr<UniformRandomVariable> clientDimensionIndex = CreateObject<UniformRandomVariable> ();
      clientDimensionIndex->SetAttribute ("Min", DoubleValue (0.0));
      clientDimensionIndex->SetAttribute ("Max", DoubleValue (1.99));
      int clientType = floor(clientDimensionIndex->GetValue ());

      /*if (clientType == CELLPHONE_STA)
        m_clientDimension = Vector (0.157, 0.075, 0.008);  
      else if (clientType == LAPTOP_STA)
        m_clientDimension = Vector (0.33, 0.23, 0.25);*/
      // based on antenna size
      if (clientType == CELLPHONE_STA)
        m_clientDimension = Vector (0.001, 0.001, 0.001);  
      else if (clientType == LAPTOP_STA)
        m_clientDimension = Vector (0.001, 0.001, 0.001);

      if (distributionType == POISSON_DISTRIBUTION_CLIENT)
        {
          // Poisson distribution
          Ptr<UniformRandomVariable> xUV = CreateObject<UniformRandomVariable> ();
          xUV->SetAttribute ("Min", DoubleValue (0.0));
          xUV->SetAttribute ("Max", DoubleValue (m_roomSize.x));
          Ptr<UniformRandomVariable> yUV = CreateObject<UniformRandomVariable> ();
          yUV->SetAttribute ("Min", DoubleValue (0.0));
          yUV->SetAttribute ("Max", DoubleValue (m_roomSize.y));
          x=xUV->GetValue ();   
          y=yUV->GetValue ();  
        }
      else if (distributionType == TRUNCATED_NORMAL_DISTRIBUTION_CLIENT)
        {
          RngSeedManager::SetSeed (distRS);
          Ptr<UniformRandomVariable> Mean = CreateObject<UniformRandomVariable> ();
          Mean->SetAttribute ("Min", DoubleValue (0.0));
          Mean->SetAttribute ("Max", DoubleValue (m_roomSize.x));
          m_xMean = Mean->GetValue ();
          Mean->SetAttribute ("Min", DoubleValue (0.0));
          Mean->SetAttribute ("Max", DoubleValue (m_roomSize.y));
          m_yMean = Mean->GetValue ();

          // Truncated Normal Distribution 
          std::vector<double> xTNStaDistConf = {0, m_roomSize.x, m_xMean, 1};
          std::vector<double> yTNStaDistConf = {0, m_roomSize.y, m_yMean, 1};

          std::pair<double, double> distSample;
          distSample = m_tNDist->rtnorm(gen,xTNStaDistConf.at(0),xTNStaDistConf.at(1),xTNStaDistConf.at(2),xTNStaDistConf.at(3));
          x = distSample.first;
          distSample = m_tNDist->rtnorm(gen,yTNStaDistConf.at(0),yTNStaDistConf.at(1),yTNStaDistConf.at(2),yTNStaDistConf.at(3));
          y = distSample.first;
          //std::cout<< m_xMean << " " << m_yMean << " " << x << " " << y << std::endl;
        }
      else if (distributionType == OBSTACLE_DEPENDENT_TRUNCATED_NORMAL_DISTRIBUTION_CLIENT)
        {
          // Identify which obstacle to select
          RngSeedManager::SetSeed (clientRS*distRS);
          Ptr<UniformRandomVariable> obstacleIndex = CreateObject<UniformRandomVariable> ();
          obstacleIndex->SetAttribute ("Min", DoubleValue (0.0));
          obstacleIndex->SetAttribute ("Max", DoubleValue (m_obstalceNumber-0.01));
          int obsId = floor(obstacleIndex->GetValue ());

          // Identify which side of obstacle to be distributed
          std::vector<double> xTNDistConf;
          std::vector<double> yTNDistConf; 
          obstacleIndex->SetAttribute ("Min", DoubleValue (0.0));
          obstacleIndex->SetAttribute ("Max", DoubleValue (1.99));
 
          int distSide = floor(obstacleIndex->GetValue ());
          if (distSide == 0)
              xTNDistConf = {0, m_obstacleDimension.at(obsId).xMin, 0.5*(m_obstacleDimension.at(obsId).xMax+m_obstacleDimension.at(obsId).xMin), variance}; 
          else
              xTNDistConf = {m_obstacleDimension.at(obsId).xMax, m_roomSize.x, 0.5*(m_obstacleDimension.at(obsId).xMax+m_obstacleDimension.at(obsId).xMin), variance};

          distSide = floor(obstacleIndex->GetValue ());
          if (distSide == 0)
              yTNDistConf = {0, m_obstacleDimension.at(obsId).yMin, 0.5*(m_obstacleDimension.at(obsId).yMax+m_obstacleDimension.at(obsId).yMin), variance};
          else
              yTNDistConf = {m_obstacleDimension.at(obsId).yMax, m_roomSize.y, 0.5*(m_obstacleDimension.at(obsId).yMax+m_obstacleDimension.at(obsId).yMin), variance};

          // Truncated Normal Distribution
          std::pair<double, double> distSample;
          distSample = m_tNDist->rtnorm(gen,xTNDistConf.at(0),xTNDistConf.at(1),xTNDistConf.at(2),xTNDistConf.at(3));
          x = distSample.first;
          distSample = m_tNDist->rtnorm(gen,yTNDistConf.at(0),yTNDistConf.at(1),yTNDistConf.at(2),yTNDistConf.at(3));
          y = distSample.first;
          //std::cout<< m_obstacleDimension.at(obsId) << " " << obsId << " " << x << " " << y << std::endl; 
        }   
      else if (distributionType == OBSTACLE_DEPENDENT_DISTRIBUTION_CLIENT)
        {
          heightDefined = true;
          bool STAOutobs = false;
          while (STAOutobs == false)
            {
              // Identify which obstacle to select
              RngSeedManager::SetSeed (clientRS*distRS);
              Ptr<UniformRandomVariable> obstacleIndex = CreateObject<UniformRandomVariable> ();
              obstacleIndex->SetAttribute ("Min", DoubleValue (0.0));
              obstacleIndex->SetAttribute ("Max", DoubleValue (m_obstalceNumber-0.01));
              int obsId = floor(obstacleIndex->GetValue ());

              // Identify which side of obstacle to be distributed
              std::vector<double> xTNDistConf;
              std::vector<double> yTNDistConf; 
              obstacleIndex->SetAttribute ("Min", DoubleValue (0.0));
              obstacleIndex->SetAttribute ("Max", DoubleValue (4.99));
 
              RngSeedManager::SetSeed (clientRS*distRS);
              Ptr<UniformRandomVariable> clientCoordinateX = CreateObject<UniformRandomVariable> ();
              clientCoordinateX->SetAttribute ("Min", DoubleValue (m_obstacleDimension.at(obsId).xMin)); 
              clientCoordinateX->SetAttribute ("Max", DoubleValue (m_obstacleDimension.at(obsId).xMax));
              Ptr<UniformRandomVariable> clientCoordinateY = CreateObject<UniformRandomVariable> ();
              clientCoordinateY->SetAttribute ("Min", DoubleValue (m_obstacleDimension.at(obsId).yMin)); 
              clientCoordinateY->SetAttribute ("Max", DoubleValue (m_obstacleDimension.at(obsId).yMax));
              Ptr<UniformRandomVariable> clientCoordinateZ = CreateObject<UniformRandomVariable> ();
              clientCoordinateZ->SetAttribute ("Min", DoubleValue (m_obstacleDimension.at(obsId).zMin)); 
              clientCoordinateZ->SetAttribute ("Max", DoubleValue (m_obstacleDimension.at(obsId).zMax));

              int distSide = floor(obstacleIndex->GetValue ()); // left, front, right, rear, up
              if (distSide == 0)
                {
                  x = m_obstacleDimension.at(obsId).xMin;
                  y = clientCoordinateY->GetValue (); 
                  z = clientCoordinateZ->GetValue ();   
                  x -= m_clientDimension.x*0.5;          
                }
              else if (distSide == 1)
                {
                  x = clientCoordinateX->GetValue (); 
                  y = m_obstacleDimension.at(obsId).yMin;
                  z = clientCoordinateZ->GetValue ();
                  y -= m_clientDimension.y*0.5;         
                }
              else if (distSide == 2)
                {
                  x = m_obstacleDimension.at(obsId).xMax;
                  y = clientCoordinateY->GetValue (); 
                  z = clientCoordinateZ->GetValue (); 
                  x += m_clientDimension.x*0.5;             
                }
              else if (distSide == 3)
                {
                  x = clientCoordinateX->GetValue (); 
                  y = m_obstacleDimension.at(obsId).yMax;
                  z = clientCoordinateZ->GetValue (); 
                  y += m_clientDimension.y*0.5;     
                }
              else if (distSide == 4)
                {
                  x = clientCoordinateX->GetValue ();
                  y = clientCoordinateY->GetValue (); 
                  z = m_obstacleDimension.at(obsId).zMax; 
                  z += m_clientDimension.z*0.5;    
                }
              STAOutobs = true;
              for (uint32_t i=0; i<m_obstalceNumber; i++) 
              if (x <= m_obstacleDimension.at(i).xMax && x >= m_obstacleDimension.at(i).xMin && y <= m_obstacleDimension.at(i).yMax && y >= m_obstacleDimension.at(i).yMin && z <= m_obstacleDimension.at(i).zMax && z >= m_obstacleDimension.at(i).zMin)
                {
                  STAOutobs = false;
                  break; 
                }
            }
          //std::cout<< m_obstacleDimension.at(obsId) << " " << m_clientDimension << " " << distSide << " " << x << " " << y << " " << z << std::endl; 
        } 
      else if (distributionType == UNIFORMLLY_DISTRIBUTION_CLIENT)
        {
          double grandularity = 1;
          x = (clientRS-1)%(int)(m_roomSize.x/grandularity)*grandularity;
          y = floor((clientRS-1)/(m_roomSize.x/grandularity))*grandularity;
        }

      // Identigy height of STA
      if (heightDefined == false)
        {
          for (uint32_t i=0; i<m_obstalceNumber; i++) 
          if (x <= m_obstacleDimension.at(i).xMax && x >= m_obstacleDimension.at(i).xMin && y <= m_obstacleDimension.at(i).yMax && y >= m_obstacleDimension.at(i).yMin)
            {
              z = m_obstacleDimension.at(i).zMax;
              break;
            } 
          z += m_clientDimension.z*0.5; 
        }
      // Identify client dimension
      m_clientSize = Box (x-m_clientDimension.x*0.5, x+m_clientDimension.x*0.5, y-m_clientDimension.y*0.5, y+m_clientDimension.y*0.5, z-m_clientDimension.z*0.5, z+m_clientDimension.z*0.5);

      NS_LOG_FUNCTION ("client location" << x << y << z << " client dimension " << m_clientDimension);
      clientPos.push_back (Vector (x, y, z));
    }
  return clientPos;  
}

} //namespace ns3
