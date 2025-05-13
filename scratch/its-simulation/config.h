#ifndef ITS_SIMULATION_CONFIG_H
#define ITS_SIMULATION_CONFIG_H

// 模拟参数
const int NUM_VEHICLES = 10;
const int NUM_RSU = 4;              // 路侧单元数量
const int NUM_TRAFFIC_LIGHTS = 4;   // 交通信号灯数量
const double SIMULATION_TIME = 10.0;  // 秒
const double PACKET_INTERVAL = 0.1;   // 秒
const int PACKET_SIZE = 1024;         // 字节
const int MAX_PACKETS = 100;

//轨迹文件路径
const char* TRACE_FILE = "scratch/its-simulation/sumo/trajectory.tcl";

// 移动模型参数
const double MIN_X = 0.0;
const double MIN_Y = 0.0;
const double MAX_X = 50.0;
const double MAX_Y = 50.0;
const double DELTA_X = 5.0;
const double DELTA_Y = 5.0;
const int GRID_WIDTH = 3;

// 网络参数
const char* IP_BASE = "10.1.1.0";
const char* IP_MASK = "255.255.255.0";
const int PORT = 9;

// 基础设施位置
const double RSU_POSITIONS[4][2] = {
    {10.0, 10.0},  // RSU 1
    {10.0, 40.0},  // RSU 2
    {40.0, 10.0},  // RSU 3
    {40.0, 40.0}   // RSU 4
};

const double TRAFFIC_LIGHT_POSITIONS[4][2] = {
    {15.0, 15.0},  // 交通灯 1
    {15.0, 35.0},  // 交通灯 2
    {35.0, 15.0},  // 交通灯 3
    {35.0, 35.0}   // 交通灯 4
};

#endif // ITS_SIMULATION_CONFIG_H 