/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2018 YUbing
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

#ifndef OBSTACLE_H
#define OBSTACLE_H

#include <ns3/vector.h>
#include <ns3/box.h>
#include <ns3/object.h>
#include "rtnorm.h"

namespace ns3 {

#define POISSON_DISTRIBUTION_CLIENT 0
#define TRUNCATED_NORMAL_DISTRIBUTION_CLIENT  1
#define OBSTACLE_DEPENDENT_TRUNCATED_NORMAL_DISTRIBUTION_CLIENT 2
#define OBSTACLE_DEPENDENT_DISTRIBUTION_CLIENT 3
#define UNIFORMLLY_DISTRIBUTION_CLIENT 4
#define LINE_OF_SIGHT 0
#define NON_LINE_OF_SIGHT 1
#define CELLPHONE_STA 0
#define LAPTOP_STA 1
#define TABLET_STA 2

class Obstacle : public Object
{
public:
  Obstacle ();
  virtual ~Obstacle ();

  std::pair<double, double> GetFadingInfo (Vector txPos, Vector rxPos);
  void SetObstacleNumber (uint16_t obsNumber);
  void AllocateObstacle (Box railLocation, Vector roomSize,  uint16_t clientRS);
  void LoSAnalysis (Vector apLocation, std::vector<Vector> clientLocation, Vector apDimension);
  std::vector<Vector> IdentifyCLientLocation (uint16_t clientRS, uint16_t distRS, double variance, uint32_t clientNo, uint16_t clientDistType);
  int GetIntersection(float fDst1, float fDst2, Vector P1, Vector P2, Vector &Hit);
  int InBox(Vector Hit, Vector B1, Vector B2, const int Axis);

private:
  typedef struct {
    bool losFlag;
    double fadingLoss;
    Vector apPos;
    Vector clientPos;
  } ChannelInfo;

  std::vector<ChannelInfo> m_channelInfo;
  uint32_t m_obstalceNumber;
  std::vector<Box> m_obstacleDimension;
  std::vector<Vector> m_hitList;
  std::vector<double> m_obstaclePenetrationLoss;
  std::vector<double> m_xObsTNDistConf, m_yObsTNDistConf, m_zObsTNDistConf;

  Vector m_roomSize;
  double m_xMean;
  double m_yMean;
  Vector m_clientDimension;
  Ptr<TruncatedNormalDistribution> m_tNDist;
  Box m_clientSize;
  uint16_t m_clientRS;
};


} //namespace ns3

#endif /* OBSTACLE_H */
