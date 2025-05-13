#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/applications-module.h"
#include "ns3/mobility-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/energy-module.h"
#include "ns3/basic-energy-source.h"
#include "ns3/basic-energy-source-helper.h"
#include "ns3/spectrum-module.h"
#include "ns3/propagation-module.h"
#include <iostream>
#include <fstream>
#include <string>
#include <cmath>
#include <vector>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("AmbientIoTSimulation");

// 声明距离计算函数
double CalcDistance(const Vector &a, const Vector &b);

// 定义环境物联网设备类型
enum AmbientIoTDeviceType {
  ACTIVE_DEVICE,             // 由环境能量供电的主动传输设备
  MONOSTATIC_BACKSCATTER,    // 单站式反向散射（读取器同时作为载波源）
  BISTATIC_BACKSCATTER,      // 双站式反向散射（读取器和载波源不同）
  CARRIER_WAVE_SOURCE        // 提供载波的源设备
};

// 环境数据包处理回调
void EnvironmentalDataReceived(Ptr<const Packet> packet, const Address& from)
{
  NS_LOG_INFO("环境数据接收时间: " << Simulator::Now().GetSeconds() << "秒");
}

// 环境物联网设备自定义类
class AmbientIoTDevice : public Object
{
public:
  static TypeId GetTypeId (void)
  {
    static TypeId tid = TypeId ("ns3::AmbientIoTDevice")
      .SetParent<Object> ()
      .SetGroupName ("AmbientIoT")
      .AddConstructor<AmbientIoTDevice> ()
    ;
    return tid;
  }

  AmbientIoTDevice() : 
    m_deviceType(ACTIVE_DEVICE),
    m_hasEnergyHarvester(false),
    m_currentEnergy(0.0),
    m_lastHarvestTime(Seconds(0)) {}

  void SetDeviceType(AmbientIoTDeviceType type) { m_deviceType = type; }
  AmbientIoTDeviceType GetDeviceType() const { return m_deviceType; }
  
  void EnableEnergyHarvester(double harvestRate) { 
    m_hasEnergyHarvester = true; 
    m_harvestRate = harvestRate; 
  }
  
  bool CanTransmit() {
    // 根据收获率更新当前能量
    if (m_hasEnergyHarvester) {
      Time now = Simulator::Now();
      double duration = (now - m_lastHarvestTime).GetSeconds();
      m_currentEnergy += m_harvestRate * duration;
      m_lastHarvestTime = now;
    }
    
    // 检查设备是否有足够的能量传输
    bool canTransmit = false;
    
    switch (m_deviceType) {
      case ACTIVE_DEVICE:
        // 主动传输需要更多能量
        if (m_currentEnergy >= 0.1) {
          m_currentEnergy -= 0.1;
          canTransmit = true;
        }
        break;
      case MONOSTATIC_BACKSCATTER:
      case BISTATIC_BACKSCATTER:
        // 反向散射需要很少的能量
        if (m_currentEnergy >= 0.01) {
          m_currentEnergy -= 0.01;
          canTransmit = true;
        }
        break;
      case CARRIER_WAVE_SOURCE:
        // 载波源假定有稳定的电源
        canTransmit = true;
        break;
    }
    
    return canTransmit;
  }
  
  void SetAssociatedNode(Ptr<Node> node) { m_node = node; }
  Ptr<Node> GetAssociatedNode() const { return m_node; }
  
  void SetCarrierWaveSource(Ptr<AmbientIoTDevice> source) { m_carrierSource = source; }
  Ptr<AmbientIoTDevice> GetCarrierWaveSource() const { return m_carrierSource; }
  
  void SetReaderNode(Ptr<Node> reader) { m_readerNode = reader; }
  Ptr<Node> GetReaderNode() const { return m_readerNode; }

private:
  AmbientIoTDeviceType m_deviceType;
  bool m_hasEnergyHarvester;
  double m_harvestRate;        // 能量收获率（焦耳/秒）
  double m_currentEnergy;      // 当前能量水平（焦耳）
  Time m_lastHarvestTime;      // 上次能量收获时间
  Ptr<Node> m_node;            // 关联的ns-3节点
  Ptr<AmbientIoTDevice> m_carrierSource; // 针对反向散射设备
  Ptr<Node> m_readerNode;      // 该设备的读取器节点
};

