#include "ns3/aodv-module.h"
#include "ns3/applications-module.h"
#include "ns3/core-module.h"
#include "ns3/dsdv-module.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/internet-module.h"
#include "ns3/network-module.h"
#include "ns3/olsr-module.h"
#include "ns3/point-to-point-module.h"

#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <sstream>
#include <vector>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("SimpleTopologySimulator");

// Topology types enumeration
enum TopologyType
{
  LINEAR = 1,
  STAR = 2,
  RING = 3,
  MESH = 4,
  TREE = 5
};

// Traffic application types
enum TrafficType
{
  BULK_SEND = 1,
  ON_OFF = 2,
  CBR = 3
};

// TCP variants
enum TcpVariant
{
  TCP_NEWRENO = 1,
  TCP_CUBIC = 2,
  TCP_HIGHSPEED = 3,
  TCP_VEGAS = 4
};

// Routing protocols
enum RoutingProtocol
{
  STATIC = 1,
  DSDV = 2,
  AODV = 3,
  OLSR = 4
};

class NetworkTopologySimulator
{
private:
  uint32_t m_numNodes;
  TopologyType m_topology;
  TrafficType m_trafficType;
  TcpVariant m_tcpVariant;
  std::string m_dataRate;
  std::string m_delay;
  uint32_t m_queueSize;
  double m_startTime;
  double m_stopTime;
  RoutingProtocol m_routingProtocol;
  std::vector<uint32_t> m_senders;
  std::vector<uint32_t> m_receivers;
  std::string m_outputPrefix;

  NodeContainer m_nodes;
  std::vector<NetDeviceContainer> m_devices;
  std::vector<Ipv4InterfaceContainer> m_interfaces;
  ApplicationContainer m_sinkApps;
  ApplicationContainer m_sourceApps;

  // Performance monitoring
  Ptr<FlowMonitor> m_flowMonitor;
  FlowMonitorHelper m_flowHelper;
  std::map<uint32_t, Ptr<PacketSink>> m_sinks;
  // traffic parameters
  std::vector<uint32_t> m_injectionSenders;
  std::vector<uint32_t> m_injectionReceivers;
  bool m_enableInjection;
  double m_injectionStartDelay;
  void FilterAndDisplayFlows();

public:
  NetworkTopologySimulator()
      : m_numNodes(6),
        m_topology(LINEAR),
        m_trafficType(BULK_SEND),
        m_tcpVariant(TCP_NEWRENO),
        m_dataRate("10Mbps"),
        m_delay("2ms"),
        m_queueSize(100),
        m_startTime(1.0),
        m_stopTime(10.0),
        m_routingProtocol(STATIC),
        m_outputPrefix("results"),
        m_enableInjection(false),
        m_injectionStartDelay(2.0)
  {
  }

  void Configure(int argc, char *argv[]);
  void CreateTopology();
  void SetupApplications();
  void RunSimulation();
  void PrintResults();
  void ShowMenu();
  void SetupRoutingProtocol();
  void SetupTrafficApplicationWithDelay(uint32_t sender,
                                        uint32_t receiver,
                                        uint16_t port,
                                        double startDelay);
  void VerifyInjectionFlows();

private:
  void CreateLinearTopology();
  void CreateStarTopology();
  void CreateRingTopology();
  void CreateMeshTopology();
  void CreateTreeTopology();
  void SetupTcpVariant();
  void InstallInternetStack();
  void AssignIpAddresses();
  void SetupTrafficApplication(uint32_t sender, uint32_t receiver, uint16_t port);
  Ipv4Address GetNodeIpAddress(uint32_t nodeId);
  void PrintTopologyInfo();
  void PrintPerformanceMetrics();
  void SaveResultsToFile();
};

void NetworkTopologySimulator::ShowMenu()
{
  std::cout << "\n===============================================\n";
  std::cout << "    SIMPLE NS-3 NETWORK TOPOLOGY SIMULATOR\n";
  std::cout << "===============================================\n";
  std::cout << "Topology Options:\n";
  std::cout << "1. Linear/Line Topology\n";
  std::cout << "2. Star Topology\n";
  std::cout << "3. Ring/Circle Topology\n";
  std::cout << "4. Full Mesh Topology\n";
  std::cout << "5. Tree Topology\n";
  std::cout << "\nTraffic Types:\n";
  std::cout << "1. Bulk Send (High throughput)\n";
  std::cout << "2. On-Off (Bursty traffic)\n";
  std::cout << "3. CBR (Constant Bit Rate)\n";
  std::cout << "\nTCP Variants:\n";
  std::cout << "1. TCP NewReno\n";
  std::cout << "2. TCP Cubic\n";
  std::cout << "3. TCP HighSpeed\n";
  std::cout << "4. TCP Vegas\n";
  std::cout << "\nRouting Protocols:\n";
  std::cout << "1. Static Routing (Default)\n";
  std::cout << "2. DSDV (Destination-Sequenced Distance Vector)\n";
  std::cout << "3. AODV (Ad-hoc On-Demand Distance Vector)\n";
  std::cout << "4. OLSR (Optimized Link State Routing)\n";
  std::cout << "\nTraffic Injection Options:\n";
  std::cout << "--inject=true --injectSender=0,2 --injectReceiver=5,3\n";
  std::cout << "--injectDelay=0.5 (start injection at 50% of simulation)\n";
  std::cout << "===============================================\n";
}

