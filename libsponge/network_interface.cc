#include "network_interface.hh"

#include "arp_message.hh"
#include "ethernet_frame.hh"

#include <iostream>

// Dummy implementation of a network interface
// Translates from {IP datagram, next hop address} to link-layer frame, and from link-layer frame to IP datagram

// For Lab 5, please replace with a real implementation that passes the
// automated checks run by `make check_lab5`.

// You will need to add private members to the class declaration in `network_interface.hh`

template <typename... Targs>
void DUMMY_CODE(Targs &&.../* unused */) {}

using namespace std;

//! \param[in] ethernet_address Ethernet (what ARP calls "hardware") address of the interface
//! \param[in] ip_address IP (what ARP calls "protocol") address of the interface
NetworkInterface::NetworkInterface(const EthernetAddress &ethernet_address, const Address &ip_address)
    : _ethernet_address(ethernet_address), _ip_address(ip_address) {
    cerr << "DEBUG: Network interface has Ethernet address " << to_string(_ethernet_address) << " and IP address "
         << ip_address.ip() << "\n";
}

//! \param[in] dgram the IPv4 datagram to be sent
//! \param[in] next_hop the IP address of the interface to send it to (typically a router or default gateway, but may also be another host if directly connected to the same network as the destination)
//! (Note: the Address type can be converted to a uint32_t (raw 32-bit IP address) with the Address::ipv4_numeric() method.)
void NetworkInterface::send_datagram(const InternetDatagram &dgram, const Address &next_hop) {
    // convert IP address of next hop to raw 32-bit representation (used in ARP header)
    const uint32_t next_hop_ip = next_hop.ipv4_numeric();

    if (_map.count(next_hop_ip)) {
        EthernetFrame frame{};
        EthernetHeader header{};

        header.dst = _map[next_hop_ip].eth_address;
        header.src = _ethernet_address;
        header.type = EthernetHeader::TYPE_IPv4;

        frame.header() = header;
        frame.payload() = dgram.serialize();

        _frames_out.push(frame);
    } else {
        if (_wait_for_ip_packages.count(next_hop_ip)) {
            _wait_for_ip_packages[next_hop_ip].push_back(dgram);
        } else {
            _wait_for_ip_packages[next_hop_ip] = {dgram};

            EthernetFrame frame{};
            EthernetHeader header{};
            ARPMessage payload{};

            header.dst = ETHERNET_BROADCAST;
            header.src = _ethernet_address;
            header.type = EthernetHeader::TYPE_ARP;

            payload.opcode = ARPMessage::OPCODE_REQUEST;
            payload.sender_ethernet_address = _ethernet_address;
            payload.sender_ip_address = _ip_address.ipv4_numeric();
            payload.target_ip_address = next_hop_ip;
            payload.target_ethernet_address = {};

            frame.header() = header;
            frame.payload() = payload.serialize();

            _frames_out.push(frame);
        }
    }

    DUMMY_CODE(dgram, next_hop, next_hop_ip);
}

//! \param[in] frame the incoming Ethernet frame
optional<InternetDatagram> NetworkInterface::recv_frame(const EthernetFrame &frame) {
    DUMMY_CODE(frame);
    if (frame.header().dst != _ethernet_address && frame.header().dst != ETHERNET_BROADCAST) {
        return nullopt;
    }

    if (frame.header().type == EthernetHeader::TYPE_IPv4) {
        InternetDatagram data;
        if (data.parse(frame.payload()) == ParseResult::NoError) {
            return data;
        } else {
            return nullopt;
        }
    } else if (frame.header().type == EthernetHeader::TYPE_ARP) {

    }
}

//! \param[in] ms_since_last_tick the number of milliseconds since the last call to this method
void NetworkInterface::tick(const size_t ms_since_last_tick) {
    DUMMY_CODE(ms_since_last_tick);
    for (auto &record : _map) {
        record.second.time += ms_since_last_tick;
        if (record.second.time >= 30 * 1000) {
            _map.erase(record.first);
        }
    }
}
