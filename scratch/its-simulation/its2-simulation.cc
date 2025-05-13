#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/mobility-module.h"
//#include "ns3/wave-module.h"
#include "ns3/wifi-module.h"
#include "ns3/internet-module.h"
#include "ns3/applications-module.h"
#include "ns3/point-to-point-module.h"
//#include "ns3/vehicle-module.h"
#include <iostream>
#include <filesystem>
#include "config.h"
#include "./bsm-header.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/netanim-module.h"
#include "ns3/olsr-helper.h"
#include "ns3/aodv-module.h"
#include "ns3/dsdv-module.h"
#include "ns3/dsr-module.h"
#include "ns3/config-store-module.h"
#include "ns3/ns2-mobility-helper.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("ITS-Simulation");

// 交通信号灯状态
enum TrafficLightState {
    RED,
    YELLOW,
    GREEN
};

// 交通信号灯类
class TrafficLight {
public:
    TrafficLight(Vector position) : m_position(position), m_state(RED) {}
    
    void SetState(TrafficLightState state) { m_state = state; }
    TrafficLightState GetState() const { return m_state; }
    Vector GetPosition() const { return m_position; }
    
private:
    Vector m_position;
    TrafficLightState m_state;
};

// 路侧单元类
class RoadSideUnit {
public:
    RoadSideUnit(Vector position) : m_position(position) {}
    
    void SendTrafficInfo(Ptr<Node> node, const std::string& info) {
        // 发送交通信息给指定车辆
    }
    
    Vector GetPosition() const { return m_position; }
    
private:
    Vector m_position;
};

// BSM消息结构
struct BsmMessage {
    uint32_t vehicleId;
    double x;
    double y;
    double speed;
    double direction;
    Time timestamp;
};

// 生成BSM消息
Ptr<Packet> GenerateBsmMessage(const BsmMessage& bsm) {
    Ptr<Packet> packet = Create<Packet>(PACKET_SIZE);
    BsmHeader header;
    header.SetVehicleId(bsm.vehicleId);
    header.SetPosition(Vector(bsm.x, bsm.y, 0));
    header.SetSpeed(bsm.speed);
    header.SetDirection(bsm.direction);
    header.SetTimestamp(bsm.timestamp);
    packet->AddHeader(header);
    return packet;
}

// 处理接收到的BSM消息 - 修改函数签名以匹配NetDevice::ReceiveCallback
bool HandleBsmMessage(Ptr<NetDevice> device, Ptr<const Packet> packet, uint16_t protocol, const Address& sender) {
    BsmHeader header;
    packet->PeekHeader(header);
    
    NS_LOG_INFO("Received BSM from " << header.GetVehicleId() 
                << " at position (" << header.GetPosition().x 
                << ", " << header.GetPosition().y << ")");
                
    // 返回 true 表示已处理数据包
    return true;
}

// 流量统计回调函数
void PacketSinkRx (Ptr<const Packet> p, const Address &addr)
{
    std::cout << "Received packet from " << addr << " at " << Simulator::Now().GetSeconds() << "s" << std::endl;
    
    // 添加详细的包信息记录
    BsmHeader header;
    p->PeekHeader(header);
    NS_LOG_INFO("Packet Details:"
                << "\n  Vehicle ID: " << header.GetVehicleId()
                << "\n  Position: (" << header.GetPosition().x << ", " << header.GetPosition().y << ")"
                << "\n  Speed: " << header.GetSpeed()
                << "\n  Direction: " << header.GetDirection()
                << "\n  Timestamp: " << header.GetTimestamp().GetSeconds() << "s");
}

