import re, sys, argparse
from datetime import datetime
from collections import defaultdict

try:
    import matplotlib; matplotlib.use("Agg")
    import matplotlib.pyplot as plt
    import matplotlib.patches as mpatches
    import matplotlib.gridspec as gridspec
    import matplotlib.ticker as ticker
except ImportError:
    sys.exit("pip install matplotlib")

LINE_RE  = re.compile(r"^(.+?)\s*\|\s*(\w+)\s*\|\s*(.+?)\s*\|\s*(.*)$")
SEQ_RE   = re.compile(r"(?:seq:|SEQUENCE:)\s*(\d+)", re.IGNORECASE)
CHAR_RE  = re.compile(r"PAYLOAD MSG CHARS:\s*\[(.)\]")
DELAY_RE = re.compile(r"(\d+)\s*ms")
MSG_RE   = re.compile(r"Received message:\s*(.+)", re.IGNORECASE)

def parse_logs(*paths):
    events = []
    for path in paths:
        try:
            with open(path, errors="replace") as fh:
                for raw in fh:
                    raw = raw.strip()
                    if not raw: continue
                    m = LINE_RE.match(raw)
                    if not m: continue
                    ts_s, prog, event, details = m.groups()
                    try: ts = datetime.fromisoformat(ts_s.strip())
                    except ValueError: continue
                    seq_m  = SEQ_RE.search(details)
                    char_m = CHAR_RE.search(details)
                    dly_m  = DELAY_RE.search(details)
                    events.append(dict(
                        ts=ts, prog=prog.upper(), event=event.strip(),
                        details=details.strip(),
                        seq     = int(seq_m.group(1))  if seq_m  else None,
                        char    = char_m.group(1)       if char_m else "",
                        delay_ms= int(dly_m.group(1))  if dly_m  else None,
                    ))
        except FileNotFoundError:
            pass
    return sorted(events, key=lambda e: e["ts"])

def reconstruct(events):
    for ev in events:
        if ev["prog"] == "SERVER" and ev["event"] == "MESSAGE CONSTRUCTED":
            m = MSG_RE.search(ev["details"])
            if m: return m.group(1).strip()
    chars = {}
    for ev in events:
        if ev["prog"] == "SERVER" and ev["event"] == "PACKET RECEIVED":
            if ev["seq"] is not None and ev["char"]:
                chars[ev["seq"]] = ev["char"]
    return "".join(v for _, v in sorted(chars.items())) if chars else ""

BLUE="#1565C0"; DBLUE="#1976D2"; GREEN="#2E7D32"; DGREEN="#388E3C"
AMBER="#F57F17"; AMBER2="#FFD54F"; RED="#C62828"; PURPLE="#7B1FA2"
DPURP="#9C27B0"; LGREY="#ECEFF1"