void NetworkTopologySimulator::Configure(int argc, char *argv[])
{
  ShowMenu();

  CommandLine cmd;
  uint32_t topology = 1, traffic = 1, tcp = 1, routing = 1;
  std::string senders = "0", receivers = "5";

  cmd.AddValue("topology", "Topology type (1:Linear, 2:Star, 3:Ring, 4:Mesh, 5:Tree)", topology);
  cmd.AddValue("nodes", "Number of nodes", m_numNodes);
  cmd.AddValue("traffic", "Traffic type (1:Bulk, 2:OnOff, 3:CBR)", traffic);
  cmd.AddValue("tcp", "TCP variant (1:NewReno, 2:Cubic, 3:HighSpeed, 4:Vegas)", tcp);
  cmd.AddValue("datarate", "Link data rate", m_dataRate);
  cmd.AddValue("delay", "Link delay", m_delay);
  cmd.AddValue("queue", "Queue size", m_queueSize);
  cmd.AddValue("startTime", "Start time", m_startTime);
  cmd.AddValue("stopTime", "Stop time", m_stopTime);
  cmd.AddValue("start", "Start time", m_startTime);
  cmd.AddValue("stop", "Stop time", m_stopTime);
  cmd.AddValue("sender", "Sender nodes (comma-separated)", senders);
  cmd.AddValue("receiver", "Receiver nodes (comma-separated)", receivers);
  cmd.AddValue("output", "Output file prefix", m_outputPrefix);
  cmd.AddValue("routing", "Routing protocol (1:Static, 2:DSDV, 3:AODV, 4:OLSR)", routing);
  std::string injectionSenders = "", injectionReceivers = "";
  cmd.AddValue("inject", "Enable traffic injection (true/false)", m_enableInjection);
  cmd.AddValue("injectSender", "Additional sender nodes (comma-separated)", injectionSenders);
  cmd.AddValue("injectReceiver",
               "Additional receiver nodes (comma-separated)",
               injectionReceivers);
  cmd.AddValue("injectDelay", "Delay before injection starts (seconds)", m_injectionStartDelay);

  cmd.Parse(argc, argv);

  // Set enums
  m_topology = static_cast<TopologyType>(topology);
  m_trafficType = static_cast<TrafficType>(traffic);
  m_tcpVariant = static_cast<TcpVariant>(tcp);
  m_routingProtocol = static_cast<RoutingProtocol>(routing); // ADD THIS LINE!

  // Parse sender and receiver lists
  std::stringstream ss_senders(senders);
  std::stringstream ss_receivers(receivers);
  std::string item;

  m_senders.clear();
  m_receivers.clear();

  // Adjust simulation time for routing protocols
  if (m_routingProtocol != STATIC)
  {
    double minTime = (m_routingProtocol == DSDV) ? 30.0 : 20.0; // Increased DSDV time
    if (m_stopTime < minTime)
    {
      std::cout << "Warning: Increasing simulation time to " << minTime << "s to allow "
                << ((m_routingProtocol == DSDV) ? "DSDV" : "routing") << " convergence\n";
      m_stopTime = std::max(m_stopTime, minTime);
    }
  }

  while (std::getline(ss_senders, item, ','))
  {
    m_senders.push_back(std::stoi(item));
  }
  while (std::getline(ss_receivers, item, ','))
  {
    m_receivers.push_back(std::stoi(item));
  }

  // Validation
  if (m_senders.size() != m_receivers.size())
  {
    NS_FATAL_ERROR("Number of senders must equal number of receivers");
  }

  for (auto sender : m_senders)
  {
    if (sender >= m_numNodes)
    {
      NS_FATAL_ERROR("Sender node " << sender << " exceeds number of nodes");
    }
  }

  for (auto receiver : m_receivers)
  {
    if (receiver >= m_numNodes)
    {
      NS_FATAL_ERROR("Receiver node " << receiver << " exceeds number of nodes");
    }
  }

  if (m_enableInjection)
  {
    double minSimTime = 15.0; // Minimum for meaningful injection analysis
    double routingDelay = (m_routingProtocol == STATIC) ? 0.1
                          : (m_routingProtocol == DSDV) ? 10.0
                                                        : 5.0;

    double effectiveInjectionStart =
        routingDelay + (m_stopTime - m_startTime) * m_injectionStartDelay;
    double remainingTime = m_stopTime - m_startTime - effectiveInjectionStart;

    if (remainingTime < 5.0)
    {
      std::cout << "WARNING: Insufficient time for injection analysis!\n";
      std::cout << "  Current remaining time after injection: " << remainingTime << "s\n";
      std::cout << "  Recommended: Increase --stopTime to at least "
                << (m_startTime + effectiveInjectionStart + 5.0) << "\n";

      // Auto-extend simulation time
      m_stopTime = m_startTime + effectiveInjectionStart + 8.0;
      std::cout << "  Auto-extending simulation to " << m_stopTime << "s\n";
    }

    std::cout << "INJECTION TIMING ANALYSIS:\n";
    std::cout << "  Routing delay: " << routingDelay << "s\n";
    std::cout << "  Injection delay factor: " << m_injectionStartDelay << "\n";
    std::cout << "  Calculated injection start: " << (m_startTime + effectiveInjectionStart)
              << "s\n";
    std::cout << "  Time for injection analysis: " << remainingTime << "s\n";
  }
  // Parse injection -->traffic parameters
  if (m_enableInjection && !injectionSenders.empty() && !injectionReceivers.empty())
  {
    // Parse injection senders
    std::stringstream ss_inject_senders(injectionSenders);
    std::string inject_item;
    m_injectionSenders.clear();
    while (std::getline(ss_inject_senders, inject_item, ','))
    {
      uint32_t sender = std::stoi(inject_item);
      if (sender >= m_numNodes)
      {
        NS_FATAL_ERROR("Injection sender node " << sender << " exceeds number of nodes");
      }
      m_injectionSenders.push_back(sender);
    }

    // Parse injection receivers
    std::stringstream ss_inject_receivers(injectionReceivers);
    m_injectionReceivers.clear();
    while (std::getline(ss_inject_receivers, inject_item, ','))
    {
      uint32_t receiver = std::stoi(inject_item);
      if (receiver >= m_numNodes)
      {
        NS_FATAL_ERROR("Injection receiver node " << receiver
                                                  << " exceeds number of nodes");
      }
      m_injectionReceivers.push_back(receiver);
    }

    // Validate injection parameters
    if (m_injectionSenders.size() != m_injectionReceivers.size())
    {
      NS_FATAL_ERROR("Number of injection senders must equal number of injection receivers");
    }

    std::cout << "Traffic injection enabled: " << m_injectionSenders.size()
              << " additional flows\n";
    for (size_t i = 0; i < m_injectionSenders.size(); ++i)
    {
      std::cout << "  Injection Flow " << (i + 1) << ": Node " << m_injectionSenders[i]
                << " -> Node " << m_injectionReceivers[i] << std::endl;
    }
  }
  else if (m_enableInjection)
  {
    std::cout << "Warning: Traffic injection enabled but no injection flows specified\n";
    m_enableInjection = false;
  }
}

void NetworkTopologySimulator::SetupTcpVariant()
{
  switch (m_tcpVariant)
  {
  case TCP_NEWRENO:
    Config::SetDefault("ns3::TcpL4Protocol::SocketType", StringValue("ns3::TcpNewReno"));
    break;
  case TCP_CUBIC:
    Config::SetDefault("ns3::TcpL4Protocol::SocketType", StringValue("ns3::TcpCubic"));
    break;
  case TCP_HIGHSPEED:
    Config::SetDefault("ns3::TcpL4Protocol::SocketType", StringValue("ns3::TcpHighSpeed"));
    break;
  case TCP_VEGAS:
    Config::SetDefault("ns3::TcpL4Protocol::SocketType", StringValue("ns3::TcpVegas"));
    break;
  }
}

void NetworkTopologySimulator::CreateTopology()
{
  NS_LOG_INFO("Creating topology with " << m_numNodes << " nodes");

  m_nodes.Create(m_numNodes);

  switch (m_topology)
  {
  case LINEAR:
    CreateLinearTopology();
    break;
  case STAR:
    CreateStarTopology();
    break;
  case RING:
    CreateRingTopology();
    break;
  case MESH:
    CreateMeshTopology();
    break;
  case TREE:
    CreateTreeTopology();
    break;
  }

  InstallInternetStack();
  AssignIpAddresses();
}

void NetworkTopologySimulator::CreateLinearTopology()
{
  NS_LOG_INFO("Creating Linear Topology");

  PointToPointHelper p2p;
  p2p.SetDeviceAttribute("DataRate", StringValue(m_dataRate));
  p2p.SetChannelAttribute("Delay", StringValue(m_delay));
  p2p.SetQueue("ns3::DropTailQueue", "MaxSize", StringValue(std::to_string(m_queueSize) + "p"));

  for (uint32_t i = 0; i < m_numNodes - 1; ++i)
  {
    NodeContainer pair = NodeContainer(m_nodes.Get(i), m_nodes.Get(i + 1));
    NetDeviceContainer devices = p2p.Install(pair);
    m_devices.push_back(devices);
  }
}

