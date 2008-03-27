/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "ns3/core-module.h"
#include "ns3/mobility-module.h"
#include "ns3/helper-module.h"
#include "ns3/simulator-module.h"

using namespace ns3;

static void 
CourseChange (std::string context, Ptr<const MobilityModel> position)
{
  Vector pos = position->GetPosition ();
  std::cout << Simulator::Now () << ", pos=" << position << ", x=" << pos.x << ", y=" << pos.y
            << ", z=" << pos.z << std::endl;
}

int main (int argc, char *argv[])
{
  CommandLine cmd;
  cmd.Parse (argc, argv);


  NodeContainer c;
  c.Create (10000);

  MobilityHelper mobility;
  mobility.EnableNotifier ();
  mobility.SetPositionAllocator ("ns3::RandomDiscPositionAllocator",
                                 "X", String ("100.0"),
                                 "Y", String ("100.0"),
                                 "Rho", String ("Uniform:0:30"));
  mobility.SetMobilityModel ("ns3::StaticMobilityModel");
  mobility.Layout (c);

  Config::Connect ("/NodeList/*/$ns3::MobilityModelNotifier/CourseChange",
                              MakeCallback (&CourseChange));
  
  Simulator::StopAt (Seconds (100.0));

  Simulator::Run ();
  
  return 0;
}
