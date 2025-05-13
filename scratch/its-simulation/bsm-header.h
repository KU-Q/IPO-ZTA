#ifndef BSM_HEADER_H
#define BSM_HEADER_H

#include "ns3/header.h"
#include "ns3/vector.h"
#include "ns3/nstime.h"

namespace ns3 {

class BsmHeader : public Header {
public:
    BsmHeader();
    virtual ~BsmHeader();

    static TypeId GetTypeId(void);
    virtual TypeId GetInstanceTypeId(void) const;
    virtual uint32_t GetSerializedSize(void) const;
    virtual void Serialize(Buffer::Iterator start) const;
    virtual uint32_t Deserialize(Buffer::Iterator start);
    virtual void Print(std::ostream &os) const;

    // Getters and Setters
    void SetVehicleId(uint32_t id);
    uint32_t GetVehicleId(void) const;
    
    void SetPosition(Vector position);
    Vector GetPosition(void) const;
    
    void SetSpeed(double speed);
    double GetSpeed(void) const;
    
    void SetDirection(double direction);
    double GetDirection(void) const;
    
    void SetTimestamp(Time time);
    Time GetTimestamp(void) const;

private:
    uint32_t m_vehicleId;
    Vector m_position;
    double m_speed;
    double m_direction;
    Time m_timestamp;
};

} // namespace ns3

#endif /* BSM_HEADER_H */ 