def draw_stats(ax, events, msg):
    c = defaultdict(int)
    for ev in events:
        c[(ev["prog"], ev["event"])] += 1

    rows = [
        ("Pkts Sent\n(client)",    c[("CLIENT","SENDING PACKET")]+c[("CLIENT","RE-SENT PACKET")], BLUE),
        ("Pkts Re-sent\n(client)",c[("CLIENT","RE-SENT PACKET")],                               "#42A5F5"),
        ("ACKs Recv\n(client)",   c[("CLIENT","RECEIVED ACK")],                                 GREEN),
        ("Retransmits\n(client)", c[("CLIENT","RETRANSMIT")],                                   "#FF7043"),
        ("Timeouts\n(client)",    c[("CLIENT","TIMEOUT")],                                      RED),
        ("Pkts Recv\n(proxy)",    c[("PROXY","RECEIVED ACK")]+c[("PROXY","RECEIVED PACKET")],   PURPLE),
        ("Fwd→Server\n(proxy)",   c[("PROXY","FORWARDING PACKET")],                             DPURP),
        ("ACKs Recv\n(proxy)",    c[("PROXY","RECEIVED ACK")],                                  DGREEN),
        ("Fwd→Client\n(proxy)",   c[("PROXY","FORWARDING ACK")],                                "#43A047"),
        ("C→S Delays\n(proxy)",   c[("PROXY","DELAYING PACKET")],                               AMBER),
        ("S→C Delays\n(proxy)",   c[("PROXY","DELAYING ACK")],                                  AMBER2),
        ("Drops\n(proxy)",        c[("PROXY","DROPPED")],                                       RED),
        ("Pkts Dropped\n(client)",c[("PROXY","DROPPING PACKET")],                               "#E53935"),
        ("Pkts Dropped\n(server)",c[("PROXY","DROPPING ACK")],                                  "#B71C1C"),
        ("Pkts Recv\n(server)",   c[("CLIENT","PACKET RECEIVED")],                              DBLUE),
        ("ACKs Sent\n(server)",   c[("SERVER","SENDING ACK")],                                  "#388E3C"),
    ]
    rows = [(lbl, val, col) for lbl, val, col in rows if val > 0]
    labels=[r[0] for r in rows]; values=[r[1] for r in rows]; colors=[r[2] for r in rows]

    bars = ax.bar(labels, values, color=colors, edgecolor="white", linewidth=0.9, zorder=3, width=0.6)
    for bar, val in zip(bars, values):
        ax.text(bar.get_x()+bar.get_width()/2, bar.get_height()+0.08,
                str(val), ha="center", va="bottom", fontsize=11, fontweight="bold", color="#212121")

    ax.set_ylabel("Packet", fontsize=11)
    ax.grid(axis="y", alpha=0.3, linestyle=":", zorder=0)
    ax.set_axisbelow(True)
    ax.yaxis.set_major_locator(ticker.MaxNLocator(integer=True))
    plt.setp(ax.get_xticklabels(), rotation=30, ha="right", fontsize=10)
    ax.spines["top"].set_visible(False); ax.spines["right"].set_visible(False)

def make_summary(events):
    c=defaultdict(int)
    for ev in events: c[(ev["prog"],ev["event"])]+=1
    return (f"Pkts sent: {c[('CLIENT','SENDING PACKET')]}  |  "
            f"ACKed: {c[('CLIENT','RECEIVED ACK')]}  |  "
            f"Retransmits: {c[('CLIENT','RETRANSMIT')]}  |  "
            f"Timeouts: {c[('CLIENT','TIMEOUT')]}  |  "
            f"Lost: {c[('CLIENT','LOST')]}  |  "
            f"Proxy drops: {c[('PROXY','DROPPED')]}  |  "
            f"C→S delays: {c[('PROXY','DELAYING PACKET')]}  |  "
            f"S→C delays: {c[('PROXY','DELAYING ACK')]}  |  "
            f"Server received: {c[('SERVER','PACKET RECEIVED')]}")

def render(client_log, proxy_log, server_log, out_file):
    events=parse_logs(client_log, proxy_log, server_log)
    msg=reconstruct(events)
    has_delays=any(ev["prog"]=="PROXY" and ev["event"] in ("DELAYING PACKET","DELAYING ACK") for ev in events)

    fig=plt.figure(figsize=(16, 12 if has_delays else 7), facecolor="#FAFAFA")
    fig.suptitle("COMP7005  –  Reliable UDP Log Visualizer",fontsize=15,fontweight="bold",y=0.98)

    gs=gridspec.GridSpec(1,1,figure=fig,left=0.08,right=0.97,top=0.93,bottom=0.14)
    ax_stats=fig.add_subplot(gs[0]); ax_delay=None

    if events:
        draw_stats(ax_stats, events, msg)

    else:
        ax_stats.text(0.5,0.5,"No log data found.",ha="center",va="center",
                      transform=ax_stats.transAxes,fontsize=14,color="gray")

    fig.savefig(out_file,dpi=150,bbox_inches="tight")
    print(f"Saved → {out_file}")

if __name__=="__main__":
    ap=argparse.ArgumentParser()
    ap.add_argument("--client-log",default="client.log")
    ap.add_argument("--proxy-log", default="proxy.log")
    ap.add_argument("--server-log",default="server.log")
    ap.add_argument("--out",       default="visualization.png")
    args=ap.parse_args()
    render(args.client_log, args.proxy_log, args.server_log, args.out)