// 反向散射接收器自定义类
class BackscatterReceiver : public Object
{
public:
  static TypeId GetTypeId (void)
  {
    static TypeId tid = TypeId ("ns3::BackscatterReceiver")
      .SetParent<Object> ()
      .SetGroupName ("AmbientIoT")
      .AddConstructor<BackscatterReceiver> ()
    ;
    return tid;
  }
  
  void ReceivePacket(Ptr<NetDevice> device, Ptr<const Packet> packet, 
                   uint16_t protocol, const Address& sender)
  {
    NS_LOG_INFO("网关收到反向散射数据包: 大小=" << packet->GetSize() << " 字节，时间=" 
              << Simulator::Now().GetSeconds() << "秒");
  }
  
  void SetIsMonostatic(bool isMonostatic) { m_isMonostatic = isMonostatic; }
  bool IsMonostatic() const { return m_isMonostatic; }

private:
  bool m_isMonostatic;  // 这是单站式还是双站式接收器
};

// 环境能量源
class AmbientEnergySource : public Object
{
public:
  static TypeId GetTypeId (void)
  {
    static TypeId tid = TypeId ("ns3::AmbientEnergySource")
      .SetParent<Object> ()
      .SetGroupName ("AmbientIoT")
      .AddConstructor<AmbientEnergySource> ()
    ;
    return tid;
  }
  
  AmbientEnergySource() : m_energyDensity(0.001) {} // 默认1mW/cm²
  
  void SetEnergyDensity(double density) { m_energyDensity = density; }
  double GetEnergyDensity() const { return m_energyDensity; }
  
  double CalculateHarvestRate(double distance) {
    // 简单模型：收获率随距离平方减小
    if (distance <= 0.1) distance = 0.1; // 避免除以零
    return m_energyDensity / (distance * distance);
  }

private:
  double m_energyDensity; // 能量密度，单位：焦耳/(秒*平方厘米)
};

