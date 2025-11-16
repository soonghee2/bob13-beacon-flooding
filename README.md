# 1. 이 프로그램의 목적

**임의의 SSID 목록 파일(예: ssid-list.txt)에 적힌 이름들을 이용해,
수백 개의 가짜 AP(Access Point) Beacon 신호를 계속 전송하여
주변 기기에서 와이파이 목록이 난장판처럼 보이게 만드는 툴.**

일명 **Beacon Flooder** / **Beacon Spammer**.

---

# 2. 주요 기능 요약

### ✓ Radiotap + 802.11 Beacon 프레임을 직접 구성

* Radiotap Header를 직접 만들고
* 802.11 Management Frame(Beacon)을 조립
* Fixed Parameters, Tagged Parameters, SSID Tag까지 직접 넣음
* pcap_sendpacket으로 생패킷(raw packet)을 무선 인터페이스로 송출

### ✓ SSID 목록 파일을 읽어서 각 SSID로 Beacon 프레임 생성

ssids 벡터에 파일에서 읽은 문자열을 넣음.
각 SSID마다 한 번씩 Beacon 프레임 생성 및 전송.

### ✓ 가짜 MAC 주소 랜덤(혹은 증가값) 생성

address2(송신자 MAC), address3(BSSID)를
`mac_seed++`로 뒷자리만 바꿔가며 생성해서
**다른 AP처럼 보이게** 함.

---

# 3. 코드가 실제로 하는 흐름

1. 실행 예:

   ```
   ./beacon-flood mon0 ssid-list.txt
   ```

2. ssid-list.txt 내용 예:

   ```
   Starbucks_Free_WiFi
   KT_GiGA
   Drone_Hacking_Lab
   Virus_AP
   ...
   ```

3. 프로그램은 이 SSID들을 하나씩 꺼내

   * Radiotap Header 구성
   * Beacon Frame Header 구성
   * Fixed Params 삽입
   * SSID Tag 삽입
   * Tagged Params(지원 속도 등) 삽입

4. pcap_sendpacket() 사용해 무한 반복 전송

결과: 주변 와이파이 목록에 **수십~수백 개의 가짜 AP가 나타남.**

---

# 4. 이 코드의 특징

* 모니터 모드 인터페이스 필요(mon0, wlan0mon 등)
* root 권한 필요
* 주파수 및 채널은 자동 조절 안함
* 단순히 Beacon만 스팸으로 뿌림
* Aircrack-ng의 airodump, aireplay beacon flood와 유사

---

# 5. 한 줄 정리

**이 코드는 SSID 리스트를 기반으로 무한히 가짜 Beacon 프레임을 뿌리는
와이파이 Beacon Flood 공격 툴이다.**
