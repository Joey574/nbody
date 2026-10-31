import re
import sys
from collections import Counter, deque

import matplotlib.pyplot as plt
from matplotlib.collections import PatchCollection
from matplotlib.patches import Rectangle

NUM = r"'?([^,\]'\s]+)'?"
LINE = re.compile(
    rf"(\d+):\s*cm={NUM},\s*cx={NUM},\s*cy={NUM},"
    rf"\s*x=\[\s*{NUM},\s*{NUM}\s*\],\s*y=\[\s*{NUM},\s*{NUM}\s*\],"
    r"\s*ne='?(\d+)u'?,\s*nw='?(\d+)u'?,\s*se='?(\d+)u'?,\s*sw='?(\d+)u'?"
)
QUADS = ("ne", "nw", "se", "sw")
EAST, WEST = ("ne", "se"), ("nw", "sw")
NORTH, SOUTH = ("ne", "nw"), ("se", "sw")
EPS = 0.011  # values are printed to 2 decimals


class Node:
    __slots__ = ("idx", "cm", "cx", "cy", "xmin", "xmax", "ymin", "ymax",
                 "children", "px", "py")

    def __init__(self, idx, cm, cx, cy, xmin, xmax, ymin, ymax, children):
        self.idx, self.cm, self.cx, self.cy = idx, cm, cx, cy
        self.xmin, self.xmax, self.ymin, self.ymax = xmin, xmax, ymin, ymax
        self.children = children      # {'ne': idx, ...}, 0 == no child
        self.px = self.py = None      # this node's own body position (recovered)

    @property
    def is_leaf(self):
        return all(c == 0 for c in self.children.values())


def parse_nodes(raw):
    nodes, skipped = {}, 0
    for line in raw.splitlines():
        line = line.strip()
        if not line:
            continue
        m = LINE.match(line)
        if not m:
            skipped += 1
            continue
        g = m.groups()
        idx = int(g[0])
        cm, cx, cy, xmin, xmax, ymin, ymax = map(float, g[1:8])
        kids = dict(zip(QUADS, map(int, g[8:12])))
        nodes[idx] = Node(idx, cm, cx, cy, xmin, xmax, ymin, ymax, kids)
    if skipped:
        print(f"warning: {skipped} line(s) did not match the expected format")
    return nodes


def walk_tree(nodes, root=0):
    """BFS from the root; detects dangling children, shared children and cycles."""
    parent, problems = {root: None}, []
    q = deque([root])
    while q:
        i = q.popleft()
        for quad, c in nodes[i].children.items():
            if c == 0:
                continue
            if c not in nodes:
                problems.append(f"{i}.{quad} -> {c}: child not in dump")
            elif c in parent:
                problems.append(f"{i}.{quad} -> {c}: already reached via {parent[c]}")
            else:
                parent[c] = i
                q.append(c)
    return parent, problems


def validate(nodes, parent):
    issues = {k: [] for k in (
        "com_outside_extent", "child_outside_parent", "leaf_not_point",
        "quadrant_overlap", "pivot_mismatch", "bad_own_mass")}
    own_mass = Counter()

    for i, n in nodes.items():
        if i not in parent:
            continue
        kids = [(q, nodes[c]) for q, c in n.children.items() if c and c in nodes]

        if not (n.xmin - EPS <= n.cx <= n.xmax + EPS and
                n.ymin - EPS <= n.cy <= n.ymax + EPS):
            issues["com_outside_extent"].append(i)

        if not kids:  # leaf: extent must collapse to the body itself
            if (n.xmax - n.xmin > EPS or n.ymax - n.ymin > EPS or
                    abs(n.cx - n.xmin) > EPS or abs(n.cy - n.ymin) > EPS):
                issues["leaf_not_point"].append(i)
            n.px, n.py = n.cx, n.cy
            own_mass[round(n.cm, 3)] += 1
            continue

        for _, k in kids:
            if (k.xmin < n.xmin - EPS or k.xmax > n.xmax + EPS or
                    k.ymin < n.ymin - EPS or k.ymax > n.ymax + EPS):
                issues["child_outside_parent"].append(k.idx)

        ksum = sum(k.cm for _, k in kids)
        m = n.cm - ksum                      # mass of this node's own body
        own_mass[round(m, 3)] += 1
        if m <= EPS:
            issues["bad_own_mass"].append(i)
            continue

        # Recover own body position from the COM sums (noisy: inputs are rounded)
        n.px = (n.cm * n.cx - sum(k.cm * k.cx for _, k in kids)) / m
        n.py = (n.cm * n.cy - sum(k.cm * k.cy for _, k in kids)) / m
        tol = 0.005 * (n.cm + ksum) / m + EPS

        # Children are split at the node's body: west kids must lie left of
        # east kids, south kids below north kids.
        inf = float("inf")
        lo_x = max((k.xmax for q, k in kids if q in WEST), default=-inf)
        hi_x = min((k.xmin for q, k in kids if q in EAST), default=inf)
        lo_y = max((k.ymax for q, k in kids if q in SOUTH), default=-inf)
        hi_y = min((k.ymin for q, k in kids if q in NORTH), default=inf)

        if lo_x > hi_x + EPS or lo_y > hi_y + EPS:
            issues["quadrant_overlap"].append(i)
        elif not (lo_x - tol <= n.px <= hi_x + tol and lo_y - tol <= n.py <= hi_y + tol):
            issues["pivot_mismatch"].append(i)

    return issues, own_mass


