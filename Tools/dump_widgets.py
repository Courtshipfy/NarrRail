#!/usr/bin/env python3
"""Dump all PIE UI widgets with hierarchy and state.
Usage:  python Tools/dump_widgets.py
"""

import http.client, json

HOST, PORT = "127.0.0.1", 19807

PY_CODE = """
import unreal
worlds = unreal.EditorLevelLibrary.get_pie_worlds(False)
if not worlds:
    result = {"error": "No PIE worlds. Start PIE first."}
else:
    pw = worlds[0]
    nodes, parents, seen = {}, {}, set()
    for obj in unreal.ObjectIterator(unreal.Widget):
        try:
            if obj.get_world() != pw: continue
            uid = id(obj)
            if uid in seen: continue
            seen.add(uid)
            n = obj.get_name()
            if "Default__" in n or "SKEL" in n: continue
            info = {"class": obj.get_class().get_name()}
            for attr in ["visible", "enabled", "opacity"]:
                try:
                    if attr == "visible": info[attr] = obj.is_visible()
                    elif attr == "enabled": info[attr] = obj.b_is_enabled if hasattr(obj, "b_is_enabled") else obj.get_is_enabled()
                    elif attr == "opacity": info[attr] = obj.render_opacity
                except: pass
            if hasattr(obj, "text"):
                try:
                    t = obj.text
                    if t: info["text"] = t.to_string()[:80]
                except: pass
            if hasattr(obj, "is_pressed"):
                try: info.update(pressed=obj.is_pressed(), hovered=obj.is_hovered())
                except: pass
            if hasattr(obj, "b_is_focusable"):
                try: info["focusable"] = obj.b_is_focusable
                except: pass
            nodes[n] = info
            try:
                p = obj.get_parent()
                if p: parents[n] = p.get_name()
            except: pass
        except: pass
    children_of, roots = {}, []
    for n in nodes:
        p = parents.get(n)
        if p is not None and p in nodes: children_of.setdefault(p, []).append(n)
        else: roots.append(n)
    result_tree = []
    for r in roots:
        stack = [(r, result_tree)]
        while stack:
            name, pl = stack.pop()
            e = dict(nodes[name]); e["name"] = name
            ks = children_of.get(name)
            if ks:
                e["children"] = []
                for k in ks: stack.append((k, e["children"]))
            pl.append(e)
    pc = unreal.GameplayStatics.get_player_controller(pw, 0)
    ps = {}
    if pc:
        for a in ["show_mouse_cursor","enable_click_events","enable_mouse_over_events"]:
            try: ps[a] = getattr(pc, a)
            except: pass
    result = {"player_controller": ps, "widget_count": len(nodes), "tree": result_tree}
"""

conn = http.client.HTTPConnection(HOST, PORT, timeout=60)
body = json.dumps({"command": "exec", "code": PY_CODE})
conn.request("POST", "/", body, {"Content-Type": "application/json"})
r = json.loads(conn.getresponse().read())
data = r.get("result", r)
if isinstance(data, dict) and "error" in data:
    print("ERROR:", data["error"])
else:
    print(json.dumps(data, indent=2, ensure_ascii=False, default=str))