int main (int argc, char *argv[])
{
    // 设置日志级别
    LogComponentEnable ("ITS-Simulation", LOG_LEVEL_INFO);
    // LogComponentEnable ("UdpServer", LOG_LEVEL_INFO);
    // LogComponentEnable ("UdpClient", LOG_LEVEL_INFO);
    // LogComponentEnable ("WaveNetDevice", LOG_LEVEL_INFO);
    
    // 创建输出目录
    std::filesystem::path outputDir = std::filesystem::current_path() / "output";
    std::filesystem::create_directories(outputDir);
    
    // 创建车辆节点
    NodeContainer vehicles;
    vehicles.Create(NUM_VEHICLES);
    
    // 创建RSU节点
    NodeContainer rsuNodes;
    rsuNodes.Create(NUM_RSU);
    
    // 创建交通信号灯节点
    NodeContainer trafficLightNodes;
    trafficLightNodes.Create(NUM_TRAFFIC_LIGHTS);
    
    // 设置移动模型
    Ns2MobilityHelper mobility(TRACE_FILE);
    mobility.Install();  // Install mobility model on all nodes
    
    // 设置RSU和交通信号灯的位置
    MobilityHelper rsuMobility;
    Ptr<ListPositionAllocator> rsuPositionAlloc = CreateObject<ListPositionAllocator>();
    for (int i = 0; i < NUM_RSU; i++) {
        rsuPositionAlloc->Add(Vector(RSU_POSITIONS[i][0], RSU_POSITIONS[i][1], 0.0));
    }
    rsuMobility.SetPositionAllocator(rsuPositionAlloc);
    rsuMobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
    rsuMobility.Install(rsuNodes);
    
    MobilityHelper trafficLightMobility;
    Ptr<ListPositionAllocator> trafficLightPositionAlloc = CreateObject<ListPositionAllocator>();
    for (int i = 0; i < NUM_TRAFFIC_LIGHTS; i++) {
        trafficLightPositionAlloc->Add(Vector(TRAFFIC_LIGHT_POSITIONS[i][0], TRAFFIC_LIGHT_POSITIONS[i][1], 0.0));
    }
    trafficLightMobility.SetPositionAllocator(trafficLightPositionAlloc);
    trafficLightMobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
    trafficLightMobility.Install(trafficLightNodes);
    
    // 配置WIFI设备
    YansWifiChannelHelper wifiChannel;
    wifiChannel.SetPropagationDelay("ns3::ConstantSpeedPropagationDelayModel");
    wifiChannel.AddPropagationLoss("ns3::FriisPropagationLossModel");
    
    YansWifiPhyHelper wifiPhy;
    wifiPhy.SetChannel(wifiChannel.Create());
    wifiPhy.SetPcapDataLinkType(WifiPhyHelper::DLT_IEEE802_11_RADIO);
    
    WifiMacHelper wifiMac;
    wifiMac.SetType("ns3::AdhocWifiMac");
    
    WifiHelper wifiHelper;
    wifiHelper.SetStandard(WIFI_STANDARD_80211a);
    wifiHelper.SetRemoteStationManager("ns3::ConstantRateWifiManager",
                                      "DataMode", StringValue("OfdmRate6Mbps"),
                                      "ControlMode", StringValue("OfdmRate6Mbps"));
    
    // 为所有节点安装WIFI设备
    NetDeviceContainer vehicleDevices = wifiHelper.Install(wifiPhy, wifiMac, vehicles);
    NetDeviceContainer rsuDevices = wifiHelper.Install(wifiPhy, wifiMac, rsuNodes);
    NetDeviceContainer trafficLightDevices = wifiHelper.Install(wifiPhy, wifiMac, trafficLightNodes);
    
    // 安装Internet协议栈
    InternetStackHelper internet;
    internet.Install(vehicles);
    internet.Install(rsuNodes);
    internet.Install(trafficLightNodes);
    
    // 分配IP地址
    Ipv4AddressHelper ipv4;
    ipv4.SetBase(IP_BASE, IP_MASK);
    Ipv4InterfaceContainer vehicleInterfaces = ipv4.Assign(vehicleDevices);
    ipv4.SetBase("10.1.2.0", IP_MASK);
    Ipv4InterfaceContainer rsuInterfaces = ipv4.Assign(rsuDevices);
    ipv4.SetBase("10.1.3.0", IP_MASK);
    Ipv4InterfaceContainer trafficLightInterfaces = ipv4.Assign(trafficLightDevices);
    
    // 创建UDP服务器（在RSU上）
    UdpServerHelper server(PORT);
    ApplicationContainer serverApps = server.Install(rsuNodes);
    serverApps.Start(Seconds(1.0));
    serverApps.Stop(Seconds(SIMULATION_TIME));
    
    // 配置UDP客户端（车辆发送数据给RSU）
    for (uint32_t i = 0; i < rsuInterfaces.GetN(); ++i) {
        UdpClientHelper client(rsuInterfaces.GetAddress(i), PORT);
        client.SetAttribute("MaxPackets", UintegerValue(MAX_PACKETS));
        client.SetAttribute("Interval", TimeValue(Seconds(PACKET_INTERVAL)));
        client.SetAttribute("PacketSize", UintegerValue(PACKET_SIZE));
        
        ApplicationContainer clientApps = client.Install(vehicles);
        clientApps.Start(Seconds(2.0));
        clientApps.Stop(Seconds(SIMULATION_TIME));
    }
    
    // 设置流量统计回调
    Ptr<UdpServer> udpServer = DynamicCast<UdpServer>(serverApps.Get(0));
    udpServer->TraceConnectWithoutContext("Rx", MakeCallback(&HandleBsmMessage));
    
    // 启用PCAP跟踪 - 为不同类型的节点创建不同的pcap文件
    std::string pcapPath = (outputDir / "its-simulation").string();
    wifiPhy.EnablePcap(pcapPath + "-vehicles", vehicleDevices);
    wifiPhy.EnablePcap(pcapPath + "-rsus", rsuDevices);
    wifiPhy.EnablePcap(pcapPath + "-traffic-lights", trafficLightDevices);
    
    // 启用ASCII跟踪
    AsciiTraceHelper ascii;
    wifiPhy.EnableAsciiAll(ascii.CreateFileStream((outputDir / "its-simulation.tr").string()));
    
    // 添加流量统计
    for (uint32_t i = 0; i < vehicleDevices.GetN(); ++i) {
        Ptr<WifiNetDevice> device = DynamicCast<WifiNetDevice>(vehicleDevices.Get(i));
        device->SetReceiveCallback(MakeCallback(&HandleBsmMessage));
    }
    
    // 运行模拟
    Simulator::Stop(Seconds(SIMULATION_TIME));
    Simulator::Run();
    Simulator::Destroy();
    
    return 0;
} 