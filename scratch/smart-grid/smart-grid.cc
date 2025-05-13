#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/applications-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/mobility-module.h"
#include "ns3/wifi-module.h"
#include "ns3/csma-module.h"
#include <iostream>
#include <fstream>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("SmartGridSimulation");

// 智能电表数据包处理回调
void SmartMeterPacketReceived(Ptr<const Packet> packet, const Address& from)
{
    NS_LOG_INFO("Smart meter data received at " << Simulator::Now().GetSeconds() << "s");
}

// 配电自动化数据包处理回调
void AutomationPacketReceived(Ptr<const Packet> packet, const Address& from)
{
    NS_LOG_INFO("Automation data received at " << Simulator::Now().GetSeconds() << "s");
}

int main(int argc, char* argv[])
{
    // 启用日志
    LogComponentEnable("SmartGridSimulation", LOG_LEVEL_INFO);

    // 创建节点
    NodeContainer controlCenter;      // 控制中心
    controlCenter.Create(1);
    
    NodeContainer substations;        // 变电站
    substations.Create(5);
    
    NodeContainer smartMeters;        // 智能电表
    smartMeters.Create(50);
    
    // 创建变电站到控制中心的高速链接
    PointToPointHelper p2p;
    p2p.SetDeviceAttribute("DataRate", StringValue("100Mbps"));
    p2p.SetChannelAttribute("Delay", StringValue("2ms"));
    
    NetDeviceContainer substationDevices;
    for (uint32_t i = 0; i < substations.GetN(); ++i)
    {
        NetDeviceContainer link = p2p.Install(controlCenter.Get(0), substations.Get(i));
        substationDevices.Add(link);
    }
    
    // 配置智能电表的无线网络
    YansWifiChannelHelper channel = YansWifiChannelHelper::Default();
    YansWifiPhyHelper phy;
    phy.SetChannel(channel.Create());
    
    WifiMacHelper mac;
    WifiHelper wifi;
    wifi.SetRemoteStationManager("ns3::AarfWifiManager");
    
    NetDeviceContainer meterDevices = wifi.Install(phy, mac, smartMeters);
    
    // 配置移动性模型（固定位置）
    MobilityHelper mobility;
    mobility.SetPositionAllocator("ns3::GridPositionAllocator",
                                 "MinX", DoubleValue(0.0),
                                 "MinY", DoubleValue(0.0),
                                 "DeltaX", DoubleValue(10.0),
                                 "DeltaY", DoubleValue(10.0),
                                 "GridWidth", UintegerValue(10),
                                 "LayoutType", StringValue("RowFirst"));
    
    mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
    mobility.Install(smartMeters);
    mobility.Install(substations);
    mobility.Install(controlCenter);
    
    // 安装Internet协议栈
    InternetStackHelper internet;
    internet.Install(controlCenter);
    internet.Install(substations);
    internet.Install(smartMeters);
    
    // 分配IP地址
    Ipv4AddressHelper ipv4;
    ipv4.SetBase("10.1.1.0", "255.255.255.0");
    
    // 为变电站分配IP
    std::vector<Ipv4InterfaceContainer> subnetInterfaces;
    for (uint32_t i = 0; i < substations.GetN(); ++i)
    {
        ipv4.SetBase("10.1." + std::to_string(i+2) + ".0", "255.255.255.0");
        Ipv4InterfaceContainer interface = ipv4.Assign(substationDevices.Get(i*2), substationDevices.Get(i*2+1));
        subnetInterfaces.push_back(interface);
    }
    
    // 为智能电表分配IP
    ipv4.SetBase("10.2.1.0", "255.255.255.0");
    Ipv4InterfaceContainer meterInterfaces = ipv4.Assign(meterDevices);
    
    // 创建应用程序
    uint16_t port = 9;
    
    // 在控制中心安装数据接收服务器
    UdpServerHelper server(port);
    ApplicationContainer serverApps = server.Install(controlCenter);
    serverApps.Start(Seconds(1.0));
    serverApps.Stop(Seconds(100.0));
    
    // 在智能电表上安装数据发送客户端
    UdpClientHelper client(subnetInterfaces[0].GetAddress(0), port);
    client.SetAttribute("MaxPackets", UintegerValue(100));
    client.SetAttribute("Interval", TimeValue(Seconds(1.0)));
    client.SetAttribute("PacketSize", UintegerValue(1024));
    
    ApplicationContainer clientApps = client.Install(smartMeters);
    clientApps.Start(Seconds(2.0));
    clientApps.Stop(Seconds(100.0));
    
    // 启用PCAP跟踪
    p2p.EnablePcapAll("smart-grid-p2p");
    phy.EnablePcap("smart-grid-wifi", meterDevices);
    
    // 运行仿真
    Simulator::Stop(Seconds(100.0));
    Simulator::Run();
    Simulator::Destroy();
    
    return 0;
} 