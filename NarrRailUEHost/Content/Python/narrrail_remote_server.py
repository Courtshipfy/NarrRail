import unreal
import json
import threading
import queue
import traceback
from http.server import HTTPServer, BaseHTTPRequestHandler

HOST = "127.0.0.1"
PORT = 19807

_cmd_queue = queue.Queue()
_cmd_results = {}

def _on_slate_tick(delta_time):
    try:
        while not _cmd_queue.empty():
            cmd_id, command, body = _cmd_queue.get_nowait()
            try:
                result = _execute(command, body)
                _cmd_results[cmd_id] = {"ok": True, "result": result}
            except Exception as e:
                _cmd_results[cmd_id] = {"ok": False, "error": str(e), "traceback": traceback.format_exc()}
    except Exception:
        pass

# Register tick callback on main thread (runs during import, Slate is ready at PreDefault phase)
_tick_handle = None
try:
    _tick_handle = unreal.register_slate_post_tick_callback(_on_slate_tick)
    unreal.log("NarrRailRemote: slate tick registered on main thread")
except Exception as e:
    unreal.log("NarrRailRemote: slate tick registration failed: " + str(e))


class CommandHandler(BaseHTTPRequestHandler):
    def log_message(self, format, *args):
        unreal.log("NarrRailRemote: " + format % args)

    def _send_json(self, code, data):
        self.send_response(code)
        self.send_header("Content-Type", "application/json")
        self.end_headers()
        self.wfile.write(json.dumps(data).encode())

    def do_POST(self):
        try:
            length = int(self.headers.get("Content-Length", 0))
            body = json.loads(self.rfile.read(length)) if length else {}
            command = body.get("command", "")
            unreal.log("NarrRailRemote: queuing command=%s" % command)

            cmd_id = id(body)
            _cmd_queue.put((cmd_id, command, body))

            import time
            deadline = time.time() + 30
            while time.time() < deadline:
                if cmd_id in _cmd_results:
                    result = _cmd_results.pop(cmd_id)
                    if result["ok"]:
                        self._send_json(200, result)
                    else:
                        self._send_json(500, result)
                    return
                time.sleep(0.1)

            self._send_json(504, {"ok": False, "error": "Command timed out"})
        except Exception as e:
            self._send_json(500, {"ok": False, "error": str(e), "traceback": traceback.format_exc()})

    def do_GET(self):
        self._send_json(200, {
            "service": "NarrRail Remote Control",
            "commands": ["ping", "pie", "stop_pie", "load_map", "exec", "get_asset_list"],
            "usage": "POST with JSON body {command: ..., ...}"
        })

    def do_OPTIONS(self):
        self.send_response(204)
        self.end_headers()


def _execute(command, body):
    if command == "pie":
        return _cmd_pie()
    elif command == "stop_pie":
        return _cmd_stop_pie()
    elif command == "ping":
        return "pong"
    elif command == "load_map":
        return _cmd_load_map(body.get("map"))
    elif command == "exec":
        return _cmd_exec(body.get("code"))
    elif command == "get_asset_list":
        return _cmd_get_asset_list(body.get("path", "/Game"))
    else:
        raise ValueError("Unknown command: %s" % command)


def _cmd_pie():
    unreal.EditorLevelLibrary.editor_play_simulate()
    return "PIE started"

def _cmd_stop_pie():
    unreal.EditorLevelLibrary.editor_end_play()
    return "PIE stop requested"

def _cmd_load_map(map_name):
    if not map_name:
        raise ValueError("map parameter required")
    success = unreal.EditorLevelLibrary.load_level(map_name)
    return "Map loaded: %s" % map_name if success else "Failed to load map: %s" % map_name

def _cmd_exec(code):
    if not code:
        raise ValueError("code parameter required")
    local_vars = {}
    exec(code, {"unreal": unreal}, local_vars)
    return local_vars.get("result", "exec ok")

def _cmd_get_asset_list(path):
    asset_registry = unreal.AssetRegistryHelpers.get_asset_registry()
    ar_filter = unreal.ARFilter()
    ar_filter.package_paths = [path]
    assets = asset_registry.get_assets(ar_filter)
    return [{"name": a.asset_name, "path": a.object_path} for a in assets[:50]]


def _run_server():
    server = HTTPServer((HOST, PORT), CommandHandler)
    unreal.log("NarrRailRemote: listening on http://%s:%d" % (HOST, PORT))
    server.serve_forever()


# Top-level code runs on main thread during import
unreal.log("NarrRailRemote: starting HTTP server background thread")
thread = threading.Thread(target=_run_server, daemon=True)
thread.start()
unreal.log("NarrRailRemote: ready - send POST to http://%s:%d" % (HOST, PORT))
