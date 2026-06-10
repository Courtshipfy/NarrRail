#!/usr/bin/env python3
"""CLI client for NarrRail UE Remote Control Server.

Usage:
  python Tools/unreal_remote.py ping
  python Tools/unreal_remote.py pie
  python Tools/unreal_remote.py stop_pie
  python Tools/unreal_remote.py exec <code>
"""

import sys, json, urllib.request, urllib.error

HOST, PORT = "127.0.0.1", 19807


def request(command, **kw):
    url = "http://%s:%d/" % (HOST, PORT)
    data = json.dumps({"command": command, **kw}).encode()
    req = urllib.request.Request(
        url, data=data, headers={"Content-Type": "application/json"}
    )
    return json.loads(urllib.request.urlopen(req, timeout=10).read())


cmds = {
    "ping": ("ping", "Check if server is running"),
    "pie": ("pie", "Start Play In Editor"),
    "stop_pie": ("stop_pie", "Stop Play In Editor"),
    "exec": ("exec", "Execute arbitrary Python in UE"),
}

if len(sys.argv) < 2:
    print("Usage: python Tools/unreal_remote.py <command> [args]")
    print("\nCommands:")
    for k, (_, desc) in cmds.items():
        print(f"  {k:12s} {desc}")
    sys.exit(1)

try:
    r = request(
        sys.argv[1], code=" ".join(sys.argv[2:]) if sys.argv[1] == "exec" else None
    )
    print(
        r.get("result", r.get("error", str(r)))
        if r.get("ok")
        else "ERROR: " + r.get("error", str(r))
    )
except urllib.error.URLError as e:
    print("Connection failed:", e.reason)
    print("Make sure UE editor is running and narrrail_remote_server is loaded.")
except Exception as e:
    print("Error:", e)