void NetworkTopologySimulator::CreateStarTopology()
{
  NS_LOG_INFO("Creating Star Topology");

  PointToPointHelper p2p;
  p2p.SetDeviceAttribute("DataRate", StringValue(m_dataRate));
  p2p.SetChannelAttribute("Delay", StringValue(m_delay));
  p2p.SetQueue("ns3::DropTailQueue", "MaxSize", StringValue(std::to_string(m_queueSize) + "p"));

  uint32_t centerNode = 0;

  for (uint32_t i = 1; i < m_numNodes; ++i)
  {
    NodeContainer pair = NodeContainer(m_nodes.Get(centerNode), m_nodes.Get(i));
    NetDeviceContainer devices = p2p.Install(pair);
    m_devices.push_back(devices);
  }
}

void NetworkTopologySimulator::CreateRingTopology()
{
  NS_LOG_INFO("Creating Ring Topology");

  PointToPointHelper p2p;
  p2p.SetDeviceAttribute("DataRate", StringValue(m_dataRate));
  p2p.SetChannelAttribute("Delay", StringValue(m_delay));
  p2p.SetQueue("ns3::DropTailQueue", "MaxSize", StringValue(std::to_string(m_queueSize) + "p"));

  for (uint32_t i = 0; i < m_numNodes; ++i)
  {
    uint32_t next = (i + 1) % m_numNodes;
    NodeContainer pair = NodeContainer(m_nodes.Get(i), m_nodes.Get(next));
    NetDeviceContainer devices = p2p.Install(pair);
    m_devices.push_back(devices);
  }
}

void NetworkTopologySimulator::CreateMeshTopology()
{
  NS_LOG_INFO("Creating Full Mesh Topology");

  PointToPointHelper p2p;
  p2p.SetDeviceAttribute("DataRate", StringValue(m_dataRate));
  p2p.SetChannelAttribute("Delay", StringValue(m_delay));
  p2p.SetQueue("ns3::DropTailQueue", "MaxSize", StringValue(std::to_string(m_queueSize) + "p"));

  for (uint32_t i = 0; i < m_numNodes; ++i)
  {
    for (uint32_t j = i + 1; j < m_numNodes; ++j)
    {
      NodeContainer pair = NodeContainer(m_nodes.Get(i), m_nodes.Get(j));
      NetDeviceContainer devices = p2p.Install(pair);
      m_devices.push_back(devices);
    }
  }
}

void NetworkTopologySimulator::CreateTreeTopology()
{
  NS_LOG_INFO("Creating Tree Topology");

  PointToPointHelper p2p;
  p2p.SetDeviceAttribute("DataRate", StringValue(m_dataRate));
  p2p.SetChannelAttribute("Delay", StringValue(m_delay));
  p2p.SetQueue("ns3::DropTailQueue", "MaxSize", StringValue(std::to_string(m_queueSize) + "p"));

  for (uint32_t i = 0; i < m_numNodes; ++i)
  {
    uint32_t leftChild = 2 * i + 1;
    uint32_t rightChild = 2 * i + 2;

    if (leftChild < m_numNodes)
    {
      NodeContainer pair = NodeContainer(m_nodes.Get(i), m_nodes.Get(leftChild));
      NetDeviceContainer devices = p2p.Install(pair);
      m_devices.push_back(devices);
    }

    if (rightChild < m_numNodes)
    {
      NodeContainer pair = NodeContainer(m_nodes.Get(i), m_nodes.Get(rightChild));
      NetDeviceContainer devices = p2p.Install(pair);
      m_devices.push_back(devices);
    }
  }
}

void NetworkTopologySimulator::SetupRoutingProtocol()
{
  switch (m_routingProtocol)
  {
  case STATIC:
  {
    InternetStackHelper stack;
    stack.Install(m_nodes);
    break;
  }
  case DSDV:
  {
    DsdvHelper dsdv;
    dsdv.Set("PeriodicUpdateInterval", TimeValue(Seconds(3)));
    dsdv.Set("SettlingTime", TimeValue(Seconds(1)));
    InternetStackHelper stack;
    stack.SetRoutingHelper(dsdv);
    stack.Install(m_nodes);
    break;
  }
  case AODV:
  {
    AodvHelper aodv;
    InternetStackHelper stack;
    stack.SetRoutingHelper(aodv);
    stack.Install(m_nodes);
    break;
  }
  case OLSR:
  {
    OlsrHelper olsr;
    InternetStackHelper stack;
    stack.SetRoutingHelper(olsr);
    stack.Install(m_nodes);
    break;
  }
  }
}

void NetworkTopologySimulator::InstallInternetStack()
{
  SetupRoutingProtocol();
}

void NetworkTopologySimulator::AssignIpAddresses()
{
  Ipv4AddressHelper address;

  for (size_t i = 0; i < m_devices.size(); ++i)
  {
    std::ostringstream subnet;
    subnet << "10.1." << i + 1 << ".0";
    address.SetBase(subnet.str().c_str(), "255.255.255.0");
    Ipv4InterfaceContainer interfaces = address.Assign(m_devices[i]);
    m_interfaces.push_back(interfaces);
  }

  // Only populate static routing tables for STATIC protocol
  if (m_routingProtocol == STATIC)
  {
    Ipv4GlobalRoutingHelper::PopulateRoutingTables();
  }
  else
  {
    // For dynamic routing protocols, give them time to build routing tables
    std::cout << "Using "
              << ((m_routingProtocol == DSDV)   ? "DSDV"
                  : (m_routingProtocol == AODV) ? "AODV"
                                                : "OLSR")
              << " routing protocol - allowing convergence time\n";

    if (m_routingProtocol == DSDV)
    {
      std::cout << "DSDV requires additional settling time...\n";
    }
  }
}

Ipv4Address
NetworkTopologySimulator::GetNodeIpAddress(uint32_t nodeId)
{
  Ptr<Node> node = m_nodes.Get(nodeId);
  Ptr<Ipv4> ipv4 = node->GetObject<Ipv4>();

  if (!ipv4)
  {
    NS_FATAL_ERROR("Node " << nodeId << " does not have IPv4 installed");
  }

  if (m_routingProtocol == DSDV)
  {
    // Give DSDV time to properly set up interfaces
    for (uint32_t i = 0; i < ipv4->GetNInterfaces(); ++i)
    {
      if (ipv4->IsUp(i) && ipv4->GetNAddresses(i) > 0) // Check if interface is up
      {
        Ipv4Address addr = ipv4->GetAddress(i, 0).GetLocal();
        if (addr != Ipv4Address::GetLoopback() && addr != Ipv4Address("0.0.0.0"))
        {
          return addr;
        }
      }
    }
  }

  // For dynamic routing protocols, we need to be more careful
  if (m_routingProtocol != STATIC)
  {
    for (uint32_t i = 1; i < ipv4->GetNInterfaces(); ++i)
    {
      if (ipv4->GetNAddresses(i) > 0)
      {
        Ipv4Address addr = ipv4->GetAddress(i, 0).GetLocal();
        if (addr != Ipv4Address::GetLoopback() && addr != Ipv4Address("0.0.0.0"))
        {
          return addr;
        }
      }
    }
    NS_FATAL_ERROR("Could not find valid IP address for node " << nodeId);
  }
  else
  {
    // Static routing - original logic
    for (uint32_t j = 1; j < ipv4->GetNInterfaces(); ++j)
    {
      if (ipv4->GetNAddresses(j) > 0)
      {
        Ipv4Address addr = ipv4->GetAddress(j, 0).GetLocal();
        if (addr != Ipv4Address::GetLoopback())
        {
          return addr;
        }
      }
    }
  }

  NS_FATAL_ERROR("Could not find valid IP address for node " << nodeId);
  return Ipv4Address("127.0.0.1");
}

