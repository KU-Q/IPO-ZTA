#include "bsm-header.h"
#include "ns3/log.h"
#include "ns3/simulator.h"
#include "ns3/packet.h"
#include "ns3/config.h"
#include "ns3/names.h"
#include "ns3/node.h"
#include "ns3/uinteger.h"
#include "ns3/abort.h"
#include "ns3/mac48-address.h"
#include "ns3/lora-net-device.h"
#include "ns3/pcap-file-wrapper.h"
NS_LOG_COMPONENT_DEFINE ("BsmHeader");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (BsmHeader);

BsmHeader::BsmHeader()
    : m_vehicleId(0),
      m_position(Vector(0,0,0)),
      m_speed(0),
      m_direction(0),
      m_timestamp(Seconds(0))
{
}

BsmHeader::~BsmHeader()
{
}

TypeId
BsmHeader::GetTypeId(void)
{
    static TypeId tid = TypeId("ns3::BsmHeader")
        .SetParent<Header>()
        .AddConstructor<BsmHeader>()
        ;
    return tid;
}

TypeId
BsmHeader::GetInstanceTypeId(void) const
{
    return GetTypeId();
}

uint32_t
BsmHeader::GetSerializedSize(void) const
{
    return sizeof(m_vehicleId) + 
           sizeof(m_position.x) + sizeof(m_position.y) + sizeof(m_position.z) +
           sizeof(m_speed) + 
           sizeof(m_direction) + 
           sizeof(uint64_t); // timestamp
}

void
BsmHeader::Serialize(Buffer::Iterator start) const
{
    start.WriteU32(m_vehicleId);
    //start.WriteDouble(m_position.x);
    //start.WriteDouble(m_position.y);
    //start.WriteDouble(m_position.z);
    //start.WriteDouble(m_speed);
    //start.WriteDouble(m_direction);
    //start.WriteU64(m_timestamp.GetNanoSeconds());
    uint8_t buffer[8];
    memcpy(buffer, &m_position.x, 8);
    for(int i = 0; i < 8; i ++){
        start.WriteU8(buffer[i]);
    }

    memcpy(buffer, &m_position.y, 8);
    for(int i = 0; i < 8; i ++){
        start.WriteU8(buffer[i]);
    }

    memcpy(buffer, &m_position.z, 8);
    for(int i = 0; i < 8; i ++){
        start.WriteU8(buffer[i]);
    }

    
    memcpy(buffer, &m_speed, 8);
    for(int i = 0; i < 8; i ++){
        start.WriteU8(buffer[i]);
    }

    
    memcpy(buffer, &m_direction, 8);
    for(int i = 0; i < 8; i ++){
        start.WriteU8(buffer[i]);
    }

    start.WriteHtonU64(m_timestamp.GetNanoSeconds());

}

uint32_t
BsmHeader::Deserialize(Buffer::Iterator start)
{
    m_vehicleId = start.ReadU32();
    //m_position.x = start.ReadDouble();
    //m_position.y = start.ReadDouble();
    //m_position.z = start.ReadDouble();
    //m_speed = start.ReadDouble();
    //m_direction = start.ReadDouble();
    //m_timestamp = NanoSeconds(start.ReadU64());
    uint8_t buffer[8];
    for(int i = 0; i < 8; i ++){
        buffer[i] = start.ReadU8();
    }
    memcpy(&m_position.x, buffer, 8);

    for(int i = 0; i < 8; i ++){
        buffer[i] = start.ReadU8();
    }
    memcpy(&m_position.y, buffer, 8);

    for(int i = 0; i < 8; i ++){
        buffer[i] = start.ReadU8();
    }
    memcpy(&m_position.z, buffer, 8);

    for(int i = 0; i < 8; i ++){
        buffer[i] = start.ReadU8();
    }
    memcpy(&m_speed, buffer, 8);

    for(int i = 0; i < 8; i ++){
        buffer[i] = start.ReadU8();
    }
    memcpy(&m_direction, buffer, 8);
    
    m_timestamp = NanoSeconds(start.ReadNtohU64());

    return GetSerializedSize();
}

void
BsmHeader::Print(std::ostream &os) const
{
    os << "BSM Header: "
       << "VehicleId=" << m_vehicleId << ", "
       << "Position=(" << m_position.x << "," << m_position.y << "), "
       << "Speed=" << m_speed << ", "
       << "Direction=" << m_direction << ", "
       << "Timestamp=" << m_timestamp.GetSeconds() << "s";
}

// Getters and Setters
void BsmHeader::SetVehicleId(uint32_t id) { m_vehicleId = id; }
uint32_t BsmHeader::GetVehicleId(void) const { return m_vehicleId; }

void BsmHeader::SetPosition(Vector position) { m_position = position; }
Vector BsmHeader::GetPosition(void) const { return m_position; }

void BsmHeader::SetSpeed(double speed) { m_speed = speed; }
double BsmHeader::GetSpeed(void) const { return m_speed; }

void BsmHeader::SetDirection(double direction) { m_direction = direction; }
double BsmHeader::GetDirection(void) const { return m_direction; }

void BsmHeader::SetTimestamp(Time time) { m_timestamp = time; }
Time BsmHeader::GetTimestamp(void) const { return m_timestamp; }

} // namespace ns3 