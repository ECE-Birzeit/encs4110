"""Draws the Experiment 8 and 10 figures as SVGs (white background, the site frames them on a card).

  exp8/lcd-wiring.svg     every LCD pin to its LaunchPad header pin (pin locations: setup/pinout)
  exp10/uart-path.svg     PC terminal -> USB -> ICDI bridge -> PA0/PA1 -> UART0, with the settings
  exp10/uart-frame.svg    the byte 'A' (0x41) as an 8N1 frame at 115200 baud

Run from the repo root:  python3 scripts/draw-figures.py"""
from html import escape

SANS = "ui-sans-serif, system-ui, -apple-system, 'Segoe UI', sans-serif"
MONO = "ui-monospace, 'SFMono-Regular', Menlo, Consolas, monospace"
INK, DIM, TEAL = '#1f2328', '#57606a', '#00878F'


class Svg:
    def __init__(self, w, h, label):
        self.w, self.h = w, h
        self.out = [f"<svg xmlns='http://www.w3.org/2000/svg' viewBox='0 0 {w} {h}' width='{w}' height='{h}' role='img' aria-label='{escape(label, quote=True)}'>",
                    "<defs><marker id='a' viewBox='0 0 10 10' refX='9' refY='5' markerWidth='7' markerHeight='7' orient='auto-start-reverse'><path d='M0,0 L10,5 L0,10 z' fill='#333'/></marker></defs>",
                    f"<rect width='{w}' height='{h}' fill='#fff'/>"]

    def add(self, s):
        self.out.append(s)

    def text(self, x, y, s, size=13, fill=INK, anchor='middle', weight='400', mono=False):
        f = MONO if mono else SANS
        self.add(f"<text x='{x:.1f}' y='{y:.1f}' text-anchor='{anchor}' dominant-baseline='central' font-family=\"{f}\" font-size='{size}' font-weight='{weight}' fill='{fill}'>{escape(s)}</text>")

    def rect(self, x, y, w, h, fill, stroke=None, r=0, sw=1.5, dash=None):
        st = f" stroke='{stroke}' stroke-width='{sw}'" if stroke else ''
        if dash:
            st += f" stroke-dasharray='{dash}'"
        self.add(f"<rect x='{x:.1f}' y='{y:.1f}' width='{w:.1f}' height='{h:.1f}' rx='{r}' fill='{fill}'{st}/>")

    def line(self, pts, color, w=3, dash=None, marker_end=False):
        d = f" stroke-dasharray='{dash}'" if dash else ''
        m = " marker-end='url(#a)'" if marker_end else ''
        self.add(f"<polyline fill='none' stroke='{color}' stroke-width='{w}' stroke-linejoin='round' stroke-linecap='round' points='{' '.join(f'{x:.1f},{y:.1f}' for x, y in pts)}'{d}{m}/>")

    def save(self, path):
        open(path, 'w').write(''.join(self.out) + '</svg>')
        print('wrote', path)


# ------------------------------------------------------------------ Experiment 8: LCD wiring
WIRE = {'gnd': '#1f2328', 'pwr': '#d93025', 'ctl': '#d9a400', 'data': '#2e9d4b', 'pot': '#2563eb'}
BOX = {'gnd': ('#1f2328', '#ffffff'), 'pwr': ('#d93025', '#ffffff'), 'ctl': ('#fde68a', '#5c4600'),
       'data': ('#2e9d4b', '#ffffff'), 'pot': ('#dbeafe', '#1e3a8a')}
LCD_PINS = ['VSS', 'VDD', 'V0', 'RS', 'RW', 'E', 'D0', 'D1', 'D2', 'D3', 'D4', 'D5', 'D6', 'D7', 'A', 'K']
# LCD pin -> (kind, destination, where on the LaunchPad). Header pins from setup/pinout.
LCD_WIRES = {
    1: ('gnd', 'GND', 'J3 pin 2, or any GND'),
    2: ('pwr', 'VBUS (5 V)', 'J3 pin 1'),
    3: ('pot', 'Potentiometer wiper', 'ends to 5 V and GND'),
    4: ('ctl', 'PB0', 'J1 pin 3'),
    5: ('gnd', 'GND', 'write only'),
    6: ('ctl', 'PB2', 'J2 pin 2'),
    11: ('data', 'PB4', 'J1 pin 7'),
    12: ('data', 'PB5', 'J1 pin 2'),
    13: ('data', 'PB6', 'J2 pin 7'),
    14: ('data', 'PB7', 'J2 pin 6'),
    15: ('pwr', 'VBUS (5 V)', 'backlight +'),
    16: ('gnd', 'GND', 'backlight -'),
}