void NetworkTopologySimulator::SetupTrafficApplication(uint32_t sender, uint32_t receiver, uint16_t port)
{
  Ipv4Address receiverIp = GetNodeIpAddress(receiver);
  Address sinkAddress(InetSocketAddress(receiverIp, port));

  // Setup sink application
  PacketSinkHelper packetSinkHelper("ns3::TcpSocketFactory",
                                    InetSocketAddress(Ipv4Address::GetAny(), port));
  ApplicationContainer sinkApp = packetSinkHelper.Install(m_nodes.Get(receiver));
  sinkApp.Start(Seconds(m_startTime));
  sinkApp.Stop(Seconds(m_stopTime));
  m_sinkApps.Add(sinkApp);

  Ptr<PacketSink> sink = DynamicCast<PacketSink>(sinkApp.Get(0));
  m_sinks[receiver] = sink;

  // Setup source application based on traffic type
  ApplicationContainer sourceApp;

  switch (m_trafficType)
  {
  case BULK_SEND:
  {
    BulkSendHelper source("ns3::TcpSocketFactory", sinkAddress);
    source.SetAttribute("MaxBytes", UintegerValue(0)); // Unlimited
    source.SetAttribute("SendSize", UintegerValue(1024));
    sourceApp = source.Install(m_nodes.Get(sender));
    break;
  }
  case ON_OFF:
  {
    OnOffHelper source("ns3::TcpSocketFactory", sinkAddress);
    source.SetAttribute("OnTime", StringValue("ns3::ConstantRandomVariable[Constant=1]"));
    source.SetAttribute("OffTime", StringValue("ns3::ConstantRandomVariable[Constant=0]"));
    source.SetAttribute("DataRate", StringValue("1Mbps"));
    source.SetAttribute("PacketSize", UintegerValue(1024));
    sourceApp = source.Install(m_nodes.Get(sender));
    break;
  }
  case CBR:
  {
    OnOffHelper source("ns3::TcpSocketFactory", sinkAddress);
    source.SetAttribute("OnTime", StringValue("ns3::ConstantRandomVariable[Constant=1]"));
    source.SetAttribute("OffTime", StringValue("ns3::ConstantRandomVariable[Constant=0]"));
    source.SetAttribute("DataRate", StringValue("512Kbps"));
    source.SetAttribute("PacketSize", UintegerValue(512));
    sourceApp = source.Install(m_nodes.Get(sender));
    break;
  }
  }
  // Give more time for routing protocols to converge
  double appStartDelay = (m_routingProtocol == STATIC) ? 0.1
                         : (m_routingProtocol == DSDV) ? 10.0
                         : (m_routingProtocol == AODV) ? 0.3
                                                       : 5;
  sourceApp.Start(Seconds(m_startTime + appStartDelay));
  sourceApp.Stop(Seconds(m_stopTime));
  m_sourceApps.Add(sourceApp);
}

void NetworkTopologySimulator::SetupTrafficApplicationWithDelay(uint32_t sender,
                                                                uint32_t receiver,
                                                                uint16_t port,
                                                                double startDelay)
{
  Ipv4Address receiverIp = GetNodeIpAddress(receiver);
  Address sinkAddress(InetSocketAddress(receiverIp, port));

  if (m_sinks.find(receiver) == m_sinks.end())
  {
    PacketSinkHelper packetSinkHelper("ns3::TcpSocketFactory",
                                      InetSocketAddress(Ipv4Address::GetAny(), port));
    ApplicationContainer sinkApp = packetSinkHelper.Install(m_nodes.Get(receiver));
    sinkApp.Start(Seconds(m_startTime));
    sinkApp.Stop(Seconds(m_stopTime));
    m_sinkApps.Add(sinkApp);

    Ptr<PacketSink> sink = DynamicCast<PacketSink>(sinkApp.Get(0));
    m_sinks[receiver] = sink;
  }

  // Setup source application based on traffic type
  ApplicationContainer sourceApp;

  switch (m_trafficType)
  {
  case BULK_SEND:
  {
    BulkSendHelper source("ns3::TcpSocketFactory", sinkAddress);
    source.SetAttribute("MaxBytes", UintegerValue(0)); // Unlimited
    source.SetAttribute("SendSize", UintegerValue(1024));
    sourceApp = source.Install(m_nodes.Get(sender));
    break;
  }
  case ON_OFF:
  {
    OnOffHelper source("ns3::TcpSocketFactory", sinkAddress);
    source.SetAttribute("OnTime", StringValue("ns3::ConstantRandomVariable[Constant=1]"));
    source.SetAttribute("OffTime", StringValue("ns3::ConstantRandomVariable[Constant=0]"));
    source.SetAttribute("DataRate", StringValue("1Mbps"));
    source.SetAttribute("PacketSize", UintegerValue(1024));
    sourceApp = source.Install(m_nodes.Get(sender));
    break;
  }
  case CBR:
  {
    OnOffHelper source("ns3::TcpSocketFactory", sinkAddress);
    source.SetAttribute("OnTime", StringValue("ns3::ConstantRandomVariable[Constant=1]"));
    source.SetAttribute("OffTime", StringValue("ns3::ConstantRandomVariable[Constant=0]"));
    source.SetAttribute("DataRate", StringValue("512Kbps"));
    source.SetAttribute("PacketSize", UintegerValue(512));
    sourceApp = source.Install(m_nodes.Get(sender));
    break;
  }
  }

  // Calculate actual start time with delay
  double actualStartTime = m_startTime + startDelay;
  sourceApp.Start(Seconds(actualStartTime));
  sourceApp.Stop(Seconds(m_stopTime));
  m_sourceApps.Add(sourceApp);

  std::cout << "Injection traffic scheduled: Node " << sender << " -> Node " << receiver
            << " starting at " << actualStartTime << "s\n";
}

void NetworkTopologySimulator::SetupApplications()
{
  uint16_t port = 8080;

  // Setup original traffic flows
  for (size_t i = 0; i < m_senders.size(); ++i)
  {
    SetupTrafficApplication(m_senders[i], m_receivers[i], port + i);
  }

  // Setup injection traffic flows if enabled
  if (m_enableInjection)
  {
    uint16_t injectionPort = port + m_senders.size(); // Continue from regular ports

    // OLD PROBLEMATIC CODE:
    // double injectionStartTime = (m_stopTime - m_startTime) * m_injectionStartDelay;

    // FIXED CODE - Account for routing protocol delays:
    double baseAppDelay = (m_routingProtocol == STATIC) ? 0.1
                          : (m_routingProtocol == DSDV) ? 10.0
                          : (m_routingProtocol == AODV) ? 0.3
                                                        : 5;

    double injectionStartTime =
        baseAppDelay + (m_stopTime - m_startTime) * m_injectionStartDelay;

    std::cout << "\nSetting up traffic injection flows...\n";
    std::cout << "Base app delay: " << baseAppDelay << "s\n";
    std::cout << "Injection will start at: " << (m_startTime + injectionStartTime) << "s\n";

    for (size_t i = 0; i < m_injectionSenders.size(); ++i)
    {
      SetupTrafficApplicationWithDelay(m_injectionSenders[i],
                                       m_injectionReceivers[i],
                                       injectionPort + i,
                                       injectionStartTime);
    }
  }
}

