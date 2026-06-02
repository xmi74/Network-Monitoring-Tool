# isa-top: Network Flow Monitoring Tool

A Linux CLI utility designed for real-time network traffic monitoring. Inspired by tools like `iftop`, this application sniffs packets on a specified interface, tracks network flows, and dynamically renders live bandwidth statistics.

---

## Usage
```bash
sudo ./isa-top -i INTERFACE [-t TIME] [-s SORT_METHOD]
```

## Options

| Switch | Argument | Requirement | Description |
| :--- | :--- | :--- | :--- |
| **`-i, --interface`** | `<string>` | **Required** | Network interface to sniff on (e.g., `eth0`, `enp0s3`). |
| **`-t, --time`**      | `<int>`    | *Optional*   | Update/refresh interval in seconds (range: 1..100). <br>_Default: 1s._ |
| **`-s, --sort`**      | `b` \| `p` | *Optional*   | Defines the sorting metric: <br>• `b` – sort by bits/s (default) <br>• `p` – sort by packets/s |

## Key Features

* Uses `libpcap` to capture live packets directly from the network layer.
* Parses and distinguishes between `TCP`, `UDP`, `ICMP`, `IGMP`, and `ICMPv6` protocols.
* Implements a hash table with bucket chaining to aggregate bidirectional traffic metrics (RX/TX bits and packets) independently of the initial packet direction.
* Uses POSIX threads to separate the infinite packet capturing loop from the timed statistics display loop.
* Implements an optimized Bubble Sort to display the top 10 most active flows using a clean, auto-scaled visual interface (K, M, G suffixes for speeds).

---

## Technical Stack

* **Language:** C (C99 standard compliant)
* **Libraries:** 
  * `libpcap` – low-level packet capture
  * `pthread` – POSIX multi-threading
  * `ncurses` – terminal screen rendering
* **Target OS:** Linux (developed & tested on Ubuntu)

---

## Installation & Requirements

```bash
sudo apt update
sudo apt install libpcap-dev libncurses5-dev libncursesw5-dev build-essential
```

## Compilation
```bash
make build
```

clean up compiled binaries with
```bash
make clean
```


