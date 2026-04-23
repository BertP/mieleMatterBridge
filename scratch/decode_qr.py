import re

# SVG Path-Daten aus deiner Datei
path_d = 'M4 4.5h7m5 0h1m1 0h1m3 0h7M4 5.5h1m5 0h1m1 0h1m2 0h3m4 0h1m5 0h1M4 6.5h1m1 0h3m1 0h1m2 0h1m1 0h2m2 0h1m2 0h1m1 0h3m1 0h1M4 7.5h1m1 0h3m1 0h1m6 0h1m4 0h1m1 0h3m1 0h1M4 8.5h1m1 0h3m1 0h1m1 0h1m1 0h2m1 0h1m2 0h1m1 0h1m1 0h3m1 0h1M4 9.5h1m5 0h1m2 0h1m4 0h2m2 0h1m5 0h1M4 10.5h7m1 0h1m1 0h1m1 0h1m1 0h1m1 0h1m1 0h7M15 11.5h2m2 0h1M4 12.5h1m1 0h1m1 0h1m1 0h1m5 0h1m3 0h1m3 0h1m2 0h1M7 13.5h1m1 0h1m1 0h2m1 0h4m1 0h1m6 0h1m1 0h1M6 14.5h1m1 0h1m1 0h2m1 0h1m1 0h1m2 0h1m1 0h1m1 0h1m1 0h3M4 15.5h3m1 0h2m1 0h1m1 0h2m1 0h4m1 0h1m1 0h1m2 0h1m1 0h1M6 16.5h1m1 0h1m1 0h2m1 0h2m3 0h1m1 0h2m2 0h4M6 17.5h1m1 0h1m3 0h1m2 0h1m2 0h2m1 0h3m1 0h2m1 0h1M4 18.5h1m1 0h1m1 0h5m4 0h1m2 0h1m1 0h5M5 19.5h3m1 0h1m2 0h1m2 0h1m3 0h1m4 0h1m3 0h1M4 20.5h1m4 0h2m2 0h1m2 0h1m3 0h5m1 0h2M12 21.5h6m2 0h1m3 0h1m1 0h1m1 0h1M4 22.5h7m4 0h2m1 0h3m1 0h1m1 0h1m1 0h1m1 0h1M4 23.5h1m5 0h1m5 0h3m1 0h1m3 0h3m1 0h1M4 24.5h1m1 0h3m1 0h1m1 0h1m2 0h1m2 0h7M4 25.5h1m1 0h3m1 0h1m4 0h1m2 0h4m1 0h2m1 0h2M4 26.5h1m1 0h3m1 0h1m1 0h1m4 0h1m2 0h3m1 0h1m3 0h1M4 27.5h1m5 0h1m3 0h2m3 0h3m2 0h2m1 0h2M4 28.5h7m1 0h1m3 0h1m3 0h3m1 0h1m1 0h1m1 0h1'

# Matrix erstellen (25x25 für Matter QR Version 2)
# Margin im SVG ist 4 (M4 4.5)
rows = []
for row_idx in range(25):
    y = row_idx + 4.5
    row_pattern = f'M(\\d+) {y}h(\\d+)'
    # Finde alle h Segmente in dieser Zeile
    # Da das SVG 'm' für relative Sprünge nutzt, müssen wir den Pfad parsen
    line_full = re.search(f'M\\d+ {y}h[^M]+', path_d)
    if line_full:
        data = line_full.group(0)
        # Rekonstruktion
        bits = ['0'] * 25
        curr_x = int(re.search(r'M(\d+)', data).group(1)) - 4
        segments = re.findall(r'h(\d+)|m(\d+)', data)
        for h, m in segments:
            if h:
                length = int(h)
                for i in range(length):
                    if 0 <= curr_x + i < 25:
                        bits[curr_x + i] = '1'
                curr_x += length
            if m:
                curr_x += int(m)
        print("".join(bits))