void NetworkTopologySimulator::VerifyInjectionFlows()
{
  if (!m_enableInjection)
  {
    return;
  }

  std::cout << "\n=== VERIFYING INJECTION FLOWS ===\n";

  m_flowMonitor->CheckForLostPackets();
  Ptr<Ipv4FlowClassifier> classifier =
      DynamicCast<Ipv4FlowClassifier>(m_flowHelper.GetClassifier());
  FlowMonitor::FlowStatsContainer stats = m_flowMonitor->GetFlowStats();

  int actualAppFlows = 0;
  int discarded = 0;

  // Build set of expected (src, dst) IP pairs
  std::set<std::pair<Ipv4Address, Ipv4Address>> expectedAppFlows;
  for (size_t i = 0; i < m_senders.size(); ++i)
  {
    expectedAppFlows.insert({GetNodeIpAddress(m_senders[i]), GetNodeIpAddress(m_receivers[i])});
  }

  for (size_t i = 0; i < m_injectionSenders.size(); ++i)
  {
    expectedAppFlows.insert(
        {GetNodeIpAddress(m_injectionSenders[i]), GetNodeIpAddress(m_injectionReceivers[i])});
  }

  // Track matched flows to avoid counting duplicates
  std::set<std::pair<Ipv4Address, Ipv4Address>> matchedAppFlows;

  for (auto i = stats.begin(); i != stats.end(); ++i)
  {
    Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow(i->first);

    // Must be TCP and on app port range
    if (t.protocol != 6 || t.destinationPort < 8080 || t.destinationPort > 8089)
    {
      discarded++;
      continue;
    }

    std::pair<Ipv4Address, Ipv4Address> flowKey = {t.sourceAddress, t.destinationAddress};

    // Count each (src, dst) pair only once
    if (expectedAppFlows.find(flowKey) != expectedAppFlows.end() &&
        matchedAppFlows.find(flowKey) == matchedAppFlows.end())
    {
      matchedAppFlows.insert(flowKey);
      actualAppFlows++;

      std::cout << "✅ Valid App Flow: " << t.sourceAddress << " → " << t.destinationAddress
                << " Port: " << t.destinationPort << " (Tx: " << i->second.txPackets
                << ", Rx: " << i->second.rxPackets << ")\n";
    }
    else
    {
      discarded++;
    }
  }

  std::cout << "Matched Valid App Flows: " << actualAppFlows << std::endl;
  std::cout << "Discarded FlowMonitor Entries: " << discarded << std::endl;
  std::cout << "Expected App Flows: " << (m_senders.size() + m_injectionSenders.size())
            << std::endl;

  if (actualAppFlows == (int)(m_senders.size() + m_injectionSenders.size()))
  {
    std::cout << "✅ All expected flows matched!\n";
  }
  else
  {
    std::cout << "❌ Some expected flows are missing or unmatched!\n";
  }
}

void NetworkTopologySimulator::RunSimulation()
{
  SetupTcpVariant();
  CreateTopology();
  SetupApplications();

  // Install Flow Monitor
  // Only monitor application nodes, not all traffic
  // Install Flow Monitor only on application nodes
  NodeContainer appNodes;
  std::set<uint32_t> uniqueNodes;

  // Add sender and receiver nodes
  for (uint32_t sender : m_senders)
  {
    uniqueNodes.insert(sender);
  }
  for (uint32_t receiver : m_receivers)
  {
    uniqueNodes.insert(receiver);
  }
  if (m_enableInjection)
  {
    for (uint32_t sender : m_injectionSenders)
    {
      uniqueNodes.insert(sender);
    }
    for (uint32_t receiver : m_injectionReceivers)
    {
      uniqueNodes.insert(receiver);
    }
  }

  // Create NodeContainer with unique nodes
  for (uint32_t nodeId : uniqueNodes)
  {
    appNodes.Add(m_nodes.Get(nodeId));
  }

  m_flowMonitor = m_flowHelper.Install(appNodes);

  PrintTopologyInfo();

  NS_LOG_INFO("Starting simulation...");
  Simulator::Stop(Seconds(m_stopTime + 1));
  Simulator::Run();

  PrintResults();
  SaveResultsToFile();

  Simulator::Destroy();
}

void NetworkTopologySimulator::PrintTopologyInfo()
{
  std::cout << "\n==================== SIMULATION CONFIGURATION ====================\n";

  const char *topologyNames[] = {"", "Linear", "Star", "Ring", "Mesh", "Tree"};
  const char *trafficNames[] = {"", "Bulk Send", "On-Off", "CBR"};
  const char *tcpNames[] = {"", "NewReno", "Cubic", "HighSpeed", "Vegas"};
  const char *routingNames[] = {"", "Static", "DSDV", "AODV", "OLSR"};

  std::cout << "Topology Type:      " << topologyNames[m_topology] << std::endl;
  std::cout << "Number of Nodes:    " << m_numNodes << std::endl;
  std::cout << "Traffic Type:       " << trafficNames[m_trafficType] << std::endl;
  std::cout << "TCP Variant:        " << tcpNames[m_tcpVariant] << std::endl;
  std::cout << "Data Rate:          " << m_dataRate << std::endl;
  std::cout << "Delay:              " << m_delay << std::endl;
  std::cout << "Queue Size:         " << m_queueSize << " packets" << std::endl;
  std::cout << "Routing Protocol:   " << routingNames[m_routingProtocol] << std::endl;

  std::cout << "Simulation Time:    " << m_startTime << "s - " << m_stopTime << "s" << std::endl;

  std::cout << "Primary Traffic Flows:" << std::endl;
  for (size_t i = 0; i < m_senders.size(); ++i)
  {
    std::cout << "  Node " << m_senders[i] << " -> Node " << m_receivers[i] << std::endl;
  }

  if (m_enableInjection)
  {
    std::cout << "Injection Traffic Flows (starting at "
              << (m_startTime + (m_stopTime - m_startTime) * m_injectionStartDelay)
              << "s):" << std::endl;
    for (size_t i = 0; i < m_injectionSenders.size(); ++i)
    {
      std::cout << "  Node " << m_injectionSenders[i] << " -> Node "
                << m_injectionReceivers[i] << std::endl;
    }
  }
  std::cout << "================================================================\n";
}