def lcd_wiring(path):
    s = Svg(1120, 700, 'Wiring the 16x2 LCD: VSS, RW and K to GND; VDD and A to VBUS; V0 to the wiper of a potentiometer between 5 V and GND; RS to PB0, E to PB2, D4 to D7 to PB4 to PB7; D0 to D3 not connected')
    x0, dx = 90, 44                     # LCD pin 1, pin pitch in the drawing
    top = 470                           # top edge of the LCD board
    row0, drow = 50, 33                 # first destination row, row spacing
    bx, bw, bh = 800, 290, 28           # destination boxes
    # Each wire turns right at its own row; pins further right turn lower, so no wire crosses another
    used = [p for p in range(1, 17) if p in LCD_WIRES]
    for i, p in enumerate(used):
        kind, dest, where = LCD_WIRES[p]
        x, y = x0 + (p - 1) * dx, row0 + i * drow + bh / 2
        s.line([(x, top + 4), (x, y), (bx, y)], WIRE[kind])
        fill, fg = BOX[kind]
        s.rect(bx, y - bh / 2, bw, bh, fill, r=6)
        s.text(bx + 12, y, dest, 13.5, fg, anchor='start', weight='700')
        s.text(bx + bw - 12, y, where, 11.5, fg, anchor='end')
        s.text(x - 6, y - 11, LCD_PINS[p - 1], 11, DIM, anchor='end', mono=True)
    # The potentiometer, drawn next to its box
    s.text(bx + bw / 2, row0 + len(used) * drow + 12, 'Potentiometer: 10 kΩ, outer pins to 5 V and GND', 11.5, DIM)

    # The LCD module
    s.rect(40, top, 720, 210, '#2f6b3a', r=10)
    s.rect(80, top + 70, 640, 110, '#1b2b20', r=6)
    s.rect(96, top + 84, 608, 82, '#8bc34a', r=3)
    for p in range(1, 17):
        x = x0 + (p - 1) * dx
        nc = p in (7, 8, 9, 10)
        s.add(f"<circle cx='{x}' cy='{top + 14}' r='8' fill='{'#c9a24a' if not nc else '#9aa4ae'}' stroke='#7a5d1c' stroke-width='1.5'/>")
        s.text(x, top + 14, str(p), 8.5, '#2b2f36', weight='800')
        s.text(x, top + 40, LCD_PINS[p - 1], 12, '#ffffff', weight='700', mono=True)
    s.text(400, top + 125, '16x2 LCD (HD44780)', 15, '#1b2b20', weight='700')
    nx = x0 + 7.5 * dx                  # middle of pins 7 to 10
    s.line([(x0 + 6 * dx - 8, top - 18), (x0 + 9 * dx + 8, top - 18)], '#9aa4ae', 1.5, dash='4 3')
    s.text(nx, top - 46, 'D0-D3', 11.5, DIM, weight='700')
    s.text(nx, top - 31, 'not connected', 11.5, DIM)
    s.save(path)