int main(int argc, char* argv[])
{
  // 启用日志
  LogComponentEnable("AmbientIoTSimulation", LOG_LEVEL_INFO);
  
  // 模拟参数
  uint32_t numActiveDevices = 20;
  uint32_t numMonostaticDevices = 40;
  uint32_t numBistaticDevices = 40;
  uint32_t numCarrierSources = 5;
  double simTime = 3600.0;
  
  // 创建节点
  NodeContainer baseStations;       // 基站（NR Uu链路）
  baseStations.Create(3);
  
  NodeContainer carrierWaveSources; // 载波源
  carrierWaveSources.Create(numCarrierSources);
  
  NodeContainer activeDevices;      // 主动传输设备
  activeDevices.Create(numActiveDevices);
  
  NodeContainer monostaticDevices;  // 单站式反向散射设备
  monostaticDevices.Create(numMonostaticDevices);
  
  NodeContainer bistaticDevices;    // 双站式反向散射设备
  bistaticDevices.Create(numBistaticDevices);
  
  NodeContainer userEquipment;      // UE（移动设备）
  userEquipment.Create(10);
  
  NodeContainer dataCenter;         // 数据中心
  dataCenter.Create(1);
  
  // 配置移动性
  MobilityHelper mobility;
  
  // 基站 - 固定位置
  Ptr<ListPositionAllocator> bsPositions = CreateObject<ListPositionAllocator>();
  bsPositions->Add(Vector(0.0, 0.0, 30.0));  // 30米高
  bsPositions->Add(Vector(866.0, 500.0, 30.0));
  bsPositions->Add(Vector(-866.0, 500.0, 30.0));
  
  mobility.SetPositionAllocator(bsPositions);
  mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
  mobility.Install(baseStations);
  
  // 载波源 - 固定位置
  Ptr<ListPositionAllocator> cwPositions = CreateObject<ListPositionAllocator>();
  cwPositions->Add(Vector(-300.0, -300.0, 10.0));
  cwPositions->Add(Vector(300.0, -300.0, 10.0));
  cwPositions->Add(Vector(-300.0, 300.0, 10.0));
  cwPositions->Add(Vector(300.0, 300.0, 10.0));
  cwPositions->Add(Vector(0.0, 0.0, 10.0));
  
  mobility.SetPositionAllocator(cwPositions);
  mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
  mobility.Install(carrierWaveSources);
  
  // 主动设备 - 随机分布
  mobility.SetPositionAllocator("ns3::RandomDiscPositionAllocator",
                               "X", DoubleValue(0.0),
                               "Y", DoubleValue(0.0),
                               "Rho", StringValue("ns3::UniformRandomVariable[Min=0|Max=500]"));
  mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
  mobility.Install(activeDevices);
  
  // 单站式反向散射设备 - 在基站附近随机分布
  mobility.SetPositionAllocator("ns3::RandomDiscPositionAllocator",
                               "X", DoubleValue(0.0),
                               "Y", DoubleValue(0.0),
                               "Rho", StringValue("ns3::UniformRandomVariable[Min=0|Max=200]"));
  mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
  mobility.Install(monostaticDevices);
  
  // 双站式反向散射设备 - 随机分布
  mobility.SetPositionAllocator("ns3::RandomDiscPositionAllocator",
                               "X", DoubleValue(0.0),
                               "Y", DoubleValue(0.0),
                               "Rho", StringValue("ns3::UniformRandomVariable[Min=200|Max=800]"));
  mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
  mobility.Install(bistaticDevices);
  
  // UE - 具有随机行走移动性的移动设备
  mobility.SetPositionAllocator("ns3::RandomDiscPositionAllocator",
                               "X", DoubleValue(0.0),
                               "Y", DoubleValue(0.0),
                               "Rho", StringValue("ns3::UniformRandomVariable[Min=0|Max=1000]"));
  mobility.SetMobilityModel("ns3::RandomWalk2dMobilityModel",
                           "Bounds", RectangleValue(Rectangle(-1000, 1000, -1000, 1000)),
                           "Speed", StringValue("ns3::ConstantRandomVariable[Constant=1.4]")); // 行走速度
  mobility.Install(userEquipment);
  
  // 数据中心 - 固定位置
  Ptr<ListPositionAllocator> centerPosition = CreateObject<ListPositionAllocator>();
  centerPosition->Add(Vector(0.0, -500.0, 0.0));
  mobility.SetPositionAllocator(centerPosition);
  mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
  mobility.Install(dataCenter);
  
  // 创建环境能量源
  AmbientEnergySource ambientSource;
  ambientSource.SetEnergyDensity(0.001); // 1mW/cm²
  
  // 利用ns-3现有的能量模块为相关节点添加基础能量源
  BasicEnergySourceHelper energySourceHelper;
  energySourceHelper.Set("BasicEnergySourceInitialEnergyJ", DoubleValue(1000.0)); // 初始能量1000焦耳
  
  // 创建基站到数据中心的点对点连接
  PointToPointHelper p2p;
  p2p.SetDeviceAttribute("DataRate", StringValue("100Mbps"));
  p2p.SetChannelAttribute("Delay", StringValue("2ms"));
  
  NetDeviceContainer baseStationLinks;
  for (uint32_t i = 0; i < baseStations.GetN(); ++i)
  {
    NetDeviceContainer link = p2p.Install(baseStations.Get(i), dataCenter.Get(0));
    baseStationLinks.Add(link);
  }
  
  // 安装Internet协议栈
  InternetStackHelper internet;
  internet.Install(baseStations);
  internet.Install(userEquipment);
  internet.Install(dataCenter);
  
  // 分配IP地址
  Ipv4AddressHelper ipv4;
  std::vector<Ipv4InterfaceContainer> backboneInterfaces;
  
  for (uint32_t i = 0; i < baseStations.GetN(); ++i)
  {
    std::string base = "10.1." + std::to_string(i+1) + ".0";
    ipv4.SetBase(Ipv4Address(base.c_str()), "255.255.255.0");
    
    NetDeviceContainer linkDevices;
    linkDevices.Add(baseStationLinks.Get(i*2));
    linkDevices.Add(baseStationLinks.Get(i*2+1));
    
    Ipv4InterfaceContainer interface = ipv4.Assign(linkDevices);
    backboneInterfaces.push_back(interface);
  }
  
  // 设置环境物联网设备
  std::vector<Ptr<AmbientIoTDevice>> ambientDevices;
  
  // 设置载波源
  for (uint32_t i = 0; i < carrierWaveSources.GetN(); ++i) {
    Ptr<AmbientIoTDevice> device = CreateObject<AmbientIoTDevice>();
    device->SetDeviceType(CARRIER_WAVE_SOURCE);
    device->SetAssociatedNode(carrierWaveSources.Get(i));
    // 载波源不需要能量收集 - 假设有电源供电
    ambientDevices.push_back(device);
    
    NS_LOG_INFO("在位置 " 
               << carrierWaveSources.Get(i)->GetObject<MobilityModel>()->GetPosition()
               << " 创建了载波源");
  }
  
  // 设置具有能量收集功能的主动设备
  for (uint32_t i = 0; i < activeDevices.GetN(); ++i) {
    Ptr<AmbientIoTDevice> device = CreateObject<AmbientIoTDevice>();
    device->SetDeviceType(ACTIVE_DEVICE);
    device->SetAssociatedNode(activeDevices.Get(i));
    
    // 计算到最近基站和能量源的距离
    double minDist = std::numeric_limits<double>::max();
    Ptr<Node> closestBS = nullptr;
    
    Vector devicePos = activeDevices.Get(i)->GetObject<MobilityModel>()->GetPosition();
    
    for (uint32_t j = 0; j < baseStations.GetN(); ++j) {
      Vector bsPos = baseStations.Get(j)->GetObject<MobilityModel>()->GetPosition();
      double dist = CalcDistance(devicePos, bsPos);
      if (dist < minDist) {
        minDist = dist;
        closestBS = baseStations.Get(j);
      }
    }
    
    device->SetReaderNode(closestBS);
    
    // 基于距离启用能量收集
    double harvestRate = ambientSource.CalculateHarvestRate(minDist);
    device->EnableEnergyHarvester(harvestRate);
    
    ambientDevices.push_back(device);
    
    NS_LOG_INFO("在位置 " << devicePos << " 创建了主动设备，收获率为 " << harvestRate);
  }
  
  // 设置单站式反向散射设备
  for (uint32_t i = 0; i < monostaticDevices.GetN(); ++i) {
    Ptr<AmbientIoTDevice> device = CreateObject<AmbientIoTDevice>();
    device->SetDeviceType(MONOSTATIC_BACKSCATTER);
    device->SetAssociatedNode(monostaticDevices.Get(i));
    
    // 找到最近的基站进行单站式操作
    double minDist = std::numeric_limits<double>::max();
    Ptr<Node> closestBS = nullptr;
    
    Vector devicePos = monostaticDevices.Get(i)->GetObject<MobilityModel>()->GetPosition();
    
    for (uint32_t j = 0; j < baseStations.GetN(); ++j) {
      Vector bsPos = baseStations.Get(j)->GetObject<MobilityModel>()->GetPosition();
      double dist = CalcDistance(devicePos, bsPos);
      if (dist < minDist) {
        minDist = dist;
        closestBS = baseStations.Get(j);
      }
    }
    
    device->SetReaderNode(closestBS);
    device->SetCarrierWaveSource(nullptr); // 在单站式模式下，读取器即是载波源
    
    // 基于距离启用能量收集
    double harvestRate = ambientSource.CalculateHarvestRate(minDist);
    device->EnableEnergyHarvester(harvestRate);
    
    ambientDevices.push_back(device);
    
    NS_LOG_INFO("在位置 " << devicePos << " 创建了单站式反向散射设备，收获率为 " << harvestRate);
  }
  
  // 设置双站式反向散射设备
  for (uint32_t i = 0; i < bistaticDevices.GetN(); ++i) {
    Ptr<AmbientIoTDevice> device = CreateObject<AmbientIoTDevice>();
    device->SetDeviceType(BISTATIC_BACKSCATTER);
    device->SetAssociatedNode(bistaticDevices.Get(i));
    
    // 找到最近的基站作为读取器
    double minDistBS = std::numeric_limits<double>::max();
    Ptr<Node> closestBS = nullptr;
    
    Vector devicePos = bistaticDevices.Get(i)->GetObject<MobilityModel>()->GetPosition();
    
    for (uint32_t j = 0; j < baseStations.GetN(); ++j) {
      Vector bsPos = baseStations.Get(j)->GetObject<MobilityModel>()->GetPosition();
      double dist = CalcDistance(devicePos, bsPos);
      if (dist < minDistBS) {
        minDistBS = dist;
        closestBS = baseStations.Get(j);
      }
    }
    
    device->SetReaderNode(closestBS);
    
    // 找到最近的载波源
    double minDistCW = std::numeric_limits<double>::max();
    Ptr<AmbientIoTDevice> closestCW = nullptr;
    
    for (uint32_t j = 0; j < carrierWaveSources.GetN(); ++j) {
      Vector cwPos = carrierWaveSources.Get(j)->GetObject<MobilityModel>()->GetPosition();
      double dist = CalcDistance(devicePos, cwPos);
      if (dist < minDistCW) {
        minDistCW = dist;
        closestCW = ambientDevices[j]; // 载波源是向量中的第一个
      }
    }
    
    device->SetCarrierWaveSource(closestCW);
    
    // 基于距离启用能量收集
    double harvestRate = ambientSource.CalculateHarvestRate(std::min(minDistBS, minDistCW));
    device->EnableEnergyHarvester(harvestRate);
    
    ambientDevices.push_back(device);
    
    NS_LOG_INFO("在位置 " << devicePos << " 创建了双站式反向散射设备，收获率为 " << harvestRate);
  }
  
  // 在数据中心设置UDP服务器
  uint16_t port = 9;
  UdpServerHelper server(port);
  ApplicationContainer serverApps = server.Install(dataCenter);
  serverApps.Start(Seconds(1.0));
  serverApps.Stop(Seconds(simTime));
  
  // 设置自定义接收器回调
  BackscatterReceiver monostaticReceiver;
  monostaticReceiver.SetIsMonostatic(true);
  
  BackscatterReceiver bistaticReceiver;
  bistaticReceiver.SetIsMonostatic(false);
  
  // 为在线捕获创建PCAP文件
  p2p.EnablePcapAll("ambient-iot", true);
  
  // 调度传输事件
  for (uint32_t i = 0; i < ambientDevices.size(); ++i) {
    Ptr<AmbientIoTDevice> device = ambientDevices[i];
    
    if (device->GetDeviceType() == CARRIER_WAVE_SOURCE) {
      // 调度连续载波生成
      Simulator::Schedule(Seconds(10.0), &AmbientIoTDevice::CanTransmit, device);
      continue;
    }
    
    // 调度传感器数据传输
    Time startTime = Seconds(60.0 + (i % 60)); // 错开启动时间
    
    switch (device->GetDeviceType()) {
      case ACTIVE_DEVICE:
        // 主动设备传输频率较低但数据量较大
        Simulator::Schedule(startTime, [device, i](void) {
          if (device->CanTransmit()) {
            NS_LOG_INFO("主动设备 " << i << " 在 " << Simulator::Now().GetSeconds() << " 秒传输");
            // 在实际实现中，这里会发送实际的数据包
          }
        });
        break;
        
      case MONOSTATIC_BACKSCATTER:
        // 单站式设备传输更频繁，但负载小
        Simulator::Schedule(startTime, [device, i](void) {
          if (device->CanTransmit()) {
            NS_LOG_INFO("单站式反向散射设备 " << i << " 在 " << Simulator::Now().GetSeconds() << " 秒传输");
            // 在实际实现中，这会在载波上调制数据
          }
        });
        break;
        
      case BISTATIC_BACKSCATTER:
        // 双站式设备需要同时有载波源和读取器
        Simulator::Schedule(startTime, [device, i](void) {
          if (device->CanTransmit() && device->GetCarrierWaveSource()->CanTransmit()) {
            NS_LOG_INFO("双站式反向散射设备 " << i << " 在 " << Simulator::Now().GetSeconds() << " 秒传输");
            // 在实际实现中，会检查载波存在并调制数据
          }
        });
        break;
        
      default:
        break;
    }
  }
  
  // 运行仿真
  Simulator::Stop(Seconds(simTime));
  Simulator::Run();
  Simulator::Destroy();
  
  return 0;
}

// 计算两点之间距离的辅助函数
double CalcDistance(const Vector &a, const Vector &b)
{
  double dx = a.x - b.x;
  double dy = a.y - b.y;
  double dz = a.z - b.z;
  return std::sqrt(dx*dx + dy*dy + dz*dz);
} 