def report(nodes, parent, problems, issues, own_mass):
    leaves = sum(n.is_leaf for n in nodes.values())
    print(f"{len(nodes)} nodes, {leaves} leaves, {len(nodes) - leaves} internal")
    for p in problems[:20]:
        print("  STRUCT:", p)
    unreachable = sorted(set(nodes) - set(parent))
    print(f"unreachable from root: {len(unreachable)} {unreachable[:10]}")
    for name, ids in issues.items():
        print(f"{name}: {len(ids)} {sorted(set(ids))[:10]}")
    print("own-body masses:", dict(own_mass.most_common(5)))
    return unreachable


def plot(nodes, parent, issues, unreachable):
    fig, ax = plt.subplots(figsize=(10, 10))

    # Subtree extents (leaves are zero-size, skip them)
    rects = [Rectangle((n.xmin, n.ymin), n.xmax - n.xmin, n.ymax - n.ymin)
             for n in nodes.values()
             if i_reach(n, parent) and (n.xmax - n.xmin > EPS or n.ymax - n.ymin > EPS)]
    ax.add_collection(PatchCollection(rects, fc="none", ec="navy", lw=0.6, alpha=0.3))

    flagged = {i for ids in issues.values() for i in ids}
    unr = set(unreachable)

    def pts(pred):
        sel = [n for n in nodes.values() if n.px is not None and pred(n)]
        return [n.px for n in sel], [n.py for n in sel]

    ok = lambda n: n.idx not in flagged and n.idx not in unr
    ax.scatter(*pts(lambda n: n.is_leaf and ok(n)), c="crimson", s=25,
               label="Leaf body", zorder=3)
    ax.scatter(*pts(lambda n: not n.is_leaf and n.idx != 0 and ok(n)), c="blue",
               marker="x", s=40, label="Internal body (recovered)", zorder=3)
    ax.scatter(*pts(lambda n: n.idx in flagged), c="lime", edgecolors="black",
               s=90, label=f"Flagged ({len(flagged)})", zorder=5)
    if unr:
        ux = [nodes[i].cx for i in unr]
        uy = [nodes[i].cy for i in unr]
        ax.scatter(ux, uy, c="orange", marker="s", s=40,
                   label=f"Unreachable ({len(unr)})", zorder=4)
    if 0 in nodes and nodes[0].px is not None:
        ax.scatter(nodes[0].px, nodes[0].py, c="gold", edgecolors="black",
                   marker="*", s=250, label="Body 0 (root)", zorder=6)

    ax.autoscale_view()
    ax.set_aspect("equal")
    ax.grid(True, linestyle=":", alpha=0.5)
    ax.legend(loc="upper right")
    ax.set_title("Quadtree subtree extents & body positions")
    plt.show()


def i_reach(n, parent):
    return n.idx in parent


if __name__ == "__main__":
    if len(sys.argv) < 2:
        sys.exit(f"Usage: python {sys.argv[0]} <data_file>")
    try:
        raw = open(sys.argv[1]).read()
    except FileNotFoundError:
        sys.exit(f"Error: File '{sys.argv[1]}' not found.")

    nodes = parse_nodes(raw)
    parent, problems = walk_tree(nodes)
    issues, own_mass = validate(nodes, parent)
    unreachable = report(nodes, parent, problems, issues, own_mass)
    plot(nodes, parent, issues, unreachable)