# ------------------------------------------------------------------ Experiment 10: the serial path
def uart_path(path):
    s = Svg(1120, 400, "UART0's path to the PC: the TM4C123's UART0 on PA0 and PA1 connects on the board to the ICDI debug chip, which passes the bytes over the debug USB cable to a virtual COM port on the PC; the terminal opens that port at 115200 baud, 8 data bits, no parity, 1 stop bit")
    # PC
    s.rect(30, 70, 300, 270, '#f6f8fa', '#57606a', r=12)
    s.text(180, 98, 'PC', 15, INK, weight='700')
    s.text(180, 122, 'terminal (PuTTY)', 12.5, DIM)
    rows = [('Connection', 'Serial'), ('Port', 'COM n (Device Manager)'), ('Speed', '115200'),
            ('Data bits', '8'), ('Parity', 'None'), ('Stop bits', '1'), ('Flow control', 'None')]
    for i, (k, v) in enumerate(rows):
        y = 152 + i * 25
        s.text(56, y, k, 12.5, DIM, anchor='start')
        s.text(304, y, v, 12.5, INK, anchor='end', weight='700', mono=True)
    # USB cable
    s.line([(330, 205), (500, 205)], '#57606a', 6)
    s.text(415, 184, 'USB cable', 12.5, INK, weight='700')
    s.text(415, 228, 'DEBUG port', 11.5, DIM)
    # LaunchPad
    s.rect(500, 40, 590, 330, '#f4d6d6', '#c99a9a', r=14)
    s.text(795, 64, 'LaunchPad', 15, INK, weight='700')
    s.rect(530, 130, 210, 150, '#2b2f36', r=8)
    s.text(635, 172, 'ICDI', 15, '#ffffff', weight='800')
    s.text(635, 196, 'debug probe', 12, '#c3c7cc')
    s.text(635, 222, 'USB ↔ serial bridge', 12, '#c3c7cc')
    s.text(635, 246, '"Stellaris Virtual', 11, '#c3c7cc')
    s.text(635, 262, 'Serial Port" on the PC', 11, '#c3c7cc')
    s.rect(880, 130, 180, 150, '#2b2f36', r=8)
    s.text(970, 186, 'TM4C123', 15, '#ffffff', weight='800')
    s.text(970, 210, 'UART0', 13, '#c3c7cc', weight='700')
    # TX and RX on the board
    s.line([(880, 180), (754, 180)], TEAL, 3, marker_end=True)
    s.text(812, 166, 'PA1 = U0Tx', 12, INK, weight='700', mono=True)
    s.line([(740, 232), (868, 232)], '#2563eb', 3, marker_end=True)
    s.text(812, 252, 'PA0 = U0Rx', 12, INK, weight='700', mono=True)
    s.text(795, 330, 'Wired on the board: nothing to connect but the USB cable.', 12.5, DIM)
    s.save(path)


# ------------------------------------------------------------------ Experiment 10: one frame
def uart_frame(path):
    byte = 0x41  # 'A'
    bits = [(byte >> i) & 1 for i in range(8)]      # least significant first
    cells = [('Idle', 1)] + [('Start', 0)] + [(f'D{i}', b) for i, b in enumerate(bits)] + [('Stop', 1), ('Idle', 1)]
    s = Svg(1120, 350, "The character A (0x41, binary 0100 0001) as one 8N1 frame: the line idles high, a low start bit, then D0 to D7 least significant first (1, 0, 0, 0, 0, 0, 1, 0), a high stop bit, and idle again; each bit lasts 8.68 microseconds at 115200 baud")
    x0, cw = 60, 80
    hi, lo = 120, 190
    for i, (name, v) in enumerate(cells):
        x = x0 + i * cw
        if name in ('Start', 'Stop'):
            s.rect(x, 80, cw, 140, '#eef2f7')
        elif name.startswith('D'):
            s.rect(x + 1, 80, cw - 2, 140, '#f7faf7')
        s.text(x + cw / 2, 62, name, 13, INK if name != 'Idle' else DIM, weight='700' if name != 'Idle' else '400', mono=True)
        if name != 'Idle':
            s.text(x + cw / 2, 238, str(v), 15, TEAL, weight='700', mono=True)
    pts, prev = [], None
    for i, (_, v) in enumerate(cells):
        y = hi if v else lo
        x = x0 + i * cw
        if prev is not None and prev != y:
            pts.append((x, prev))
        pts.append((x, y))
        prev = y
    pts.append((x0 + len(cells) * cw, prev))
    s.line(pts, TEAL, 3)
    s.text(x0 - 12, hi, '1', 13, DIM, anchor='end', mono=True)
    s.text(x0 - 12, lo, '0', 13, DIM, anchor='end', mono=True)
    # Brackets: one bit, the data byte, the whole frame
    def bracket(xa, xb, y, label):
        s.line([(xa + 3, y), (xb - 3, y)], '#333', 1.5)
        s.add(f"<line x1='{xa:.1f}' y1='{y - 6}' x2='{xa:.1f}' y2='{y + 6}' stroke='#333' stroke-width='1.5'/><line x1='{xb:.1f}' y1='{y - 6}' x2='{xb:.1f}' y2='{y + 6}' stroke='#333' stroke-width='1.5'/>")
        s.text((xa + xb) / 2, y + 18, label, 12.5, INK)
    bracket(x0 + 2 * cw, x0 + 10 * cw, 266, "Data, least significant bit first: 1000 0010 read backwards is 0100 0001 = 0x41 = 'A'")
    bracket(x0 + cw, x0 + 11 * cw, 304, 'One frame: 10 bits = 86.8 µs at 115200 baud (1 bit = 8.68 µs)')
    s.save(path)


lcd_wiring('public/images/exp8/lcd-wiring.svg')
uart_path('public/images/exp10/uart-path.svg')
uart_frame('public/images/exp10/uart-frame.svg')