void NetworkTopologySimulator::PrintPerformanceMetrics()
{
  m_flowMonitor->CheckForLostPackets();
  Ptr<Ipv4FlowClassifier> classifier =
      DynamicCast<Ipv4FlowClassifier>(m_flowHelper.GetClassifier());
  FlowMonitor::FlowStatsContainer stats = m_flowMonitor->GetFlowStats();

  std::cout << "\n==================== FLOW MONITOR STATISTICS ====================\n";

  double totalThroughput = 0.0;
  double totalDelay = 0.0;
  double totalJitter = 0.0; // ADD THIS LINE
  uint32_t totalFlows = 0;

  for (auto i = stats.begin(); i != stats.end(); ++i)
  {
    Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow(i->first);

    if (t.protocol == 6 && t.destinationPort >= 8080 && t.destinationPort <= 8089 &&
        i->second.rxPackets > 0)
    {
      double throughput = i->second.rxBytes * 8.0 /
                          (i->second.timeLastRxPacket.GetSeconds() -
                           i->second.timeFirstTxPacket.GetSeconds()) /
                          1024 / 1024;
      double avgDelay = i->second.delaySum.GetSeconds() / i->second.rxPackets;

      // CORRECT JITTER CALCULATION
      double avgJitter = 0.0;
      if (i->second.rxPackets > 1)
      {
        avgJitter = i->second.jitterSum.GetSeconds() / (i->second.rxPackets - 1);
      }

      double packetLoss =
          ((double)(i->second.txPackets - i->second.rxPackets) / i->second.txPackets) * 100;

      std::cout << "Flow " << i->first << " (" << t.sourceAddress << " -> "
                << t.destinationAddress << ")\n";
      std::cout << "  Throughput:     " << std::fixed << std::setprecision(2) << throughput
                << " Mbits/s\n";
      std::cout << "  Avg Delay:      " << std::fixed << std::setprecision(6) << avgDelay
                << " seconds\n";
      std::cout << "  Avg Jitter:     " << std::fixed << std::setprecision(6) << avgJitter
                << " seconds\n"; // ADD THIS LINE
      std::cout << "  Packet Loss:    " << std::fixed << std::setprecision(2) << packetLoss
                << "%\n";
      std::cout << "  Tx Packets:     " << i->second.txPackets << "\n";
      std::cout << "  Rx Packets:     " << i->second.rxPackets << "\n";
      std::cout << "  Tx Bytes:       " << i->second.txBytes << "\n";
      std::cout << "  Rx Bytes:       " << i->second.rxBytes << "\n\n";

      totalThroughput += throughput;
      totalDelay += avgDelay;
      totalJitter += avgJitter; // ADD THIS LINE
      totalFlows++;
    }
  }

  if (totalFlows > 0)
  {
    std::cout << "OVERALL PERFORMANCE:\n";
    std::cout << "  Total Throughput:   " << std::fixed << std::setprecision(2)
              << totalThroughput << " Mbits/s\n";
    std::cout << "  Average Delay:      " << std::fixed << std::setprecision(6)
              << (totalDelay / totalFlows) << " seconds\n";
    std::cout << "  Average Jitter:     " << std::fixed << std::setprecision(6)
              << (totalJitter / totalFlows) << " seconds\n"; // ADD THIS LINE
  }
  std::cout << "================================================================\n";
}

void NetworkTopologySimulator::PrintResults()
{
  double simTime = Simulator::Now().GetSeconds();

  std::cout << "\n==================== SIMULATION RESULTS ====================\n";

  // Enhanced PacketSink Statistics with injection awareness
  std::cout << "PACKETSINK STATISTICS:\n";

  double totalThroughput = 0.0;
  int regularFlows = 0;
  int injectionFlows = 0;

  for (auto &pair : m_sinks)
  {
    uint32_t nodeId = pair.first;
    Ptr<PacketSink> sink = pair.second;

    uint64_t totalBytes = sink->GetTotalRx();
    double throughputMbps = (totalBytes * 8.0) / 1e6 / (simTime - m_startTime);
    totalThroughput += throughputMbps;

    // Determine if this is an injection receiver
    bool isInjectionReceiver = false;
    if (m_enableInjection)
    {
      for (uint32_t injReceiver : m_injectionReceivers)
      {
        if (nodeId == injReceiver)
        {
          isInjectionReceiver = true;
          injectionFlows++;
          break;
        }
      }
    }
    if (!isInjectionReceiver)
    {
      regularFlows++;
    }

    std::cout << "Receiver Node " << nodeId;
    if (isInjectionReceiver)
    {
      std::cout << " [INJECTION RECEIVER]";
    }
    std::cout << ":\n";
    std::cout << "  Total Bytes Received: " << totalBytes << " bytes\n";
    std::cout << "  Throughput:           " << std::fixed << std::setprecision(2)
              << throughputMbps << " Mbits/s\n\n";
  }

  std::cout << "SUMMARY:\n";
  std::cout << "  Regular flows: " << regularFlows << ", Injection flows: " << injectionFlows
            << std::endl;
  std::cout << "  Combined throughput: " << std::fixed << std::setprecision(2) << totalThroughput
            << " Mbits/s\n\n";

  // Verify injection flows
  FilterAndDisplayFlows();

  PrintPerformanceMetrics();
}

