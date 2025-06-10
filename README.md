# NS-3 Network Topology Simulator & Visualization Toolkit

This project provides a customizable simulation environment using NS-3 (Network Simulator 3) to evaluate the performance of different network topologies, TCP variants, traffic models, and routing protocols. It also includes a web-based XML parser for FlowMonitor results and NetAnim support for visualizing packet-level simulation behavior.

---

## 📦 Features

- Simulate **5 topology types**: Linear, Star, Ring, Mesh, Tree
- Supports **4 TCP variants**: NewReno, Cubic, HighSpeed, Vegas
- Multiple **traffic models**: BulkSend, On-Off, CBR
- **Routing protocols**: Static, DSDV, AODV, OLSR
- Optional **traffic injection** during simulation
- Logs metrics using **PacketSink** and **FlowMonitor**
- Exports:
  - `results.txt` (detailed per-flow breakdown)
  - `flowmon.xml` (for visual analysis via parser)
- **Web-based FlowMonitor parser** with graphs
- **NetAnim visualization** support

---

## 🖥 How to Install NS-3 on Windows (via WSL)

> These instructions guide you through installing NS-3 using Windows Subsystem for Linux (WSL).

### 1. Install WSL

```bash
wsl --install
```

Or manually install Ubuntu via Microsoft Store (Ubuntu 22.04 LTS recommended).

### 2. Launch Ubuntu and Update

```bash
sudo apt update
sudo apt upgrade -y
```

### 3. Install Dependencies

```bash
sudo apt install -y   g++ cmake qtbase5-dev qtchooser qt5-qmake qtbase5-dev-tools   libsqlite3-dev libxml2-dev libgtk-3-dev   python3 python3-dev python3-setuptools git mercurial unzip curl
```

### 4. Download and Build NS-3

```bash
git clone https://gitlab.com/nsnam/ns-3-dev.git ns-3
cd ns-3
./ns3 configure
./ns3 build
```

### 5. Test Your Installation

```bash
./ns3 run hello-simulator
```

---

## 🚀 Usage Instructions

### A. Compile

Place your simulation code (e.g., `hello.cc`) into `ns-3/scratch`.

```bash
./ns3 build
```

### B. Run a Simulation

Example command:

```bash
./ns3 run "scratch/hello --topology=4 --tcp=2 --traffic=1 --routing=1 --stopTime=30 --sender=0 --receiver=5"
```

### C. View Results

- `results_.txt` – full analysis
- `results_flowmon.xml` – load in NetAnim or the parser

---

## 📊 Web-based FlowMonitor Parser

A browser-based HTML tool for parsing and visualizing `flowmon.xml` outputs.

### Features:
- Bar and line charts for:
  - Throughput
  - Delay
  - Jitter
  - Packet loss
- Dynamic flow table with status indicators
- No server/backend required

🔗 GitHub Link:
[NS-3 FlowMonitor Visualization](https://github.com/Akshith-desu/NS-3-FlowMonitor-Visualization)

### Setup:
Simply open `index.html` in your browser and upload a `*_flowmon.xml` file.

---

## 📽 NetAnim Visualization

Use NS-3’s `NetAnim` tool to load the generated `.xml` animation traces:

1. Run simulation with `--animFile=output.xml`
2. Open NetAnim GUI:
   ```bash
   ./NetAnim
   ```
3. Load and explore packet flow animation.

---

## 🔧 Simulation Configuration Parameters

| Parameter      | Description                                 | Example               |
|----------------|---------------------------------------------|------------------------|
| `--topology`   | 1: Linear, 2: Star, 3: Ring, 4: Mesh, 5: Tree | `--topology=4`         |
| `--tcp`        | 1: NewReno, 2: Cubic, 3: HighSpeed, 4: Vegas | `--tcp=2`              |
| `--traffic`    | 1: BulkSend, 2: OnOff, 3: CBR                | `--traffic=1`          |
| `--routing`    | 1: Static, 2: DSDV, 3: AODV, 4: OLSR         | `--routing=3`          |
| `--inject`     | Enable mid-simulation traffic injection     | `--inject=true`        |
| `--injectSender`, `--injectReceiver`, `--injectDelay` | Configure injection behavior | `--injectSender=1 --injectReceiver=3 --injectDelay=0.3` |

---

## 📚 Educational Use

This project is designed to help students:

- Understand protocol and topology effects on network metrics
- Visualize concepts like delay, jitter, and packet loss
- Perform repeatable experiments via command line
- Use FlowMonitor vs. PacketSink outputs for layered insights



---
