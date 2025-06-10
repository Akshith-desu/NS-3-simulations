# NS-3 Network Topology Simulator & Visualization Toolkit

This project is a modular and extensible NS-3 simulation suite developed to explore how various network design parameters affect performance. It allows users—particularly students and researchers—to simulate different topologies, TCP variants, traffic models, and routing protocols. The simulation results are logged and visualized through a browser-based FlowMonitor parser and NS-3's NetAnim animator.

---

## 🧠 Project Overview

This toolkit was built to help learners and network researchers answer questions like:

- How does a **mesh** network perform compared to a **linear** topology?
- What is the difference in **delay** when using **AODV** versus **OLSR**?
- How do **Cubic** and **Vegas** TCP variants affect **throughput**?
- What is the **impact of bursty traffic** (On-Off) vs. continuous flow (BulkSend)?
- How does **injection of additional traffic** mid-simulation influence network behavior?

### Components:

- NS-3 C++ simulator (`hello.cc`)
- Traffic generation and flow injection system
- Per-flow statistics with **FlowMonitor** and **PacketSink**
- Automatic generation of:
  - `results.txt` — human-readable summary
  - `flowmon.xml` — for NetAnim or the web parser
- Web-based **FlowMonitor Parser** (Chart.js)
- NetAnim `.xml` support for visualization

---

## 📦 Features

- Simulate **5 topology types**: Linear, Star, Ring, Mesh, Tree
- Supports **4 TCP variants**: NewReno, Cubic, HighSpeed, Vegas
- Multiple **traffic models**: BulkSend, On-Off, CBR
- **Routing protocols**: Static, DSDV, AODV, OLSR
- **Traffic injection** with delay adjustment
- Metrics: **throughput, delay, jitter, packet loss**
- Exports:
  - `results.txt` (formatted results)
  - `flowmon.xml` (FlowMonitor export)
- Web parser + NetAnim support for interactive visualization

---

## 🖥 How to Install NS-3 on Windows (via WSL)

### 1. Enable WSL and Install Ubuntu

```bash
wsl --install
```

> Or use Microsoft Store to install Ubuntu 22.04 LTS manually

### 2. Update and Upgrade

```bash
sudo apt update
sudo apt upgrade -y
```

### 3. Install NS-3 Dependencies

```bash
sudo apt install -y \
  g++ cmake qtbase5-dev qtchooser qt5-qmake qtbase5-dev-tools \
  libsqlite3-dev libxml2-dev libgtk-3-dev \
  python3 python3-dev python3-setuptools git mercurial unzip curl
```

### 4. Clone and Build NS-3

```bash
git clone https://gitlab.com/nsnam/ns-3-dev.git ns-3
cd ns-3
./ns3 configure
./ns3 build
```

### 5. Run Test Simulation

```bash
./ns3 run hello-simulator
```

---

## 🚀 How to Use

### A. Add Your Simulation

Put your simulation C++ file (e.g., `hello.cc`) into:

```bash
ns-3/scratch/
```

### B. Compile It

```bash
./ns3 build
```

### C. Run It

Example:

```bash
./ns3 run "scratch/hello --topology=4 --tcp=2 --traffic=1 --routing=1 --stopTime=30 --sender=0 --receiver=5"
```

### D. Outputs

- `results.txt` – PacketSink + FlowMonitor summary
- `flowmon.xml` – used for parser and NetAnim
- `output.xml` (optional) – NetAnim trace file

---

## 📊 FlowMonitor Parser (Web-based)

A browser-based HTML tool for parsing and visualizing flowmon.xml outputs.

### Features:
- Bar and line charts for:
  - Throughput
  - Delay
  - Jitter
  - Packet loss
- Dynamic flow table with status indicators
-Flow breakdown table
- No server/backend required

🔗 GitHub Link:
[NS-3 FlowMonitor Visualization](https://github.com/Akshith-desu/NS-3-FlowMonitor-Visualization)

### Setup:
Simply open index.html in your browser and upload a *_flowmon.xml file.
---

## 📽 NetAnim Visualization

NS-3’s native animation tool for flow-level simulation replay.

### Steps:

1. Run simulation with `--animFile=output.xml`
2. Launch NetAnim:
   ```bash
   ./NetAnim
   ```
3. Load `output.xml` and observe node activity, packet flow, etc.

---

## 🔧 Simulation Parameters

| Parameter          | Description                               | Example                  |
| ------------------ | ----------------------------------------- | ------------------------ |
| `--topology`       | Topology type (1–5)                       | `--topology=3` (Ring)    |
| `--tcp`            | TCP variant                               | `--tcp=2` (Cubic)        |
| `--traffic`        | Traffic model                             | `--traffic=1` (BulkSend) |
| `--routing`        | Routing protocol                          | `--routing=4` (OLSR)     |
| `--inject`         | Enable flow injection                     | `--inject=true`          |
| `--injectSender`   | Comma-separated injection sender node IDs | `--injectSender=2,3`     |
| `--injectReceiver` | Comma-separated receiver node IDs         | `--injectReceiver=5,4`   |
| `--injectDelay`    | Start time factor (e.g., 0.3 = 30%)       | `--injectDelay=0.3`      |

---

## 🎓 Educational Focus

This project is ideal for undergraduate and graduate students learning:

- Network simulation and protocol behavior
- Real-world impact of delay, jitter, and congestion
- TCP variant performance comparison
- Flow-level vs application-level metrics
- Visualization-based validation of assumptions

All output is **readable, repeatable, and graphically explainable**.

---