void NetworkTopologySimulator::SaveResultsToFile()
{
  m_flowMonitor->SerializeToXmlFile(m_outputPrefix + "_flowmon.xml", true, true);
  std::ofstream outFile(m_outputPrefix + "_.txt");

  if (outFile.is_open())
  {
    outFile << "===============================================\n";
    outFile << "    NS-3 NETWORK TOPOLOGY SIMULATION RESULTS\n";
    outFile << "===============================================\n\n";

    const char *topologyNames[] = {"", "Linear", "Star", "Ring", "Mesh", "Tree"};
    const char *trafficNames[] = {"", "Bulk Send", "On-Off", "CBR"};
    const char *tcpNames[] = {"", "NewReno", "Cubic", "HighSpeed", "Vegas"};
    const char *routingNames[] = {"", "Static", "DSDV", "AODV", "OLSR"};

    // Configuration Section
    outFile << "==================== SIMULATION CONFIGURATION ====================\n";
    outFile << "Topology Type:      " << topologyNames[m_topology] << std::endl;
    outFile << "Number of Nodes:    " << m_numNodes << std::endl;
    outFile << "Traffic Type:       " << trafficNames[m_trafficType] << std::endl;
    outFile << "TCP Variant:        " << tcpNames[m_tcpVariant] << std::endl;
    outFile << "Data Rate:          " << m_dataRate << std::endl;
    outFile << "Delay:              " << m_delay << std::endl;
    outFile << "Queue Size:         " << m_queueSize << " packets" << std::endl;
    outFile << "Routing Protocol:   " << routingNames[m_routingProtocol] << std::endl;

    outFile << "Simulation Time:    " << m_startTime << "s - " << m_stopTime << "s"
            << std::endl;
    outFile << "Actual Duration:    " << (m_stopTime - m_startTime) << " seconds" << std::endl;

    outFile << "\nSender-Receiver Pairs:" << std::endl;
    for (size_t i = 0; i < m_senders.size(); ++i)
    {
      outFile << "  Node " << m_senders[i] << " -> Node " << m_receivers[i] << std::endl;
    }
    outFile << "================================================================\n\n";

    // PacketSink Results Section
    outFile << "==================== PACKETSINK STATISTICS ====================\n";
    double simTime = Simulator::Now().GetSeconds();
    double actualDuration = simTime - m_startTime;

    for (auto &pair : m_sinks)
    {
      uint32_t nodeId = pair.first;
      Ptr<PacketSink> sink = pair.second;

      uint64_t totalBytes = sink->GetTotalRx();
      double throughputMbps = (totalBytes * 8.0) / 1e6 / actualDuration;

      outFile << "Receiver Node " << nodeId << ":\n";
      outFile << "  Total Bytes Received: " << totalBytes << " bytes\n";
      outFile << "  Simulation Duration:  " << std::fixed << std::setprecision(3)
              << actualDuration << " seconds\n";
      outFile << "  Throughput Calculation:\n";
      outFile << "    Formula: (TotalBytes × 8) ÷ 1,000,000 ÷ Duration\n";
      outFile << "    Calculation: (" << totalBytes << " × 8) ÷ 1,000,000 ÷ "
              << actualDuration << "\n";
      outFile << "    Result: " << std::fixed << std::setprecision(2) << throughputMbps
              << " Mbits/s\n\n";
    }
    outFile << "================================================================\n\n";

    // Flow Monitor Results Section
    outFile << "==================== FLOW MONITOR STATISTICS ====================\n";

    m_flowMonitor->CheckForLostPackets();
    Ptr<Ipv4FlowClassifier> classifier =
        DynamicCast<Ipv4FlowClassifier>(m_flowHelper.GetClassifier());
    FlowMonitor::FlowStatsContainer stats = m_flowMonitor->GetFlowStats();

    double totalThroughput = 0.0;
    double totalDelay = 0.0;
    uint32_t totalFlows = 0;
    uint64_t totalTxPackets = 0;
    uint64_t totalRxPackets = 0;
    uint64_t totalTxBytes = 0;
    uint64_t totalRxBytes = 0;
    double totalJitter = 0.0;

    for (auto i = stats.begin(); i != stats.end(); ++i)
    {
      Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow(i->first);

      // FILTER ONLY TCP APPLICATION FLOWS (ports 8080-8089) - THIS IS THE KEY FIX
      if (t.protocol == 6 && t.destinationPort >= 8080 && t.destinationPort <= 8089 &&
          i->second.rxPackets > 0)
      {
        double flowDuration = i->second.timeLastRxPacket.GetSeconds() -
                              i->second.timeFirstTxPacket.GetSeconds();
        double throughput = i->second.rxBytes * 8.0 / flowDuration / 1024 / 1024;
        double avgDelay = i->second.delaySum.GetSeconds() / i->second.rxPackets;
        double packetLoss =
            ((double)(i->second.txPackets - i->second.rxPackets) / i->second.txPackets) *
            100;

        outFile << "Flow " << i->first << " (" << t.sourceAddress << " -> "
                << t.destinationAddress << ")\n";

        // Detailed throughput calculation
        outFile << "  THROUGHPUT ANALYSIS:\n";
        outFile << "    Received Bytes:     " << i->second.rxBytes << " bytes\n";
        outFile << "    Flow Duration:      " << std::fixed << std::setprecision(6)
                << flowDuration << " seconds\n";
        outFile << "    First Tx Time:      " << std::fixed << std::setprecision(6)
                << i->second.timeFirstTxPacket.GetSeconds() << " seconds\n";
        outFile << "    Last Rx Time:       " << std::fixed << std::setprecision(6)
                << i->second.timeLastRxPacket.GetSeconds() << " seconds\n";
        outFile << "    Throughput Formula: (RxBytes × 8) ÷ Duration ÷ 1024 ÷ 1024\n";
        outFile << "    Calculation:        (" << i->second.rxBytes << " × 8) ÷ "
                << flowDuration << " ÷ 1048576\n";
        outFile << "    Result:             " << std::fixed << std::setprecision(2)
                << throughput << " Mbits/s\n\n";

        // Detailed delay calculation
        outFile << "  DELAY ANALYSIS:\n";
        outFile << "    Total Delay Sum:    " << std::fixed << std::setprecision(6)
                << i->second.delaySum.GetSeconds() << " seconds\n";
        outFile << "    Received Packets:   " << i->second.rxPackets << " packets\n";
        outFile << "    Delay Formula:      DelaySum ÷ RxPackets\n";
        outFile << "    Calculation:        " << i->second.delaySum.GetSeconds() << " ÷ "
                << i->second.rxPackets << "\n";
        outFile << "    Average Delay:      " << std::fixed << std::setprecision(6)
                << avgDelay << " seconds\n\n";

        // Detailed packet loss calculation
        outFile << "  PACKET LOSS ANALYSIS:\n";
        outFile << "    Transmitted Packets: " << i->second.txPackets << " packets\n";
        outFile << "    Received Packets:    " << i->second.rxPackets << " packets\n";
        outFile << "    Lost Packets:        "
                << (i->second.txPackets - i->second.rxPackets) << " packets\n";
        outFile << "    Loss Formula:        ((TxPackets - RxPackets) ÷ TxPackets) × 100\n";
        outFile << "    Calculation:         ((" << i->second.txPackets << " - "
                << i->second.rxPackets << ") ÷ " << i->second.txPackets << ") × 100\n";
        outFile << "    Packet Loss Rate:    " << std::fixed << std::setprecision(2)
                << packetLoss << "%\n\n";

        // Additional metrics
        outFile << "  ADDITIONAL METRICS:\n";
        outFile << "    Transmitted Bytes:   " << i->second.txBytes << " bytes\n";
        outFile << "    Received Bytes:      " << i->second.rxBytes << " bytes\n";
        outFile << "    Lost Bytes:          " << (i->second.txBytes - i->second.rxBytes)
                << " bytes\n";

        if (i->second.rxPackets > 0)
        {
          outFile << "    Avg Packet Size:     "
                  << (i->second.rxBytes / i->second.rxPackets) << " bytes\n";
        }

        double avgJitter = 0.0;
        if (i->second.rxPackets > 1)
        {
          avgJitter = i->second.jitterSum.GetSeconds() / (i->second.rxPackets - 1);
        }
        outFile << "  JITTER ANALYSIS:\n";
        outFile << "    Total Jitter Sum:    " << std::fixed << std::setprecision(6)
                << i->second.jitterSum.GetSeconds() << " seconds\n";
        outFile << "    Received Packets:    " << i->second.rxPackets << " packets\n";
        outFile << "    Jitter Formula:      JitterSum ÷ (RxPackets - 1)\n";
        outFile << "    Note:                Jitter requires at least 2 packets\n";
        if (i->second.rxPackets > 1)
        {
          outFile << "    Calculation:         " << i->second.jitterSum.GetSeconds()
                  << " ÷ " << (i->second.rxPackets - 1) << "\n";
          outFile << "    Average Jitter:      " << std::fixed << std::setprecision(6)
                  << avgJitter << " seconds\n";
        }
        else
        {
          outFile << "    Average Jitter:      N/A (insufficient packets)\n";
        }
        outFile << "\n";

        outFile << "\n"
                << std::string(60, '-') << "\n\n";

        // Accumulate totals
        totalThroughput += throughput;
        totalDelay += avgDelay;
        totalFlows++;
        totalTxPackets += i->second.txPackets;

        totalRxPackets += i->second.rxPackets;
        totalTxBytes += i->second.txBytes;
        totalRxBytes += i->second.rxBytes;
        if (i->second.rxPackets > 1)
        {
          double flowAvgJitter =
              i->second.jitterSum.GetSeconds() / (i->second.rxPackets - 1);
          totalJitter += flowAvgJitter;
        }
      }
    }

    // Overall Performance Summary
    if (totalFlows > 0)
    {
      outFile << "==================== OVERALL PERFORMANCE SUMMARY ====================\n";
      outFile << "Total Active Flows:         " << totalFlows << "\n";
      outFile << "Combined Throughput:        " << std::fixed << std::setprecision(2)
              << totalThroughput << " Mbits/s\n";
      outFile << "Average Flow Delay:         " << std::fixed << std::setprecision(6)
              << (totalDelay / totalFlows) << " seconds\n";
      outFile << "Total Packets Transmitted:  " << totalTxPackets << " packets\n";
      outFile << "Total Packets Received:     " << totalRxPackets << " packets\n";
      outFile << "Total Packets Lost:         " << (totalTxPackets - totalRxPackets)
              << " packets\n";
      outFile << "Overall Packet Loss Rate:   " << std::fixed << std::setprecision(2)
              << (((double)(totalTxPackets - totalRxPackets) / totalTxPackets) * 100)
              << "%\n";
      outFile << "Total Bytes Transmitted:    " << totalTxBytes << " bytes\n";
      outFile << "Total Bytes Received:       " << totalRxBytes << " bytes\n";
      outFile << "Total Bytes Lost:           " << (totalTxBytes - totalRxBytes)
              << " bytes\n";
      outFile << "Average Jitter:             " << std::fixed << std::setprecision(6)
              << (totalJitter / totalFlows) << " seconds\n";

      // Network utilization
      double theoreticThroughput = 10.0; // 10 Mbps from your config
      double utilization = (totalThroughput / theoreticThroughput) * 100;
      outFile << "Network Utilization:        " << std::fixed << std::setprecision(2)
              << utilization << "% of " << theoreticThroughput << " Mbps\n";
    }
    outFile << "===================================================================\n\n";

    // Comparison Section
    outFile << "==================== MEASUREMENT COMPARISON ====================\n";
    outFile << "Why PacketSink and Flow Monitor show different throughput values:\n\n";
    outFile << "1. PACKETSINK MEASUREMENT:\n";
    outFile << "   - Measures APPLICATION LAYER data only\n";
    outFile << "   - Excludes network headers (IP, TCP, Ethernet)\n";
    outFile << "   - Uses simulation time intervals\n";
    outFile << "   - Formula: (AppBytes × 8) ÷ 1,000,000 ÷ SimDuration\n\n";

    outFile << "2. FLOW MONITOR MEASUREMENT:\n";
    outFile << "   - Measures NETWORK LAYER data (includes headers)\n";
    outFile << "   - Uses actual packet transmission timing\n";
    outFile << "   - More precise timing measurements\n";
    outFile << "   - Formula: (NetBytes × 8) ÷ FlowDuration ÷ 1,048,576\n\n";

    outFile << "3. DIFFERENCES EXPLAINED:\n";
    outFile << "   - Flow Monitor typically shows HIGHER throughput\n";
    outFile << "   - Difference represents network protocol overhead\n";
    outFile << "   - Flow Monitor timing is more accurate\n";
    outFile << "   - Both measurements are correct for their respective layers\n";
    outFile << "=================================================================\n\n";

    // Topology Analysis

    // MOVED ROUTING PROTOCOL ANALYSIS BEFORE outFile.close() - THIS IS THE FIX!
    outFile << "==================== ROUTING PROTOCOL ANALYSIS ====================\n";
    outFile << "Selected Protocol: " << routingNames[m_routingProtocol] << "\n\n";

    switch (m_routingProtocol)
    {
    case STATIC:
      outFile << "STATIC ROUTING CHARACTERISTICS:\n";
      outFile << "- Pre-computed routing tables using global topology knowledge\n";
      outFile << "- No routing overhead during simulation\n";
      outFile << "- Optimal paths based on shortest hop count\n";
      outFile << "- No adaptation to network changes\n";
      outFile << "- Suitable for stable network topologies\n";
      break;

    case DSDV:
      outFile << "DSDV ROUTING CHARACTERISTICS:\n";
      outFile << "- Proactive (table-driven) routing protocol\n";
      outFile << "- Maintains routing tables with sequence numbers\n";
      outFile << "- Periodic and triggered route updates\n";
      outFile << "- Prevents routing loops using destination sequence numbers\n";
      outFile << "- Higher control overhead but faster route discovery\n";
      outFile << "- Suitable for networks with moderate mobility\n";
      break;

    case AODV:
      outFile << "AODV ROUTING CHARACTERISTICS:\n";
      outFile << "- Reactive (on-demand) routing protocol\n";
      outFile << "- Route discovery only when needed (RREQ/RREP)\n";
      outFile << "- Lower control overhead in sparse traffic scenarios\n";
      outFile << "- Uses sequence numbers to prevent loops\n";
      outFile << "- Higher initial delay for route establishment\n";
      outFile << "- Suitable for networks with high mobility and sparse traffic\n";
      break;

    case OLSR:
      outFile << "OLSR ROUTING CHARACTERISTICS:\n";
      outFile << "- Proactive link-state routing protocol\n";
      outFile << "- Uses Multi-Point Relays (MPRs) to reduce flooding\n";
      outFile << "- Maintains topology information for entire network\n";
      outFile << "- Regular HELLO and TC (Topology Control) messages\n";
      outFile << "- Optimized for dense networks with frequent communication\n";
      outFile << "- Lower end-to-end delay but higher control overhead\n";
      break;
    }

    outFile << "\nROUTING PERFORMANCE IMPACT:\n";
    if (m_routingProtocol != STATIC)
    {
      outFile
          << "- Additional control packet overhead not captured in application throughput\n";
      outFile << "- Route establishment delay may affect initial performance\n";
      outFile << "- Dynamic adaptation capability enables resilience to topology changes\n";
      outFile << "- Actual performance depends on network topology and traffic patterns\n";
    }
    else
    {
      outFile << "- No routing protocol overhead\n";
      outFile << "- Optimal performance for stable topologies\n";
      outFile << "- Pre-computed shortest paths\n";
    }
    outFile << "=================================================================\n\n";

    // NOW close the file - all content has been written
    outFile.close();
    std::cout << "Detailed results saved to: " << m_outputPrefix << "_results.txt\n";
  }
  else
  {
    std::cerr << "Error: Could not open output file for writing!" << std::endl;
  }
}

