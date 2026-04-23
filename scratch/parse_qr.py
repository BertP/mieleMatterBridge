import re

svg_path = r"c:\Users\bertp\work\Antigravity\mieleMatterBridge\main\www\assets\qrcode.svg"

with open(svg_path, "r") as f:
    content = f.read()

# Get the stroke path (black modules)
matches = re.findall(r'd="([^"]+)"', content)
d = matches[1] if len(matches) > 1 else matches[0]

matrix = [[0 for _ in range(25)] for _ in range(25)]

# Simplified parser
# Commands: M x y, m dx dy, h dx
parts = re.findall(r'([Mmh])\s*(-?[\d\.]+)(?:\s+(-?[\d\.]+))?', d)

cx, cy = 0, 0

for cmd, v1, v2 in parts:
    v1f = float(v1)
    v2f = float(v2) if v2 else 0.0
    
    if cmd == 'M':
        cx = v1f
        cy = v2f
    elif cmd == 'm':
        cx += v1f
        cy += v2f
    elif cmd == 'h':
        # Draw horizontal segment
        y_idx = int(cy - 0.5) - 4
        start_x = int(cx) - 4
        length = int(v1f)
        for i in range(length):
            ix = start_x + i
            if 0 <= ix < 25 and 0 <= y_idx < 25:
                matrix[y_idx][ix] = 1
        cx += v1f

print('{')
for row in matrix:
    print('    {' + ', '.join(map(str, row)) + '},')
print('}')