void NetworkTopologySimulator::FilterAndDisplayFlows()
{
  m_flowMonitor->CheckForLostPackets();
  Ptr<Ipv4FlowClassifier> classifier =
      DynamicCast<Ipv4FlowClassifier>(m_flowHelper.GetClassifier());
  FlowMonitor::FlowStatsContainer stats = m_flowMonitor->GetFlowStats();

  std::cout << "\n=== APPLICATION FLOW ANALYSIS ===\n";

  int applicationFlows = 0;
  int expectedFlows = m_senders.size() + (m_enableInjection ? m_injectionSenders.size() : 0);

  for (auto i = stats.begin(); i != stats.end(); ++i)
  {
    Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow(i->first);

    // Filter ONLY TCP application flows (ports 8080-8089)
    if (t.protocol == 6 && t.destinationPort >= 8080 && t.destinationPort <= 8089)
    {
      applicationFlows++;

      // Determine flow type
      std::string flowType = "ORIGINAL";
      if (m_enableInjection && t.destinationPort >= (8080 + m_senders.size()))
      {
        flowType = "INJECTION";
      }

      std::cout << flowType << " Flow " << applicationFlows << ": " << t.sourceAddress
                << " -> " << t.destinationAddress << " Port:" << t.destinationPort
                << " (Tx: " << i->second.txPackets << ", Rx: " << i->second.rxPackets
                << ")\n";
    }
  }

  std::cout << "Total Application Flows: " << applicationFlows << std::endl;
  std::cout << "Expected Flows: " << expectedFlows << std::endl;

  if (applicationFlows != expectedFlows)
  {
    std::cout << "WARNING: Flow count mismatch! Check configuration.\n";
  }
}

int main(int argc, char *argv[])
{
  Time::SetResolution(Time::NS);
  LogComponentEnable("SimpleTopologySimulator", LOG_LEVEL_INFO);

  NetworkTopologySimulator simulator;
  simulator.Configure(argc, argv);
  simulator.RunSimulation();

  return 0;
}
